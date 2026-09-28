import sys
import struct
import unittest
from pathlib import Path

sys.path.insert(0,str(Path(__file__).resolve().parents[1] / 'tools'))
from oracle_capture import (IOP_TOC_DMA_BCR,IOP_TOC_DMA_CHCR,IOP_TOC_DMA_SIZE,
                            extract_iop_toc_candidate,find_iop_toc_dma_descriptors,
                            find_iop_gettoc_request_frames)


class OracleCaptureTests(unittest.TestCase):
    def test_finds_only_original_gettoc_request_frame(self):
        data=bytearray(0x4000);module_base=0x800;descriptor=0x1800
        struct.pack_into('<HHIIHHI',data,descriptor,4,0x81,0x2000,0,0,0x84,0)
        struct.pack_into('<I',data,descriptor+0x30,module_base+0x70c4)
        self.assertEqual(find_iop_gettoc_request_frames(data,module_base),[{
            'frame':descriptor-0x18,'descriptor':descriptor,'madr':0x2000,
            'saved_return':module_base+0x70c4}])
        struct.pack_into('<I',data,descriptor+0x30,module_base+0x70c8)
        self.assertEqual(find_iop_gettoc_request_frames(data,module_base),[])

        window=bytearray(0x200);descriptor=0x40
        struct.pack_into('<HHIIHHI',window,descriptor,4,0x81,0x2000,0,0,0x84,0)
        struct.pack_into('<I',window,descriptor+0x30,module_base+0x70c4)
        self.assertEqual(find_iop_gettoc_request_frames(window,module_base,0x1e0000,0x200000)[0]['descriptor'],
                         0x1e0040)

    def test_toc_candidate_extracts_exact_bounded_range(self):
        offset=0x1234
        data=bytearray(offset+IOP_TOC_DMA_SIZE+4)
        data[offset]=0x12
        data[offset+IOP_TOC_DMA_SIZE-1]=0x34
        data[offset-1]=0x56
        self.assertEqual(len(extract_iop_toc_candidate(data,offset)),IOP_TOC_DMA_SIZE)
        self.assertEqual(extract_iop_toc_candidate(data,offset)[0],0x12)
        self.assertEqual(extract_iop_toc_candidate(data,offset)[-1],0x34)

    def test_toc_candidate_rejects_short_iop_map(self):
        with self.assertRaises(ValueError):
            extract_iop_toc_candidate(bytes(IOP_TOC_DMA_SIZE-1),0)

    def test_finds_only_bounded_aligned_active_toc_descriptors(self):
        data=bytearray(64)
        struct.pack_into('<III',data,8,0x12340,IOP_TOC_DMA_BCR,IOP_TOC_DMA_CHCR)
        struct.pack_into('<III',data,28,0x1ffffc,IOP_TOC_DMA_BCR,IOP_TOC_DMA_CHCR)
        self.assertEqual(find_iop_toc_dma_descriptors(data,0x10000000),
                         [{'host_address':0x10000008,'madr':0x12340}])
