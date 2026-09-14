# Architecture and milestones

## Independent static translation

1. Parse and validate the user's ELF32/MIPS executable; check its exact digest.
2. Follow control flow from entry and manually configured function roots.
3. Decode R5900 instructions using independently written code and hardware manuals.
4. Generate C++ instruction effects at build time; compile with the host compiler.
5. Resolve guest addresses only to already-compiled blocks. Never fetch/decode guest
   instructions at runtime. Unknown PCs, instructions and hardware accesses fault.

The initial emitter uses one switch case per known instruction address, combining
a control transfer and its delay slot in a single dispatch. This is a correctness
baseline; later group straight-line instructions into basic blocks for performance.
Branch conditions and register jump destinations are sampled before the delay slot.
Likely branches annul it when not taken. Calls write the link before executing it.
The execution budget counts dispatches, not EE cycles or individual instructions.

State holds 128-bit GPRs as two portable 64-bit halves, both HI/LO banks, SA, raw
FPR bits, 32 MiB RAM and 16 KiB scratchpad. Scalar register writes preserve the
upper half; packed instructions explicitly write both. Memory uses byte operations
and defined unsigned arithmetic, avoiding host alignment and endian assumptions.
Direct RAM, KSEG0/KSEG1 aliases, scratchpad, timer and INTC registers are implemented.
Other MMIO, TLB and guest architectural exception delivery remain incomplete; Fault stops host
execution and identifies the current instruction, including a delay-slot fault.

Discovery is deliberately incomplete at unsupported instructions and system calls.
It does not infer all functions, recover jump tables, decode embedded data blindly,
or automatically recognize overlays. Explicit mappings compile known copied code. A discovered call continuation is a candidate reachable
address, not a proof of callee return. No instruction-count percentage measures
whole-game completion. Manual entries repair discovery without depending on
another recompilation project's results.

## Graphics direction

OpenGL 3.3 core is the chosen backend baseline; GLFW provides the platform window.
The host probe compiles a core shader pair, uploads presentation-layer triangle
geometry to a VAO/VBO, rasterizes it, and validates a non-clear framebuffer
readback before swapping. It is not game-driven rendering: GS viewport, XYOFFSET,
depth, blending, texture sampling, and guest draw submission remain unfinished.
The independent graphics transport layer now parses bounded GIF
PACKED, REGLIST and IMAGE packets; commits documented state-only attributes
(including A+D, PRE, RGBAQ/ST/UV, TEX0 and CLAMP); and writes documented
PSMCT32/PSMCT24/PSMCT16/PSMT8/PSMT4 IMAGE payloads to the 4 MiB GS
page/block/column VRAM layout. Same-depth local-to-local copies across the
implemented CT/Z/indexed layouts honor TRXPOS.DIR; local-to-host transfers reject
explicitly.

The EE scalar MMIO path also exposes the documented 64-bit GS privileged mapping
and mirrors for PCRTC/display state, IMR/BUSDIR, CSR, and SIGLBLID. Supported GS
events feed EE INTC cause 0; reset/flush and FIFO readback remain checked gaps.
One enabled PCRTC read circuit can extract its DISPFB/DISPLAY-selected CT32/24/16
rectangle from swizzled VRAM into portable RGBA, forming the CPU-to-OpenGL
scanout boundary. Dual-circuit merge policy remains explicit.
The OpenGL host consumes that portable image as an RGBA texture and verifies an
presented pixel. The default producer is a synthetic GS state. In
`--watch-display` mode it instead reads the connected diagnostic's complete P6
framebuffer snapshots, retains the last frame between updates, and preserves
the source aspect ratio. The producer samples committed VRAM every million
slices with `--preview-file`, without flushing queued guest draws. This exposes
startup tests to the user; integrated frame-paced presentation remains unfinished.

Renderer-facing TEX0 extraction reads those swizzled direct-color formats and
indexed formats through CSM1 and CSM2 16-bit (CSA=0) CLUT placement. TEX0/TEX2
load control fills a shared 1 KiB temporary palette buffer at CSA*16; texture
sampling reuses its entries until another applicable load. Pending draws are
committed before cache replacement or source-palette reads. Unknown cache
contents and unsupported cache/source spans remain explicit faults. See the
current progress and source notes for verified sampling/rasterization coverage.

PRIM writes reset a bounded GS vertex queue. XYZ2/XYZF2 and XYZ3/XYZF3 retain
the documented kick/draw distinction and assemble raw point, line, triangle,
strip, fan, and sprite events for the presentation layer. This is not yet a
rasterizer: clipping, texture sampling, blending, depth, scissor, and OpenGL
publication remain unfinished.

`hg/presentation.hpp` converts completed GS draw records into ordered,
backend-neutral point/line/triangle geometry while retaining PRIM texture,
Gouraud, alpha, and context flags. It applies the active context's documented
XYOFFSET subtraction in signed 12.4 coordinates; viewport transforms and other
raster rules belong to the OpenGL backend.

`GifPath` accepts split qwords and only commits complete EOP-terminated packets
to GS state. EE `State::pump_gif()` owns the connected normal/source-chain GIF
DMA endpoint. VIF1 normal/source-chain DMA and its bounded DIRECT/DIRECTHL
PATH2 endpoint now feed the same GIF path; both native diagnostics service them
after EE slices. VIF1 also retains checked UNPACK writes and aligned MPG uploads
to VU1 memory, but MSCAL-family execution and VU XGKICK remain explicit
boundaries until a separate AOT VU path exists.

Implement a testable guest graphics layer separately from presentation: DMA chains
feed VIF/VU and GIF packet decoding; GS state and VRAM updates feed OpenGL buffers,
textures and shaders. Build synthetic packets with known expected results before
comparing captured game workloads. Address GS pixel layouts, CLUTs, alpha/depth,
blending, feedback and render targets explicitly. A generic textured triangle is
not evidence that Haunting Ground renders correctly.

## Milestones (in order)

1. Foundation: config, validated ELF parsing, subset translator, compiled synthetic
   tests, OpenGL host. Initial slice implemented; expand tests with each opcode.
2. Boot: correct remaining startup FPU/EE operations, load ELF segments and BSS,
   verify memory against independent expectations, model required kernel calls.
3. Discovery: reachable functions beyond boot, return/jump-table recovery, explicit
   overlay identities and per-image configuration. Do not treat mutable code as
   automatically supported; compile known images ahead of time.
4. Platform services: asset/CVM I/O, SIF/IOP boundaries, threads/synchronization,
   DMA/VIF/VU, input, audio and memory-card saves according to actual game needs.
5. Rendering: GS packet/state tests, first correct game frame, scene comparisons.
6. Playability: menus, new game, gameplay progression, audio, saves and regression
   scenes. Establish Windows and Linux builds/tests, then additional platforms.

No milestone beyond the first is complete. Oracle observations are recorded in docs/ORACLE.md.
## Current boot compatibility boundary

The native ELF loader checks the compiled image digest before copying load segments
and zeroing BSS. Boot ABI services construct stack/heap metadata and argv storage;
the observed stack profile is specific to the USA launch. CreateSema stores real
counts/limits, while wait queues/scheduling remain unimplemented.

The guest startup installs helper callbacks and searches for the syscall table.
The native runtime exposes explicit compatibility DATA in low RAM, including a
table at 0x10000. Entries refer to native gateways or translated guest callbacks.
No BIOS implementation is embedded. Guest callbacks preserve their syscall caller
through a return stack and return gateway. Unknown hardware/callback addresses trap.

FPU add, multiply, MADD and MSUB use integer mantissa/exponent arithmetic with
the EE's documented chop-toward-zero, signed-zero, saturation and flag behavior.
Remaining conversion, division/square-root and exceptional-operation cases stay
explicitly unsupported until independently implemented.
SYNC orders synchronous operations with a host fence, and must gain device-drain
semantics when asynchronous devices exist. LQ/SQ mask address low bits as documented.

## Asset and SIF foundation

`tools/hgtool/volume.py` independently parses the ECMA-119 subset used by DATA.CVM.
The volume origin is an explicit input (0x1800 for this dump). Both-endian fields,
descriptor termination, directory bounds/cycles and extent bounds are checked.
The reader indexes 2,425 files in 207 directories; it streams bounded reads and
hashes without extraction. This Python inspection tool is not yet the native asset
service. The native I/O layer must expose these files through the actual game protocol.

SIF transport has two bounded FIFO directions. EE source/destination DMA and IOP
source/destination DMA move actual qwords and track completion. Unsupported tag
modes, stalls and device endpoints fail explicitly. Native cooperative dispatch
runs original AOT DMA callbacks with saved CPU context; it is not yet a complete
cycle-accurate exception controller or general thread scheduler.

The IOP build plan assigns game-supplied IRXs explicit native guest addresses from
`config/iop_modules.toml`. Hashes, reservations, segment ranges, function roots and
exports are checked. Build-time relocation applies the independently decoded
symbol-zero R_MIPS_32/26/HI16/LO16 subset. Linked calls require original LOADCORE
stub writes to match their precompiled target; no runtime decoder or linker fallback.
Combined module loading is transactional. Explicit native cache/interrupt/event
adapters replace only configured ABI entries and preserve actual service state.
The one configured MODLOAD selector-12 syscall is likewise lowered only after
the planner verifies its exact local `v0=12; syscall` byte pattern; it transfers
to the checked guest callback and does not introduce a general syscall fallback.

`hg_system_diagnostic` runs the EE executable and a six-module IOP bundle together.
Original SIFMAN/SIFCMD entries initialize their buffers and handlers; original
SIFCMD RPC initialization publishes readiness and waits for real command events.
The EE sends two commands through native SifSetDma, the IOP sends its reply through
original SIFMAN, and both sides process DMA callbacks. `--verify-sif` checks that
complete exchange. Full worker scheduling, I/O servers and rendering remain ahead.
