# Project rules and map

## Goal and authority
Build an independent ahead-of-time static recompilation of Haunting Ground (USA)
for Windows, Linux, and other desktop systems where the dependencies permit.
Use OpenGL for graphics. The user's instructions and root SKILL.md govern work.
This project is solely a static recompilation, not a general-purpose PS2 emulator
(user reaffirmed 2026-09-13). Translate the original game code ahead of time to
native code; implement only independently derived runtime services required by
that compiled program. Never introduce runtime instruction decoding, an
interpreter, JIT, emulator execution backend, or an emulator dependency.

## Hard rules
- Never copy ps2recomp implementation into this project. The user permits looking at the shared HG configuration for context and ideas; independently verify relevant facts from original inputs/specifications and write our own implementation.
- For new non-rendering stops, check the user-supplied HG configuration lists for relevant leads while continuing diagnostics. Independently verify and try applicable leads; if they do not help, continue tracing. Rendering is excluded from this reference workflow.
- The user explicitly prohibits copying ANY rendering-related material from the shared HG/ps2recomp project, including code, configuration, mappings, patches and algorithms. Derive rendering independently from original inputs and hardware specifications.
- The user now permits scanning PCSX2 to investigate timing (2026-09-13).
  Do not copy its code, algorithms, comments or implementations. Treat inspected
  settings/implementation as leads; independently verify behavior from the original
  executable and hardware specifications. Record provenance. PCSX2 remains an
  external reference only, never an execution backend or dependency.
- The user authorizes access to the local PCSX2 installation and supporting
  files under `emu/`, including `emu/PS2 emu/`, for these memory-oracle checks.
  This authorization persists across continuations; the independence and
  memory-oracle-only restrictions above still apply.
- No runtime instruction interpreter/JIT fallback. Unsupported translations and
  unimplemented hardware must fail explicitly, never silently become no-ops.
- Keep game files, BIOS, emulator files, memory captures and generated game-derived
  source out of version control. Consume the user's local dump at build/run time.
- Temporary files belong outside the repository. If a temporary image is necessary
  inside it, use only `UNEEDED images/` (the user's existing spelling).
- Maintain this file as the rules and project map, not a growing work log.
  Refresh `docs/PROGRESS.md` every 20–30 minutes during active work, before handoff,
  and at the end of a work session. Update this map when structure/rules change.
- On handoff use SKILL.md's complete briefing structure and outer-fence rule.
- Do not claim playability or cross-platform verification without evidence.
- Label and account for diagnostic code and experimental substitutes in
  `docs/DIAGNOSTICS.md` using stable `HG-DIAG-NNN` identifiers. Record the source
  location, purpose, activation/default, guest-state effects, measured or unknown
  host cost, and removal/retention criterion. Link identifiers from source when
  adding or changing a probe. Update the inventory and active entries in
  `docs/PROGRESS.md` during optimization and before handoff. Do not leave temporary
  watches, logging, synthetic inputs or timing overrides disguised as normal
  behavior. Keep disabled diagnostics cheap; preserve explicit correctness faults.
- The user explicitly requested continued work until told to stop. Keep advancing
  through milestones without treating a successful checkpoint as a stopping point.
- The user requested faster progress. Prioritize the startup/EE-IOP communication
  path; batch validation around meaningful changes and avoid unrelated leaf-function expansion.
- Standing user preference: target faithful 1:1 behavior while minimizing wasted
  time and usage. Do not linger on repeated analysis, setup or low-value checks.
  Choose the next action by the concrete blocker it resolves and evidence it yields;
  reuse verified findings, batch related investigations and validation, and automate
  repeated startup/input sequences. Repeat expensive replays or research only when
  new evidence or a relevant change justifies them. Speed must not weaken correctness,
  independence constraints or explicit-stop diagnostics. Report substantive runtime
  progress and remaining blockers concisely.

## Active work and continuation
- At the start of a continuation, identify the next measurable major milestone
  and its verification evidence in `docs/PROGRESS.md`. Prefer a later original
  startup stage, a newly rendered screen responding to input, or a subsystem
  exercised end to end by the original program.
- Minor edits, successful builds, isolated test passes, removal of one explicit
  fault and exhaustion of a diagnostic slice budget are checkpoints, not reasons
  to end the work session. Trace the next observed blocker and continue.
- A milestone counts only with captured runtime evidence and appropriate
  regression checks. Never suppress an unsupported-hardware fault, substitute
  guessed behavior or weaken a test just to report progress.
- Keep advancing after a major milestone while the user's continuation request
  remains active and tools permit. Batch independent investigation and validation;
  report meaningful findings without asking again for already authorized work.
- Stop only for the user's instruction, completion of the authorized goal, a real
  external/permission blocker, or an actual session/tool limit. Do not promise
  background or unlimited execution. Before yielding or forced handoff, record
  the active milestone, evidence, exact blocker, live command sessions and next
  executable action in `docs/PROGRESS.md`; carry them into the SKILL.md handoff.

## Read first
At each continuation, scan every project `.md` file, including ignored folders,
for current instructions, evidence and pending work. Read the current sections
before acting; dated historical notes do not override newer verified state.

1. `docs/PROGRESS.md`: verified state, blockers, exact next steps and commands.
2. `README.md`: build and analysis workflow.
3. `docs/ARCHITECTURE.md`: execution model and staged roadmap.
4. `docs/SOURCES.md`: specification sources and provenance rules.

## Map
- `docs/DIAGNOSTICS.md`: diagnostic/probe inventory, activation, performance cost
  and cleanup criteria; distinguish observation from guest-affecting experiments.
- `tools/hgtool/`: independent ELF parser, EE decoder, CFG/callback discovery, C++ emitter.
- EE/IOP emitters partition static C++ dispatch by4KiB address region, preserving
  original budget/delay-slot/wait semantics. `docs/PERFORMANCE.md` records profiling
  and state-equivalence evidence; diagnostic `--profile` samples host phase costs.
- EE static intra-page branches jump to compiled labels through their existing
  budget/trace prologues. Fixed-width scalar RAM methods retain checked fallback
  for kernel mappings, devices, invalid spans/alignment and diagnostic watches.
- `tools/hg.py analyze --triage`: groups boundaries and reports installed object-table
  candidates with store provenance, slot offsets and compiled/missing targets.
  Bounded pointer runs require independent extent/live-use verification; never auto-root them.
- `config/haunting_ground_us.toml`: executable identity, manual functions/targets, pointer tables and static overlays.
- `runtime/include/hg/`: portable state, memory, FPU, timers, INTC and DMA/SIF0 primitives, including checked EE word/doubleword unaligned merges.
- `runtime/include/hg/iop_dmac.hpp` / `sif_link.hpp`: bounded bidirectional DMA and shared transport state.
- `runtime.hpp` / `dmac.hpp`: connected normal scratchpad DMA channels8/9;
  checked RAM spans,14-bit SADR wrap and channel completion. toSPR also supports
  bounded source chains; fromSPR chain/interleave and cycle-accurate arbitration
  remain unfinished.
- `runtime/system_diagnostic.cpp` / `runtime/include/hg/system.hpp`: connected EE/IOP startup, checked IOP reboot, original service startup and FIFO servicing; `--verify-loadfile` checks native LOADFILE RPC registration before reboot.
- `runtime/include/hg/iop_interrupt.hpp`: cooperative AOT IOP DMA interrupt delivery.
- `runtime/include/hg/iop_reboot.hpp`: validated native reboot request callback; lifecycle remains in progress.
- `runtime/include/hg/iop_thread.hpp`: cooperative native worker scheduling and reserved stacks.
- `runtime/include/hg/iop.hpp`: IOP state/services, event wakeups, partial CD/DVD command protocol and RTC profile.
- Buffered IOP inputs retain verified source bytes through `iop_image.cpp`; AOT entry hooks select static allocation plans before the original MODLOAD body executes.
- Connected MODLOAD scratch, worker stacks and HEAPLIB arenas use original
  AOT SYSMEM alongside SIF heap clients. Static module spans and native call
  stacks are reserved there; isolated tests retain bounded synthetic arenas.
- `iop.hpp` parks WaitVblankStart callers until the connected display timing producer publishes an edge.
- SIO2 in `runtime/iop_sio2.cpp` / `iop_interrupt.hpp`: bounded two-port CPU FIFO transport
  with original IRQ17 delivery. `controller.hpp` / `runtime/controller.cpp` retain
  virtual DualShock2 config, axes, pressure and motor-map state; partial reply
  a one-block absent-card DMA probe is bounded. Partial reply masks, general
  DMA, saves and host input remain unfinished.
- SPU2 in `iop.hpp` / `spu2_input.hpp`: bounded one-shot DMA and paced stereo input halves; raw input capture is separate from unfinished synthesis/mixing/output.
- `runtime/include/hg/ee_interrupt.hpp`: resumable native EE SIF0 callback dispatch/context restoration.
- `runtime/include/hg/display_timing.hpp`: configured interlaced NTSC field edges;
  `ee_interrupt.hpp` now also dispatches ordered EE INTC callback chains.
  Connected display starts wake IOP WaitVblankStart workers. Other timing modes
  and complete architectural interrupt exceptions remain unfinished.
- `runtime/include/hg/ee_thread.hpp`: cooperative EE priority scheduling with per-thread CPU/kernel-frame preservation and semaphore/sleep waits.
- `vif.hpp` / `runtime.hpp`: VIF1 FBRST reset, ERR ME0, completed-packet STAT and MARK access; CPU SQ FIFO submission uses the existing VIF1 parser.
- `runtime.hpp`: bounded VU0 macro arithmetic/ACC/flags, synchronized Q, FBRST and idle status; timed Q polling and VU microprogram execution remain explicit gaps.
- IPU input DMA in `dmac.hpp` / `runtime.hpp`: normal and bounded source-chain
  channel4 transport, saved MADR/QWC continuation and eight-qword FIFO pressure.
  Shared internal bit buffering, FDEC and pending SETIQ/SETVQ consume this stream.
  Normal IPU output DMA3 writes real FIFO output to RAM/scratchpad.
  runtime/ipu.cpp contains bounded MPEG2 intra/non-intra frame BDEC and RAW8-to-RGB32 CSC;
  ipu_dct_lookup.hpp indexes the independently recorded H.262 numeric codebooks;
  invalid prefixes retain explicit faults and full-prefix regression coverage.
  MPEG1/field-DCT, RGB16 and dithering remain explicit gaps.
  Measured arithmetic profiles and physical-console limitations are in ORACLE.md.
- `runtime/`: checked ELF loader, native entry points and OpenGL host smoke test.
- `system_diagnostic --preview-file` publishes sampled committed framebuffers;
  `hg_opengl_host --watch-display` shows them in a persistent OpenGL window.
  Keep the user able to see startup tests; display meaningful captured frames
  in conversation too. Previewing must not flush or otherwise mutate guest draws.
- Connected device service executes submitted GS draws after VIF/GIF pumping;
  FINISH is an ordering/completion event, not a prerequisite for drawing.
  Completed host transfers retain measured surplus-payload state separately
  from uninitialized/cancelled transfers; limitations are in `docs/ORACLE.md`.
- `tools/hg.py inspect`: own-decoder instruction/caller views of external native snapshots.
- `tools/hg.py assets`: read-only CVM/ECMA-119 index and streaming asset hashes.
- `tools/hg.py modules`: IRX import/export/relocation inventory and build-time relocation dry-run.
- `config/iop_modules.toml` / `tools/hg.py iop-plan`: checked IOP placement, manual roots and import bindings.
- `tools/hgtool/iop_source.py`: verifies standalone modules, ROMDIR members, or bounded embedded regions and container identities.
- `tools/hg.py iop-emit`: separate MIPS-I AOT path; `runtime/iop_*.cpp` supplies checked module loading/diagnostics.
- `tools/hg.py iop-audit` / `iop-bundle`: all-module discovery audit and combined static IOP builds.
- `tools/hg.py reboot`: read-only inventory of embedded modules in the game's IOPRP image.
- `tests/`: synthetic, redistributable instruction/ELF tests; no game bytes.
- `out/`: ignored local reports and game-derived generated C++.
- `build/`: ignored native build products.
- `Haunting Ground (USA)/`: user-provided dump; read-only inputs.
- `emu/`: user-provided emulator; memory oracle only; observations in `docs/ORACLE.md`.

## Completion criteria
A playable build requires verified EE/VU translations, kernel/IOP services,
DMA/VIF/GIF/GS rendering, audio, input, saves and asset streaming. A diagnostic
host or clear-color OpenGL window is not a playable recompilation.
