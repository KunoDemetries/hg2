# Haunting Ground throughput root-cause audit — 2026-09-22

## Scope

This audit was requested after a long series of correct but flat/marginal micro-optimizations. It is read from independent Sony-manual notes already recorded in SOURCES/ORACLE, captured Haunting Ground DMA/RAM evidence, current first-party runtime/AOT source, and retained timing diagnostics. PCSX2 remains an external oracle only; no emulator implementation is a runtime dependency or source of copied algorithms.

Qualified heavy gameplay remains the original 0x001bef84 completed-frame boundary, final two gameplay33m intervals. Recent restored production samples drift roughly 10.4–11.2 FPS, so sub-percent differences are not treated as gains.30FPS is unmet.

## Main finding

The largest structural restraint is the VIF1/VU1/GIF graphics pipeline.

Sony's VIF1 double-buffer design exists specifically so VIF can write one VU1 data buffer while VU1 executes from the other. MSCAL/MSCNT activate the microprogram and switch TOP/TOPS/DBF; FLUSHE/FLUSH/FLUSHA are the explicit synchronization commands. VU1 XGKICK starts PATH1 transfer and subsequent VU instructions may continue until a later transfer/resource conflict requires a stall.

Current runtime collapses those hardware producer/consumer stages into synchronous nested host calls:

- runtime/system_diagnostic.cpp drains the VIF1 DMA channel completely with while(ee.pump_vif1()){}, then drains GIF, then rasterizes GS work, then services IOP.
- Vif1Path::activate_mscal/activate_mscnt call the complete build-time VU executor inline.
- Vif1Path::xgkick walks the entire VU-memory GIF packet and submits it synchronously before VU execution resumes.
- VIF DIRECT submits PATH2 qwords synchronously into the same GIF/GS path.

The game is not merely compatible with double buffering; the captured heavy-scene DMA chain proves it actively uses it:
- source chain at0x004f1ca0 NEXT ->0x005f3560 CALL ->0x003a7530.
- tag-transfer VIFcodes at0x003a7530 are BASE0 and OFFSET0x200, establishing two 0x200-vector (8KiB) VU1 data buffers.
- 0x005f36d0 transfers FLG=1/TOPS-relative UNPACK.
- 0x005f3760 transfers MSCAL.
- nested chain0x006f1ff0 contains repeated FLG=1 UNPACK tags.
- 0x006f2040,0x006f20a0,0x006f2100 transfer MSCNT between further TOPS-relative UNPACK groups.
This is the manual's double-buffer producer/consumer pattern in the actual recorded heavy scene.

Original EE code also contains explicit hardware synchronization rather than assuming everything completes immediately. Helper0x0010c9a0 polls VIF1 CHCR0x10009000, GIF CHCR0x1000a000, VIF1 STAT0x10003c00, CFC2 VU status, and GIF status0x10003020. Thus a future concurrent model must preserve those guest-visible waits/fences; it must not simply let work run freely.

## Measured cost alignment

Retained diagnostics point to the same subsystem:
- HG-DIAG-032 heavy31M..33M: ~4.861s loop, VIF service~3.585s, direct GIF service~0.040s, explicit raster~0.306s. Nested GS work means these are attribution, not additive phases.
- HG-DIAG-034:60 heavy VIF bursts~4.682s inclusive. MSCNT 91,140 calls~2.560s; MSCAL35,040~0.0058s; DIRECT61,146 commands/7,306,814 qwords~1.608s; completed UNPACK~0.176s; MPG~0.0016s. Nested VU/GIF/GS costs overlap.
- HG-DIAG-033: heavy worker CPU time accounts for~97% of unpaced wall time. This is primarily host CPU work, not sleep/wait.
- HG-DIAG-037 native sample: hot symbols are dispersed through VU arithmetic, Vif1Path::process_pending, generated VU bodies, conversion helpers and FPU helpers rather than one GPU driver stall.
- Recovered GPU flush/readback timings are secondary compared with VIF/VU CPU work; multiple readback/API experiments remained flat.

## AOT VU compiler finding

The VU path is build-time decoded but still executes a runtime software scoreboard almost pair-by-pair.

The13 compiled program images contain 95,83,276,304,336,340,368,286,292,474,115,294,219 instruction pairs. Generated translated.cpp contains3477 calls to v.vu1.advance_pipeline(1), essentially one per reachable pair. Each pair may also access vf_ready/vi_ready through require_* and produced_* bookkeeping.

The linked map shows the generated VU runners occupy on the order of megabytes of native text for only ~3.5k static pairs. Example: program8 is a292-pair program and its linked run body spans roughly hundreds of KiB before the next program symbol. The exact code-layout boundaries are linker ordering, not a per-instruction cost proof, but the scale plus prior code-bloat regressions makes instruction-cache pressure a credible contributor.

Existing accepted build-time VF readiness proof already demonstrated that moving pipeline knowledge from runtime to emit time can help (~2.6% in its original A/B). Prior no-Q/no-P advance_pipeline shortcut was flat because it still performed a runtime state update/check per pair. Full static operand templating regressed due code-size expansion. Therefore the next candidate must remove whole stretches of scoreboard traffic without multiplying code bodies.

## Evidence matrix

| PS2 subsystem | Hardware/manual behavior | Current implementation | Heavy-scene evidence | Priority |
|---|---|---|---|---|
| EE <-> VIF1 DMA | independent DMA producer; explicit status/CHCR waits | EE service, then complete VIF drain | original game polls VIF CHCR only at explicit fences | high |
| VIF1 <-> VU1 | BASE/OFFSET/TOPS double buffer permits concurrent VIF writes and VU execution | MSCAL/MSCNT execute whole VU function inline in parser | captured BASE0/OFFSET0x200 + FLG1 UNPACK/MSCAL/MSCNT;~2.56s MSCNT attribution | critical |
| VU1 <-> GIF PATH1 | XGKICK starts transfer; VU may continue until later resource conflict | whole EOP packet drained synchronously inside xgkick |~91k XGKICK calls in retained diagnostic | high |
| PATH1/2/3 arbitration | hardware GIF arbitrates paths deterministically | one serial GifPath receives nested calls | DIRECT~1.61s inclusive; must preserve path ordering before host parallelism | high |
| VU pipeline scoreboard | hardware pipeline latency; static microprogram known AOT | runtime advance/require/produced state almost every pair |3477 static advance calls; VU/FPU symbols dominate CPU sample | critical |
| GS raster/readback | GS operates independently; FINISH/readback are sync points | queued compute backend but CPU coherence can sync | explicit raster~0.3s heavy; full-run readback~1s; API variants flat | medium |
| IPU | coprocessor/DMA can progress independently | bounded per-quantum service | heavy gameplay IPU largely inactive | low for current scene |
| IOP/SIF | separate processor/transport | serviced later in same outer worker loop | important for fidelity/audio but not dominant current heavy profile | low/medium |

## First implementation candidate after audit

Before attempting host multithreading, remove avoidable runtime VU-pipeline bookkeeping while preserving the existing single-thread ordering.

Candidate: one function-level scheduled-clock accumulator in each generated VU runner.

- Keep a local uint64 issue cycle in a host register.
- For Q/P-neutral microinstruction pairs, advance the local cycle instead of calling Vu1State::advance_pipeline per pair.
- Readiness checks and produced timestamps use the local cycle but the same vf_ready/vi_ready arrays and exact unsigned comparisons/latencies.
- Defer Q/P countdown materialization until a Q/P-observing/producing instruction (DIV/WAITQ/ERLENG/WAITP/MFP/Q arithmetic) or function exit.
- Because no instruction in a neutral span observes Q/P, applying the total elapsed-cycle decrement at the boundary is architecturally equivalent for final/inter-boundary state.
- A small RAII commit guard must materialize elapsed cycle/Q/P state during exception unwinding so explicit operation faults see the same pipeline state as the current implementation.
- Dynamic Q/P pairs continue through the existing exact _pair_lines route after syncing local time, then reload the accumulator.
- Do not duplicate template bodies or change arithmetic, branch, XGKICK, VIF, GIF, GS, input, clocks or fault policy.

Current generated program has only43 DIV,50 WAITQ emissions (including DIV pre-waits),47 ERLENG,20 WAITP,53 MFP and134 Q-arithmetic operations versus3477 per-pair pipeline advances, so most static pairs are candidates for accumulator scheduling.

If this candidate is exact and materially faster, retain it and then tackle real VIF/VU/PATH concurrency. If it is flat, restore it and move directly to deterministic device overlap rather than further arithmetic micro-tweaks.
