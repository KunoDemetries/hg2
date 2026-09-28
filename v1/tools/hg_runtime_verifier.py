#!/usr/bin/env python3
"""Fixed Haunting Ground runtime verifier.

Frames mode measures a qualified original game synchronization boundary.  It never
uses host swaps, changed images, vblank edges alone, or modeled guest time as FPS.
Menu/audio modes remain explicitly unverified until independent semantic verifiers
exist.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import shutil
import statistics
import subprocess
import sys
import tempfile
import time
from pathlib import Path
from typing import Any

from hg_build_provenance import BuildProvenanceError, validate_build

EXPECTED_ELF_SHA256 = "3b374d53a499d2c17b205274ee9eb34280768f294f970ebf6ae6731f6a2dacb8"
EXPECTED_WRITER_PC = 0x001BEF84
EXPECTED_CALLER = 0x002D1C50
FRAME_PREFIX = "Original frame candidate "
CHECKPOINT_US = 1_000_000
WINDOW_COUNT = 5
WINDOW_MIN_US = 800_000
WINDOW_MAX_US = 1_200_000
TARGET_FPS = 30.0


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def inside(child: Path, parent: Path) -> bool:
    try:
        child.relative_to(parent)
        return True
    except ValueError:
        return False


def parse_scalar(value: str) -> Any:
    if value.startswith(("0x", "0X")):
        try:
            return int(value, 16)
        except ValueError:
            return value
    try:
        return int(value, 10)
    except ValueError:
        pass
    try:
        return float(value)
    except ValueError:
        return value


def parse_frame_reports(text: str) -> list[dict[str, Any]]:
    reports: list[dict[str, Any]] = []
    for line in text.splitlines():
        if not line.startswith(FRAME_PREFIX):
            continue
        item: dict[str, Any] = {}
        for token in line[len(FRAME_PREFIX):].split():
            if "=" not in token:
                continue
            key, value = token.split("=", 1)
            if "," in value and key in {"dispfb", "display"}:
                item[key] = [parse_scalar(part) for part in value.split(",")]
            else:
                item[key] = parse_scalar(value)
        if item:
            reports.append(item)
    return reports


def resolve_pinned_toc(catalog: dict[str, Any], evidence_dir: Path) -> tuple[Path, Path]:
    toc_meta = catalog.get("toc") or {}
    expected_bytes = toc_meta.get("bytes")
    expected_sha = toc_meta.get("sha256")
    if expected_bytes != 2064 or not isinstance(expected_sha, str):
        raise ValueError("recordings catalog TOC metadata is malformed")

    destination = evidence_dir / "toc.bin"
    candidates: list[Path] = []
    if destination.is_file():
        candidates.append(destination)

    # The fixed verifier wrapper isolates the executable and recordings catalog but
    # currently does not copy the already-pinned TOC record. Reuse only an existing
    # Project Link evidence toc.bin whose exact size and digest match the catalog.
    evidence_root = Path(tempfile.gettempdir()) / "haunting-toc-probe"
    if evidence_root.is_dir():
        candidates.extend(evidence_root.rglob("toc.bin"))

    seen: set[Path] = set()
    for candidate in candidates:
        try:
            resolved = candidate.resolve()
        except OSError:
            continue
        if resolved in seen or not resolved.is_file():
            continue
        seen.add(resolved)
        try:
            if resolved.stat().st_size != expected_bytes:
                continue
            if sha256_file(resolved) != expected_sha:
                continue
        except OSError:
            continue
        if resolved != destination:
            shutil.copyfile(resolved, destination)
        if destination.stat().st_size != expected_bytes or sha256_file(destination) != expected_sha:
            raise ValueError("copied TOC record failed exact digest validation")
        return destination, resolved

    raise ValueError("no exact pinned toc.bin match exists under bounded Project Link evidence root")


def validate_recordings(
    path: Path, profile: str, evidence_dir: Path
) -> tuple[dict[str, Any], dict[str, Any], Path, Path]:
    catalog = json.loads(path.read_text(encoding="utf-8"))
    if catalog.get("schema_version") != 1:
        raise ValueError("unsupported recordings schema")
    elf = catalog.get("elf") or {}
    if elf.get("sha256") != EXPECTED_ELF_SHA256:
        raise ValueError("recordings catalog original ELF hash does not match qualified source")
    profiles = catalog.get("profiles") or {}
    if profile not in profiles:
        raise ValueError(f"profile {profile!r} is not in recordings catalog")
    selected = profiles[profile]
    events = selected.get("events")
    if not isinstance(events, list):
        raise ValueError("recording profile has no event list")
    previous = -1
    for event in events:
        if not isinstance(event, dict) or not isinstance(event.get("slice"), int) or not isinstance(event.get("command"), str):
            raise ValueError("recording event is malformed")
        if event["slice"] <= previous:
            raise ValueError("recording events are not strictly increasing")
        previous = event["slice"]
    toc, toc_source = resolve_pinned_toc(catalog, evidence_dir)
    return catalog, selected, toc, toc_source


def make_base_result(profile: str, mode: str, evidence_dir: Path) -> dict[str, Any]:
    return {
        "schema_version": 1,
        "status": "unverified",
        "profile": profile,
        "mode": mode,
        "metric": "original_1bef84_display_sync_boundary",
        "source_elf_sha256": EXPECTED_ELF_SHA256,
        "gameplay_fps": None,
        "passes_30fps": False,
        "menu_verified": False,
        "audio_fixed": None,
        "evidence_dir": str(evidence_dir),
    }


def save_result(evidence_dir: Path, result: dict[str, Any]) -> None:
    target = evidence_dir / "verifier-result.json"
    target.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def steady_gameplay_summary(windows: list[dict[str, Any]]) -> dict[str, Any]:
    """Measure the fixed scene's last two intervals, never menu/startup averages."""
    if len(windows) != WINDOW_COUNT:
        raise ValueError("steady gameplay needs the complete five-window audit trail")
    tail = windows[-2:]
    start, end = tail[0]["guest_start_us"], tail[-1]["guest_end_us"]
    if not 30_900_000 <= start <= 31_100_000 or not 32_800_000 <= end <= 33_100_000:
        raise ValueError("steady gameplay windows are outside the qualified 31M..33M scene")
    if tail[0]["guest_end_us"] != tail[1]["guest_start_us"]:
        raise ValueError("steady gameplay windows are not contiguous")
    rates = []
    for window in tail:
        elapsed, count = float(window["host_seconds"]), window["boundaries"]
        delta = window["guest_end_us"] - window["guest_start_us"]
        if not math.isfinite(elapsed) or elapsed <= 0:
            raise ValueError("steady gameplay has non-finite or non-positive host time")
        if isinstance(count, bool) or not isinstance(count, int) or count < 0:
            raise ValueError("steady gameplay has invalid boundary counts")
        if not WINDOW_MIN_US <= delta <= WINDOW_MAX_US:
            raise ValueError("steady gameplay interval has an invalid guest-time span")
        rate = count / elapsed
        if not math.isfinite(rate):
            raise ValueError("steady gameplay rate is non-finite")
        rates.append(rate)
    seconds = sum(float(window["host_seconds"]) for window in tail)
    if not math.isfinite(seconds):
        raise ValueError("steady gameplay total host time is non-finite")
    boundaries = sum(window["boundaries"] for window in tail)
    return {
        "scope": "fixed_gameplay33m_final_two_intervals",
        "guest_start_us": start, "guest_end_us": end,
        "boundaries": boundaries, "host_seconds": seconds,
        "fps": boundaries / seconds,
        "minimum_fps": min(rates), "maximum_fps": max(rates),
        "passes_30fps": all(rate >= TARGET_FPS for rate in rates),
        "window_indices": [3, 4], "window_count": 2,
    }


def normalized_iop_digest(data: bytes) -> dict[str, Any]:
    """Mask only independently documented RTC payload bytes; retain raw evidence."""
    if len(data) != 2 * 1024 * 1024:
        raise ValueError("IOP capture must be exactly 2 MiB")
    offsets = (0xBFCD1, 0xBFCD2, 0xBFCD3)
    values = [data[offset] for offset in offsets]
    for value, limit in zip(values, (59, 59, 23)):
        decimal = (value >> 4) * 10 + (value & 15)
        if (value & 15) > 9 or (value >> 4) > 9 or decimal > limit:
            raise ValueError("documented IOP RTC field is not valid BCD time")
    normalized = bytearray(data)
    for offset in offsets:
        normalized[offset] = 0
    return {
        "sha256": hashlib.sha256(normalized).hexdigest(),
        "masked_offsets": [f"0x{offset:x}" for offset in offsets],
        "original_bytes": values,
        "scope": "all IOP bytes except independently identified RTC seconds/minutes/hours",
    }


def capture_digests(evidence_dir: Path) -> dict[str, Any]:
    """After execution, hash only fixed capture names within this evidence directory."""
    names = ["verifier-frame.ppm", "verifier-state.ram"] + [
        "verifier-state.ram" + suffix for suffix in (
            ".ee-ram.bin", ".gs-vram.bin", ".gs.json", ".json",
            ".vu1-data.bin", ".vu1-defined.bin", ".vu1-micro.bin", ".vu1-state.json",
        )
    ]
    output: dict[str, Any] = {}
    parent = evidence_dir.resolve()
    for name in names:
        path = (parent / name).resolve()
        if not inside(path, parent) or not path.is_file():
            raise ValueError(f"missing or out-of-directory capture: {name}")
        size = path.stat().st_size
        if size <= 0 or size > 64 * 1024 * 1024:
            raise ValueError(f"invalid capture byte count: {name}")
        output[name] = {"bytes": size, "sha256": sha256_file(path)}
    output["iop_without_verified_rtc"] = normalized_iop_digest((parent / "verifier-state.ram").read_bytes())
    return output


def verify_frames(exe: Path, recordings: Path, evidence_dir: Path, profile: str) -> tuple[dict[str, Any], int]:
    result = make_base_result(profile, "frames", evidence_dir)
    if profile != "gameplay33m":
        result.update(
            status="unverified_profile",
            reason="The 0x1bef84 boundary is qualified only for gameplay33m.",
        )
        save_result(evidence_dir, result)
        return result, 2

    _, selected, toc, toc_source = validate_recordings(recordings, profile, evidence_dir)
    minimum = int(selected.get("minimum_slices", 0))
    slices = int(selected.get("default_slices", 0))
    if slices < minimum or slices < 33_000_000:
        raise ValueError("gameplay33m recording budget is below qualified minimum")

    frame = evidence_dir / "verifier-frame.ppm"
    state = evidence_dir / "verifier-state.ram"
    log_path = evidence_dir / "verifier-native.log"

    command = [
        str(exe),
        "--profile",
        "--clock-profile", "issue-slots",
        "--slices", str(slices),
        "--checkpoint-every", str(CHECKPOINT_US),
        "--toc-record", str(toc),
        "--preview-file", str(frame),
        "--dump-iop", str(state),
        "--gpu-sprites",
        "--gpu-resident",
        "--gpu-triangles",
        "--no-host-input",
        "--hidden-host",
        "--host-frames", "1000000",
        "--realtime",
    ]
    for event in selected["events"]:
        command.extend(["--input-at", str(event["slice"]), event["command"]])

    started = time.monotonic()
    try:
        completed = subprocess.run(
            command,
            cwd=Path.cwd(),
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=1800,
            shell=False,
            check=False,
        )
        output = completed.stdout or ""
        native_exit = completed.returncode
        timed_out = False
    except subprocess.TimeoutExpired as exc:
        output = exc.stdout or ""
        if isinstance(output, bytes):
            output = output.decode("utf-8", errors="replace")
        native_exit = None
        timed_out = True
    wall_seconds = time.monotonic() - started
    log_path.write_text(output, encoding="utf-8")

    reports = parse_frame_reports(output)
    failures: list[str] = []
    if timed_out:
        failures.append("native run timed out")
    if native_exit != 2:
        failures.append(f"native exit code {native_exit!r} is not the expected budget stop 2")
    if "EE stopped at" in output or "IOP stopped at" in output:
        failures.append("native run reported an EE/IOP fault")
    if len(reports) < WINDOW_COUNT + 1:
        failures.append("too few original frame-boundary reports")

    final = reports[-1] if reports else {}
    writes = int(final.get("writes", 0) or 0)
    expected_writer_count = int(final.get("writer_1bef84", 0) or 0)
    unexpected_writers = int(final.get("unexpected_writers", 0) or 0)
    lost_trace = int(final.get("lost_trace", 0) or 0)
    caller_mismatches = int(final.get("caller_mismatches", 0) or 0)
    invalid_stack = int(final.get("invalid_stack", 0) or 0)
    caller = int(final.get("caller", 0) or 0)
    last_caller = int(final.get("last_caller", 0) or 0)
    counter_two = int(final.get("counter_delta2", 0) or 0)
    counter_other = int(final.get("counter_other", 0) or 0)
    vblank_two = int(final.get("vblank_delta2", 0) or 0)
    vblank_other = int(final.get("vblank_other", 0) or 0)
    raster_advanced = int(final.get("raster_advanced_boundaries", 0) or 0)

    if writes <= 1:
        failures.append("no sustained original boundary writes")
    if expected_writer_count != writes or unexpected_writers:
        failures.append("watched boundary has a writer other than original PC 0x1bef84")
    if lost_trace:
        failures.append("diagnostic RAM-write history lost boundary records")
    if caller_mismatches or invalid_stack or caller != EXPECTED_CALLER or last_caller != EXPECTED_CALLER:
        failures.append("original boundary caller provenance is inconsistent")
    counter_total = counter_two + counter_other
    vblank_total = vblank_two + vblank_other
    counter_ratio = (counter_two / counter_total) if counter_total else 0.0
    vblank_ratio = (vblank_two / vblank_total) if vblank_total else 0.0
    if counter_ratio < 0.98 or vblank_ratio < 0.98:
        failures.append("original counter/vblank two-edge relationship is not stable enough")
    if writes > 1 and raster_advanced < int((writes - 1) * 0.99):
        failures.append("render work does not advance across enough candidate boundaries")

    usable = [
        report for report in reports
        if int(report.get("writes", 0) or 0) > 0
        and float(report.get("interval_host_seconds", 0.0) or 0.0) > 0.0
        and int(report.get("guest_last_us", 0) or 0) > 0
    ]
    windows: list[dict[str, Any]] = []
    if len(usable) >= WINDOW_COUNT + 1:
        chain = usable[-(WINDOW_COUNT + 1):]
        for previous, current in zip(chain, chain[1:]):
            guest_delta = int(current["guest_last_us"]) - int(previous["guest_last_us"])
            interval_seconds = float(current["interval_host_seconds"])
            interval_writes = int(current["interval_writes"])
            fps = interval_writes / interval_seconds if interval_seconds > 0 else 0.0
            window = {
                "stage": current.get("stage"),
                "guest_start_us": int(previous["guest_last_us"]),
                "guest_end_us": int(current["guest_last_us"]),
                "guest_delta_us": guest_delta,
                "boundaries": interval_writes,
                "host_seconds": interval_seconds,
                "fps": fps,
            }
            windows.append(window)
            if guest_delta < WINDOW_MIN_US or guest_delta > WINDOW_MAX_US:
                failures.append("final FPS windows are not consecutive approximately-one-second guest windows")
                break
    else:
        failures.append("fewer than five consecutive late gameplay windows are available")

    fps_values = [float(window["fps"]) for window in windows] if len(windows) == WINDOW_COUNT else []
    legacy_mean_fps = statistics.fmean(fps_values) if fps_values else None
    steady: dict[str, Any] | None = None
    digests: dict[str, Any] = {}
    try:
        steady = steady_gameplay_summary(windows)
        digests = capture_digests(evidence_dir)
    except (ValueError, KeyError, TypeError, OSError) as exc:
        failures.append(str(exc))
    qualified = not failures and steady is not None and len(windows) == WINDOW_COUNT

    result.update({
        "status": "measured" if qualified else "invalid_frame_evidence",
        "qualified": qualified,
        "gameplay_fps": steady["fps"] if qualified else None,
        "minimum_fps": steady["minimum_fps"] if qualified else None,
        "maximum_fps": steady["maximum_fps"] if qualified else None,
        "passes_30fps": bool(qualified and steady["passes_30fps"]),
        "fps_scope": "steady gameplay only; final two intervals, no menu/transition average",
        "steady_gameplay": steady,
        "legacy_mixed_window_mean_fps": legacy_mean_fps,
        "capture_digests": digests,
        "target_fps": TARGET_FPS,
        "window_count": WINDOW_COUNT,
        "windows": windows,
        "qualification": {
            "writes": writes,
            "writer_1bef84": expected_writer_count,
            "unexpected_writers": unexpected_writers,
            "lost_trace": lost_trace,
            "caller": f"0x{caller:08x}",
            "last_caller": f"0x{last_caller:08x}",
            "caller_mismatches": caller_mismatches,
            "invalid_stack": invalid_stack,
            "counter_delta2_ratio": counter_ratio,
            "vblank_delta2_ratio": vblank_ratio,
            "raster_advanced_boundaries": raster_advanced,
            "object": final.get("object"),
            "watched_word": final.get("word"),
            "guest_first_us": final.get("guest_first_us"),
            "guest_last_us": final.get("guest_last_us"),
        },
        "validation_failures": failures,
        "native_exit_code": native_exit,
        "native_timed_out": timed_out,
        "wall_seconds": wall_seconds,
        "executable": {
            "bytes": exe.stat().st_size,
            "sha256": sha256_file(exe),
        },
        "recordings_sha256": sha256_file(recordings),
        "toc_sha256": sha256_file(toc),
        "toc_source": str(toc_source),
        "native_log": {
            "path": str(log_path),
            "bytes": log_path.stat().st_size,
            "sha256": sha256_file(log_path),
        },
        "frame_artifact": str(frame) if frame.is_file() else None,
        "state_artifact": str(state) if state.is_file() else None,
        "notes": [
            "FPS is derived only from successive original PC 0x1bef84 boundary writes and host elapsed time.",
            "Host video updates, window swaps, changed images, vblank counts alone, and guest modeled time are not FPS.",
            "All five late windows are retained for audit; lighter and transition intervals do not enter gameplay_fps.",
            "gameplay_fps is total original boundaries / total host seconds for the final two steady-gameplay intervals.",
            "passes_30fps requires both steady-gameplay intervals to reach 30; it is not a whole-game guarantee.",
            "Capture hashes are computed after native execution; IOP normalization masks only documented RTC fields.",
        ],
    })
    save_result(evidence_dir, result)
    return result, 0 if qualified else 2


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--profile", choices=("gameplay33m", "start_menu"), required=True)
    parser.add_argument("--mode", choices=("menu", "frames", "audio"), required=True)
    parser.add_argument("--exe", required=True)
    parser.add_argument("--recordings", required=True)
    parser.add_argument("--evidence-dir", required=True)
    args = parser.parse_args()

    repo = Path.cwd().resolve()
    exe = Path(args.exe).resolve()
    recordings = Path(args.recordings).resolve()
    evidence_dir = Path(args.evidence_dir).resolve()
    evidence_dir.mkdir(parents=True, exist_ok=True)

    try:
        if inside(evidence_dir, repo):
            raise ValueError("evidence directory must be outside the repository")
        if not exe.is_file():
            raise ValueError("isolated executable is missing")
        if not recordings.is_file():
            raise ValueError("recordings catalog is missing")
        if not inside(exe, evidence_dir) or not inside(recordings, evidence_dir):
            raise ValueError("executable and recordings catalog must be isolated inside evidence directory")

        if args.mode == "frames":
            # HG-DIAG-028: hash validation is before native launch, not measured work.
            provenance = validate_build(repo, exe, evidence_dir)
            result, code = verify_frames(exe, recordings, evidence_dir, args.profile)
            result["build_provenance"] = provenance
            save_result(evidence_dir, result)
        else:
            result = make_base_result(args.profile, args.mode, evidence_dir)
            result.update({
                "status": "unverified_mode",
                "reason": (
                    "No independent menu semantic verifier is implemented."
                    if args.mode == "menu"
                    else "No independent final mixed-audio verifier is implemented."
                ),
            })
            save_result(evidence_dir, result)
            code = 2
    except Exception as exc:
        result = make_base_result(args.profile, args.mode, evidence_dir)
        result.update(status="stale_build" if isinstance(exc, BuildProvenanceError) else "verifier_error", reason=str(exc))
        try:
            save_result(evidence_dir, result)
        except Exception:
            pass
        code = 2

    print(json.dumps(result, sort_keys=True))
    return code


if __name__ == "__main__":
    raise SystemExit(main())
