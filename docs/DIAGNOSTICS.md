# Diagnostic and experimental-code inventory

Current audit: 2026-09-13. Stable identifiers make these items searchable and
accountable during optimization and handoff. This inventory covers the identified
active families; it does not certify that every historical diagnostic is removed.
Correctness validation and explicit unsupported-hardware faults are required
runtime behavior, not debug code to delete for speed.

| ID | Source / purpose | Activation and state effects | Cost / disposition |
|---|---|---|---|
| HG-DIAG-001 | `runtime.hpp::trace_pc`, EE emitter: instruction/register history | HG_EE_INSTRUCTION_TRACE defaults ON; history records only with EE watches. OFF removes calls and rejects PC watches explicitly. No guest writes. | Native profile39:161/3254 samples in the function with no watches. Combined candidate40 (call-site guard plus forced register inlining) preserved state but slowed playback and was reverted. Isolated OFF replay42 takes50.2381s vs51.8379s37 with matching guest times, EE/GS/machine state and1311images. Synthetic branch/watch/fault checks pass. Retain opt-in traces. |
| HG-DIAG-002 | `iop.hpp::trace_pc`, IOP emitter: recent instructions and SIFMAN submission/return captures | Currently every IOP instruction, including no-watch runs. Host history only; downstream diagnostic reporting consumes it. | Profile39:50/3254 samples. Still active. Audit consumers before gating; retain explicit guest load hazards and faults. |
| HG-DIAG-003 | `runtime.hpp::store`: fixed EE write watch at0198cbc8 | Always checks overlap; records writes to the startup semaphore word. No substituted value. | Cost not isolated; scalar/quad RAM paths preserve watch behavior. Replace with an explicit diagnostic option after startup regressions no longer require its history. |
| HG-DIAG-004 | `system_diagnostic.cpp`: SIF/RPC/worker histories, `iop_dmac.hpp` histories | Automatic startup provenance; several host scans occur each quantum. Reads original state, records host history. | Dense profile35 isolates `diagnostic_history`; native profile39 is available. Still active; gate only after tracing all consumers and comparing guest state. |
| HG-DIAG-005 | `diagnostic_profile.hpp`, system diagnostic phase marks and CSC/disc event output | `--profile`, default off. Host timestamps, sampled PC costs and event logs; no guest time substitution. | Sampling1/256,11phases. Needed for current evidence; compare runs with identical options. Disable for final presentation measurements and report that setting. |
| HG-DIAG-006 | `system_diagnostic.cpp` preview/dumps and external replay helpers | Explicit `--preview-file`, `--dump-iop`, input file and checkpoint options. Preview copies committed scanout; dumps are diagnostic artifacts. Input scripts intentionally operate controls. | Filesystem/image sampling adds host cost; no renderer flush allowed. Keep visible preview during development and record capture settings. External helpers retain short input pulses. |
| HG-DIAG-007 | `system_diagnostic.cpp --toc-record`, IOP TOC input | Explicit external TOC record. Current replay helpers supply a synthetic index-pattern record. **Guest-visible experimental input.** | Quarantined startup probe, not physical-drive correctness. Never promote this input to a normal production default; replace only with independently verified disc behavior. |
| HG-DIAG-008 | `execution_clock.hpp`, `--clock-profile issue-slots`, worker budgets | Explicit experimental clock profile; legacy is default. **Changes guest work/time allocation**, including bounded polling yields and DMA pacing. | Not a PS2 cycle/cache model. Preserve and disclose exact profile in comparisons; retire the approximation only with independent timing evidence. |
| HG-DIAG-009 | `iop.hpp` event/CDVD/RAM traces; kernel semaphore/SIF histories | Event-driven bounded history, some default-on, optional address/PC watches. Host records of native services. | Aggregate cost not isolated. Keep until consumers audited; new instrumentation must state its activation and cleanup criterion. |
| HG-DIAG-010 | External native sampler and ignored Release linker map | Explicit profile-only replay39: bounded suspend/GetThreadContext/resume sampling for five seconds. No guest-memory edits. |3254 samples,0context errors. Perturbs host timing; never use this replay as a speed comparison. Map enabled only in ignored vcxproj; regeneration removes it. |

The current successful opening still runs in a diagnostic host. Synthetic TOC
input and experimental timing are meaningful limitations, not evidence that
missing translated instructions are being silently skipped. The fresh complete
scan and unresolved-site classification are in `DECODING_SCAN.md`.

Rejected experiments: sparse IDCT26, alternate bit reader34 and byte-copy
batching36, combined trace/inlining40 and framebuffer-address41 showed no connected speed benefit and were reverted. Their external
evidence remains for accounting; do not reintroduce them without new evidence.
