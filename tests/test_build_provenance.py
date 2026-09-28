"""Synthetic freshness checks; no game execution or original game files."""
import contextlib
import io
import json
import os
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

from hg_build_provenance import BuildProvenanceError, digest, record_build, validate_build
import hg_runtime_verifier


class BuildProvenanceTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.repo = self.root / "repo"
        self.build = self.repo / "build"
        self.exe = self.build / "Release/hg_game.exe"
        for name in ("runtime/vif.hpp", "out/translated.cpp", "config/game.toml",
                     "tools/hgtool/emit.py", "CMakeLists.txt", "tools/hg.py",
                     "tools/hg_build_provenance.py", "build/CMakeCache.txt"):
            path = self.repo / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(name + "\n", encoding="utf-8")
        self.exe.parent.mkdir(parents=True)
        self.exe.write_bytes(b"synthetic native executable; not runnable")
        with contextlib.redirect_stdout(io.StringIO()):
            self.manifest = record_build(self.repo, self.build, self.exe, "Release")
        self.evidence = self.root / "evidence"
        self.evidence.mkdir()
        self.isolated = self.evidence / "hg_game.exe"
        self.isolated.write_bytes(self.exe.read_bytes())

    def test_exact_binary_and_sources_pass_and_pin_evidence(self):
        result = validate_build(self.repo, self.isolated, self.evidence)
        self.assertEqual(result["status"], "matched")
        self.assertEqual(result["executable_sha256"], digest(self.exe)["sha256"])
        self.assertEqual((self.evidence / "verified-build-manifest.json").read_bytes(), self.manifest.read_bytes())

    def test_missing_manifest_is_rejected(self):
        self.manifest.unlink()
        with self.assertRaisesRegex(BuildProvenanceError, "missing post-link"):
            validate_build(self.repo, self.isolated)

    def test_changed_source_with_preserved_timestamp_is_rejected(self):
        source = self.repo / "runtime/vif.hpp"
        before = source.stat()
        source.write_text("changed header", encoding="utf-8")
        os.utime(source, ns=(before.st_atime_ns, before.st_mtime_ns))
        with self.assertRaisesRegex(BuildProvenanceError, "stale game executable"):
            validate_build(self.repo, self.isolated)

    def test_new_source_is_rejected(self):
        (self.repo / "runtime/new.cpp").write_text("new source")
        with self.assertRaisesRegex(BuildProvenanceError, "new.cpp"):
            validate_build(self.repo, self.isolated)

    def test_deleted_source_is_rejected(self):
        (self.repo / "out/translated.cpp").unlink()
        with self.assertRaisesRegex(BuildProvenanceError, "translated.cpp"):
            validate_build(self.repo, self.isolated)

    def test_wrong_same_size_binary_is_rejected(self):
        self.isolated.write_bytes(b"x" * self.isolated.stat().st_size)
        with self.assertRaisesRegex(BuildProvenanceError, "isolated executable"):
            validate_build(self.repo, self.isolated)

    def test_changed_build_config_is_rejected(self):
        (self.build / "CMakeCache.txt").write_text("changed settings")
        with self.assertRaisesRegex(BuildProvenanceError, "CMakeCache"):
            validate_build(self.repo, self.isolated)

    def test_documentation_only_change_does_not_stale_binary(self):
        (self.repo / "PROGRESS.md").write_text("documentation only")
        self.assertEqual(validate_build(self.repo, self.isolated)["status"], "matched")

    def test_debug_manifest_is_rejected(self):
        data = json.loads(self.manifest.read_text())
        data["configuration"] = "Debug"
        self.manifest.write_text(json.dumps(data))
        with self.assertRaisesRegex(BuildProvenanceError, "non-Release"):
            validate_build(self.repo, self.isolated)

    def test_corrupt_input_digest_is_rejected(self):
        data = json.loads(self.manifest.read_text())
        data["inputs_sha256"] = "0" * 64
        self.manifest.write_text(json.dumps(data))
        with self.assertRaisesRegex(BuildProvenanceError, "malformed"):
            validate_build(self.repo, self.isolated)

    def test_reject_snapshot_newer_than_link(self):
        before = self.exe.stat()
        source = self.repo / "runtime/vif.hpp"
        os.utime(source, ns=(before.st_atime_ns, before.st_mtime_ns + 1_000_000_000))
        with self.assertRaisesRegex(BuildProvenanceError, "newer than linked"):
            record_build(self.repo, self.build, self.exe, "Release")

    def test_fixed_verifier_does_not_launch_stale_binary(self):
        self.manifest.unlink()
        recordings = self.evidence / "recordings.json"
        recordings.write_text("{}")
        argv = ["verifier", "--profile", "gameplay33m", "--mode", "frames",
                "--exe", str(self.isolated), "--recordings", str(recordings),
                "--evidence-dir", str(self.evidence)]
        with patch("sys.argv", argv), patch.object(Path, "cwd", return_value=self.repo), \
                patch.object(hg_runtime_verifier, "verify_frames") as launch, \
                contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(hg_runtime_verifier.main(), 2)
            launch.assert_not_called()
        result = json.loads((self.evidence / "verifier-result.json").read_text())
        self.assertEqual(result["status"], "stale_build")
        self.assertIsNone(result["gameplay_fps"])
