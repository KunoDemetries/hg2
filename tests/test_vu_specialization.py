"""Synthetic operand and surrounding-state checks for the optional AOT MUL route.

The unchanged reference route remains legal when a measured candidate is disabled.
Linked native callsite inspection, not these tests, proves production activation.
"""
import unittest
from unittest.mock import patch

from hgtool import vu_emit
from hgtool.vu_decode import decode_pair
from test_vu_decode import pair, upper_direct


class StaticMultiplyEmissionTests(unittest.TestCase):
    def test_exact_operands_and_unchanged_other_masks(self):
        for destination, source, other in ((3, 1, 2), (1, 1, 2), (2, 1, 2),
                                           (1, 1, 1), (0, 1, 2), (3, 0, 2),
                                           (3, 1, 0), (31, 31, 31)):
            for mask in range(16):
                for broadcast in range(4):
                    decoded = decode_pair(0, pair(0x8000033c, upper_direct(
                        0x18 + broadcast, dest=mask, fd=destination, fs=source, ft=other)))
                    self.assertEqual(decoded.upper_name, "mul" + "xyzw"[broadcast])
                    reference = f"v.vu1.multiply_vector({destination},{source},{other},{mask},{broadcast});"
                    compact = f"hg::vu_masked_multiply<{mask},{broadcast}>(v.vu1,{destination},{source},{other});"
                    allowed = ([reference], [compact]) if mask in (14, 15) else ([reference],)
                    self.assertIn(vu_emit._upper(decoded), allowed)

    def test_only_the_upper_call_changes_in_pair(self):
        for mask in (14, 15):
            for broadcast in range(4):
                decoded = decode_pair(0x20, pair(0x8000033c, upper_direct(
                    0x18 + broadcast, dest=mask, fd=1, fs=1, ft=2)))
                reference = f"v.vu1.multiply_vector(1,1,2,{mask},{broadcast});"
                compact = f"hg::vu_masked_multiply<{mask},{broadcast}>(v.vu1,1,1,2);"
                actual = vu_emit._pair_lines(decoded)
                with patch.object(vu_emit, "_upper", return_value=[reference]):
                    expected = vu_emit._pair_lines(decoded)
                self.assertEqual([line.replace(compact, reference) for line in actual], expected)


if __name__ == "__main__":
    unittest.main()
