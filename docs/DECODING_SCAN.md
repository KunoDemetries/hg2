# Comprehensive decoding scan

Latest speed audit: 2026-09-13, original USA executable and all29 configured
IOP modules. External report: %TEMP%/haunting-major-speed-audit/major-scan.json.
EE:162833 reachable words,1071 boundaries (1059 indirect transfers,7 syscalls,
4 unsupported sites,1 exception return). All-module IOP:94718 words,668 guarded
links,149 native adapter sites,125 residual boundaries (121 indirect,4 outside
module words),244 original BREAK traps. No unconfigured module files found.
The current28-module generated build omits rom_romdrv, exactly178 words;
this accounts for the difference from94540 compiled IOP words. This inventory
does not establish that ROMDRV is required by the observed opening path.

Original EE inspection places unsupported words at10dabc/10dda4 in vector
bodies and26cb74/26cbb0 in cache bodies. These remain explicit faults, not
silently skipped instructions. Replay33 reaches60M slices without invoking
those faults. Boundary counts are incomplete static proofs, not a count of
missing executed routines. More code can be required in later gameplay.

Dense host profiling of replay35 still identifies EE execution as the largest
sampled opening cost. Original115bf4 and115cf8 byte-copy loops are statically
proved optimization candidates; their batching has synthetic budget/state
coverage but awaits connected replay validation. No roots were guessed or
added merely to reduce boundary counts.

## Historical inventory (2026-09-05)

The following tables are historical, superseded by the audit above.

This is a prioritization inventory, not a playability claim. The complete
machine-readable site list is regenerated with `python tools/hg.py major-scan`
into ignored `out/major-scan.json`.

## Coverage

| Domain | Reachable translated words | Explicit boundaries |
|---|---:|---:|
| EE executable | 34,624 | 307 |
| IOP modules, isolated total | 76,531 | 534 |
| IOP all-module static bundle | 76,531 | 55 residual + 193 defined BREAK traps |

The planner proves 664 guarded original-module import links and 130 native ABI
adapter sites. No residual instruction is silently skipped.

## Remaining all-module IOP boundaries

| Family | Count |
|---|---:|
| `jalr $v0` dynamic calls | 44 |
| `jr $v0` dynamic tail calls | 1 |
| `jalr $v1` dynamic calls | 5 |
| Other indirect transfers (`$t0`, `$s0`, `$t4`) | 3 |
| SECRMAN 1.3 authentication ordinal 6 | 2 |
| IOP syscall (THREADMAN selector 32) | 1 |

MODLOAD's selector at `+0x0bbc` independently proves an unsigned `<8` check
before its relocated read-only table at `+0x35b0`; its eight code targets are
now statically rooted.  This removes that transfer boundary but exposes 438
additional reachable original words, including two still-unproven dynamic calls
and the already-inventoried selector-12 syscall.  The higher residual count is
therefore coverage growth, not a fallback or a fabricated resolution.

## Verified static-selector batch

Directly decoded unsigned bounds checks and relocated table loads now root seven
MODHSYN tables, two MODMIDI tables, four LIBSD tables, four CDVDMAN tables,
and one table each in THREADMAN, FILEIO, TIMEMANI, and MCMAN.  Every configured
table entry is aligned and executable-range checked by the planner.  MODHSYN
`+0x9918` separately lists only its six executable targets: selector 10 contains
a non-code value and remains an explicit generated-dispatch fault if encountered.
The batch expands coverage by 3,787 words relative to the preceding MODLOAD-only
checkpoint and exposes 31 additional defined BREAK traps while reducing residual
boundaries from 76 to 61.  A subsequent ROM MODLOAD seven-entry switch at
`+0x14c8` reduces the current residual total to 60.

The remaining transfers are dominated by 44 `jalr $v0` calls and five
`jalr $v1` calls; most load mutable callback fields. A setup shape is evidence
only and never becomes a target by itself.

ROM MODLOAD's selector-12 syscall is separately lowered as a checked AOT
`CpuInvokeInKmode`-shape transfer. Configuration is accepted only when the
original site is a syscall immediately preceded by an exact `v0=12` setup;
this does not authorize any other syscall. THREADMAN selector 32 remains an
explicit scheduler/context-switch boundary.

A single native diagnostic run watched all 56 prior dynamic sites. Four were
executed before GetToc and supplied four guarded, executable-range-checked
targets across LOADCORE, SIFCMD, and MODLOAD. Any unobserved runtime target at
those dispatches still faults; the current residual total is 55, including 52
dynamic transfers.

## Verified IOMAN callback batch

A native-generated external 2 MiB IOP RAM dump at the exact GetToc boundary
contained two original registered drivers: `tty` and `cdrom`. `tty` operation
slots through `+0x40` point to IOMAN `+0x18e0`; `cdrom` points into CDVDMAN.

| IOMAN site | Slot | Verified targets |
|---:|---:|---|
| `+0x19a8` | `+0x00` | IOMAN `+0x18e0`, CDVDMAN `+0x00b0` |
| `+0x1ae4` | `+0x04` | IOMAN `+0x18e0`, CDVDMAN `+0x0390` |
| `+0x1564` | `+0x64` | CDVDMAN `+0x33d4` |
| `+0x1458` | `+0x5c` | CDVDMAN `+0x2c44` |
| `+0x1338` | `+0x54` | CDVDMAN `+0x33d4` |
| `+0x123c` | `+0x50` | CDVDMAN `+0x33d4` |
| `+0x10c0` | `+0x2c` | IOMAN `+0x18e0`, CDVDMAN `+0x33d4` |
| `+0x10e8` | `+0x4c` | CDVDMAN `+0x33d4` |
| `+0x0ee0` | `+0x60` | CDVDMAN `+0x33d4` |
| `+0x0d48` | `+0x08` | IOMAN `+0x18e0`, CDVDMAN `+0x33d4` |
| `+0x0be8` | `+0x3c` | IOMAN `+0x18e0`, CDVDMAN `+0x0964` |
| `+0x0bbc` | `+0x40` | IOMAN `+0x18e0`, CDVDMAN `+0x33d4` |
| `+0x0a7c` | `+0x24` | IOMAN `+0x18e0`, CDVDMAN `+0x33d4` |
| `+0x09c0` | `+0x38` | IOMAN `+0x18e0`, CDVDMAN `+0x0b28` |
| `+0x0920` | `+0x30` | IOMAN `+0x18e0`, CDVDMAN `+0x0658` |
| `+0x0860` | `+0x68` | CDVDMAN `+0x2a7c` |
| `+0x0790` | `+0x20` | IOMAN `+0x18e0`, CDVDMAN `+0x2a2c` |
| `+0x06f8` | `+0x34` | IOMAN `+0x18e0`, CDVDMAN `+0x1ef0` |
| `+0x0644` | `+0x18` | IOMAN `+0x18e0`, CDVDMAN `+0x33d4` |
| `+0x05a4` | `+0x14` | IOMAN `+0x18e0`, CDVDMAN `+0x2828` |
| `+0x04b8` | `+0x58` | CDVDMAN `+0x33fc` |
| `+0x03a4` | `+0x1c` | IOMAN `+0x18e0`, CDVDMAN `+0x321c` |
| `+0x02f4` | `+0x0c` | IOMAN `+0x18e0`, CDVDMAN `+0x19e0` |
| `+0x00f4` | `+0x04` | IOMAN `+0x18e0`, CDVDMAN `+0x0390` |

Module-qualified references are validated against destination executable
sections and add destination routines as AOT roots. Future driver targets still
fault. This batch reduced residuals from 97 to 75 while adding 2,339 reachable
IOP words. Release tests pass 18/18. A 20-million-slice run reaches the unchanged
GetToc boundary (`MADR=0x000c3464`, `BCR=0x00810004`, `CHCR=0x41000200`).

## Priority order

1. Obtain the real external 2,064-byte GetToc DMA record, the immediate startup
   gate; it cannot be derived from ordinary ISO user sectors.
2. Batch the remaining `jalr $v0` and `jr $v0` families by
   live table provenance.
3. Establish syscall 12/32 effects independently. Keep SECRMAN authentication a
   fault until real card/SIO2 behavior exists.
4. Continue EE/VU/GIF/GS/audio/input/save work after module loading advances.
   Opening a host window alone is not game startup.
