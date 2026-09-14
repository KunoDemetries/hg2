# Sources and provenance

## OPENING audio buffer overlap (2026-09-13)

Replay18 starts OPENING but stalls after four conversions. Original2429c8
gets audio position through242a88/1d3210/1d30c8. Replay19 shows8192 produced
samples minus two8192 queued counts, clamped to0 by original1d31c0..1d31e4.
The live audio synchronization buffer121b00..1223d0 matches2254/2256 bytes of
the original DS2O_S1.IRX at file offset1d60. Its0x880-byte transport payload
at121b40 exactly matches EE3cf1c0. External opening-audio-buffer-overlap.json
records hashes and extents. Original inputs and our native captures only;
no emulator or shared implementation used to derive the allocation fix.

MODLOAD's native scratch arena started at1MiB independently of original
SYSMEM, whose SIF heap client had allocated this live audio buffer. Connected
ordinary native storage now delegates to original compiled SYSMEM exports4/5.
Static module space and native call stacks are reserved; worker stacks and
HEAPLIB storage share that allocator. This executes original AOT routines,
with CPU context preserved on a separate call stack, without decoding code
at runtime or substituting audio time. Connected verification is pending.

## GS device progress after CAPCOM (2026-09-13)

Replay12 reaches100M modeled microseconds without fault but retains4830
submitted draws after the last15355completed draws, leaving scanout stale.
Original submissions continue each field. GS User Manual v6 pp38-43 states
drawing begins when a kick has sufficient vertices and maintains register and
transfer order. The connected device phase now services the existing ordered
draw queue after VIF/GIF submission, independently of preview observation.
FINISH/transfer ordering checks remain. This is functional execution, not exact
pixel-pipeline latency. Derived from the local manual and native evidence only;
no shared rendering material or emulator implementation inspected or copied.

## Visible CAPCOM movie evidence (2026-09-13)

startup-movie-services runs the independently translated original ELF through input streaming, intra/non-intra BDEC, native original motion-copy code, CSC and original draw submission. External movie-services-frames/manifest.json hashes sequential captured PPM bytes; PNG006/007/008/013/028 visibly show the CAPCOM logo with Fiona/Hewie fading in. OpenGL watch-display uses the same published PPM. Runtime reached500M slices without a fault; final native-iop-budget sidecars are retained. This establishes advancing movie imagery, not full playability or real-time/audio correctness. Registry expansion used seven14-entry tables, verified against original ELF and RAM in movie-service-tables.json; no shared rendering reference was used.

## Motion prediction transport and instructions (2026-09-13)

Original2a0dd4..e6c loads two8-entry callback tables3ed6d0..710, storescontext+5c8/+5d8; consumers2a0f20/34. All entries agree with startup-csc RAM. QFSRV/PEXTLB/PEXTUB/PCGTH/PSLLH/PSRLH/PSRAH derive from EE Core Manual pp286/203/206/185/261/267/264. Tests check all16 byte-funnel shifts, source aliases, byte order, signed comparison and lane shifts. No shared rendering material consulted. toSPR source-chain derives from EE User Manual DMA chapter45-46/73-76 and original tags1990500..530 in startup-nonintra. Supports CNT/NEXT/REF/REFE/END, TIE, saved transfer continuation, SADR wrap; unsupported modes remain faults.

## Non-intra BDEC and prediction copy (2026-09-13)

Original2a1fe4 issues20010000 in startup-csc; adjacent2a28d0 copy uses PADDH. EE manual pp191-192 and H.262 TableB.9/PDF140 define CBP. H.262 sections7.4.2-7.4.4 define inverse quantization. CBP table independently transcribed with64-entry, complete-mask and prefix-free validation. Signed floor quantization and IDCT limitations are recorded in ORACLE.md. PADDH is derived from EE Core Instruction Manual p161 with synthetic modulo-overflow/lane-alias checks.

## CSC RGB32 (2026-09-13)

Original2a7110 issues70000380 (896 macroblocks), captured startup-movie-callbacks. RAW8/RGB32 layout and conversion derive independently from EE manual pp177-180,197,205-206. Integer coefficient and threshold formulas plus measured rounding discrepancy are recorded in docs/ORACLE.md. RGB32 without dithering only; native input/output pressure and multi-block state retain actual data.

## Movie pixel-copy translations (2026-09-13)

Original ELF routine2a2998 uses ADDI, PMAXH, PMINH and PPACB. Independently derived from EE Core Instruction Manual pages25/396,222,235,256. Page396 opcode map resolves the erroneous ADDI encoding graphic on page25. Native synthetic tests cover overflow, sign extension, signed lane selection, source/destination aliasing and byte order. Reserved-field mask word00ff00ff is separately measured in docs/ORACLE.md; only that exact encoding is admitted as DSRA32.

## IPU output and intra BDEC (2026-09-13)

Original2a392c arms DMA3 to scratchpad80001800/QWC48;2a1fe4 issues BDEC
2c010000. EE manual pp174-176,180,191-192,201 defines RAW16 output,
commands, frame layout and symbols; DMA register chapter defines normal
fromIPU transport. Native output waits for real data and validates destinations.

Coefficient/DC tables, inverse scanning, inverse quantization and mathematical
IDCT are independently derived from ITU H.262 (2000), sections7.2-7.5,
Annex A and tablesB.12-B.16. Official PDF downloaded to %TEMP%/hg-itu-h262.pdf,
SHA2563d38fcd0096da6d27b4c63f3483cc65e55bbd61e254c9213fccf8f0d2df23617.
Source https://www.itu.int/rec/T-REC-H.262-200002-S/en . Numeric codebooks
are specification data, not copied software algorithms. runtime/ipu.cpp uses
a directly derived separable cosine transform. Intra no-mismatch and0..255
clipping follow the measured synthetic profile in docs/ORACLE.md, whose
limitations must not be represented as physical-console accuracy. Non-intra,
MPEG1, field-DCT and CSC remain explicit gaps. hg_ipu is a separate native
library to avoid repeated AOT recompilation for block-decoder implementation
changes. This is data decoding for the statically recompiled program, not
a runtime instruction interpreter or an emulator execution backend.

## IPU VDEC (2026-09-13)

Original2a2174 in startup-picture-switch writes30000000, address increment.
EE User Manual v6 pp193,202-204 specifies all seven table variants: address,
I/P/B/D type, motion and DMVector. ipu_vlc.hpp contains declarative data
transcribed from those specification tables, including MP1-only stuffing
and packed length/result. Cross-check for address increments: ITU H.262
Annex B.1, https://www.itu.int/rec/T-REC-H.262-200002-S/en . No third-party
implementation read or copied. Tests cover every entry at all128 offsets,
independent packed examples, error/ECD and pending input before/after symbol
consumption. Cycle timing and memory-oracle parity are not claimed.

## Shared IPU stream and FDEC (2026-09-13)

Independent implementation from EE User Manual v6 pp182-188,194-196: separate
eight-qword FIFO/internal qwords, MSB-first FDEC with FB0..32 and a retained
32-bit result, CMD/TOP busy reads, BCLR, and shared SETIQ/SETVQ consumption.
Commands wait for absent input and resume when DMA/CPU supplies qwords.
Synthetic observation matches and limitations are in docs/ORACLE.md. The
empty-input FB crossing discrepancy is explicitly rejected. Non-byte-aligned
table extraction follows the manual and has synthetic tests; it is not an
oracle measurement. Reset-only prefetch timing remains unverified; reset
retains the existing no-prefetch profile until BCLR or a stream command.
No image decode/output or cycle-accurate processing is claimed. No external
implementation or shared rendering material was consulted.

## IPU input DMA endpoint (2026-09-13)

Native startup-movie-timing reaches original24ed44, writing CHCR30000105
to1000b400 after original24f230/24f26c constructs REF/REFE tags through
24ed68. EE User Manual v6.0 pp44-46 defines normal/source-chain copying;
p62 defines suspend/restart and retained TAG, pp73-76 identifies channel4
and its registers. Page182 specifies an eight-qword IPU input FIFO.
Implemented independent channel4 transport for normal and CNT/NEXT/REF/REFE/END
chains, saved MADR/QWC continuation, FIFO backpressure and completion flags.
The connected runner pumps one bounded transfer step per slice. Full FIFO
retains STR and pending bytes; it never invents consumption. Unsupported
tag priority/stall/stack and tag transfer remain explicit faults. IPU decode
commands and output are still absent. Tests exercise copying, pressure/resume,
chain continuation/IRQ completion, invalid input and mode rejection.
No shared HG rendering material or PCSX2 implementation was consulted.

## EE second-pipeline division (2026-09-13)

Connected startup-stream-copy stops at1e3774, original word7066001a.
EE Core Instruction Set Manual v6.0 pp138/140 defines MMI functions26/27,
zero rd/sa fields, signed/unsigned 32-bit division into sign-extended LO1/HI1.
Original1e3770 executes DIV and1e3774 DIV1, then reads HI/HI1 separately
to compare ring-buffer remainders. Extended the existing checked division
helper with bank selection and added both encodings; undefined zero-divisor
and noncanonical operands retain explicit diagnostics. Synthetic AOT tests
cover four sign combinations, INT_MIN/-1, unsigned results/sign extension,
bank preservation and fault atomicity. Timing remains the existing functional
interlock model, without cycle-accurate pipeline latency. No reference
implementation was consulted; shared non-rendering address search had no lead.

## EE unaligned-store byte preservation (2026-09-13)

EE Core Instruction Set Manual v6.0 pp99-102 (SDL/SDR),117-120 (SWL/SWR),
read from external hg-ee-core-instruction-manual.pdf, defines left stores into
the low memory lanes and right stores into the high lanes. Our EE helpers
incorrectly reused load-direction masks while shifting store data in the
opposite direction, clearing unrelated bytes and retaining stale destination
bits. Corrected only the two store masks. Byte-wise synthetic tests cover
every offset for both widths, individual partial writes, and paired writes in
both orders with nonzero neighboring bytes; the old implementation fails.
Original2437f0 constructs a descriptor with capacity input minus padding, and
243d48..78 copies it using LDL/LDR and SDL/SDR. Stream1 record10091bc has
its descriptor at10091cc (four-byte offset); its zero capacity and callback
pointer in startup-parser.ram are consistent with this corruption. Connected
replay startup-unaligned-fixed confirms capacity0x5b800 and callback0x3cffa8
are now preserved; it reaches callback2419d0 before the former parser error.
All21 Windows Release tests pass. No creation parameter is patched.
No emulator implementation or shared rendering material used.

## Normal scratchpad DMA (2026-09-13)

Original23c144 selects channel8;23c160 writes SADR and23c170 calls the
original normal transfer helper10d888, which stops at STR write10d8f4.
Live startup-movie-vtable capture has s16=1000d000, destination197aa80,
QWC argument400 (1024 qwords). Derived normal burst copying independently
from EE User Manual v6 pp43-44, fixed channels p73, DIR applicability p74,
main-memory MADR p75 and14-bit SADR p78. Local manual is
%TEMP%/hg-ee-users-manual.pdf. Both fixed directions8/9 are implemented in
the connected runner; full RAM-span validation precedes copying, SADR wraps
to14 bits, completion clears STR and raises channel D_STAT. Chain/interleave,
MFIFO/stall modes remain explicit gaps. Cycle-accurate arbitration is not claimed.
No shared rendering or emulator implementation material was consulted.

## Positive SQRT power-of-two profile (2026-09-13)

Original1c6c50/54 loads40000000 into ft22;1c6c64 executes SQRT.S fd20,ft22.
EE Core Instruction Set Manual v6.0 p376 specifies the operand fields,
square-root result and I/D/SI flags. Synthetic memory-only observations in
ORACLE.md establish truncated b504f3 significand for positive normal powers
of two at2.0 and0.5, and positive-result I/D clearing with sticky/U/O retained.
The independent integer-root implementation now accepts that significand
profile in addition to exact squares. sqrt(5) and negative/extended operands
remain explicit validation boundaries; do not infer universal truncation from
the supported cases. External fpu-sqrt-oracle/verified.json has five agreeing
views,26 cases, hashes and UTC provenance. No emulator implementation used.

## Connected EE timer clock (2026-09-13)

Original startup-global-callbacks capture stops at EE25c04c, reading T1_COUNT
10000800 after25c01c sets mode82 (enabled BUSCLK/256). Virtual time197454482us.
The existing timer model had no clock connected in the system runner. Sony EE
User's Manual v6.0 p36 defines BUSCLK147.456MHz and the three internal divisors;
p20 independently states the same bus frequency. Local source is external
hg-ee-users-manual.pdf. Public documentation:
https://psi-rockin.github.io/ps2tek/#eetimers and
https://ps2dev.github.io/ps2sdk/timer_8h.html . Only register/frequency definitions
were consulted; no emulator implementation was used.

EeTimerClock converts the existing diagnostic microseconds to bus cycles with
an integer fractional remainder. Timer flag rising edges feed INTC9..12;
acknowledging INTC alone does not create another timer edge. Tests cover576
ticks per1000us at /256, wrap, disabled counting, compare/overflow delivery,
flag acknowledgement/rearm, and transactional rejection of unsupported gating.
This preserves the existing1us/slice compatibility clock, not cycle-accurate
EE instruction timing. External/HBLANK and gated counting remain explicit gaps.

## Odd indexed host-to-local transfer stride

The original submitted packet captured in `startup-transfer-provenance.ram.ee-ram.bin`
contains BITBLTBUF0x130b220000000000 at0x4f20f0 and TRXREG720x540 at0x4f2110.
Its IMAGE REF at0x4f2140 names source0x8f0080/QWC0x5eec. The independently decoded
original builder at0x1bb230 stores this layout and derives DBW as width>>6;
the exact dynamic builder invocation has not been watched yet.

Sony GS User's Manual v6.0, printedp101, specifies BITBLTBUF widths1..32 in
64-pixel units; pp162-163 specify8KiB pages,128-pixel indexed page widths and
row-major page arrangement. These pages alone do not establish odd-DBW rounding.
An independently assembled synthetic upload observed as memory-only evidence
settles DBW11's five-page row stride for PSMT8 and PSMT4; see docs/ORACLE.md for
all inputs, observed physical offsets, identities and limitations. Runtime code
uses those observations with the existing independently derived page layout;
no shared rendering material or emulator implementation was consulted.
Further independent DBW1/3/31 observations establish zero/one/fifteen-page
transfer strides and alias ordering; their identities and exact physical bytes
are documented in ORACLE.md and covered by the GIF regressions.
Manual: https://usermanual.wiki/Pdf/GSUsersManual.1012076781.pdf

## Odd indexed texture sampling: observed caller and measured profile

The original queued draw62385 in `startup-indexed-stride.ram.gs.json` has
TEX0=0x2007e006a932e200: PSMT8,TBP0x2200,TBW11,TW10,TH10,CBP0x3f00,
CLD1. It follows the now-supported720x540 indexed upload and reaches the
explicit texture-width guard at EE0x10d744. This identifies the actual next
path; it does not establish how the unsupported texture width should sample.

Sony GS User's Manual v6.0 printedp27 requires128-pixel buffer alignment for
PSMT8/PSMT4; p125 defines TEX0 fields and p162 indexed page dimensions.
The prepared synthetic sampling probe therefore treats odd widths as behavior
requiring measurement rather than claiming the manual guarantees support.
Its framebuffer download follows the GS manual p77 FINISH/CSR/BUSDIR ordering;
the VIF1 FIFO-direction bit23/register address follows the independently
documented hardware register interface at https://psi-rockin.github.io/ps2tek/ .
The former approval blocker is resolved. At2026-09-12T17:18:40Z an independent
one-pixel sprite probe produced all56 expected colors and200 unchanged sentinel
pixels in three stable guest-memory readbacks. PSMT8/PSMT4 at TBW1,2,3,10,11,30,31
use the observed whole-page row stride, including the zero/one-page alias cases.
ORACLE.md records hashes, exact indices, capture provenance and limitations.
The source coordinates and all expected pixels are unchanged from the original
point probe. Sprite coverage follows the original GS manual printedp46; a
separate drawing FINISH wait precedes the documented readback setup sequence.
The native even-width-only preparation check also passes24 samples, but it is
not the independent measurement. Runtime support uses our existing physical
address layout with fixed-marker regressions derived from observed values.
The earlier point probe still missed pixel55 after extra FINISH and reordered
submissions; that discrepancy is preserved as unresolved evidence, not silently
accepted. No emulator rendering source, algorithms or shared configuration
were read or copied. Physical-console behavior is not claimed.

## Prompt-selection sound completion

Own `startup-clut-left-cross.log` reaches EE0x21f280 through original SIFRPC
completion (return0x26fad4) immediately after the diagnostic Left pulse at180m
slices. Our original-ELF decoder verifies `sync; jr ra; ei` at0x21f280..288.
Original0x21fd80/0x21fda0 constructs that callback in a7 and0x21fda4 passes it
to0x2700e8;0x220114/0x220118 supplies it on a second call path. The user-supplied
non-rendering manual function list corroborates the12-byte range. Only this
observed root is added; generated instruction effects come from the original ELF.
No sound completion, menu branch or guest state is synthesized.

## Controller configuration protocol

Next own `startup-controller-config.ram.json` requests41h with nine bytes.
The implementation follows ps2tek's mode-dependent response: zero capability
bytes in digital mode; maskffff03 plus trailing5a in analog mode. psx-spx
instead describes a constant mask and zero final byte, so this distinction
is explicitly provisional and should be checked against a physical/oracle
observation if original protocol negotiation fails. No data was copied from
an emulator. The connected runner still faults on unsupported commands.

Own `startup-controller-states.ram.json` records SIO2 input `[1,67,0,1,0]`
and SEND3=0x140540: enter configuration mode from the initial digital mode.
The virtual pad now retains configuration, analog/lock, axes and motor-map
state. Commands42/43/44/45/46/47/4c/4d follow the documented DualShock2
wire protocol, preserving the old mode in an enter-config reply and the
old map in a rumble-map reply. The controller remains a native device model,
not a native replacement for the original DS2O protocol worker. Pressure,
watchdog reset timing and host input/rumble output remain unfinished.
SIO2 validates equal5/9-byte send/receive profiles and commits controller
state only after a valid transaction. Unsupported commands still fault.
Protocol sources: https://psx-spx.consoledev.net/controllersandmemorycards/
and https://psi-rockin.github.io/ps2tek/ . These are hardware protocol
references; no emulator or shared recompilation implementation was used.

## Display-woken controller state dispatch

Own `startup-display-edges.log` reaches DS2O_S1 at0x2c2c0 after publishing
real timed display edges to WaitVblankStart. Original module base0x2c000
code at+0x294..+0x2b8 loads the controller state byte+66, checks unsigned
<14, then dispatches through relocated table0x2df10..0x2df48. Added only
that bounded table hint; the original code remains the controller protocol
implementation. Searches of supplied non-rendering lists found no entry.

## Configured display timing and EE INTC chains

Original SetGsCrt arguments in `startup-intc-state.log` establish interlaced
NTSC mode2, frame0. The independent timing producer uses27MHz units,
858 units per half-line,525 half-lines per field, and240 active lines;
that yields exactly60 fields per1001ms. TI's TVP5147M1 data sheet documents
525-line NTSC and13.5MHz/858 full-line sampling (equivalently27MHz/1716).
TI's TVP5158 data sheet gives480 active lines per interlaced frame. The
native phase starts with active video on configuration, followed by blank;
this establishes a deterministic startup phase, not a measured hardware
power-on phase. Custom scan registers and other video modes still fault.
Sources: https://www.ti.com/document-viewer/TVP5147M1/datasheet and
https://www.ti.com/lit/pdf/sles243 .

ps2tek's EE BIOS specification defines AddIntcHandler insertion:0 at the
front,-1 at the back, otherwise before the existing ID. The new dispatcher
reconstructs this order from stable registered IDs, retaining removed nodes
when reconstructing historical relative insertions. The original observed
callbacks return0; unverified other returns remain explicit faults. CPU
state and pending kernel-frame depth are preserved around the chain; a
new edge during it remains latched for subsequent service. The diagnostic
uses its existing cooperative interrupt-enable profile rather than claiming
full architectural exception/CP0 interrupt-mask emulation. PS2SDK kernel.h
provides the public one-argument and extended callback prototypes.
https://psi-rockin.github.io/ps2tek/
https://raw.githubusercontent.com/ps2dev/ps2sdk/master/ee/kernel/include/kernel.h
No emulator implementation or shared-project rendering material was used.

## Startup two-flag wait

Latest own `startup-intc-state.log` records configured display interlace1,
mode2, frame0; INTC status0/mask0x80c and CP0 status0x70010001. Registered
cause2 handler is0x1beda0; cause3 has both0x1aac30 and0x1bed80. Prior
unconfigured-display fault snapshots preceded this initialization. This
establishes missing edge generation/delivery as the current wait boundary,
while timing and handler-chain semantics still require implementation.

Own `startup-formatter-switch.log` reaches0x1bed00 from object dispatch
0x1696c8 while0x169680 formats original `ST_%03X` at ELF0x44f268.
Original pointer0x46ae0c is slot+0x1c of table0x46adf0, installed at
0x1be7c8/0x1be7d4. The method clears/polls GP-relative bytes-30444 and
-30440. Original0x1beda0 and0x1bed80 set these respective flags. Original
0x1bf198..0x1bf1d4 registers them for INTC causes2 and3. The native
`startup-two-flag-wait.log` records the registrations and then reaches its
30m-slice budget at0x1bed1c. Neither callback has executed. Existing EE
runtime only dispatches SIF0 DMA, and has no display-edge producer.
The supplied manual range for the initial wait target corroborated that
entry only; further display/interrupt investigation uses original inputs
and hardware documentation exclusively. No shared rendering material used.

## EE formatter switch

Own `startup-multi-record.log` advances past DVD transfer and reaches
EE0x26e054. Original0x26e010..0x26e03c adjusts the format character,
checks unsigned index<89, and loads a destination from0x45ae00..0x45af64.
Our ELF reader verifies19 distinct executable targets across the89 entries.
Configured that bounded table as an additive static hint. Exact-address
searches in the supplied non-rendering lists found no match. Runtime
continues to use the actual original format character and target.

## Multi-record DVD DMA

Own `startup-ee-read-complete.log` reaches channel3 descriptor MADR0xc3d88,
BCR0x0056000c, CHCR0x41000200. This is86 blocks of12 words, or two2064-byte
DVD records. Original CDVDMAN0xbaa68..0xbaa98 constructs blocksize12 and
blockcount43 times the transfer record count. The prior native endpoint
accepted one record only and always cleared the entire descriptor afterward.
The extension validates whole records and the remaining command/RAM bounds,
advances MADR and decrements BA per record, keeps STR active between records,
and publishes DMA/device completion at the end of that descriptor. The
successful local-image device IRQ remains the existing native completion
policy; physical drive timing is not modeled. Synthetic tests cover two
independent payloads, no premature IRQ, final completion, and rejection of a
descriptor longer than the command before writes.
DMA block-count/STR rules are independently documented at
https://psx-spx.consoledev.net/dmachannels/
(the sync-mode1 block count decrements to zero while the blocksize remains).
The supplied non-rendering sceCdRead label adds no DMA implementation evidence.

## Archive-sector DVD retry profile

Own `startup-archive-dma-complete.log` advances through IOP completion to
EE callback0x10f0a8, dispatched by original SIFRPC0x26facc with argument
0x3adf80. Own ELF decoding shows leading/trailing byte-fragment copies
through coherent RAM aliases followed by tail transfer0x10ee98. The supplied
non-rendering list labels this `_sceCd_cd_read_intr`; that label corroborates
the observed path. Added an AOT root with no native replacement.

After enabling the observed profile, own `startup-archive-retry0.ram` contains
all2048 bytes of the local DATA.CVM header exactly at IOP0xc3d94. Completion
then reaches0xceb58 from the original CDVD DMA handler, ra0xb82b0. Original
CDVDFSV0xcf0f0/0xcf0f4 and0xcf118/0xcf11c construct that callback and pass
it on the stack to original read calls. Own decoding verifies callback+0xb58
sets event bit0x20 through imported thevent:7 and returns zero. Added this
static root; no matching reference-list entry. Capture is native execution,
not emulator output, and remains external under TEMP/haunting-toc-probe.

Own `startup-cdvd-n-command.log` requests DVD LBA0x164972/count1 with
packet bytes `[114,73,22,0,1,0,0,0,0,2,0]`. This is the independently
verified DATA.CVM extent. Original CDVDMAN0xba9f0/0xba9f8 copies byte0
of the caller's mode structure into packet byte8, while0xbaa0c supplies
converted spindle2 and0xbaa14 writes final0. Existing successful local-image
reads accepted only retry16; now the observed retry0 profile also proceeds
through the same real sector/DMA path. Other profiles remain checked faults;
error retry/default-retry semantics are not implemented. A synthetic test
verifies retry0 LBA, transferred payload and completion state. Supplied
non-rendering lists label sceCdRead but provide no parameter implementation.

## CDVDFSV N-command RPC coverage

Own `startup-backend-extended.log` reaches IOP0xd1c1c with command14
and buffer0xd4288, returning to original SIFCMD0x998bc. Identity-checked
CDVDFSV at base0xce000 constructs handler0xd1c1c in a2 at0xd2508/0xd250c
and registers it at0xd2520 for ID0x80000595. Original handler bounds
unsigned(command-1)<19 at0xd1c8c..0xd1c94 and dispatches via0xd1cac
through relocated table0xd33d0..0xd341c. Added the module+0x3c1c root
and table hint+0x53d0/count19. Exact-address and non-rendering RPC searches
of the supplied reference lists found no applicable entry. Original module
bytes and our relocation/decoder provide all implementation evidence.

## Archive object callback

Own `startup-archive-dispatch.log` reaches0x1e6198 via0x1eb138. The
original wrapper loads backend slot+0x68, confirmed at0x455630; own decoder
verifies its tail jump to0x1d8668. The external non-rendering manual list
also identifies that wrapper. Extended stream dispatch uses slot+0x60;
its original pointer0x1db790 was already configured. Added only0x1e6198.

Own `startup-service-backend.log` reaches0x1e9260 through0x1e70b8,
slot+0x30 of the descriptor installed at0x1e6c2c. Original instructions
at0x1e6c18..0x1e6c2c establish the base as0x4558a0 (the constructor
pointer is at0x4558ac, slot+0xc). Non-null original methods end at+0x60,
followed by zero and strings; rooted the related methods together. Original
0x1e9260 polls archive state under the existing critical-section helpers.
External non-rendering range agrees; no implementation imported.

Own `startup-stream-status.log` reaches0x1ed1e0 via0x1d74ec after the
status switch. Original method tail-calls0x1e70a0 and is slot0 in original
archive service table0x3d5b18. Related non-null entries through+0x48
are rooted together from that table; the external non-rendering range
only corroborates the observed20-byte wrapper.

Own `startup-worker-table.log` then reaches0x1dadcc through the stream
status switch at0x1dad74. Original instructions at0x1dad58 bound the index
to ten entries; original pointer table0x453090..0x4530b8 contains four
distinct branch destinations. Configured that bounded table as an indirect
target hint, preserving the actual guest-selected status branch. The
supplied non-rendering function range for0x1dad28 agrees; no code imported.

Own `startup-backend-table.log` next reaches0x1df1c8 from0x1d74ec.
Our original ELF decoder shows a40-record worker loop, each record56 bytes,
calling0x1df1c0 for active records. Its pointer is slot0 of original table
0x3cacc0; related non-null entries through+0x38 are statically rooted.
The supplied non-rendering list agrees on the entry range, but all pointers
and generated instruction effects derive from the original ELF.

Own `startup-stream-table.log` passes the stream methods and reaches backend
0x1e6168 via indirect call0x1eb078, which loads slot+0x30. Original ELF
table0x4555c8 contains that pointer at0x4555f8. Own decoder verifies the
method tail-transfers to0x1d78b0; the supplied non-rendering analysis range
agrees on its20-byte entry. Remaining non-null table entries through+0x38
are rooted as a batch from original pointers, with existing0x1e6078 retained.

Own connected `startup-archive-found.log` reaches0x1e8580 from the object
slot+0x18 dispatch at0x1e6d48. Original ELF decoding verifies the entry,
argument checks, and tail transfer at0x1e8648 to0x1e8650. The supplied
non-rendering analysis list agrees on the entry range; no implementation
was imported. An explicit root lets our decoder follow its reachable code.
The next native stop0x1eae68 at0x1e7dc4 identifies slot2 of the original
15-pointer wrapper table0x455c38 (followed by zero). Related wrapper entries
are included together from the original ELF. The resulting wrapper dispatch
at0x1eae88 reaches backend0x1e6078, also found in original table0x4555c8
at+0x18. That observed backend entry is separately rooted; table contents
are evidence of static pointers, not a claim that every method has executed.
Next observed stream dispatch0x1d7360 reaches0x1db268. Its original code
updates a bounded stream position for origins0/1/2, and its pointer occurs
at0x3c50b0 in table0x3c5098. Non-null original callbacks through table+0x40
are rooted together, excluding already configured roots. This is independent
ELF discovery; the external analysis range was only corroboration.

## CDVD DMA interrupt delivery

Follow-up: own `startup-dvd-header-check.ram.json` showed the callback reading
synthetic TOC header0x29 before DVD DMA. `startup-read-error-write.log` identifies
original0xba268 setting error0x20 from that callback's header validation flags.
Completion was incorrectly latched while channel35 was disabled, then delivered
when a new DVD operation enabled it. Native channel-enable gating now prevents
that stale completion; global CPU interrupt disabling still retains pending
enabled-channel completions. DICR per-channel enable gates completion flags:
https://psx-spx.consoledev.net/dmachannels/#1f8010f4h-dicr-dma-interrupt-register-rw.
This is the native CDVD route's mapping; complete raw IOP DICR emulation remains
outside this change. The decoder-status zero was not the cause of error0x20.

After this correction, own `startup-dvd-gated-irq.ram` has a DATA.CVM search
record at0xd4698 (LBA0x164972, size0x5e018000) and error0 at0xccfd1.
EE next reaches original callback0x1ed1d0 via0x1d6a70. Own ELF decoding
confirms its three instructions return0x3d5b80. The supplied non-rendering
manual list contains the same range; our explicit root follows the observed
call and original bytes, without importing code or rendering material.

Identity-checked original CDVDMAN at0xb70b0..0xb70c4 registers interrupt35,
mode1, callback0xb8270. The read setup at0xb8488/0xb848c enables interrupt35
when the descriptor requests completion handling. The native sector copy
previously cleared DMA busy but never published channel3 completion, and
the cooperative dispatcher omitted the35 route. Successful DMA completion
now marks channel3 pending after copying the bytes; the existing dispatcher
delivers the registered handler when enabled, before the CDVD device IRQ in
the connected scheduler. No emulator implementation or shared rendering
material was used.

Own `startup-dvd-irq.ram.json` then reaches indirect DVD callback0xb98cc
from the DMA handler at0xb82a8. Its pointer also appears in the original
retained DMA descriptor at0x179458 in `startup-dvd-profile.ram`. Our decoder
confirms the original prologue at module+0x78cc; this is now an explicit
static translation root. The supplied non-rendering lists have no match.

## Eleven-byte DVD read command

Own native capture `startup-dvd-builder.ram.json` follows the original caller
at0xbaab4 into0xb8524 with command8, a packet pointer, and length11.
Identity-checked relocated CDVDMAN code at0xba9e8/0xba9ec writes LBA/count;
0xba9f0/0xba9f8 copies mode byte0 as retry count;0xba9fc..0xbaa0c
converts mode byte1 through0xb91c8 and stores the spindle selection;
0xbaa14 sets the final byte to zero. Original diagnostic format at0xbf3ac
names `spin` and `trycnt`, confirming their roles. The observed packet has
retry16, spindle2, final0. The runtime accepts only this successful local-image
profile, retaining explicit faults for other modes and incomplete packets.
Retry/error recovery and physical spindle timing are not implemented.
The same original builder at0xbaa68..0xbaa98 constructs block size12 and
block count43 for a one-sector read. Its retained descriptor in own capture
`startup-dvd-profile.ram` at0x179450 is0x002b000c, representing516 words.
ReadDvd DMA validation now checks total words rather than requiring the
alternative129-block/four-word encoding. It still requires the supported
control register value, alignment, RAM bounds, and exactly one DVD record.
No shared-project implementation was used. The supplied reference label
sceCdRead at EE0x110380 did not establish this IOP protocol.

## Native IOP event status

The identity-checked ROM THREADMAN module, relocated by our own tooling,
contains status wrappers at +0x36dc and +0x3774. Its copier at +0x36a0
establishes the five-word output: attributes, option, initial bits, current
bits, waiter count. Native handlers use this ABI with the existing native
event IDs. Waiters remain counted until the cooperative wait resumes.
Own startup capture `startup-toc-completion-state.ram.json` showed the
original encoded-handle validator rejecting native ID4 with -409.
Targeted searches of the supplied non-rendering lists found no event-status
lead; no external implementation was used.

## CDVD successful-completion result

Own native captures `startup-toc-write-trace.log` and decoding of the original
CDVDMAN establish a descriptor byte written to0x1f402006 at0xb84a4, followed
by GetToc and the IRQ reading that byte as an error. The runtime previously
retained it even after validated successful DMA completion. Completion now
publishes error0 before raising the IRQ, for both TOC and DVD reads. This is
the native success-result contract; exact hardware reset timing and the
write-side semantics of0x1f402006 remain unverified. The ps2tek register table
labels the error port read-only, insufficient to justify its existing write
alias: <https://psi-rockin.github.io/ps2tek/#cdvdioports>. No emulator code was
used; search results containing emulator sources were not used as evidence.

## Local input

- The user supplied a separate `HG/config` set containing Ghidra-derived
  function ranges and ps2recomp-oriented configuration as low-confidence
  reference material. It is not an implementation or discovery dependency:
  no translated code, rendering behavior, runtime binding, function boundary,
  or patch is imported from it. At most, it may be consulted after this native
  diagnostic has already observed an exact target, followed by independent
  verification against the original ELF bytes. The files remain outside this
  repository.
- `Haunting Ground (USA)/SYSTEM.CNF` identifies `SLUS_210.75` as BOOT2, VER 1.01,
  NTSC. Treat these as disc metadata, not independently verified release history.
- Executable SHA-256:
  `3b374d53a499d2c17b205274ee9eb34280768f294f970ebf6ae6731f6a2dacb8`.
- All instruction observations so far come from that executable's bytes through
  this project's own ELF reader. PCSX2 memory observations validate selected ABI results; see docs/ORACLE.md.
  No PCSX2 implementation was used.

## Hardware and host specifications consulted

- Sony **SPU2 Overview Manual, version 6.0**, archived at
  <https://github.com/ninjadynamics/PS2Docs/blob/main/SPU2_Overview_Manual.pdf>,
  p12 and p55: the host can transfer data to SPU2 local memory through DMA;
  TSAH/TSAL select its source/destination address, the low three TSA bits must
  be zero, and changing TSA during a transfer makes the result indeterminate.
  The native endpoint implements only the exact bounded IOP-to-SPU descriptor
  reached by the supplied game and retains bytes without claiming synthesis.
- SPU2 AutoDMA boundary (2026-09-08): the same Sony manual, pp10 and 28,
  defines 48 kHz processing and per-channel 512-short-word sound-input areas,
  each split into 256-short-word halves. Page 55's TSA transfer list excludes
  AutoDMA write; it must not be implemented as an ordinary copy to TSA.
  Independently decoded supplied LIBSD at relocated `0x0003208c..0x000320c8`
  clears TSA and writes `1 << core` to `0xbf9001b0 + core*0x400` before DMA.
  Its interrupt path at `0x00031dcc..0x00031df0` reloads BCR/MADR/CHCR, then
  calls the registered CRI callback at `0x00031e4c`. Native external trace
  `%TEMP%/haunting-toc-probe/resume-worker.ram.json` records that callback
  returning immediately before CRI worker 19 consumes semaphore 7 again.
  A nonzero per-core AutoDMA control now faults before copying or completing
  DMA. Sound-input pacing, buffer routing, and consumption remain unimplemented.
  Search results included SDK implementation and emulator-related snippets;
  none were used. This boundary derives from the supplied module and Sony manual.
- Paced input implementation (2026-09-08): independent decoding of supplied
  CRI_ADXI `0x00014744..0x000147d4` proves 512-byte blocks assigned alternately
  to the two channel queues; `0x00014850..0x00014864` returns the queue selected
  by channel index. The new input transport maps alternating channel blocks to
  Sony's documented MEMIN(L/R) areas (short-word addresses `0x2000/0x2200` and
  `0x2400/0x2600`). Channel-index 0=L and 1=R is an inference from this layout,
  pending a nonzero original-hardware observation. No SDK implementation is used.
  The endpoint consumes one sample pair per 1/48000 second, makes a consumed
  256-sample half available, and fills it from a bounded pending DMA. Completion
  follows transfer of every requested byte, not an arbitrary timeout.
  Initially priming both empty halves and starting phase zero at the first fill
  is an explicit native startup policy, not a measured hardware phase. Raw input
  is exposed to a callback and bounded, explicitly truncated diagnostic history;
  this is not mixed audio output. Underrun, unsupported control values, malformed
  descriptors, and active-register edits fault. Sony pp13-14 establish short-word
  addressing; pp10/28 establish the clock and buffer shape.
- Sony **EE Core User's Manual, version 6.0**, archived at
  <https://github.com/ninjadynamics/PS2Docs/blob/main/EE_Core_Users_Manual.pdf>,
  pp156,158–159,162–165: floating-point formats, FCR31 masks, signed zero,
  overflow saturation, underflow to signed zero, and round-toward-zero without
  guard/round/sticky bits. Add, multiply, MADD and MSUB use integer mantissa
  arithmetic so discarded bits are chopped without host floating-point.
- Sony **EE Core Instruction Set Manual, version 6.0**, MADD.S pp359–360 and
  MUL.S p372, supplies the accumulator/product operation shapes used by that
  independent integer implementation.
- DIV.S continuation: original instruction manual p357 and core manual
  pp156,158–165 specify signed-zero operands, finite exponent255, saturation,
  exception flags and truncation. `Fpu::divide` independently computes the
  normalized integer quotient and discards the remainder; it uses no host FP
  or emulator implementation. Synthetic tests cover sign, both normalization
  branches, 1/3 versus round-to-nearest, zeros, exponent255, range limits and
  sticky flags. The core manual warns that hardware can differ from IEEE
  truncation in the last bit; no divider microalgorithm or oracle comparison
  establishes those deviations yet. This is a specification-based implementation,
  not a claim of hardware-bit-exact division. Manuals were read from external
  `hg-ee-core-instruction-manual.pdf` and `hg-ee-core-users-manual.pdf`; core
  text also checked at <https://docs.alexrp.com/mips/ee.pdf>.
- The EE Core User's Manual DMAC chapters (pp62 and 71–72) define the global
  hold state and `D_ENABLER`/`D_ENABLEW`; active channel registers remain
  editable while held and resume only after the documented release.
- The same manual's IPU register/command chapters (pp175–176, 182–188,
  195–196, 199) define reset, BCLR, SETIQ, SETVQ and SETTH. The runtime retains
  only their input FIFO, tables, thresholds and command completion; MPEG decode,
  output and rendering are still explicit unsupported boundaries.
- EE instruction manual p121: SYNC ordering; p287: SQ masks low four address bits;
  pp344/354: ADDA.S and CTC1. SYNC has a host ordering fence because implemented
  guest operations are synchronous; asynchronous devices will require more work.
- NIST FIPS 180-4: <https://csrc.nist.gov/pubs/fips/180-4/upd1/final>.
  Independent SHA-256 implementation checked with empty, abc and million-a vectors.
- PS2SDK public ABI declarations and call-number definitions (not implementations):
  <https://ps2dev.github.io/ps2sdk/kernel_8h.html> and
  <https://github.com/ps2dev/ps2sdk/blob/master/ee/kernel/include/syscallnr.h>.
  These identify 0x3c/0x3d as SetupThread/SetupHeap and their parameter interfaces.
- PS2SDK's public EE DECI2 declaration identifies operation `0x10` as `kputs`
  and its first argument as a string pointer:
  <https://raw.githubusercontent.com/ps2dev/ps2sdk/master/ee/kernel/include/deci2.h>.
  The runtime uses only that public ABI shape for a bounded diagnostic sink;
  its return value is an explicit native compatibility policy pending an
  original-hardware observation, and no emulator implementation is used.
- PS2SDK's public EE kernel declarations and thread-status constants identify
  the `SuspendThread` interface and dormant thread state:
  <https://raw.githubusercontent.com/ps2dev/ps2sdk/master/ee/kernel/include/kernel.h>.
  The supplied game probes a dormant thread during startup; returning `-1`
  without mutating it is documented as a native compatibility choice rather
  than claimed as hardware-verified behavior.
- PS2SDK's public IOP INTRMAN ABI declarations at
  <https://ps2dev.github.io/ps2sdk/intrman_8h.html> identify ordinal 8 as
  `CpuDisableIntr()`: it changes CPU interrupt delivery, not individual cause
  masks, and returns a status. The native adapter derives only that contract;
  it does not reuse any INTRMAN implementation.

- Sony **EE Core Instruction Set Manual, version 6.0**, archived at
  <https://github.com/ninjadynamics/PS2Docs/blob/main/EE_Core_Instruction_Set_Manual.pdf>.
  Pages 146–147 (MFHI1/MFLO1), 172 (PADDUW), 284 (PSUBW), 285 (PXOR), 152–153 (MTSAB/MTSAH) distinguish packed
  saturating addition from XOR and define shift-count behavior. The downloaded
  reference is outside the repository in the OS temporary directory. No emulator
  implementation or the archive's unrelated implementation handbooks were used.
- Sony **VU User's Manual, version 6.0**, pp285–286, independently consulted at
  <https://studylib.net/doc/25815876/vuusersmanual.158394566>. This specifies
  macro-mode VMOVE's component mask and VMR32's y/z/w/x source-field mapping.
  The implementation is a direct raw-bit field transfer; it uses no emulator or
  host floating-point behavior.
- The same manual, pp240 and 66, defines macro-mode `VABS` as its upper-pipeline
  ABS counterpart over selected VF x/y/z/w fields. The AOT implementation
  clears only the selected IEEE-754 sign bits as a raw-bit operation, preserving
  unselected fields and NaN payloads without using host floating-point behavior.
- The VU microinstruction layout and lower-pipeline branch encodings were
  consulted from `vu-instruction-manual.pdf` in the Sony-manual archive at
  <https://github.com/ninjadynamics/PS2Docs/blob/main/vu-instruction-manual.pdf>.
  The build-time pair decoder retains only the documented 64-bit pairing,
  immediate/end flags, and lower control-flow fields; it does not execute VU
  instructions or use emulator code.
- Sony **VU User's Manual, version 6.0**, §3.4.8 (p50) specifies a one-pair
  branch delay slot and defines branch targets relative to that slot. The VU
  structural report records that slot and flags it when a supplied capture ends
  before it, instead of treating a partial capture as a valid CFG.
- Sony **EE Core Instruction Set Manual, version 6.0**, BREAK instruction
  entry: BREAK is a synchronous guest exception. The AOT emitter preserves it
  as an explicit fault with the encoded trap code; it is never treated as an
  unsupported opcode or a no-op. The analyzer knows a branch slot containing
  BREAK cannot reach the branch target (except the documented annulled
  not-taken path of a branch-likely instruction).
- GLFW official getting-started documentation:
  <https://www.glfw.org/docs/latest/quick.html> (context/window lifecycle).
- Khronos OpenGL reference pages:
  <https://registry.khronos.org/OpenGL-Refpages/> (rendering API reference).
- PS2TEK GIF reference, sections "GIFtags" and "GIF Data Formats":
  <https://psi-rockin.github.io/ps2tek/index.html>. This documents GIFtag bit
  fields, little-endian descriptor order, PACKED quadword payloads, REGLIST
  doubleword payloads and odd-entry padding. The initial decoder only preserves
  those structural transfers and explicitly rejects IMAGE until a checked GS
  HWREG/VRAM endpoint exists.
- Sony **GS User's Manual, version 6.0**, pp79,162–169, archived at
  <https://github.com/ninjadynamics/PS2Docs/blob/main/GS_Users_Manual.pdf>:
  GS local memory is 4 MiB; PSMCT32 has 64x32-pixel pages, 8x8-pixel blocks,
  8x2-pixel columns, and the illustrated page/block/column word arrangement.
  The initial image-transfer endpoint independently implements only that
  documented PSMCT32, PSMCT24, PSMCT16, PSMT8, and PSMT4 layouts and p79's
  transfer packing. The corresponding PSMZ32/24/16/16S layouts now accept the
  same documented 32/24/16-bit host packing through their distinct Z swizzles;
  unsupported formats continue to fault.
- GS User's Manual pp74–77 and pp132–134 defines BITBLTBUF source/destination
  fields, TRXPOS's four traversal directions, TRXREG dimensions, and synchronous
  local-to-local activation through TRXDIR.XDIR=2. Same-depth copies now convert
  between every independently implemented CT/Z/indexed layout in the selected
  traversal order so overlapping rectangles retain GS ordering, validate the
  documented width and destination-X restrictions, and treat XDIR=3 as transfer
  deactivation. Local-to-host FIFO reads stay an explicit boundary.
- GS User's Manual p76 requires transfer widths of 2/8/4/8 pixels for
  32/24/16/indexed storage respectively, with PSMT8/8H destination X aligned to
  two pixels and PSMT4/4HL/4HH to four. Both host-to-local and local-to-local
  setup validate those constraints and BITBLTBUF's 1..32 width units before
  modifying local memory.
- EE User's Manual pp26 and 149 maps GS privileged registers into
  `0x12000000..0x13ffffff`, requires LD/SD, and defines address bits 12 and 9:4
  as the mirrored GS register selector. GS User's Manual pp94–95, 122, 144–146,
  154, and 156 defines SIGNAL/FINISH/LABEL events and masked IDs, CSR event
  clearing/FIFO state, IMR masks, BUSDIR, and SIGLBLID. The native bridge models
  that checked subset and latches an unmasked GS event into EE INTC cause 0;
  CSR reset/flush and the second-SIGNAL drawing stall remain explicit.
- GS User's Manual pp85–88 and pp147–150 defines the PCRTC rectangular read
  circuits: DISPFB selects swizzled buffer base/width/format and DBX/DBY, while
  DISPLAY supplies screen placement, magnification, and output dimensions.
  The native single-circuit extractor reverses the magnification to a bounded
  source rectangle and expands PSMCT32/24/16/16S alpha as documented. The
  two-circuit PMODE merge and PS-GPU24 are not inferred.
- GS User's Manual pp125–126 defines TEX0's base, buffer-width, PSM,
  power-of-two dimensions, CLUT base/storage format/mode, and the CSM2
  `CSA=0` constraint; p130 defines TEXCLUT's CBW/COU/COV units. The
  renderer-facing extraction uses these with pp30–31's CSM1 IDTEX8/IDTEX4
  palette arrangements and p29/p72's direct texel and RGBA16 rules for checked
  PSMCT32, PSMCT24, PSMCT16, plus indexed textures through CSM1 and CSM2
  16-bit (CSA=0) CLUTs. The load/cache behavior is specified separately below.
- GS User's Manual **pp55–57, 125–126 and 128** (sections 3.4.7 and the
  TEX0/TEX2 register reference), also available as a transcription at
  <https://www.scribd.com/document/784545197/GS-Users-Manual>, defines the shared
  1 KiB CLUT temporary buffer. CSA selects the destination entry `16*CSA`,
  not a source VRAM offset. Source CSM1 palettes begin at CBP's upper-left:
  pp30–31 give the IDTEX4 8x2 and IDTEX8 16x16 arrangements. The p56 layout
  pairs halfword entries n and n+256 for PSMCT32; PSMCT16/16S use individual
  halfwords. TEX0/TEX2 access performs CLD 1–5 loads, preserves other banks,
  and tracks independent CBP0/CBP1 comparison registers. CLD 0 retains cached
  entries even if VRAM/CBP later changes. TEXA expansion remains at sample time.
  The native implementation flushes pending draws before reading or replacing
  a palette, so old draws keep their old cache and may supply the new VRAM
  palette. Synthetic tests cover bank 9/15/31, all four-bit index layouts,
  cross-context TEX2 reuse, 16/32-bit plane sharing, eight-to-four-bit reuse,
  both conditional registers and queued-draw ordering. Uninitialized cache
  reads/comparison registers, reserved CLD, CSM2 conditional loads, and loads
  crossing the bounded cache/source row remain explicit boundaries; no reset
  contents or wraparound behavior is guessed. Existing CSM2 16S handling is
  retained; no additional pixel formats are inferred.
- GS User's Manual p128 defines TEX2_1/2 as context-specific subsets of
  TEX0 containing PSM, CBP, CPSM, CSM, CSA, and CLD. A TEX2 write now merges
  exactly those fields into its corresponding TEX0 state while preserving
  TBP0, TBW, TW, TH, TCC, and TFX; texture extraction therefore observes later
  TEX2 format/CLUT changes instead of silently using stale TEX0 fields.
- GS User's Manual pp117–118 defines PRMODE's attribute bits and
  PRMODECONT.AC: PRIM always supplies primitive type, while AC selects PRIM or
  PRMODE for IIP/TME/FGE/ABE/AA1/FST/CTXT/FIX. Draw records now resolve that
  selection at the drawing kick and use the selected context's XYOFFSET.
- GS User's Manual pp115–116 describes the drawing environment registers and
  permits environment changes between drawing kicks (including within connected
  primitives). Each queued draw therefore captures the complete register file at
  its kick; deferred texture, fog, tests, blending, frame, and depth work cannot
  observe later register writes. Because drawing and transfers share local
  memory, a valid host-to-local TRXDIR commits earlier queued draws before IMAGE
  payload writes, preserving their command order for frame/depth/texture/CLUT
  aliases.
- GS User's Manual pp37–38 fixes Point and Sprite shading to flat and
  antialiasing to off, independent of their attribute bits; p43 says flat color
  is the value set immediately before the drawing kick. Sprite rasterization
  consequently ignores IIP/AA1 and uses the second (drawing-kick) vertex color.
- GS User's Manual pp54–55, section 3.4.6, defines RGB5-to-RGB8 expansion and
  TEXA's TA0/TA1/AEM alpha conversion for RGB24 and RGBA16 (including RGBA16
  values fetched through a CLUT).  TEX0 extraction applies those rules before
  exposing portable host RGBA pixels.
- GS User's Manual pp52–54 and p102 defines point-sampling coordinates and
  CLAMP's REPEAT, CLAMP, REGION_CLAMP, and REGION_REPEAT fields. The portable
  point sampler applies those modes before indexing the checked TEX0 image.
- GS User's Manual p135 defines UV as unsigned 10.4 texel coordinates and
  specifies that PRIM.FST selects them. Presentation records preserve both
  fixed UV values and the FST selection for backend texture submission.
  For the perspective path, p134's ST floating-point pair and RGBAQ.Q are now
  retained together on every kicked vertex; perspective division itself stays
  explicit until its interpolation/rounding behavior is implemented.
- GS User's Manual p59 defines TEX0 TFX/TCC texture-function modes and its
  0x80-unity `A*B=(A x B)>>7` multiplication. The portable texture-function
  helper implements MODULATE, DECAL, HIGHLIGHT, and HIGHLIGHT2 before host
  submission is introduced.
- GS User's Manual pp69–70 and p100 define ALPHA's `(A-B)*C>>7+D` RGB
  calculation, its A/B/C/D selectors, and later frame-buffer clamping. The
  portable blend stage preserves unclamped RGB results for the later writer.
  Page 115 defines PABE's source-alpha-MSB per-pixel blend gate, applied by
  the common frame path before selecting the blend calculation.
- GS User's Manual pp66–68 and p124 define TEST's alpha, destination-alpha,
  and depth gates. Portable predicates reproduce those comparisons before
  blending/frame-buffer work; prohibited ZTE=0 rejects explicitly.
- GS User's Manual pp70–72 and p110 define PSMCT32 frame-buffer writing,
  PSMCT24's RGB-only packing, COLCLAMP, FBA, and FBMSK. The portable PSMCT32
  and PSMCT24 writers apply those stages to GS local memory; RGB24 preserves
  its unused stored byte and uses its documented 0x80 destination alpha.
  PSMCT16 additionally has a checked non-dithered/unmasked RGB5A1 path with
  documented alpha-bit correction and 0x80/zero destination-alpha expansion.
  Pages 70 and 104–105 specify DIMX's signed offsets and DTHE's coordinate
  selection; the portable writer applies them before clamp/RGB5 conversion.
  Page 72 maps PSMCT16 FBMSK's pre-conversion R[7:3], G[15:11], B[23:19], and
  A[31] bits to RGB5A1; the writer follows that map.
- GS User's Manual pp165-166 specifies PSMCT16S's separate 8×4 block table
  with the same 16×2 column/lane arrangement. The frame writer, destination
  alpha, normal depth draw, and TEX0 direct-color extraction each use that
  layout rather than aliasing PSMCT16.
- GS User's Manual pp29 and 164 specifies PSMT8H, PSMT4HL, and PSMT4HH as
  CT32-addressed indexed texels in bits 31:24, 27:24, and 31:28,
  respectively. TEX0 extraction routes those lanes through the existing
  checked CSM1/CSM2 CLUT conversion. Page 79's unpadded IDTEX8/IDTEX4 IMAGE
  payload packing is used for host-to-local uploads: IDTEX8 bytes write the
  high byte and IDTEX4's low-first nibbles write the indicated high nibble
  lane, preserving all other CT32 bits.
- GS User's Manual p125 permits PSMCT16S as TEX0.CPSM. Both CSM1 and CSM2
  indexed conversion paths select PSMCT16S's documented storage layout when
  CPSM is 10, retaining the existing palette-index rules.
- GS User's Manual pp141 and 162–165 defines ZBUF, ZMSK, and the distinct
  PSMZ32/PSMZ24 page/block arrangement. The portable Z32/Z24 paths use that
  layout with the frame buffer's documented width; Z24 preserves the unused
  upper local-memory byte. PSMZ16 uses p165's separate block order with the
  documented 16-bit column lanes; PSMZ16S uses its separate p165 block table.
- GS User's Manual pp24, 44, and 121 defines pixel centers, the point
  closest-pixel rule, and inclusive SCISSOR bounds. The PSMCT32/PSMZ32 point
  rasterizer follows that path. Textured points support either FST/UV or the
  single vertex's normalized S/Q and T/Q coordinates; the STQ conversion uses
  exact IEEE-single mantissa/exponent ratios rather than host floating-point.
- GS User's Manual pp36 and 44 defines independent Line and connected
  LineStrip primitives, their per-pixel diamond coverage, and the included
  start/excluded endpoint rule. The portable line path implements that exact
  coverage geometry with rational interval comparisons rather than host
  floating point. Lines use their dominant-axis DDA position for Gouraud
  RGBA, depth, fog, and FST UV while diamond traversal independently selects
  covered pixels; horizontal, vertical, and diagonal cases share the checked
  pixel/texture pipeline. FST-clear lines use signed dominant-axis weights to
  form exact weighted S, T, and Q sums, including diamond samples outside the
  endpoint projection; the common interpolation denominator cancels in the
  perspective division. Antialiasing remains explicit.
- GS User's Manual Supplement v6.0 pp17–18, archived at
  <https://github.com/ninjadynamics/PS2Docs/blob/main/GS_Users_Manual_Supplement.pdf>,
  specifies AA1 Line/LineStrip as a one-pixel widening perpendicular to the
  dominant axis, with coverage converted from 0..1 to alpha 0..0x80, forced
  antialias blending, no Z-buffer write, and no special shared-endpoint handling
  for strips. The current non-AA rational diamond path does not approximate
  these effects; AA1 remains an explicit fault until its widened coverage and
  blend rules are implemented together.
- GS User's Manual p120 defines SCANMSK as a primitive-only framebuffer-row
  parity mask. The common primitive pixel path rejects its prohibited even or
  odd rows before pixel tests or framebuffer/depth writes.
- GS User's Manual pp66-68 defines pixel-test order and alpha-test failure
  controls. Primitive pixels now carry failed alpha tests through destination-
  alpha and depth tests, then apply KEEP, FB_ONLY, ZB_ONLY, or RGBA32-only
  RGB_ONLY at output selection; non-RGBA32 RGB_ONLY follows FB_ONLY.
- GS User's Manual p64 defines fogging as independent unsigned `>>8` blends
  between texture-function RGB and FOGCOL, retaining source alpha. The checked
  primitive paths apply it before pixel tests, with line and triangle fog
  interpolated by their existing coverage weights.
- GS User's Manual p46 defines sprites as rectangles with top/left-inclusive
  and bottom/right-exclusive coverage. The portable sprite path implements the
  flat subset with that coverage rule. Per p43, its second vertex supplies Z
  (the first is ignored). For FST sprites, p135's unsigned 10.4 UV endpoints
  are interpolated independently over the two rectangle axes before the checked
  TEX0 point sampler. FST-clear sprites interpolate S, T, and Q on those same
  axes, then resolve each normalized coordinate from exact weighted integer
  ratios; this avoids host floating-point and preserves negative-floor behavior.
  Varying Gouraud color remains explicit.
- GS User's Manual pp44–46 defines triangle DDA coverage: pixel centers on top
  or left edges draw, while bottom/right-edge centers do not. The portable
  triangle path normalizes winding before applying that rule. FST triangles barycentrically
  interpolate p135's unsigned 10.4 UV coordinates at each covered pixel before
  using the checked TEX0 sampler; winding normalization swaps vertex attributes
  with geometry. The same integer edge weights interpolate p43's Gouraud RGBA
  and p64's fog value, keeping every varying attribute on one coverage sample.
  The same weights interpolate unsigned 32-bit Z exactly in 64-bit arithmetic;
  the 16-bit coordinate domain bounds the weighted sum below `2^64`. Page 43
  specifies flat color as the RGBAQ value immediately before the drawing kick;
  FST-clear triangles use the same barycentric weights to form exact weighted
  S, T, and Q sums, then divide those sums for perspective-correct texels.
- GS User's Manual p136 defines context-specific XYOFFSET's 12.4 OFX/OFY
  fields as the primitive-to-window coordinate conversion offset. Completed
  draw records retain the active context's raw XYOFFSET, and presentation
  subtracts it in signed 12.4 units before handing coordinates to the host.
- Sony **EE User's Manual, version 6.0**, pp86–87 and 121–122, archived at
  <https://github.com/ninjadynamics/PS2Docs/blob/main/EE_Users_Manual.pdf>:
  VIF packets are 32-bit VIFcodes in DMA data; VIF1 DIRECT/DIRECTHL forward
  the following `IMMEDIATE` 128-bit units to GIF PATH2 (`IMMEDIATE=0` means
  65536). The bounded VIF1 transport implements only those commands plus the
  listed state-only VIFcodes. The bounded VIF1 path also transports MPG's
  documented 64-bit instruction units into checked VU1 MicroMem without
  executing them; activation remains explicit.
- The same manual, pp89–96 and 123, specifies UNPACK's V4-32 payload length,
  VU1 vector destination, and the separate cycle/mask/TOPS behaviors. The
  bounded VIF path writes S-32/S-16/S-8 and V4-32/V4-16/V4-8/V4-5 vectors with
  documented scalar replication, signed/unsigned, or RGBA5 expansion to checked
  VU1 data memory only. V2/V3 are accepted only when every documented
  indeterminate component is explicitly masked, filled, or preserved; otherwise
  they and VU execution remain explicit.
- Page 97 specifies normal, offset, and difference additive decompression.
  VIF input-selected lanes add Row fields in offset/difference mode and update
  Row only for difference mode; Row/Col/preserved masked lanes do not consume
  input or apply addition.
- Page 99 specifies that OFFSET initializes VIF1_TOPS from BASE and that
  FLG-relative UNPACK destinations add TOPS. The VIF path models that initial
  destination state; MSCAL-driven double-buffer switching remains explicit.
- The same manual, pp118–119, specifies STROW/STCOL as four following words
  stored in the VIF Row/Col registers. The VIF path preserves those split DMA
  payloads for the later documented masked/fill UNPACK behavior.
- Page 117 specifies STMASK as one following word. The VIF path retains its
  pattern separately. For V4 UNPACK, the VIF path applies its documented
  input/Row/Col/preserve selectors, including `CL < WL` fill cycles that
  consume only their documented input vectors; selector-0 fill slots fault
  because no input exists to supply them.
- Pages 88 and 120 specify MPG's 64-bit payload alignment and `NUM` following 64-bit microinstructions
  (`NUM=0` means 256) and its destination in eight-byte units. The VIF1 path
  preserves those words in bounded VU1 MicroMem across split DMA input; it does
  not infer or invoke VU execution.
- The same EE User's Manual, pp150–160, specifies GIF packet alignment,
  descriptor packing, packed XYZ ADC routing, and the distinct PACKED/REGLIST
  rules. In particular PRE applies to PACKED only, and REGLIST A+D is a NOP;
  these are covered by synthetic GIF transport tests.
- EE User's Manual p153–154 additionally specifies PACKED ST's internal Q and
  RGBAQ consumption: Q initializes to 1.0 whenever a GIFtag is read, ST updates
  it for later packed RGBAQ records in that tag. The GS transport stores this as
  explicit GIF state rather than reusing incidental host register contents.
- Sony **GS User's Manual, version 6.0**, pp38–42, 116, and 137–139: PRIM
  initializes the vertex queue; XYZ2/XYZF2 perform a vertex and drawing kick,
  while XYZ3/XYZF3 only advance the queue. Primitive topology and raw vertex
  ordering are retained in a bounded host-side queue; no rendering semantics
  beyond those documented queue events are claimed.
- GLFW 3.4 source dependency (window/context library, not PS2 implementation):
  <https://github.com/glfw/glfw/tree/7b6aead9fb88b3623e3b3725ebb42670cbe4c579>.
  Its upstream license stays with the dependency in the ignored build tree.

## Oracle observations and further work

For each needed PCSX2 memory observation record the executable hash, emulator
version, game state/reproduction steps, EE address range, capture hash, and the
hypothesis being tested. Keep captures outside version control. Summarize findings
in your own words and implement them independently; do not transcribe emulator
code, algorithms or implementation details. Compare against synthetic tests and
manuals, since emulator behavior alone is not proof of hardware semantics.

## Additional implemented specifications
EE Core Instruction Set Manual: DIV/DIVU pp51/53, MULT variants pp154-157,
MADD variants pp142-145, MFHI1/MFLO1 pp146-147, PMFHL.LW p228, PSUBB p270, PSUBW p284, PCPYH/LD/UD pp189-191,
SUB p114, PADDSB p162, PADDSH p164, PADDSW p166, PADDUB p168, PADDUH p170, PSUBH p271, PSUBSB p272, PSUBSH p274, PSUBSW p276, C.EQ.S p349, C.OLT.S p354, CVT.W.S p356, DIV.S p357, MADD.S p359, MADDA.S p360, MSUB.S p367, NEG.S p370, MUL.S p372, SQRT.S p376, SUB.S p377,
EI/DI pp314-315, ERET p316. COP0 register map/ErrorEPC: EE Core User's Manual
pp62/88. Operations with documented undefined results remain explicit validation
boundaries rather than fabricated guest exceptions or guessed arithmetic results.
EE User's Manual: timer configuration/counting pp34-37 and INTC pp28-31,
archived at https://github.com/ninjadynamics/PS2Docs/blob/main/EE_Users_Manual.pdf.
Native timer and interrupt delivery scheduling remains unfinished.
Public thread descriptor/status ABI: kernel.h declarations. Console ConfigParam
bit layout: https://github.com/ps2dev/ps2sdk/blob/master/ee/kernel/include/osd_config.h.
Native settings are constructed fields, not copied captures; native default UTC
differs from the original launch's observed timezone field 270.

DMA register/status and destination-chain semantics: EE User's Manual pp48,55-56,
61,65,73-79. SIF0 receives explicit qwords; other endpoints, cnts/stall control,
tag priority and automatic timing remain unsupported. Register fields whose reset
values are unspecified initialize to zero as native policy, not claimed hardware state.
SifSetDChain void ABI and channel setup: public EE kernel declarations at
https://ps2dev.github.io/ps2sdk/kernel_8h.html and
https://github.com/ps2dev/ps2sdk/blob/master/ee/kernel/include/kernel.h.
SIF register IDs/descriptor layout: public sifdma.h. Search also surfaced unrelated
implementation snippets; they were not used to derive the native receiver.

Read-only volume parser: ECMA-119 fourth edition (2019), §§7.2.3/7.3.3,
8.4, 9.1, directly from the standards organization:
https://ecma-international.org/wp-content/uploads/ECMA-119_4th_edition_june_2019.pdf.
Downloaded reference stays outside the repository at `%TEMP%/hg-ecma119.pdf`.
The CVM origin 0x1800 is independently established from the local file: the primary
descriptor is at 0x9800, logical block size 2048, and root extent 22 resolves to valid
directory records only with that origin. No CRI reader/decryption implementation used.

IOP import/export ABI declarations: public
https://raw.githubusercontent.com/ps2dev/ps2sdk/master/iop/kernel/include/irx.h.
The module inventory independently validates local ELF section and relocation bounds,
unlinked library table headers/stub encodings and export ordinals. Relocated zero
export pointers are retained. No IOP implementation or emulator code is used.
An unrelated emulator source result appeared during a header search and was not
used; subsequent inspection uses local module bytes and the public ABI above.

Build-time MIPS relocations: MIPS ABI supplement, chapter 4 pp4-18 through 4-20,
https://refspecs.linuxfoundation.org/elf/mipsabi.pdf. The local IRX subset uses
symbol index zero and types 2/4/5/6 only; every HI16 is immediately followed by LO16.
The independent relocator supports those paired records and explicitly rejects
other symbol/relocation forms. No IOP executable image is claimed from the dry-run.

MIPS-I processor-family timing reference: IDT R3051/52 hardware manual (1992),
chapter 2 p2-9 describes one-instruction load and branch delays:
https://www.bitsavers.org/components/idt/risc/1992_IDTR3051_R3052_RISController_Hardware_Users_Manual_Rev1.3_19920821.pdf.
The distinct IOP runtime implements dependency-free delayed loads and branches;
load-use/write hazards explicitly fault pending PS2-specific validation. No cache,
device or coprocessor behavior is inferred from this different chip's implementation.
The CW33000 manual was also consulted for the MIPS-I instruction overview; no device
implementation was imported. The small MODMSIN function uses dependency-free schedules.
The same IDT manual chapter2 p2-15 describes signed/unsigned multiply and divide
HI/LO results. Used only for architectural integer operations; timing and undefined
division results remain outside the implementation. Reference PDF is external TEMP.

SYSCLIB public ABI declarations and export ordinals only:
https://raw.githubusercontent.com/ps2dev/ps2sdk/master/iop/system/sysclib/include/sysclib.h.
The tested memcpy(12), memset(14), strlen(27) execute independently translated bytes
from the supplied SYSCLIB member; no SDK function implementations were used.
sprintf(19) also tested through its supplied prnt callback and relocated format table.
The callback offset0x16b0 is loaded by sprintf/vsprintf; thirteen prnt callsites load
it from stack+104. The character range check is <121 and indexes table offset0x18c0.
Those relationships were decoded directly from the supplied SYSCLIB member.

LOADCORE and INTRMAN public function signatures/ordinals were consulted at
https://raw.githubusercontent.com/ps2dev/ps2sdk/master/iop/system/loadcore/include/loadcore.h
and https://raw.githubusercontent.com/ps2dev/ps2sdk/master/iop/system/intrman/include/intrman.h.
The loadcore header includes an internal structure attributed to PCSX2; that structure
is excluded from use. Boot-mode behavior/layout was independently decoded from the
supplied LOADCORE: export12 offset0x6e0, export13 offset0x684, pointer initialization
at0xcc..0xd8 writes low-memory0x3f0/0x3f4. Tests run those original translated routines.
Original LOADCORE linker helper offset0x11d8 was decoded and executed directly:
it writes J target into each valid ordinal stub, preserves its ADDIU ordinal delay
word, and sets import flags. A diagnostic lowering only an import table's minor
version proves that it accepts a newer same-major export. Combined AOT planning
uses that observed compatibility relation, but still rejects newer imports,
different majors, and ambiguous compatible providers; guarded code checks the
exact planned words.
FlushIcache/FlushDcache adapters implement the public void ABI with native coherent
memory fences/epochs, not copied hardware cache loops.

IOP DMA register addresses, DMACMAN ordinals, and start/chain flags use public
declarations only:
https://raw.githubusercontent.com/ps2dev/ps2sdk/master/iop/system/dmacman/include/dmacman.h.
IOP MIPS-I load-delay behavior follows the R3000 User's Manual load scheduling
rules. The supplied CRI_ADXI bytes provide the concrete overwrite-in-delay-slot
case used by startup; a direct register write or a newer load to the same
destination cancels the older pending load rather than consuming its value:
https://usermanual.wiki/Document/r3000manual.723589236/pdf.
The public PS2SDK IOP register declaration identifies the channel-shaped DMAC2
register bank at `0x1f801560..0x1f801568`:
https://raw.githubusercontent.com/ps2dev/ps2sdk/master/common/include/iop_regs.h.
The supplied SIFMAN bytes independently establish the live completion-status
read of its MADR at `0x1f801560`; the runtime currently retains this otherwise
unused bank as explicit register state and does not attach a transfer endpoint.
DPCR/DPCR2's four-bit channel fields (priority bits 0..2 and enable bit 3),
including channels 11/12 at DPCR2 bits 16..23, are from
https://psi-rockin.github.io/ps2tek/index.html#iopdma. The native adapters update
only those configuration fields; they do not implement a DMA endpoint or completion.
SIF register/initialization flag ABI:
https://raw.githubusercontent.com/ps2dev/ps2sdk/master/iop/system/sifman/include/sifman.h.
IOP service error codes:
https://raw.githubusercontent.com/ps2dev/ps2sdk/master/iop/kernel/include/kerr.h.
Native INTRMAN adapters retain handlers, masks and nested interrupt state; delivery
is pending. No SDK implementation files were used. Hardware probe observations are
documented separately in ORACLE.md; they are not a full device emulation profile.
HEAPLIB public ABI declarations and import ordinals only:
https://raw.githubusercontent.com/ps2dev/ps2sdk/master/iop/system/heaplib/include/heaplib.h.
The bounded native heap uses checked guest-RAM addresses and independent metadata;
it does not use an SDK allocator implementation. INTRMAN callback setter signatures
and ordinals28/30 are from the public intrman.h header cited above.
SECRMAN memory-card callback setter signatures and ordinals 4/5 are from the
public ABI declaration at
https://raw.githubusercontent.com/ps2dev/ps2sdk/master/iop/security/secrman/include/secrman.h.
The native state retains or clears those checked pointers only. Card
authentication ordinal 6 is not implemented or reported successful.
IOP timer register banks, width, mode-write reset/mode-read acknowledgement and
clock source: https://psi-rockin.github.io/ps2tek/index.html#ioptimers. The native
implementation independently uses a fixed-point 36.864 MHz clock and checked
compare/overflow delivery; exact gated/external-clock behavior and all timer
mode combinations remain unsupported.
CDVD N-command parameter layout, DVD 2064-byte sector framing, I_STAT meanings
and one-sector DMA sequencing: https://psi-rockin.github.io/ps2tek/index.html#cdvdioports.
Only the documented ReadDvd (`0x08`) path is modeled. Local ISO bytes are read at
runtime through an explicit bounded sector request; no extracted game bytes enter
source control.

The IOP SIF0 source-chain contract is independently derived from original SIFMAN
descriptor construction plus the EE DMA tag definitions cited above. It accepts
the observed one-, two-, whole-qword, and six-word payload shapes; a non-final
EE CNT tag remains pending and is never promoted to a completion without its
own documented terminal transition.

The ROMDIR subset is independently decoded from the game-provided IOPRP300.IMG:
16-byte entries have a 10-byte name, 16-bit extended-info size and 32-bit member size;
members advance at 16-byte boundaries. Empty RESET precedes ROMDIR (320 bytes) and
EXTINFO (608 bytes). Metadata validates extents and extended-info totals; no BIOS
or emulator implementation is used, and no modules are extracted into the repository.

Connected SIF transport uses independently decoded SIFMAN tags and the EE User's
Manual source/destination-chain definitions. Original six-word reply padding was
validated as retaining the previous payload qword's upper two lanes; see the
external capture provenance in ORACLE.md. No captured payload bytes are embedded.
Original SIFCMD entry/export4/export14 and registered callbacks implement the
actual handshake. Native event/interrupt services use public ABI declarations:
https://raw.githubusercontent.com/ps2dev/ps2sdk/master/iop/system/threadman/include/thbase.h
https://raw.githubusercontent.com/ps2dev/ps2sdk/master/iop/system/threadman/include/thevent.h
https://raw.githubusercontent.com/ps2dev/ps2sdk/master/ee/kernel/include/kernel.h
Only header declarations were used, not SDK implementations.

CACHE DHWBIN opcode0x18 is from EE Core Instruction Set Manual p299; coherent
native RAM uses a store fence/epoch. The original RPC sender at0x26f6e8 uses a
0x20000000 RAM alias. Native compatibility explicitly permits that alias and
orders the same RAM stores; the hardware manual leaves uncached CACHE behavior
undefined, so this is a native policy rather than a claim of hardware accuracy.
Other uncached/accelerated/SPR cases remain unsupported. The bounded RAM aliases
are part of the native boot mapping, evidenced by original SIF buffer construction.

### Native IOP worker services, 2026-09-04 22:03
Thread/semaphore structures and export ordinals use public ps2sdk ABI headers
`iop/system/threadman/include/thbase.h` and `thsemap.h`; console signature from
`iop/system/sysmem/include/sysmem.h`. Implementations are independently written.
Offsets come from export tables in the supplied THREADMAN/SYSMEM/STDIO modules.
FILEIO worker pointers and heap service0x80000003 are from its original bytes.
Cooperative scheduling and the 1us/slice clock are explicit native compatibility
policy, not copied or inferred emulator timing behavior. Independent decoding of
the supplied THREADMAN `CreateThread` export establishes its SYSMEM ordinal-4,
mode-1 stack allocation and 256-byte size rounding. The native scheduler uses
that checked high-side system arena. The public `thbase.h` declaration identifies
ordinal 5 as `DeleteThread`; lifecycle semantics are independently implemented
and no SDK implementation file is used.

### IOP BIOS selector-32 context transfer, 2026-09-05

The user's external SCPH-39001 BIOS (SHA-256
`f4c948e61a291d4b3f92a141e550cf8357204287a31ff784cacbbedaef910c9d`)
was used only as an original hardware-software input; no BIOS bytes enter the
repository or generated source. A coherent external 2 MiB IOP RAM observation
(SHA-256 `0ff7a8e6da65772560219a4794a5c8b465f6f06746f6c91926cc4dc3d109da04`)
shows syscall-table slot `0x5a00` selecting handler `0x5830`. Independently
decoded instructions establish that this handler constructs a 0x98-byte frame,
saves GPRs, HI/LO, status and EPC+4, calls the registered new-context callback
through `0x59d0`, and restores the callback-selected context. The live callback
values were `0x0000c368` (new context) and `0x00012098` (preemption decision).

The supplied game THREADMAN bytes independently establish the only configured
selector-32 site: wrapper `+0x6640` chooses scheduler field `+0x5c` or `+0x60`
from boolean `a3`, publishes it at field `+0x54`, then invokes selector 32 at
`+0x665c`. The native adapter maps this exact, identity-checked boundary onto
the already-native cooperative thread contexts. It advances scheduling past
the current owner, releases masked ownership, and records the mode/site/count;
unknown syscalls continue to fault. This is an independent semantic adapter,
not emulator code and not a runtime interpreter.

### IOP unaligned words and CD/DVD register subset
LWL/LWR/SWL/SWR follow IDT R3051 manual Table2.4 (PDFp33). The MIPS-I
same-destination forwarding exception is explicit in NEC's archived MIPS IV
instruction-set errata, AppendixA-29:
https://www.bitsavers.org/components/nec/mips/MIPS_IV_Instruction_Set_Errata_199511.pdf
This rule permits unaligned-load merging of a preceding pending load; ordinary
register reads keep the existing delay hazard check. Synthetic tests cover all four
byte alignments with consecutive paired loads and guarded paired stores.
The same MIPS-I instruction reference defines BREAK as an exception-producing
terminator with an encoded code field. IOP AOT output preserves that code in an
explicit guest fault and does not discover or execute a fallthrough successor.
CDVD register addresses/status/W1C and ReadRTC byte layout are cross-checked with
https://psi-rockin.github.io/ps2tek/index.html#cdvdioports and the original CDVDMAN
poll/read/write sequence. Only the register protocol is used: emulator-specific
version constants, timing formulas and reported implementation choices on that page
are excluded. Native RTC is a UTC startup snapshot; empty result reads preserve a
zero-initialized last-result latch as explicit native policy. No disc command is
reported complete, and unknown S commands fault.
That hardware reference defines drive-status bits 1 (spindle spinning) and 3
(paused); the mounted local DVD compatibility profile therefore initializes
current and sticky status to `0x0a`. This is a documented physical profile state,
not a fabricated command completion.
The configured local input is classified as PS2 DVD for the read-only disk-type
register (`0x1f40200f` -> `0x14`); original CDVDMAN's decoded classifier accepts
that value. This is only mounted-media identity; the separately bounded ReadDvd
N-command, one-record DMA, and sector-read completion do not imply broader CDVD
support.
Public `sceCdGetToc` ABI documentation declares a 1024-byte destination buffer;
the independently decoded original CDVDMAN request emits N-command `0x09` with
one zero byte and arms the documented 2064-byte CDVD DMA shape. The TOC payload
itself is not inferred from any emulator implementation. The diagnostic accepts
it only as a user-supplied, exactly 2064-byte external `--toc-record` input at
run time; it is copied through the original DMA completion path and never added
to the repository. Without that artifact it remains an explicit boundary.
The original supplied CDVDMAN bytes also establish a request-frame signature:
its GetToc wrapper at module `+0x70b0` calls the internal implementation at
`+0x6f1c`, whose frame stores the `4 x 0x81` DMA descriptor at `sp+0x18` and
the wrapper return `module+0x70c4` at `sp+0x48`. The oracle helper uses both
facts only to locate a same-stage destination; it never treats a generic DVD
read descriptor or module-relative buffer guess as TOC provenance.
The same hardware reference's independent opcode tables identify COP2 `QMFC2`,
`QMTC2`, and the EE-specific `SQC2` encoding. The runtime models only their
documented 128-bit data movement and checked EE-memory store; it does not derive
any VU arithmetic, microprogram behavior, or data payload from emulator code.
Its COP0 register map identifies register 28 as TagLo; the EE Instruction Set
Manual's MFC0 definition supplies the sign-extended transfer semantics. Cache
tag operations themselves remain outside this register-preservation work.
The original CDVDMAN request path also writes its request error byte to
`0x1f402006` before programming DMA channel 3; the native boundary retains this
byte as a readable latch. Its error meanings and any asynchronous hardware effects
are deliberately not inferred.
The supplied CDVDMAN setup routine at module `+0x505c` passes module `+0x4288`
as the callback while registering interrupt cause 2, establishing that callback
as a static root without dynamic target guessing. Its completion path reads only
the low nibble of byte register `0x1f402013`. That register is not named by the
available hardware table, so the native synchronous endpoint exposes only an
explicit zero idle/decoder state after it has already committed the bounded DMA;
other widths and writes remain faults. This is a native completion policy derived
from the original consumer branch, not a claim about undocumented hardware bits.
The supplied `.iso` is a separate ECMA-119 volume at origin zero (its `SLUS_210.75;1`
entry hashes to the configured executable digest); `DATA.CVM` retains its distinct
`0x1800` volume origin. CDVDMAN's channel-3 CHCR `0x41000200` setup is retained as
an active, guarded DMA arm prior to a CDVD command. The checked media endpoint
handles only an exact one-record ReadDvd transfer and its IRQ completion.

Controller version query03:00 uses an explicit native compatibility profile1.0.0
(status0, version bytes1/0/0), not an emulator's device identity. The supplied
CDVDMAN decodes the three version bytes as big-endian and selects older feature
paths below0x010700 (own routine0xb54d4). Unknown subcommands still fault; this
identity does not imply that all hardware functions of that revision are implemented.
The existing external RAM capture identifies a CDVDMAN export table at0x2e730
with entry0x22830 and exports47/50 at0x228d0/0x27548; no captured code is translated,
and no emulator-specific version bytes are embedded.

### SIO2 startup boundary (2026-09-05)
- PS2SDK's public DMACMAN header identifies ordinals 28/32 as `sceSetSliceDMA`
  and `sceStartDMA`; the runtime retains checked register setup and faults when
  a transfer lacks a peripheral endpoint.
- ps2tek's hardware table identifies `0x1f808200..0x1f808280`, including the
  SEND buffers, byte FIFOs, control register, and result registers. The native
  register bank implements reset/configuration state only; starting a command
  without a controller or card endpoint faults.
- PS2SDK's VBLANK header identifies ordinal 5 as `WaitVblankEnd`; the cooperative
  native wait uses the diagnostic clock. Its SECRMAN header identifies ordinal
  6 as `SecrAuthCard`; linking is permitted but invocation remains an explicit
  fault until a real memory-card/authentication endpoint exists.
- https://raw.githubusercontent.com/ps2dev/ps2sdk/master/iop/system/dmacman/include/dmacman.h
- https://psi-rockin.github.io/ps2tek/index.html
- https://raw.githubusercontent.com/ps2dev/ps2sdk/master/iop/system/vblank/include/vblank.h
- https://raw.githubusercontent.com/ps2dev/ps2sdk/master/iop/security/secrman/include/secrman.h

### Native reboot boundary (2026-09-04)
- PS2SDK ABI headers only: ee/kernel/include/syscallnr.h identifies SifStopDma0x6b;
  kernel.h describes its void SIF0-disable ABI. Native implementation supports idle
  packet boundaries only and leaves queued data/status intact.
- ee/kernel/include/sifdma.h names hardware IDs1..4; shared flags use direction-specific
  set/clear semantics. Returning the submitted value is an explicit native policy.
- iop/system/sifcmd/include/sifcmd.h identifies export10 as AddCmdHandler. Native
  reboot gateway is registered through the original export, receives actual DMA
  dispatch, and parses the layout derived from original EE routine0x275700.
  Original IOP0x98740 clears packet size before copying to callback storage.
- https://github.com/ps2dev/ps2sdk/blob/master/ee/kernel/include/syscallnr.h
- https://github.com/ps2dev/ps2sdk/blob/master/iop/system/sifcmd/include/sifcmd.h

### SPU2 voice-transfer completion inference (2026-09-08)
The user's original LIBSD, independently decoded by `hgtool.iop_build.prepare`,
selects voice-transfer mode at relocated `0x31a60..0x31a68`, polls bit 7 of
`0xbf900344 + core*0x400` at `0x31a80..0x31ab8`, clears ATTR bits 4:5 at
`0x31ac0..0x31adc`, and then dispatches its completion callback. The native
endpoint now reports this observed completion condition only after its checked
one-shot RAM-to-SPU copy has committed. A transfer-mode change retires the
condition; AutoDMA does not assert this voice-transfer condition. This is an
independent, bounded inference from the original consumer and actual completed
copy, not verified general STATX/DREQ semantics or a hardware timing claim.
Sony's SPU2 Overview v6 pp12/55 establishes local-memory DMA and TSA semantics
but does not document this status register. Broader status fields remain work.
Web searches for status documentation returned SDK implementation and PCSX2
archive snippets; those were not used as implementation sources. No emulator
code or algorithms were consulted to implement the endpoint.

### EE startup interrupt context (2026-09-08)
Original EE `0x26cc70` returns `(Status ^ 1) & 1`; StartThread's user wrapper
at `0x26d2f8..0x26d304` rejects a nonzero result. The native launch profile's
old `0x70010000` cleared IE, so all such calls were rejected before the syscall.
The native executable-launch profile now enables IE (`0x70010001`); it is not
represented as a hardware reset value. Independent hardware register reference
https://psi-rockin.github.io/ps2tek/index.html#eecop0exceptionhandling identifies
IE bit0, EIE bit16, EXL bit1 and ERL bit2. DMA dispatch now gates on both enables
and neither exception level, clears IE/EIE during the native callback, and
restores the saved context on return. Synthetic tests exercise the gates and
original consumer-compatible callback context. No emulator implementation used.

### EE C.LE.S startup clamp (2026-09-08)
Sony EE Core Instruction Set Manual v6 p351 specifies function bits `110110`
(0x36), zero fd field, exact less-or-equal comparison into FCR31 bit23 and
signed-zero equality. The original object method reached at `0x20e9a0` uses
this instruction at `0x20e9c4` to clamp its input against 1.0. The independent
EE decoder/emitter now supports `c.le.s`; the runtime preserves all other
control bits. Native FPU tests cover equality, signed zeros and either side
of positive/negative bounds. Existing `c.olt.s` is retained as the project's
older spelling for the manual's C.LT.S encoding. No host NaN semantics used.

### DS2O_S1 load and worker (2026-09-08)
The connected original LOADFILE path requests `MODULES/DS2O_S1.IRX` after sound
setup. Its existing configured digest and base are used without emulator data.
Original DS2O entry +0x3c/+0x40 constructs relocated worker +0x164 and stores it
at stack+40 before CreateThread (thbase ordinal4) at +0x54. That supplies the
worker's AOT root. PS2SDK's public vblank.h defines ordinal4 as WaitVblankStart:
https://raw.githubusercontent.com/ps2dev/ps2sdk/master/iop/system/vblank/include/vblank.h
The import can be linked, but invocation explicitly faults pending a connected
display-timing source. This is not implemented by aliasing the old diagnostic
WaitVblankEnd delay. Module plans are now shared between initial load and reboot.

### Corrected LOADCORE import-version comparison (2026-09-08)
The original supplied LOADCORE bytes, read with identity-checked iop_source and
independently relocated/decoded, show the full link chain: `0x96cac` scans import
tables, `0x96d14` invokes `0x97138`, and `0x9717c` calls `0x97038`. That helper
loads both version halfwords at +8, shifts right8 and subtracts; a nonzero major
difference rejects, while a zero difference proceeds to patch stubs at `0x9718c`.
No minor comparison occurs on this link path. The separate minor comparator
`0x96ff8` is called by export-registration/replacement code at `0x969c8`.
The earlier lower-import-minor experiment proved only one direction and the
planner's inferred newer-export-only constraint was too strict. Planning now
accepts unique same-major providers in either minor direction, still rejecting
major differences and ambiguity. Synthetic tests cover both minor directions.
This resolves the actual DS2O_S1 request for SIO2MAN 2.6 against the supplied
SIO2MAN 2.4 export table; all three requested ordinals are present.


### First native SIO2 digital poll (2026-09-08)
The preserved original request is FIFO `01 42 00 00 00`, SEND3[0]
`0x00140540`, SEND1[0] `0xffc00505`, SEND2[0] `0x00020014`, CTRL
`0x3bd`. External native capture: `%TEMP%/haunting-toc-probe/sio2-packet.ram.json`.
Hardware reverse-engineering reference (register/protocol descriptions only):
https://psi-rockin.github.io/ps2tek/index.html#sio2controllercommands
and its SIO2 Registers / IOP Interrupts sections establish digital poll header,
active-low button payload, connected RECV1 `0x1100`, and IRQ17. The bounded
native virtual controller accepts only this five-byte port-zero poll and exposes
mutable button bits, initially released. Analog/configuration, other ports,
DMA and memory cards remain unsupported. Completion is a cooperative native
policy after response publication, not cycle-accurate serial timing. Other
response/status registers retain the prior incomplete register-bank behavior.
Original identity-checked SIO2MAN +0x77c/+0x780 installs callback +0x584 for
IRQ17 at +0x784. Own AOT decoding of that callback shows interrupt-status
read/write then event flag `0x2000`, returning 1. The new IRQ dispatcher invokes
this original callback and restores interrupted CPU context. No emulator code
or algorithms used.


### Vblank wait before display configuration (2026-09-08)
The connected original DS2O worker calls vblank ordinal4 at `0x2de3c`,
return `0x2c1a0`, before any native SetGsCrt call: configured=0, interlace=0,
mode=0, frame=0. External evidence: `%TEMP%/haunting-toc-probe/display-mode.log`.
Public ABI names ordinal4 WaitVblankStart (vblank.h cited above). The native
wait now parks its calling worker and consumes only a subsequently published
start event, allowing independent EE startup to run. A display-event publisher
API wakes existing waiters once; elapsed instruction/time budgets do not wake
them. No display clock is invented before a mode exists. Actual display timing
production and EE/IOP vertical interrupt routing remain unimplemented.


### Second controller port (2026-09-08)
The original next poll differs only in SEND3 bit0: `0x00140541`, with
`01 42 00 00 00` input. Captured in external `dbcman-rpc.ram.json`.
The ps2tek SIO2 Registers description identifies SEND3 bits0:1 as port and
RECV1 `0x1d100` as disconnected (`0x1100` connected):
https://psi-rockin.github.io/ps2tek/index.html#sio2registers
Native launch policy now has two independently configurable ports, first
connected and second absent. For the bounded digital poll, an absent port
publishes five idle `0xff` bytes and disconnected status, then completion.
The idle-byte fill is an explicit native absent-device policy, not a claimed
hardware capture of all timeout behavior. Both connected ports use independent
active-low button words. Multitap port values2/3 and other commands still fault.


### User-shared HG configuration inspection (2026-09-08)
User authorized looking at four files under the separate `../HG/config/`
project while reiterating no copying from ps2recomp. Read the headers and
initial content of `game.toml`, `recomp.base.toml`, `functions.manual.toml`
and `functions.analysis.csv` for provenance. The base config explicitly
identifies PS2Recomp/ElfAnalyzer output; manual entries explicitly cite
PS2Recomp identification among mixed evidence. The CSV appears to be a
function-map export but its provenance is not established by its own header.
No maps, addresses, boundaries, patches, stubs, runtime bindings or behavioral
claims from these files were imported or used to guide implementation.
Continue independent discovery from the user's checked original executable.


### Reference-use clarification (2026-09-08)
The user clarified that looking at the shared HG material as a reference is
allowed, while copying it verbatim is not. It may inform context and ideas;
implementation still requires independent verification and independently written
code. The earlier provenance inspection imported nothing. This clarification
does not authorize using emulator implementation code.


### MCMAN driver initialization (2026-09-08)
Identity-checked original relocation gives driver descriptor `0x4700c`, whose
operations pointer at +16 is `0x46fa0`. The first operations entry points to
`0x44c10`, matching live IOMAN dispatch (return `0xa79b0`). Own decoding
shows `jr ra` with a delay-slot zero return. This original two-instruction
callback is now an AOT root at MCMAN +0xfc10; no host success stub replaces it.
The following operations table spans through `0x47008`; remaining callbacks
are not added until their role/use is established.


### Rendering restriction clarified (2026-09-08)
User explicitly said not to copy ANYTHING related to rendering from the shared
reference project. This includes rendering code, configuration, mappings,
patches and algorithms. Rendering remains independently derived from original
inputs and hardware specifications. No rendering material has been imported.


### MCSERV startup callbacks (2026-09-08)
Original MCSERV entry +0x10c/+0x110 constructs worker +0x280 and stores it
in the CreateThread descriptor at +0x120. That worker builds service ID
`0x80000400` at +0x2e4/+0x2e8 and handler +0x324 at +0x2ec/+0x2f0 before
SifRegisterRpc at +0x304. Both callbacks are now AOT roots derived with the
project's identity-checked original-module decoder. MCSERV is selected in the
bundle and shared initial/reboot module plan using its existing verified digest
and base `0x5d000`. No reference-project material used for these changes.


### VIF1 startup register access (2026-09-08)
Own original ELF decoding at `0x10bfb0..0x10bfd0` proves a word write of1 to
`0x10003c10`, followed by2 to `0x10003c20`. Hardware register descriptions:
https://psi-rockin.github.io/ps2tek/index.html#vifioregisters
identify these as VIF1 FBRST reset and ERR ME0 respectively. The independent
runtime resets its VIF interface/FIFO state, retaining separate VU memories,
and retains the supported ERR bit. Other stall/interrupt/error-mask operations
remain explicit gaps. Tests exercise cached/uncached MMIO routing, FIFO/phase
reset, preserved VU memory, mask readback and rejected widths/masks. No shared
HG/ps2recomp rendering material was used to derive or implement this change.


### CPU VIF1 quadword submission (2026-09-08)
The original EE stores a complete GPR quadword to `0x10005000` at `0x10c004`
and again at `0x10c00c`. The independent runtime's SQ path now sends both
64-bit halves in order into its existing VIF1 parser, including GS interrupt
synchronization after parsing. The public PS2SDK register header identifies this address as VIF1 FIFO:
https://github.com/ps2dev/ps2sdk/blob/master/common/include/ee_regs.h
Only register definitions were used; incidental emulator search results were
not consulted for implementation. Tests cover uncached alias routing, commands in both halves,
zero-register NOP submission, and unsupported scalar FIFO width rejection.
No reference-project rendering code, configuration or algorithms used.


### GIF CTRL reset (2026-09-08)
The original store at `0x10c014` writes1 to `0x10003000`. The hardware
register description identifies GIF_CTRL bit0 as GIF reset:
https://psi-rockin.github.io/ps2tek/index.html#gifio
The independent GIF endpoint currently retains only an incomplete packet
buffer; reset clears that transport/parser state without resetting the separate
GS register/VRAM object. Stop/restart operations remain explicit gaps. Tests
cover the uncached register alias and preserve state on unsupported width or
control writes. No reference-project rendering material used.


### GS CSR system reset (2026-09-08)
Sony GS User's Manual v6 pp145-146 identifies CSR bit9 as system reset and
bit8 as the distinct FLUSH operation. Page154 specifies that all five IMR
mask bits start at1 after reset. Read from the existing external manual
`%TEMP%/hg-gs-users-manual.pdf` (archived source cited earlier).
CSR reset now cancels the native GS draw/vertex/transfer state, restores its
initial register/display profile and masks events. It retains allocated local
memory contents. The consulted manual does not enumerate every drawing/display
register reset value or specify a VRAM erasure; defaults and retained VRAM are
explicit native initialization policy, not hardware-capture-verified values.
FLUSH remains an explicit unsupported operation. Tests cover cancellation,
masking, self-cleared reset indication, and retained local memory. No shared
reference-project rendering material was used.


### Controller full pressure reply (2026-09-09)
Native startup-controller-query.ram.json contains the original DS2O packet
01 4f 00 ff ff 03 00 00 00 with descriptor0x240940. Non-rendering reference
configuration searches for pressure/0x4f/SIO2/DS2O gave no applicable lead.
Derived from https://psx-spx.consoledev.net/controllersandmemorycards/
(Analog Buttons) and https://psi-rockin.github.io/ps2tek/ (SIO2 input format):
the full18-byte mask enables digital buttons, axes and twelve pressure bytes,
yielding mode79 and21 wire bytes. Other masks explicitly fault. Command44
resets pressure mode. Pressure values are retained input state, initially zero;
host input is unfinished. The config trailing zero follows psx-spx, whose
noted last-byte variability remains unverified on hardware. Protocol moved to
runtime/controller.cpp to avoid recompiling all IOP translations per edit.
No emulator or shared implementation used.


### Native IOP ExitThread (2026-09-09)
Original IOPRP300 THREADMAN export thbase:8 points at+0x1288; independently
parsed export table and decoded body mark status16 at current TCB+12,
enqueue dormant state and request scheduler transfer. DS2O at0x2c72c calls
its linked thbase:8 stub0x2ddcc; the startup-controller-pressure capture then
reaches the original panic when an unimplemented host lifecycle resumes it.
PS2SDK thbase.h API declaration confirms ordinal8 is ExitThread:
https://raw.githubusercontent.com/ps2dev/ps2sdk/master/iop/system/threadman/include/thbase.h
No SDK implementation read. Native adapter retires the current worker into
existing dormant sentinel state, retaining stack and ID. Tests check immediate
quantum termination, retained state and subsequent deletion; invalid owners fail.
Shared non-rendering configuration break/ExitThread searches supplied no useful
IOP lead. No trap bypass or guest panic patch was introduced.


### EE interrupt thread-status query (2026-09-09)
Original wrapper0x26c0b0 loads v1=-49 before syscall0x26c0b4. The native
startup-thread-exit run reaches it from0x1cb6a4 with thread4 and interrupt
stack buffer0x5ef90. ps2tek BIOS EE Thread Services specifies syscall31
as iReferThreadStatus, identical status output to30. The interrupt-only
negative selector now routes to31 and shares the existing copier. No
reschedule occurs. Regression compares all48 bytes against the ordinary call.
The following jr/slot are now discovered via returning_syscalls=-49;
EE58,372 words/353 unresolved. Reference non-rendering syscall searches
provided no further applicable lead. Source: https://psi-rockin.github.io/ps2tek/


### Short controller mode probe (2026-09-09)
Native startup-status-return packet01 42 00 00 00 clocks only five bytes
while the controller has negotiated pressure mode. SIO2 descriptor0x140540
requests five input/output bytes. ps2tek describes immediate serial replies
per clocked byte; the returned mode/length byte lets the driver discover the
full format before its next poll. Native42 now accepts five/nine-byte prefixes
of a longer reply, retaining mode79 and truncating at the requested wire length.
Full pressure polling remains21 bytes. A regression checks the precise prefix.


### Startup service callback (2026-09-09)
Native startup-pressure-60m faults at0x1ca660 with return0x1e5c18. Original
ELF0x1e5c00 loads a callback from the runtime table rooted at0x480e20,
then jalr0x1e5c10 invokes it with its adjacent argument. Independent ELF
body0x1ca660 calls0x1d3e58 and returns zero. Added the reached entry as
an AOT root, expanding reachable original code to68,314 words. No runtime
bypass; newly discovered unsupported boundaries still fault. Shared function
inventory provided only a matching32-byte entry; no implementation copied.


### Paired service callback registration (2026-09-09)
Original ELF setters1da7d0 and1da7e0 write a1/a2 to object20/24 and28/2c.
Original dispatcher1da860/1da8b4 calls those stored pointers. The native
startup-service-callback stop at1e4ee0 matches original registration1e514c,
which builds a1=1e4ee0 around an unrelated load into a0. Callback bindings
plus destination-specific invalidation for integer loads now discover both
pairs at callers1e0f34/1e0f4c and1e5138/1e514c: targets1e09c8,1e0a28,
1e4e40,1e4ee0. Only known straight-line constants are accepted; joins,
unknown operations and calls still discard stale state. Synthetic tests
exercise overwritten and unrelated destinations for all12 integer load forms.
No reference implementation copied; source is the original verified ELF.


### Service object table3d3fa8 (2026-09-09)
Native startup-paired-callbacks dispatches1e4010 through jalr1e4f94,
object3d3fe0, slot18. Original constructor1e3c10 builds table3d3fa8 and
stores it at object+0 in1e3c28. Original table has nine non-null methods
from3d3fb4 through3d3fd4; retained them as roots (one already present).
The service-method table itself and all targets came from the verified ELF,
not shared mappings. EE69,101 words/474 unresolved after generation.


### Embedded ck01 module source (2026-09-09)
Native startup-service-table.ram contains ck01 relocated at100030 after
original MODLOAD attempts LOADCORE linking. Original SLUS_210.75 contains
the source ELF at file34aa20; full section-derived extent359 bytes, SHA256
4ca27d8c457807cd6bcf5a8b4fb79f51b4a1ba00b8c10a0dc94936be9339ec51.
Container SHA256 is3b374d53a499d2c17b205274ee9eb34280768f294f970ebf6ae6731f6a2dacb8.
The captured source buffer13dc00 matches those bytes exactly. Independent IRX
parser reports entry0, one144-byte load segment,112-byte text, and cdvdman:50
import stub54. Current module source reader does not support an ELF region;
this remains a required static source/lifecycle extension. Reference config
search for ck01/cdvdman50 supplied no applicable lead. No check patched.


### Embedded region source support and MODLOAD buffer ABI (2026-09-09)
Source reader now supports offset/size with independently checked whole-file
and extracted-module SHA256 identities. Existing native region loader verifies
both identities again before relocation. Synthetic source/bounds/corruption
checks pass. Original MODLOAD exports9/10 resolve to a9440/a948c; own decoding
shows10 calling9 with zero address/offset. API names confirmed only from:
https://raw.githubusercontent.com/ps2dev/ps2sdk/master/iop/system/modload/include/modload.h
No SDK implementation read. Runtime buffered-load allocation integration is
still pending; existing explicit LOADCORE failure remains in place.


### Buffered CK01 lifecycle (2026-09-09)
CK01 static placement0xe0030 reserves its0x30-byte original MODLOAD prefix
and0x90-byte segment. Existing high-water reservation protects that range.
AOT entry hook at original MODLOAD+0x440 observes buffer/address/offset
arguments, matches all identity-checked original source bytes and selects the
allocation plan. It then executes the unchanged original prologue and loader.
No module success value is synthesized; original allocation/relocation/linking
and entry execution remain required. Source bytes are retained from verified
local files, not embedded in redistributable source. Tests check generated hook
execution, original stack adjustment, allocation, duplicate rejection, freeing,
reload, source corruption and unsupported placement without changing selection.


### CRI paired transfer callbacks (2026-09-09)
New startup runs reach original CRI_ADXI callbacks10e90 and110e8 through
jalr10b94 and10c24. Own relocation/decode shows registrations at11454/1146c:
a1 is built at11448/11460, then passed to original setters10b0c/10b18.
Both callbacks added as AOT roots. Non-rendering reference search yielded no
applicable IOP lead. No shared implementation or synthesized audio result used.
The compiled synthetic bundle now includes standalone, ROMDIR and embedded
inputs; all21 regression targets pass, including retained buffer identity,
planned allocation and ordinary execution from the embedded source.


### Object table46a1e0 (2026-09-09)
Native startup-cri-paired reaches169420 via object slot30 at2cfa3c, a0=80adc0.
Original ELF table46a210 contains that unique pointer. All14 non-null methods
from46a1e8 through46a21c were retained together, skipping roots already present.
Original destructor1692a4 explicitly publishes vptr46a1e0 before base teardown.
Wrapper169420 invokes slot2c and multiplies the result by2048. The table and
methods were read only from the verified original ELF; no shared mappings used.


### CK01 original-loader execution evidence (2026-09-09)
External startup-object-table.ram.json includes watch0xe0030 with pc0xe0030,
a0=1,a1=0x17ef80,gp=0xe80b0,sp=0x17ef70,ra=0xaa324. This proves the
original MODLOAD path reached compiled CK01 after identity selection, static
allocation and linking. No module-result or check bypass was introduced.
The run subsequently stops on EE object method1e3268 through jalr1d1f98.
Original table3cff70 slot24 at3cff94 contains that pointer; its full method
set still needs discovery. No playable-frame claim follows from this checkpoint.


### Object table3cff70 (2026-09-09)
Original constructor1e30b8 builds table3cff70 and1e30d0 stores it at object+0.
Native startup-object-table reaches its slot24 (1e3268) through jalr1d1f98.
Retained all nine non-null original table entries3cff7c..3cff9c together,
skipping already configured roots. EE71,536 words/490 unresolved. Source is
verified original ELF bytes; function inventory supplied only the matching
method boundary. No shared implementation or rendering material used.

### Priority restoration and CRI command callbacks (2026-09-09)
Original EE1cafa8 saves ChangeThreadPriority's result at3bfbf0;1cb02c loads it
for restoration at1cb030. The recorded oracle0->1 transition returned old0,
which does not establish a universal zero result. Runtime now returns old priority.
PS2tek's EE syscall list also describes the interrupt variant returning old priority:
https://psi-rockin.github.io/ps2tek/ . Shared non-rendering configuration supplied
the InitThread priority lead only; original call sites establish this behavior.
The corrected run reaches IOP17af0. Original relocated CRI_ADXI17dfc registers
17af0 through10b0c, and17e14 registers17b9c through10b18; both are retained
as static roots. Shared function inventory had no matching17af0 lead.

Original EE23ac58..23ac70 constructs240990 and registers it via2400e8.
startup-command-callbacks reaches this exact target through1e5c10 (ra1e5c18).
Retained callback root240990; shared inventory confirmed boundary only.

Original service registrars1e5898 and1e59e0 save callback/context pairs at
1e5918/1c and1e5a78/7c. Wrappers2400e8 and240198 preserve those arguments.
Declared callback ABIs allow existing constant analysis to recover original
registration pairs1ca7a4->1ca660,1ca7b8->1ca680,23ac6c->240990,
23ac7c->2409d8 for dispatcher1e5c10. This derives from original EE code,
without shared callback mappings. EE72,690 words/496 unresolved.

### Original secondary object table46bf2c (2026-09-09)
startup-service-registrars-120m reaches2107d0 through2cfa98, slot4c,
object887204. Original20e024/2c constructs46bf20/46bf2c;20e030/38
publishes these primary/secondary vptrs. Original secondary table contains
91 non-null methods46bf34..46c09c, ending before zero words46c0a0/4.
Retained all these original roots; EE74,696 words/514 unresolved.
2107d0 adjusts this by-4 and tail-calls20f810. No shared rendering material
or implementation was consulted; the original constructor/table is the evidence.

### Larger raw SIF payload (2026-09-09)
startup-secondary-object reaches original21f8e0 SifSetDma with descriptor
{1992340,136500,2800,0}. Original21f8b4..cc writes source,destination,size,
attributes directly from its arguments. The old4096-byte cap was a native
diagnostic restriction, not a hardware transfer bound. Raw snapshots now use
the available native kernel staging area60000..80000, checking each packet's
aggregate extent before mutation. Source-chain QWC remains within16bits.
Tests transfer/check every byte of a2800-byte payload through the existing
bounded FIFO, preserve snapshot semantics, and reject single/combined staging
overflow without changing transfer IDs or tags. Larger staging and other DMA
attribute/count profiles remain explicit gaps.

startup-large-sif subsequently reaches original CDVDFSVce920 via alarm
dispatch (ra a3b84). Originalcf14c/cf150 andcfba0/cfba4 construct/register
ce920 in a1; retained module offset920. The callback performs original
CDVD service calls and is not replaced with an invented timeout result.

### IOP GetThreadId interrupt return (2026-09-09)
startup-cdvd-timeout reaches9fe94 from original CDVDMANb8060 while inside
the alarm interrupt. Original THREADMAN9fea8 calls import a4b30
(intrman23, QueryIntrContext);9feb0 branches on nonzero to9fed4, which
returns-100. The native adapter now preserves that defined error result;
non-interrupt calls without an established worker still fail explicitly.
Regression checks interrupt context with and without a selected worker, and
the ordinary worker ID result. The focused IOP suite passes.

### CDVD BREAK investigation (2026-09-09)
Original CDVDMANb81b0 writes1 to1f402007; b81b4 reads the same address,
thenb81b8 overwrites its load destination. PS2tek CDVD register section
https://psi-rockin.github.io/ps2tek/ documents any write stopping the current
N-command, but does not establish readback or cancellation interrupt timing.
No emulator implementation was opened. External metadata now retains CDVD
command, remaining sectors, DMA descriptor, error/status and recent MMIO so
the original alarm's cause can be examined before implementing uncertain effects.

### IOP event grant ownership (2026-09-09)
Original THREADMAN iSetEventFlag at a11d8 copies the current pattern to the
waiter's result pointer; a11f0 clears it for mode0x10, before a1200/a120c
removes/wakes the waiter. Native event signaling now performs those effects
at grant time and retains a completed grant for the cooperative retry boundary.
Tests preserve later signals and the original captured result; waiter count
becomes zero immediately after signal. CDVD's current mode0 wait is separately
diagnosed: worker8 is ready but lower-priority than continuously ready worker19.
### EE semaphore overflow (2026-09-09)
PS2tek EE syscall42/43 documents failure return-1:
https://psi-rockin.github.io/ps2tek/index.html . The higher-IOP-budget experiment
reaches SignalSema with a full semaphore4 at26c1d4,ra10eeec. Original caller
10eee4 signals then overwrites v0 at10eef0. Native overflow now returns-1 and
preserves the count; tests verify ordinary and interrupt forms. This does not
claim the diagnostic CPU budget is cycle-accurate or fix every scheduling race.

### CDVD descriptor versus command completion (2026-09-09)
Original IRQ2 path b6404..b6458 reaches request completion b5b18, which
signals event0x29 atb5b48 and disables DMA IRQ35 atb5b54. Publishing
drive I_STAT1 after a partial DMA descriptor therefore terminates the original
request early. DMA completion now remains distinct from drive command
completion: intermediate descriptors only raise their DMA source; the final
descriptor publishes the existing full-command status. A split two-descriptor
regression verifies data, pending command, DMA IRQ and final drive IRQ.
Budget1/4/32 comparisons all reached the prior failure; no scheduling-speed
change is needed to establish this independently decoded lifecycle mismatch.

startup-cdvd-command-completion verifies progress beyond the stranded read:
read_pending0,sectors0,LBA2113120,error0, then dispatches SNDDRV86018 via
original SIFRPC (ra998bc). Original85524..85544 registers server77777778
and callback86018; retained module offsetb018. No RPC result was fabricated.

startup-snd-bank reaches original EE21f290 through SIFRPC26facc. Original
21fac8 and21faf4 select21f2b0/21f290 and pass that completion pointer onward;
the callbacks clear flag bits0/1 and return through original interrupt code.
Retained both callbacks from original executable evidence. EE74,710/514.

startup-snd-completion reaches1bb9d0 via2a7ae0, object4f1920. Original
table46ac78 is slot28 of46ac50; original1aae34 publishes that vptr.
Retained36 non-null methods46ac58..46ace4 together, deriving every address
from the original ELF. No shared rendering configuration or implementation
was consulted. EE89,219 words/590 unresolved after transitive discovery.

startup-object-state reaches2d1b20 through member-pointer helper100b6c,
called by2d1f5c. Original static descriptors414370 and414380 have fields
{0,-1,2d1b20} and{0,-1,2d1aa0}. Retained these two independently verified
direct targets for the existing member helper. EE89,472/598.

startup-member-state reaches2cfbd0 through2d1c2c, slot24 of original table
46f360. Original20dbd8 publishes this vptr; retained all eight non-null
methods46f368..46f384. EE93,808/622 after transitive discovery. Original
executable only; no shared rendering material used.

### VIF1 idle status and MARK (2026-09-09)

Original EE10ca44 reads10003c00 and masks1f000003, checking VPS/FQC.
Sony EE User Manual v6.0 pp143-144 defines idle VPS=0, empty FQC=0 and
MRK bit6, set by MARK and cleared by an EE MARK-register write. Source:
https://github.com/ninjadynamics/PS2Docs/blob/main/EE_Users_Manual.pdf
(local previously downloaded hg-ee-users-manual.pdf independently extracted).
The synchronous transport exposes completed/empty status and tracks MARK.
Incomplete buffered packets still fail explicitly: parser storage is not a
hardware FIFO, so its length is not misrepresented as FQC. VU activation and
interrupt/stall profiles remain checked unsupported boundaries. No shared
rendering configuration or emulator implementation was consulted.

### GIF completed-packet status (2026-09-09)

Original EE10cabc reads GIF_STAT10003020, masking APATH bits11:10.
Sony EE User Manual v6.0 pp149/164 defines idle APATH/OPH/FQC, VIF
mask M3P bit1 and BUSDIR-mirrored DIR bit12. The synchronous transport
reports these existing states, with explicit failure for incomplete packet
status (no invented FIFO occupancy/path arbitration). Unsupported control
and mode writes continue to fail. Source: same original manual linked above;
no shared rendering material or emulator implementation consulted.

### VIF1 DMA tag transfer (2026-09-09)

Connected startup-vif-control proves original EE10d740 writes CHCR145.
EE manual pp45/74 define TTE bit6, sending the tag before its payload;
p86 separates low64 DMAtag from upper two VIF stream words. Independent
transport forwards upper64 into the parser at byte phase8 before data.
A tag crossing an incomplete VIF packet remains an explicit boundary until
physical phase tracking handles the skipped low64 at such crossings.

### Native texture sampling cost correction (2026-09-09)

Own-process context sample PID14356/thread39448,RIP7ff609130d28,
imagebase7ff609120000 (RVA10d28), disassembled with MSVC dumpbin,
identified texture/CLUT extraction during long startup-object-draw run.
Independent runtime point_sample_tex0 invoked full texture extraction per
fragment. Refactored existing conversion into texel_from_tex0 shared by
extraction and sampling, preserving live VRAM feedback without a cache.
No external rendering algorithm or mapping consulted or copied.

### GS address-unit audit (2026-09-09)

Original GS User Manual v6.0 p162 explicitly specifies linear32-bit words
with page8192bytes,block256bytes,column64bytes,32blocks/page and
4columns/block. Existing native *_word helpers mistakenly used these byte
counts as word strides. Captured source textures show corruption, requiring
correction and independent page-coverage verification; no emulator/shared
rendering materials were used in this finding.

GS address audit correction: original rendered GS manual pp164/165
block grids prove CT32/T8 blockx bit2 maps toblockbit4; CT16/T4 blocky
bit2 maps toblockbit4. Pp167/169 indexed column grids alternate wordbit3
by rowbit1/columnparity (XOR), preserving byte/nibble lane assignments.
Full-page synthetic tests now enforce unique coverage of each8192-byte
page across all8 underlying formats and independent next-page separation.

### DualShock2 pressure negotiation (2026-09-09)

Micah Dowty controller-sniffer and packet-generator observations:
https://gist.github.com/brettwsnare/65acb5f2f007f6947c21ae78e0a9af4f
Command 4F ends its response with 5A. Correcting that missing byte lets
the original connected run startup-pad-reply advance to command 40 with
input 01 40 00 00 02 00 00 00 00, captured externally in its IOP JSON.
Command 40 initializes one of twelve pressure sensors with parameter 02;
its six-byte payload response is 00 00 02 00 00 5A. The bounded native
profile records initialized sensors and rejects other parameters explicitly.
Synthetic tests verify every sensor, exact replies, and invalid index.
Shared non-rendering config search found only pad-state function leads;
no shared implementation was read or used. No emulator was used.

DBCMAN original callback verification: identity-checked DS dump module, native
relocation base26000. Instructions263c8/263cc form callback262f4 in a2
before registration call263d0. Connected startup-pad-sensors dispatches
that address, ra9c3ac. Added only the verified AOT root+2f4; its ten words
call the original imported service and return. Bundle now91684 words.

### EE idle thread identity (2026-09-09)

Connected startup-ee-idle reproduces GetThreadId26d078 from original
1cb6c8, during INTC dispatch with current_thread0 and all seven threads
waiting or suspended. PS2SDK kernel.h declares thread0 as the idle thread:
https://github.com/ps2dev/ps2sdk/blob/master/ee/kernel/include/kernel.h
Original caller checks thread status4/12 before querying the running ID.
Native kernel now returns idle ID0 for interrupt queries after scheduler
initialization; accidental execution with no thread outside interrupts still
faults. Synthetic scheduler test parks both threads, queries ID in interrupt,
and verifies neither waiting thread was resumed. No emulator code used.

DBCMAN RPC switch: startup-idle-id reaches27cf8 with command80001303.
Original27c94..27ca4 normalizes by80001301 and bounds selector<99.
27cb0..27cc0 indexes table28560 and jumps to its relocated target.
Configured table offset2560,count99 at site1cc0. This supersedes the
incomplete manually rooted interior cases; generated bundle92344words.

DS2O driver table: original2da58..2daa4 initializes six callbacks at
2e1a0:2db80,2db88,2db90,2dbb0,2dbe4,2dc1c. Original2dab8 registers
the descriptor with pointer at stack+24. Connected startup-dbc-switch
reaches slot3 via DBCMAN27818 (selector3 supplied at27804). Added these
six statically verified roots; generated28 modules/92641 reachable words.

### Initial VU0 macro arithmetic (2026-09-09)

Original EE10db68..10db84 loads a vector, VMUL.xyz squares components,
VADDbc accumulates y/z into x, and VSQRT/WAITQ publishes its length in Q.
startup-vu-multiply captures VF4=(0,0,0,1), VF5=0 at VSQRT10db78.
Independent local original VU User Manual v6.0 pp26-29,39-42,244,296
defines finite exponent255, exponent-zero flush,24-bit truncation, flags,
VMUL and broadcast ADD. Shared own integer FPU arithmetic follows these
same declared number rules; hardware least-bit differences are unmeasured.
SQRT pp193/314 derives a truncated integer significand square root of
absolute input, updates invalid/divide status and sticky bits, and writes
a pending Q value. WAITQ publishes it; unsynchronized Q reads or another
producer explicitly fail in this bounded profile. No cycle-accurate Q
pipeline or microprogram execution is claimed.

Audit pp237-239 also corrects pre-existing decoder errors: QMFC2 rs1,
QMTC2 rs5 (bit0 interlock allowed), SQC2 opcode62. Former rs2/6 are
control transfers and remain unsupported; CACHE opcode47 is never a
vector store. Existing self-consistent decoder expectations were corrected
and negative tests prevent those misclassifications. No shared rendering
material, emulator implementation, or algorithms were consulted.

### Correctly decoded cache tag scan (2026-09-09)

startup-vu-q reaches original26ca04 CACHE0x10; formerly misdecoded as
SQC2. Original EE Instruction Manual p303 defines DXLTG: index[11:6],
way[0], load tag into TagLO. P306 defines DXWBIN opcode14hex. Native
coherent RAM retains no dirty cache copy, so tag lookup explicitly writes
invalid/clean TagLO0; index writeback orders coherent stores. Unsupported
tag-store operations remain faults, so they cannot create hidden valid
lines. Synthetic tag test verifies TagLO replacement and unchanged RAM.
Original26ca0c reads tag, checks PFN against requested range, and skips
nonmatching entries. Reference non-rendering cache-name search had no lead.

### VU FBRST and corrected synthetic fixture (2026-09-09)

startup-cache-tags reaches original10bfd8 CFC2 control28, then OR0200
and CTC2 control28 at10bfe0. Original VU manual pp21/200-203 defines
FBRST reset/enable bits and Ready-state reset. Added CFC2/CTC2 static
transfer decode (control accesses no longer use VF registers), bounded
FBRST reset and enable storage; force break/reserved bits still fault.
Reset command bits readzero and do not clear vector/local-memory data.
VU1 micro activation remains unsupported, so reset begins in Ready state.
Native full CTest found stale synthetic generator encodings atf714/18/1c;
corrected those to specification too. This was a fixture error exposed by
the decoder correction, not a reason to restore the wrong instruction map.

VU Ready status: startup-vu-reset original10ca84 reads VPU-STAT29.
VU manual203-204 documents both VUs idle after reset. Native read returns
zero while no Q result is pending; pending-Q polling remains an explicit
timed-completion gap. Original10db84..10dba0 normalization sequence
requires VNOP,VDIV,VSUB,VMULq. Manual244/253/298/304/315 and lower DIV
p129 define these. Added only these statically decoded operations with
masked arithmetic/flag state and synchronized Q. Exact VNOP preserves
status; it is not an unknown instruction fallback. Synthetic normalization
uses power-of-two length4 to independently verify final unit vector.

VMULbc original10e64c multiplies loaded VF4 by scalarVF5.x. VU manual
p299 specifies four broadcast selectors. Added static selector path with
aliased-destination test. VF0 transfer audit fixes old helpers that used
all-zeroVF0 or permitted writes: VU fixed floating register has x/y/z0,
w1. Reads now preserve that constant across vector/register/memory
transfers; writes ignored after required source-memory validation.

Original member dispatch: startup-vu-broadcast reaches37fef0 from3806bc
through100b40, object888440. Independently decoded380510 copies original
44ae70..44aed8 descriptors into a compact local array, then selects by
object+18. Seven descriptor targets added as roots. Shared non-rendering
config exact-address search found a name lead at functions.manual5780;
implementation and table facts come only from the original ELF.

Default member callback383440: startup-member-states reaches it via
384ba0 tail-call100b40, descriptorobject888570+4. Original384538/384544
loads descriptor44aef0;38456c..78 installs its three words atobject+4.
Added the original two-instruction return function as a static root.
Reference exact-address search found a non-rendering name lead only.

Vtable46f350: startup-default-member reaches2cf8c0 from1bbed0,
slot0c of object94fa10. Original2d1050..5c constructs tablepointer46f350
and writes object+0; slots8/0c contain16cc40/2cf8c0. Both are now
explicit roots. Exact-address shared-config search supplied no implementation;
verification used own ELF reader/decoder and native fault provenance.

Vtable469a60 original lifecycle1218b8..c4 and original startup callsites
2cf8f0/904/918/97c prove four needed slot targets1225c0,121970,121960,
1225d0. Only these startup-selected entries added. Next native capture
startup-object-9a60 reaches VOPMULA10db28, called by2cf92c.
VU manual116-117/305-306 defines cyclic y*z,z*x,x*y products intoACC,
then ACC minus reversed products intoVF, xyz fixed and w preserved.
Independent bounded implementation preserves intermediate sticky flags
and final MAC flags; exceptional OPMSUB accumulator cases remain explicit
faults. VADD p241 shares masked integer24-bit arithmetic. Unit-axis cross
and aliased destination tests verify order and w preservation. Hardware
manual notes one-bit multiply deviations; no hardware oracle measurement
is claimed. No external rendering algorithms or implementations used.

Object46ae90: startup-vu-cross reaches1bf340 from2bf414 slot0c.
Original1bf568..84 installs primary46ae90 atobject+0 and secondary
46aeb4 atobject+0c. Seven primary targets and secondary thunk1bf870
are independently verified from these tables and added as AOT roots.

Original3821f0 byte-command switch: startup-object-ae90 reaches38228c.
3821f8 checks selector<30;382204..218 indexes table463bd0 and jumps.
Configured exact30-entry range463bd0..463c48 as indirect targets, rather
than inventing a function entry for the interior case.

Member service226220: startup-byte-switch reaches it via22653c and
100b40, descriptor atobject4f1850+14. Original1bf350/35c loads3b3028
and1bf378..80 stores its words atobject+14..1c. Added verified function
root; committed display capture at this stop remains black.

### Empty EE PollSema (2026-09-09)

startup-service-member reaches semaphore11 empty PollSema26c1f4.
Original1113cc calls it;1113d8 tests return<0 and conditionally calls
WaitSema. PS2tek kernel documentation45h/46h specifies -1 on failure:
https://psi-rockin.github.io/ps2tek/index.html
PS2SDK kernel documentation says empty polling returns error without
putting the caller into wait state. Native empty case now returns sign-
extended-1, leaves count0 and scheduler untouched. Successful returnID
remains as independently verified by the original USA caller10fa88.
Synthetic tests cover both depletion and unchanged thread state. No
emulator implementation used; reference PollSema-name search gave no lead.

### IOP loader-header collision (2026-09-09)

External startup-sio-callback.ram.json records the only write to original
SIO2MAN callback slot 0x7afdc: thread2, LOADCORE PC0x96840, RA0xab20c,
old0/new8. Original relocated LOADCORE instruction is sw v0,12(a1),
writing the module ID into SNDDRV's header at0x7afd0. The callback setter
0x7ab18 never executes. Original SIO2MAN memory extent is0xff0 bytes,
so its old base0x7a000 overlaps that header by0x20 bytes. Moved SIO2MAN
base to0x79fd0; its full allocation [0x79fa0,0x7afc0) now fits between
SIO2D ending0x79d60 and SNDDRV header0x7afd0. Regenerated original IRX
relocations/bindings, without copying any implementation. Planner now
reserves each complete rounded allocation including the 0x30-byte prefix;
reserved_low is0xffd0 to include CRI_ADXI's existing first header. Runtime
static plan registration also rejects header overlaps before execution.
Synthetic tests cover non-overlapping images with colliding headers,
exact adjacent allocation boundaries and reserved-low rejection. Narrow
shared configuration module-name leads did not explain this corruption;
the original LOADCORE write and local capture establish its cause.

### Original DMACMAN slice setup (2026-09-09)

startup-module-spacing passes the invalid callback and reaches SIO2MAN's
memory-card DMA setup at relocated0x7adf8. Original packet at0x47020 is
81 11 00 00, SEND3=0x100472 (port2, DMA both ways, four serial bytes),
BCR=one block of36 words. Original SIO2MAN0x7a274 calls SetSliceDMA and
0x7a27c separately calls StartDMA. Read-only original BIOS inspection:
emu/ps2 bios/scph39001.bin SHA256
f4c948e61a291d4b3f92a141e550cf8357204287a31ff784caccbedaef910c9d;
ROMDIR at0x2740, DMACMAN at0x287f0 size0x36f5, member SHA256
b0c35cb83cc61385cc86b1665b7a65e49dda71588feffc3ba70575267f1cc559.
Own relocation/decoder identifies ordinal28+0xcc4: CHCR gets0x200 plus
direction bit0 and bit23 for device-to-memory; ordinal32+0xe84 adds bit24.
Corrected native SetSliceDMA which previously started prematurely and
omitted request-mode bits. No emulator source or algorithms consulted.

The bounded SIO2 DMA endpoint accepts only one 81/11 absent-card probe,
one36-word block each direction, and a four-byte serial exchange. It
validates all buffers and modes before mutation, raises the SIO2 missing-
ACK result0x1d100 and completion, and accounts for the full DMA block.
PS2SDK hardware register definitions identify SEND3 port bits0..1,
TX/RX DMA bits4/5, normal-transfer bit6 and byte counts8/18:
https://ps2dev.github.io/ps2sdk/sio2regs_8h_source.html
Wisi's own hardware investigation reports DMA blocks synchronized to
queue elements, with receive DMA triggered after a queue element:
https://www.psx-place.com/threads/mx4sio-sio2sd-sd-card-adapter-and-sd-driver-for-the-ps2-sio2-interface.29210/page-11
SCPH-39001 absent-card RECV1 observations:
https://redpanda4552.github.io/ps2-sio2-docs/#recv1-test-results-memcards-only
Fidelity limitation: the four absent-device reply bytes are pulled-high;
the remaining140 DMA bytes currently use the same0xff profile value.
This unused tail has NOT been measured on hardware or a memory oracle.
Do not claim byte-exact DMA padding, cycle timing, general queued DMA,
present memory cards, or save support. Other packet/mode profiles fault.

The next startup-card-dma capture confirms the 81/11 probe completed:
RECV1=0x1d100 and RX block is FF. MCMAN then requests a five-byte81/F3
probe (SEND3=0x140572), so the absent-card endpoint now accepts any
81-selected serial packet fitting one36-word block on card ports2/3.
It does not interpret commands for an absent device. Multi-block DMA and
other device selections remain explicit faults. This is a disconnected
card profile, not a card protocol implementation. Tail fidelity remains
unverified as above. Original-state capture is external; no game bytes
were added to tracked inputs. Synthetic tests include either card port
and differing serial lengths.

startup-card-probes proceeds to a PIO81/52 probe with SEND3=0xc0342,
three input/output bytes on card port2. Added the corresponding absent-
card PIO transport, returning pulled-high serial bytes and missing-ACK
status; the original module continues to own command/error handling.
SIO2 method bodies now reside in runtime/iop_sio2.cpp, linked through
hg_iop_peripherals, so peripheral-only edits no longer rebuild the large
AOT translation units. No runtime translation or dispatch fallback added.

### MCMAN alarm callback root (2026-09-09)

startup-card-pio reaches original relocated MCMAN0x3ef84 from native
THREADMAN0xa3b84 while serving an alarm. Original MCMAN+0xa13c/+0xa140
forms callback+0x9f84 in a1; +0xa144 calls thbase ordinal35 via import
stub+0x11b48. Three later sites install the same callback. Its body
signals the semaphore at relocated0x48f10 through thsemap ordinal7 and
returns0. Added only this verified AOT root (10words). Narrow supplied
HG configuration SetAlarm/memory-card names yielded no relevant lead.
No emulator implementation or captured code was translated.

### EE memory-card information completion (2026-09-09)

startup-card-alarm reaches EE111450 via original SIFRPC completion26fad4,
with a0=47c200, a1=198af80. Original1115a8/1115bc constructs111450 in
register11 (a7), and1115cc submits the RPC through2700e8. Added this
verified22-word callback, which copies requested information outputs
from the uncached reply. No source/reference implementation copied;
narrow shared configuration111450/mcGetInfo lookup yielded no lead.

### Rendered memory-card check and prompt state (2026-09-09)

startup-card-info runs beyond90million connected slices and captures
8061 submitted/7898 rasterized draws. The independently rendered640x448
scanout visibly reads Checking the memory card (8MB) for PlayStation2
and the do-not-turn-off warning. External capture:
%TEMP%/haunting-toc-probe/startup-card-info.ram.display.ppm.png.
This is a new visible startup screen, not playability or save support.
The next EE stop383550 is an object+4 member callback dispatched by
384bdc/100b40, object94f898. Original descriptor44af10 contains target
383550 at+8;3842e0/3842e4 loads it and38430c..384318 installs the member.
Added the verified230-word state callback. Derived only from original
ELF; no shared rendering-related configuration or implementation read.
