#!/usr/bin/env python3
"""Post-link input manifest and fail-closed preflight for the fixed HG verifier.

This records build provenance, not a proof of compiler/translation correctness.
No native execution, subprocess, network access, or guest-state modification.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
from typing import Any

SCHEMA = 1
SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".in"}


class BuildProvenanceError(ValueError):
    """The isolated executable cannot be tied to the current build inputs."""


def digest(path: Path) -> dict[str, Any]:
    state = hashlib.sha256()
    size = 0
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            state.update(chunk)
            size += len(chunk)
    return {"bytes": size, "sha256": state.hexdigest()}


def canonical_digest(value: Any) -> str:
    return hashlib.sha256(json.dumps(value, sort_keys=True, separators=(",", ":")).encode("utf-8")).hexdigest()


def _under(path: Path, root: Path) -> Path:
    resolved = path.resolve()
    if not resolved.is_relative_to(root.resolve()):
        raise BuildProvenanceError(f"input escapes configured project root: {path}")
    return resolved


def input_paths(repo: Path, build: Path) -> list[Path]:
    """Fixed first-party scope; never traverse original-game or emulator payloads."""
    fixed = [repo / "CMakeLists.txt", repo / "tools/hg.py",
             repo / "tools/hg_build_provenance.py", build / "CMakeCache.txt"]
    for item in fixed:
        if not item.is_file():
            raise BuildProvenanceError(f"required build input is missing: {item}")
    files = set(fixed)
    for folder, suffixes in (("runtime", SOURCE_SUFFIXES), ("out", SOURCE_SUFFIXES),
                             ("config", {".toml"}), ("tools/hgtool", {".py"})):
        base = repo / folder
        if not base.is_dir():
            raise BuildProvenanceError(f"required source directory is missing: {base}")
        for item in base.rglob("*"):
            if item.is_file() and item.suffix.lower() in suffixes:
                files.add(item)
    # Generated project files retain actual per-source Release overrides and links.
    files.update(build.glob("hg*.vcxproj"))
    for item in files:
        _under(item, repo)
    return sorted(files, key=lambda item: item.relative_to(repo).as_posix())


def snapshot(repo: Path, build: Path) -> dict[str, dict[str, Any]]:
    return {item.relative_to(repo).as_posix(): digest(item) for item in input_paths(repo, build)}


def write_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + ".tmp")
    temporary.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    os.replace(temporary, path)


def record_build(repo: Path, build: Path, exe: Path, configuration: str) -> Path:
    repo, build, exe = repo.resolve(), build.resolve(), exe.resolve()
    _under(build, repo)
    _under(exe, build)
    if not exe.is_file():
        raise BuildProvenanceError("post-link executable is missing")
    inputs = snapshot(repo, build)
    # Additional coarse check only. Verification below uses hashes, not timestamps.
    future = [name for name in inputs if (repo / name).stat().st_mtime_ns > exe.stat().st_mtime_ns]
    if future:
        raise BuildProvenanceError("inputs newer than linked executable: " + ", ".join(future[:8]))
    commands = {}
    for path in build.glob("hg*.dir/" + configuration + "/*/CL.command.1.tlog"):
        commands[path.relative_to(repo).as_posix()] = digest(path)
    manifest = {
        "schema_version": SCHEMA, "configuration": configuration,
        "source_root": str(repo), "build_root": str(build),
        "executable": digest(exe), "inputs": inputs,
        "inputs_sha256": canonical_digest(inputs), "compile_command_records": commands,
        "scope": "post-link first-party input snapshot; relies on configured build dependency tracking",
        "limitations": "not independent compiler correctness, hardware fidelity, or emitter regeneration proof",
    }
    destination = exe.with_suffix(exe.suffix + ".build.json")
    write_json(destination, manifest)
    print("HG build provenance: " + str(destination) + " inputs=" + str(len(inputs)) +
          " executable_sha256=" + manifest["executable"]["sha256"])
    return destination


def validate_build(repo: Path, exe: Path, evidence_dir: Path | None = None) -> dict[str, Any]:
    """The Project Link fixed verifier uses only this configured Release build."""
    repo, exe = repo.resolve(), exe.resolve()
    build = repo / "build"
    manifest_path = build / "Release/hg_game.exe.build.json"
    if not manifest_path.is_file():
        raise BuildProvenanceError("missing post-link build manifest; rebuild hg_game before measuring FPS")
    _under(manifest_path, repo)
    raw = manifest_path.read_bytes()
    manifest = json.loads(raw)
    if manifest.get("schema_version") != SCHEMA or manifest.get("configuration") != "Release":
        raise BuildProvenanceError("unsupported or non-Release build manifest")
    if Path(manifest.get("source_root", "")).resolve() != repo:
        raise BuildProvenanceError("build manifest belongs to another source tree")
    if Path(manifest.get("build_root", "")).resolve() != build.resolve():
        raise BuildProvenanceError("build manifest belongs to another build tree")
    old = manifest.get("inputs")
    if not isinstance(old, dict) or canonical_digest(old) != manifest.get("inputs_sha256"):
        raise BuildProvenanceError("malformed build input manifest")
    current = snapshot(repo, build)
    changed = sorted(name for name in old.keys() | current.keys() if old.get(name) != current.get(name))
    if changed:
        raise BuildProvenanceError("stale game executable; changed build inputs: " + ", ".join(changed[:12]))
    if digest(exe) != manifest.get("executable"):
        raise BuildProvenanceError("isolated executable does not match the post-link build manifest")
    if evidence_dir is not None:
        evidence_dir = evidence_dir.resolve()
        if evidence_dir.is_relative_to(repo):
            raise BuildProvenanceError("build evidence must be external to source tree")
        evidence_dir.mkdir(parents=True, exist_ok=True)
        target = evidence_dir / "verified-build-manifest.json"
        target.write_bytes(raw)
    return {"status": "matched", "inputs": len(current),
            "inputs_sha256": manifest["inputs_sha256"],
            "manifest_sha256": hashlib.sha256(raw).hexdigest(),
            "executable_sha256": manifest["executable"]["sha256"],
            "configuration": "Release", "scope": manifest["scope"]}


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", required=True, type=Path)
    parser.add_argument("--build", required=True, type=Path)
    parser.add_argument("--exe", required=True, type=Path)
    parser.add_argument("--configuration", required=True)
    args = parser.parse_args()
    record_build(args.repo, args.build, args.exe, args.configuration)


if __name__ == "__main__":
    main()
