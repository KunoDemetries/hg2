"""Pure report/capture tests; no game launch, network or connected-file access."""
import copy
import math
import tempfile
import unittest
from pathlib import Path

from hg_runtime_verifier import materialize_final_frame, steady_gameplay_summary, normalized_iop_digest


def windows(light_seconds=1.0, heavy_seconds=(3.0, 3.0)):
    result = []
    for index, seconds in enumerate((light_seconds, light_seconds, 2.0, *heavy_seconds)):
        result.append({
            "guest_start_us": (28 + index) * 1_000_000,
            "guest_end_us": (29 + index) * 1_000_000,
            "boundaries": 30,
            "host_seconds": seconds,
            "fps": 30 / seconds,
        })
    return result


class SteadyGameplayTests(unittest.TestCase):
    def test_menu_speed_cannot_raise_gameplay_result(self):
        for light in (0.01, 0.1, 1.0, 10.0):
            summary = steady_gameplay_summary(windows(light))
            self.assertEqual(summary["fps"], 10.0)
            self.assertEqual(summary["boundaries"], 60)
            self.assertEqual(summary["host_seconds"], 6.0)
            self.assertFalse(summary["passes_30fps"])
            self.assertEqual(summary["window_indices"], [3, 4])

    def test_total_frames_over_host_time_not_mean_of_rates(self):
        summary = steady_gameplay_summary(windows(heavy_seconds=(0.5, 2.0)))
        self.assertEqual(summary["fps"], 24.0)
        self.assertEqual(summary["minimum_fps"], 15.0)
        self.assertEqual(summary["maximum_fps"], 60.0)
        self.assertFalse(summary["passes_30fps"])

    def test_every_heavy_interval_must_pass(self):
        self.assertTrue(steady_gameplay_summary(windows(heavy_seconds=(1.0, 1.0)))["passes_30fps"])
        # Aggregate >30 is insufficient if either interval falls below target.
        summary = steady_gameplay_summary(windows(heavy_seconds=(0.5, 1.1)))
        self.assertGreater(summary["fps"], 30)
        self.assertFalse(summary["passes_30fps"])

    def test_recorded_boundary_offset_is_within_qualified_range(self):
        data = windows()
        for item in data:
            item["guest_start_us"] -= 31576
            item["guest_end_us"] -= 31576
        self.assertEqual(steady_gameplay_summary(data)["fps"], 10.0)

    def test_zero_frames_do_not_become_success(self):
        data = windows()
        data[-1]["boundaries"] = 0
        summary = steady_gameplay_summary(data)
        self.assertEqual(summary["fps"], 5.0)
        self.assertFalse(summary["passes_30fps"])

    def test_invalid_timing_counts_ranges_and_continuity_rejected(self):
        variants = []
        for value in (0.0, -1.0, math.nan, math.inf, -math.inf):
            data = windows(); data[-1]["host_seconds"] = value; variants.append(data)
        for value in (-1, True, 1.5):
            data = windows(); data[-1]["boundaries"] = value; variants.append(data)
        data = windows(); data[-2]["guest_start_us"] = 29_000_000; variants.append(data)
        data = windows(); data[-1]["guest_end_us"] = 34_000_000; variants.append(data)
        data = windows(); data[-1]["guest_start_us"] += 1; variants.append(data)
        data = windows(); data[-2]["guest_end_us"] += 300_000
        data[-1]["guest_start_us"] += 300_000; variants.append(data)
        variants.extend([windows()[:-1], windows() + [copy.deepcopy(windows()[-1])]])
        data = windows(); data[-1]["host_seconds"] = 1e308; data[-2]["host_seconds"] = 1e308; variants.append(data)
        for data in variants:
            with self.subTest(data=data), self.assertRaises(ValueError):
                steady_gameplay_summary(data)


class FinalFrameTests(unittest.TestCase):
    def test_post_budget_display_becomes_stable_frame_artifact(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            state = root / "verifier-state.ram"
            source = Path(str(state) + ".display.ppm")
            frame = root / "verifier-frame.ppm"
            payload = b"P6\n1 1\n255\n\x01\x02\x03"
            source.write_bytes(payload)
            self.assertEqual(materialize_final_frame(state, frame), source)
            self.assertEqual(frame.read_bytes(), payload)
            source.unlink()
            with self.assertRaises(ValueError):
                materialize_final_frame(state, frame)


class IopDigestTests(unittest.TestCase):
    RTC = (0xBFCD1, 0xBFCD2, 0xBFCD3, 0xBFCD5, 0xBFCD6, 0xBFCD7)

    def rtc_data(self, values=(0x00, 0x00, 0x00, 0x01, 0x01, 0x00)):
        data = bytearray(2 * 1024 * 1024)
        for offset, value in zip(self.RTC, values):
            data[offset] = value
        return data

    def test_only_documented_rtc_bytes_are_masked(self):
        data = self.rtc_data()
        a = normalized_iop_digest(data)
        values = (0x59, 0x42, 0x23, 0x30, 0x09, 0x26)
        for offset, value in zip(self.RTC, values):
            data[offset] = value
        b = normalized_iop_digest(data)
        self.assertEqual(a["sha256"], b["sha256"])
        self.assertEqual(b["original_bytes"], list(values))
        self.assertEqual(data[0xBFCD5:0xBFCD8], bytes((0x30, 0x09, 0x26)))
        for offset in (0, 0xBFCD0, 0xBFCD4, 0xBFCD8, len(data)-1):
            data[offset] ^= 1
            self.assertNotEqual(b["sha256"], normalized_iop_digest(data)["sha256"])
            data[offset] ^= 1

    def test_bad_size_and_non_bcd_fields_rejected(self):
        with self.assertRaises(ValueError):
            normalized_iop_digest(bytes(100))
        for offset, value in ((0xBFCD1, 0x60), (0xBFCD2, 0x1A), (0xBFCD3, 0x24), (0xBFCD3, 0xFF),
                              (0xBFCD5, 0x00), (0xBFCD5, 0x32), (0xBFCD6, 0x13), (0xBFCD6, 0x00), (0xBFCD7, 0xA0)):
            data = self.rtc_data(); data[offset] = value
            with self.assertRaises(ValueError):
                normalized_iop_digest(data)


if __name__ == "__main__":
    unittest.main()
