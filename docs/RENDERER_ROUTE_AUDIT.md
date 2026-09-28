> Final control: restored production measured13.49206FPS with matching captures. The prepared-coefficient candidate13.49534 was flat; the coarse worker10.68711 was slower. Both are removed.30FPS remains unmet.

## 2026-09-27 CPU construction gate closes standalone native render targets

HG-DIAG-084 partitioned the31M..33M heavy window after the ordered worker and GPU elapsed gates. Exact non-overlapping GL CPU stages totaled453.062ms in5.16963s (8.76%): sprite admission/packing34.267ms, triangle admission/binning41.572ms, upload/payload/dispatch193.563ms and83.7MiB materialization183.660ms. Complete packet decode/state/draw assembly extrapolated1.479s but is inclusive of some materialization. Deleting every measured GL stage would yield only12.72 original boundaries/s in that sample. Even deliberately double-counting the entire inclusive packet estimate as independently removable gives an impossible18.53/s ceiling, still below30. The replay reached864 original writers with exact retained frame/EE/GS-VRAM/state/VU/normalized-IOP captures, then every probe source was restored.

Therefore do not implement a standalone GPU-native render-target rewrite from this audit. It cannot close the target and carries the largest visual/order/readback validation cost. The renderer remains eligible only as part of a future multi-bucket design justified by new evidence; next work returns to a broad AOT VU execution mechanism rather than more renderer-local batching, readback or packet-worker variants.

# Renderer FPS route audit — 2026-09-26

## 2026-09-27 revised whole-frame ceiling

The exact-restored clean build qualifies at13.14495157FPS (60 original frame boundaries/4.56449s, verifier3a909f8c/evidence79b12154);30FPS remains unmet. HG-DIAG-079 estimates all nonzero-register VU VF readiness checks at~0.416s/31M..33M before probe overhead, with96.535% zero stalls. This shared helper is too small to fix the rate independently. HG-FAIL-022 already counted23,399 CLUT loads with pending work and tested barrier deferral, which regressed badly despite matching state; the proposed repeat census078 was cancelled. Do not spend another build on that identical gate. With GIF~0.741s and raster~0.506s from049, even impossible elimination of all measured renderer-side work cannot meet the~2s required for60 frames at30FPS; VU and ordered graphics work must both change structurally. Cross-run scope sums are approximate, not a guaranteed bound. Any successor worker must prove source/palette/draw fault and readback ordering before asynchronous CLUT/IMAGE packets, and must remove most of065's52,144 CLUT joins without adding equivalent bookkeeping. The current build is clean, with no active probe or worker.

## 2026-09-27 IMAGE CPU ceiling and worker proof gate

HG-DIAG-077 sampled completed IMAGE qword application by destination format in the original heavy window:~0.246s PSMT8,~0.072s PSMT4,~0.009s CT32,~0.326s total nested in complete GIF. Its probe was removed and the clean game rebuilt. This rejects a bulk IMAGE converter as the main path to30FPS even if it could remove all measured conversion time; the clean13.11FPS metric would rise only to~14.1FPS under that impossible bound. Historical direct one-tag IMAGE streaming and incremental packet-completion parsing also passed fidelity but were flat, so do not repeat unchanged parser/materialization work.

The earlier coarse VIF-producer/GS-consumer worker proved exact final captures but lost to56,877 per-packet joins. Its conservative scheduler rejected every IMAGE and TEX0/TEX2 CLUT packet. A successor needs a new safety contract, not just another queue: track a producer-side shadow of the few GS control fields needed to validate packet faults, prove immutable IMAGE payload and transfer bounds before enqueue, and prove CLUT source words defined/stable against every earlier queued write and pending draw. Any uncertainty must join and run the original checked GS path at the same observation boundary. Before reintroducing a worker, measure how many of the52,144 CLUT writes and IMAGE packets satisfy those proofs in the original heavy trace; if enough joins remain to erase overlap, reject the architecture before changing runtime execution. The renderer-private shadow must materialize original CLUT/VRAM state for CPU palette consumers, guest reads and faults. No copied external renderer code, altered guest clocks or skipped draws.

## 2026-09-27 update: measured ceiling and CLUT ownership gate

HG-DIAG-073 measured the dominant original program1 VU loop at~0.667s/31M..33M, including~0.280s fused transform; HG-DIAG-074 measured all13 completed static VU program bodies at~1.861s, matching HG-DIAG-049's exclusive~1.908s. The other~1.157s VU cost is spread across12 programs, no single one above~0.24s. Complete non-graphics host scopes in HG-DIAG-076 measured EE~0.494s, IOP~0.186s and SIF~0.054s in4.459s wall. HG-DIAG-075 showed sparse slice sampling misses bursts and must not rank phases. All probes were removed and the clean executable rebuilt. Current qualified clean gameplay remains~12.6–13.7FPS under host drift;30FPS unmet.

A CLUT-only GPU-resident rewrite is not a demonstrated shortcut. `GsRegisterState::write_ad` synchronously calls `load_clut_from_tex0` for TEX0/TEX2; that operation first rasterizes older draws, materializes VRAM, reads source pixels, updates512 halfwords/valid bits and compare-register state, and may fault before the write completes. `runtime/gs.cpp` consumes those halfwords during CPU texel sampling, sprite palette capture and triangle admission; `runtime/gl_gs.cpp` copies palette values into queued GPU jobs. Host scanout and tests also observe materialized GS state. Any worker or GPU-private palette must keep an ordered source version and materialize it before these consumers, guest reads and explicit faults. Heavy GPU readback was previously only~0.154s, so removing that alone cannot explain a jump to30FPS. The56,877 joins in HG-DIAG-065 cannot simply be omitted:~52,144 TEX0/TEX2 writes and6,739,200 IMAGE transfers occurred in its synchronous packet class. This source-level inventory makes a CLUT-only redesign a large correctness project with uncertain host saving, not the lowest-cost next experiment.

The credible throughput route must remove or overlap multiple measured categories: VU~1.9s, complete GIF~0.74s and raster~0.51s in the heavy interval. Before another worker, prove which complete packet classes can publish immutable data and faults without per-CLUT/per-transfer joins, and bound their copied bytes and guest observation points. Keep strict readback, visual and fault comparisons against the original game and frozen v1. Do not alter guest clocks or skip rendering behavior to meet the metric.

## Measured follow-up: proposed worker rejected

HG-DIAG-064 confirmed only60 heavy VIF drains. HG-DIAG-065 implemented the coarse producer/consumer, retained GS/GL ownership, and passed synthetic event/fault ordering plus exact original frame and guest-state captures. It measured10.68711FPS versus nearby synchronous13.16456/13.31100, so it was removed. The explanation census found95,409 asynchronous packets but56,877 mandatory packet joins, primarily CLUT/transfer work. Coarse launches alone did not deliver useful throughput.

The recommendation below is the pre-experiment audit, superseded by this result. Do not repeat that implementation or treat it as the next recommended optimization. Future overlap requires a different mechanism to preserve original faults/events with fewer joins.30FPS has not been achieved. See PROGRESS and HG-FAIL-044 for exact evidence and archives.

## Original decision

The best next investment supported by this scan is a bounded feasibility check for **coarse ordered GIF/GS consumer threading**, reusing the existing raster calculations. This is a candidate, not a demonstrated improvement or proven optimum. Establish whether guest-visible event boundaries permit useful overlap before implementing a worker.

Scope: project Markdown, active GIF/VIF/GS frontend, compute renderer/shaders, memory coherence, presentation, and recorded trials. No external renderer implementation consulted; no runtime edits, new probes or benchmarks. Frozen v1 untouched.

## Measured basis

HG-DIAG-049's exclusive heavy profile recorded 4.6623 s instrumented wall: VU 1.9078 s, complete GIF application 0.7406 s, pending raster work 0.5058 s, DIRECT 0.0702 s, UNPACK 0.1263 s, MPG 0.0014 s, other/overhead 1.3102 s. GIF and raster are disjoint categories here. Their combined 1.2464 s is a potential overlap allowance, not guaranteed hideable work. Ideal subtraction gives roughly 1.36x solely as an illustration of scale, not a forecast or current-build ceiling. Renderer-only 30 FPS is unsupported.

Latest retained HG-DIAG-061 qualified typical FPS: about 13.39–13.42, with adjacent disabled result 12.96 and a slower outlier separately recorded. No new measurement in this audit. Retained executable SHA256: b441802986229169860a9792093e0f748a34f18ec2d1aeae4475ab5e83a99331.

HG-DIAG-043 explains the apparent triangle fallback gap: of 602162 heavy calls, 337905 were empty; 264257 had a sample, 245616 reached GPU, and 18641 fell back (18378 feedback, 263 format). Raw call-minus-GPU counts are not CPU fill cost.

Later heavy readback and backend-flush host durations were about 154.0 ms and 154.8 ms. Do not add these overlapping scopes to the exclusive raster category or confuse them with GPU elapsed time. Current source lacks GPU elapsed-query evidence sufficient to establish the GPU critical path.

Evidence: docs/DIAGNOSTICS.md HG-DIAG-043/048/049/057/058/061, docs/PERFORMANCE.md, dated docs/PROGRESS.md records. HG-DIAG-049 replay: faffeec91c8344d08a14db75e96ce3d6. Profiles predate the current retained build and include instrumentation.

## Source findings

- runtime/system_diagnostic.cpp:1448–1461 drains VIF, GIF and pending raster work synchronously.
- runtime/include/hg/vif.hpp:665 onward submits XGKICK from mutable VU memory. Deferred consumption requires immutable packet bytes captured before reuse.
- runtime/gs.cpp:22 and :109 apply complete packets and registers in order. Preserve existing PATH1/2/3 order.
- runtime/include/hg/runtime.hpp:242, :293, :313–314 and :781 checks GS interrupts after submissions. This is the main feasibility hazard: joining after every qword destroys overlap; removing or delaying these checks is unacceptable.
- runtime/include/hg/runtime.hpp:664 and :707 exposes privileged GS reads/writes. CSR/IMR, SIGNAL/FINISH/LABEL, BUSDIR, transfer reads, DMA/VIF/GIF status, interrupts and explicit faults require an observation/ordering contract.
- runtime/include/hg/gs.hpp:369–382 snapshots the full 128-register environment per draw; runtime/gs.cpp:1491–1495 restores it around execution. Visible apparent redundancy is not measured proof for another cache experiment.
- runtime/include/hg/gs.hpp:705–748 drains old draws before CLUT source reads/mutation. Preserve the ordering. It may stay inside the consumer, subject to guest observation and fault proof.
- runtime/gl_gs.cpp:207–253 materializes GPU-owned pages for CPU consumers and enforces queued read/write hazards. These semantics cannot be skipped.
- runtime/gl_gs.cpp:332–372 uploads and dispatches; sprites dispatch/barrier per command. Prior batching trials already address this apparent opportunity.
- runtime/gl_gs.cpp:460–519 admits triangles, packs jobs/palettes and bins 16x16 tiles. runtime/gl_triangle_shader.hpp:80–123 manually performs coverage, interpolation, texture, depth and blending against packed GS memory.
- runtime/gs.cpp:1283–1403 checks samples/admission and preserves feedback fallback. Empty primitives are a large share; retain the proven same-pixel CPU feedback optimization.
- runtime/game_host.cpp:137–235 handles the CPU-image presentation bridge and GL context arrangement. Keep fresh/refresh/resize redraw gating and F1 controls. A new consumer needs exclusive GL ownership.

## Ranked routes

1. Gate coarse GIF/GS overlap first. It targets substantial measured host work without new pixel mathematics. Main risks: shared state, event and fault timing, transfers and frequent joins.
2. GPU-native render targets/hardware rasterization: longer-term structural option, substantially larger implementation and validation cost. Coverage, blending, depth, format reinterpretation, feedback and readback must survive. No measured basis yet to call a full rewrite the cheapest route.
3. Further local cache/batching/readback tuning: low priority absent new evidence or a materially different mechanism.

A failed sprite fragment-interlock trial does not disprove every native render-target architecture. Conversely, fewer dispatches or uploaded bytes do not establish better FPS.

## Closed routes: do not repeat unchanged

Recorded palette payload/cache reductions achieved substantial reuse and fewer batches without repeatable FPS gains. Sprite tile lists, fragment interlock, hazard-free waves and CT32 specialization were flat/slower. Persistent mapped/coalesced/narrower readbacks, deferred flush and direct scanout variants were flat/slower or correctness-rejected. GPU same-pixel feedback was exact but slower. Float coverage/Gouraud/Z experiments failed; distinguish these from retained limited native STQ work.

Fine-grained VIF feeder threading produced roughly 126180 handoffs for only about 0.13–0.18 s of UNPACK overlap and slowed execution. A GS consumer must target different work and amortize it in coarse chunks. Generic GIF streaming, identity caches, no-op pump gates and CLUT reload elision are also closed recorded trials.

Older proposals in docs/ROOT_CAUSE_AUDIT.md are historical leads, superseded where later trials rejected them.

## Bounded next action and acceptance gate

1. Build a source-level table of GS observations and fault boundaries. Prove where immutable complete-packet chunks can overlap VU work inside existing drain intervals while publishing events at original guest-visible boundaries. Do not assume whole frames are independent.
2. Only if static inspection leaves a decisive uncertainty, add one opt-in, inventoried probe counting eligible work/bytes and required joins. Preserve event checks and guest clocks; keep disabled overhead negligible. Use the existing fixed heavy capture. Reject the route if joins, copies and queue overhead plausibly consume the available overlap.
3. Only after that gate, prototype one persistent consumer, a single ordered queue and bounded preallocated chunks. Give it exclusive GS/GL ownership. Join before proven observations, shutdown and original fault-observation boundaries. Avoid per-qword jobs and thread creation.
4. Run existing 1177 GPU/CPU cases per mode and queued dependency cases, fixed multi-frame captures, relevant guest state/event/transfer/fault comparisons, and repeated adjacent baseline/candidate FPS runs with unchanged frame-writer metric and build provenance.
5. Retain only repeatable gains beyond observed run variation with every correctness gate passing. Otherwise revert and record the reason.

If considering a hardware-rasterization rewrite later, collect asynchronous GPU elapsed timings at batch boundaries first; no blocking immediate query reads or glFinish in the measured path.

Reuse of current raster math minimizes new visual risk but does not prove the existing renderer is physically PS2-identical. Preserve resolution, filtering, blending, depth, feedback, guest-visible materialization and explicit unsupported-state faults. No full PS2 parity, playability or cross-platform claim.
