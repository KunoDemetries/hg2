## HG-DIAG-088 exact XYZ lane ADD/SUB SIMD reuse - restored, performance not yet retained

The bounded production candidate routed only mask14 XYZ `add_vector` operations through a conservative helper that reused exact `try_fpu_add4`; invalid indices, undefined XYZ sources and unsupported host arithmetic returned before mutation to the original scalar path. Candidate build/tests passed, fixed33M replay reached864 original writers with zero unexpected/lost/caller failures, and verifier `c44e1685cc864a1c970a0c69e93a2ce5` preserved every retained frame/EE/GS/VU/RTC-normalized IOP digest at10.9755428FPS (10.8044644/11.1521262).

The candidate was byte-restored after the first measurement. Clean executable `b5beb2db...` rebuilt with151-input provenance and adjacent verifier `d3563b0ccd0d486b81f3070203e07d0f` preserved the same state/864 writers at10.4785007FPS (10.2415644/10.7266597). Candidate is ~4.74% faster versus this adjacent control, but severe host drift moved both far below the earlier clean13.5107444FPS sample. Treat088 as inconclusive-positive and unretained; production contains no088 code. Revisit only by replaying the exact archived candidate against another close restored control, never by comparing to the nonadjacent13.51 sample. `v1/` was untouched.

## HG-DIAG-087 sampled completed VU family timing - completed and removed

Temporary host-only attribution sampled approximately1/256 completed VU helper calls only in the fixed31M..33M unpaced heavy window. Replay `4a62cd52f5f0434d87b39997bc67c201` / evidence `project-link-runtime/20260928T005828Z-209d39f8ed404158bb9908c50a07d255` reached33M with864 original writers and zero unexpected/lost/caller failures. Fixed verifier `0d6dc8ec257742eab8a80cc346ad7818` / `project-link-verifier/b762cfe2c1014d368f8d498415ad8e4a` preserved exact retained frame/EE/GS/VU/RTC-normalized IOP state. Instrumented FPS is invalid.

Raw256x sampled estimates ranked convert366.82ms, Q arithmetic269.88ms, broadcast MUL242.07ms, ACC MUL/MADD169.68ms and lane ADD/SUB165.68ms, but per-sample timing exposed a ~36ns timer/scope floor: convert35.91ns and Q arithmetic36.20ns sit at that floor, so their raw totals substantially overstate removable work. Lane ADD/SUB was55.69ns/sample across2,974,020 calls and EFU79.66ns/sample across759,600 calls; lane ADD/SUB therefore has the stronger broad next-target signal. Aggregate evidence is `vu-family-sampled-timing-20260927/summary.json` SHA `50207849...`. All three profiling files were restored byte-exact; clean build `7a02d9ad2e024f80b18bd41d8d41cdef` produced `10e73dff...`, and verifier `ca3a899a53754af284188c4a898dec4f` requalified exact state/864 writers at13.5107444FPS (13.4805408/13.5410836). Active087 cost is zero.

## HG-DIAG-086 contiguous XYZ matrix-chain fusion - rejected and removed

Emitter-only AOT specialization for the measured non-hot VU XYZ arithmetic. External census `vu-xyz-chain-census-20260927/summary.json` found86 contiguous equal-frequency `mulax+madday+maddaz[+maddw]` chains across Programs2-12 covering4,115,520 dynamic XYZ upper operations; the candidate admitted only proved full four-stage mask14 chains with stable future VF sources. Runtime arithmetic reused existing exact `prepare_vu_matrix`/`commit_vu_matrix`; preparation was state-pure and failure fell through before mutation to the unchanged original per-pair route.

Correctness passed: Python/build/runtime/translation plus262,144 fused-transform arithmetic checks and98,304 compiled full-state/fault comparisons with14,044 actual matrix fast-path hits. Candidate `5ce827b5...` completed the original33M replay and fixed verifier `adf168cab7014be38c77dbcb7fd45043` with864 writers, zero validation failures and exact retained frame/EE/GS/VU/RTC-normalized IOP state at13.5823737FPS (13.5509312/13.6139625). Exact-restored adjacent control `92904fe0...`, verifier `5472728938c24884a87ad7b51f684c6b`, preserved the same state at13.6538654FPS (13.6529333/13.6547976), making the candidate ~0.52% slower. All emitter/test edits were byte-restored, VU output regenerated and ordinary game rebuilt; active086 cost is zero. Preserve the census/prototype evidence but do not repeat the same state-pure prepare/four-commit chain unchanged. `v1/` remained untouched.

## HG-DIAG-085 MSVC profile-guided native build experiment - rejected and removed

The experiment tested compiler-only layout/inlining on the measured diffuse VU/runtime cost. Broad first-party `/GL` plus exact profiling was impractically slow; selective generated-main instrumentation was unstable (`0xC0000005` before13M), while helper-only `hg_vu_products` instrumentation completed the exact33M route. The valid helper-only training path used the fixed12-event original recording, exactly864 original0x001bef84 writers, zero trace/caller/stack failures and exact retained frame/EE/GS/VU plus RTC-normalized IOP state. One fresh11,040-byte PGC was explicitly merged into its exact-link PGD with `pgomgr` before `/USEPROFILE`; the USE linker reported65/65 profiled helper functions optimized.

USE executable `cf904288...` passed Python/runtime/translation/readiness, fixed replay and two exact-state frame qualifications:13.3542847FPS (13.2998178/13.4091996) and13.2833878FPS (13.2837995/13.2829761). Adjacent ordinary OFF `5fc22723...` qualified13.4984342FPS (13.4621512/13.5349133), so both candidates were slower and all four candidate heavy subwindows lost. The >=5% retention gate failed. All PGO-only CMake, provenance and synthetic-test plumbing was removed; the temporary training wrapper/profile databases remain external evidence only. Final restored ordinary executable `93de29f8...` passes132 Python tests, runtime/translation/VU-readiness and fixed verifier `dad485f3619d40c48bf17dafcc5ba957` at13.7931352FPS (13.7588171/13.8276248), with151-input provenance,864 writers and all retained captures exact. Active diagnostic/runtime cost is zero; do not repeat this PGO surface unchanged.

## HG-DIAG-084 CPU GS construction census - completed and removed

Private aggregate-only instrumentation in `runtime/gs_cpu_profile.hpp`, `runtime/gs.cpp`, `runtime/gl_gs.cpp` and `runtime/system_diagnostic.cpp`. It arms only for the existing unpaced `--profile` path at guest slice31,000,000 and measures through the33M budget. Complete GIF packet application is sampled1/64 and reported as an inclusive, non-additive parser/state/draw-assembly estimate. GL backend stages use exact non-overlapping scopes for sprite/triangle admission and queue preparation, VRAM upload, payload/tile-list construction, dispatch/barriers and mandatory CPU materialization; the existing per-primitive inclusive raster counters reset at the same boundary. Output contains aggregate calls/times/word or byte totals only. No game-derived packet, palette, texture or VRAM payload is persisted.

Activation/default: default inactive; unavailable under paced gameplay and inactive before31M even with `--profile`. Guest-state effects: none observed; no clocks, input, packet ordering, GS state, pixels, observations or faults changed. Instrumented job `ed2f2f19fe874ac29ea8f04a1a19159e` reached33M, all864 original writer events and no fault. Frame, EE, GS VRAM/state, VU and RTC-normalized IOP captures match the established control exactly. Aggregate evidence is `%TEMP%/hg-diag084-cpu-gs-20260927` and `project-link-runtime/20260927T213702Z-75cf0fb810c346dfbf05df0e772925ea`.

The two heavy intervals totaled5.16963s. Exact non-overlapping GL CPU stages totaled453,062,000ns (8.76%): sprite analysis/pack/queue34.2672ms, triangle analysis/queue41.5715ms, flush upload/payload-tile/dispatch193.5634ms, and83,722,240-byte materialization183.6599ms. Complete packet application sampled2,384 of152,526 packets and extrapolated1.479264s inclusive; it overlaps some materialization and must not be added as an exact partition. Even the impossible deletion of every measured GL CPU stage reaches only12.72 original boundaries/s for this sample. Double-counting the inclusive packet estimate as also fully removable gives an intentionally over-optimistic18.53/s ceiling, still below30. Therefore a GPU-native render-target rewrite alone fails the feasibility gate; do not prototype this route unchanged.

All three modified sources were restored byte-exact to hashes `050855fa...`, `f40ac711...`, and `12d9ef7c...`; the private header was removed. Active timing cost is zero. Preserve only the external aggregate evidence and this record. Instrumented rate is not qualification evidence.

## HG-DIAG-083 nonblocking GPU elapsed census - completed and removed

Temporary OpenGL `GL_TIME_ELAPSED` query-ring instrumentation surrounded accepted sprite, triangle and masked-write dispatches in `runtime/gl_gs.cpp`; narrow counters in `runtime/include/hg/gs_acceleration.hpp` and heavy-window arming/reporting in `runtime/system_diagnostic.cpp` recorded flush and materialization context. It was active only under the existing unpaced `--profile-gs` path from31M..33M. A fixed512-query ring was polled without waiting during guest execution; final `glFinish` and query collection occurred only after the guest loop and timed writer windows. It changed no guest state, clocks, GS ordering, pixels, faults or input. Query submission cost is unknown, so instrumented host rate is not FPS evidence.

The repeat original replay at `%TEMP%/hg-diag083-gpu-elapsed-repeat` reached33M,864 original writers and no fault, with zero skipped queries. GPU elapsed time was703,482,880ns across2,045 batches: sprites295,785,472ns/302 batches, triangles354,574,176ns/843 and masked writes53,123,232ns/900. The same heavy window issued33,780 sprite draws and245,616 triangle draws; CPU flush time was180,787,800ns. Materialization downloaded83,722,240 bytes in97,509,700ns, while only339 of18,761 bulk scopes touched GPU-dirty storage (1,340 pages). The first replay gave the same conclusion (approximately690.75ms GPU time). This proves the GPU is not saturated in the roughly4.8-second heavy host interval and that a readback-only render-target substitution has too little ceiling to close the30FPS gap; a useful GPU-native route must also eliminate or overlap substantial CPU GS packet/draw construction.

All three touched sources were restored byte-exact from `%TEMP%/hg-diag083-preedit-20260927`, timestamps were forced and the game was rebuilt. No083 code remains. Probe-free executable966a3692 matched151-input provenance and final evidence `%TEMP%/hg-diag083-fixed-final-clean` qualified864 original writers, zero validation failures and exact retained frame/EE/GS-VRAM/GS/VU/RTC-normalized IOP state. Its11.99772043FPS sample is host variance, not a retained source regression. Retain only the external aggregate evidence and this record; do not repeat a readback-only render-target experiment unchanged.

## HG-DIAG-082 ordered GS span census and worker prototype - completed and removed

Temporary default-null observation hooks covered complete GIF packets, VIF DIRECT/GIF DMA/VU XGKICK provenance, GS status/privileged/event observations, raster, display, transfer, CLUT and CPU materialization boundaries. Active only for the external unpaced31M..33M census; disabled behavior was one null check at complete-packet boundaries. The private control shadow predicted register/vertex, transfer, CLUT compare/load and event state without rendering or palette-pixel reads. Original replay evidence `%TEMP%/hg-diag082-20260927-retry` reached33M,864 original writer events and no fault. It measured152,526 packets /285,600,224 bytes,100% eligible work,120 spans/joins,4,759,856-byte peak owned storage,7,494,500ns copy time, zero shadow/fault divergence,49,564 CLUT loads,3,240 IMAGE packets and source counts240 GIF DMA/61,146 VIF DIRECT/91,140 XGKICK. This passed the feasibility gate.

The authorized successor used one persistent consumer, fixed32MiB byte ring, fixed command ring, producer preflight shadow and synchronous fault/observation boundaries; the synchronous route remained default. Focused tests covered copied-source mutation, ring wrap/backpressure, CLUT/IMAGE/transfer/surplus/cancel, reset/shutdown, malformed prefixes and first-fault ordering. Two unpaced candidates and an adjacent control were exact on retained non-RTC state, but the fixed realtime verifier is authoritative: candidate executable5a303fff, evidence `%TEMP%/hg-diag082-fixed-candidate1`, qualified864 writers with zero validation failures and exact frame/EE/GS-VRAM/GS/VU/RTC-normalized IOP digests at13.06890581FPS. That is below protected13.12060460FPS, so the retention criterion failed. All082 source, tests, switch and verifier extension were removed; protected sources were restored byte-exact. External census/candidate evidence is retained; packet bytes are outside the repository. Guest clocks and original behavior were unchanged. Do not repeat this same raw-packet worker unchanged; next renderer route is GPU-native render targets, starting with nonblocking GPU elapsed-time evidence.

## HG-DIAG-081 external native CPU sampling - captured, no runtime probe retained

Location: external Visual Studio VSDiagnostics CPUUsageHigh sampling session and external map symbolizer under %TEMP%; no project source probe, no generated-code change, default inactive. Purpose: rank native host PCs during the original gameplay33m replay after WPR policy rejected sampling. Guest-state, clocks, output and fault effects: none intended; high-rate host sampling overhead unknown, so sampled replay throughput is invalid for FPS qualification. Capture: Project Link job38c616eb, PID16112, %TEMP%/hg-vu-native-full-high-20260927.diagsession and hg-vu-native-full-high-dump.txt. Last ETL12.0M..16.5167M us holds18,803 target samples, approximately the slow final interval but without an exact original-slice alignment marker. Top exclusive symbols: VIF process_pending848, XYZ prepare<false>741, exact AVX2 fused transform638, VU arithmetic_q517, multiply_vector429, GIF decode304, GL masked_write283, CPU triangle raster240. The ETL lacked a target loader image; module base was cross-matched to exact instruction PCs from an earlier same-executable trace with an explicit loader event. Do not interpret exclusive samples as inclusive caller cost or calibrated wall seconds. Retention criterion: keep only external evidence and this summary; no profiler running and no source instrumentation retained. Next use only for a broad architecture decision with exact fault/order/readback proof.
## HG-DIAG-080 complete GIF packet class census - completed and removed

Temporary host-only counters in runtime/include/hg/gs.hpp, runtime/gs.cpp and runtime/system_diagnostic.cpp. Default null pointer; armed only unpaced --profile31M..33M. No guest state, clocks, rendering, transfer order, interrupts, pixels, faults or input changes; active counting cost unknown and instrumented rate is not FPS evidence. Original replay7bee2f3b reached33M/864 original writers/no fault. Among152,526 completed packets:95,762 plain,50,224 CLUT-write-only (49,564 actual reload packets),3,240 IMAGE-only and3,300 transfer-control/HWREG-only, zero event-write packets. Payload totals285,600,224 bytes overall and107,879,040 IMAGE packet bytes. This rejects IMAGE-only/no-load-palette worker admission as a route around most joins, without proving all CLUT work must synchronize in a different architecture. Source files byte-restored to manifest %TEMP%/hg-diag080-preedit-20260927/manifest.txt; forced clean rebuild6b44db1b and fixed verifier9e5f8b50 passed at13.12060FPS with established state. Probe archive %TEMP%/hg-diag080-probe-20260927; full evidence in PROGRESS. No080 code remains.
## HG-DIAG-079 completed and removed

Temporary sampler in runtime/include/hg/vif.hpp, definition in runtime/vu_matrix.cpp, arm/report in runtime/system_diagnostic.cpp. Default disabled, active only unpaced --profile31M..33M. No guest-state, clock-model, arithmetic, memory, rendering or fault effects; host timer/branch cost unknown. Original replay c2df25ec reached33M/864 writer events/no fault. Nonzero VF readiness:12,489,360 calls,12,056,580 zero-wait,432,780 stall; one-in256 sampled1,554,700ns zero and69,400ns stall, fixed256x~0.41577s total before overhead. Too small to reach30FPS by eliminating this helper alone. All three source files byte-restored and clean game rebuilt with provenance (a27d9642, game6396dcd3); no079 code remains. See PROGRESS for hashes/backups. Instrumented replay rate is not FPS evidence.

## HG-DIAG-077 completed and removed

Temporary host-only1/64 completed IMAGE qword timing by destination format in runtime/include/hg/gs.hpp, runtime/gs.cpp and runtime/system_diagnostic.cpp; default disabled, armed solely by unpaced `--profile` at31M..33M. No guest-state, clock, transfer ordering, pixel or fault effects. Original replay9db72da2 reached33M/864 writers/no fault. Fixed64x estimate: PSMT8~0.2456576s, PSMT4~0.0715264s, CT32~0.0085568s, all IMAGE~0.3257408s. Sampling overhead unknown and instrumented interval rate is not qualified FPS. All three sources restored byte-exact and clean executable rebuilt with provenance; no077 code remains. Full evidence/hashes in PROGRESS.

## HG-DIAG-076 completed and removed

Temporary host-only full heavy-window phase clocks in runtime/include/hg/diagnostic_profile.hpp, default inactive, armed by unpaced `--profile`. Complete scopes covered EE run/observation, IOP run/I-O, SIF only; no guest-state/clock/ordering/graphics/fault effects. Replay30d4578a reached33M/864 writers/no fault and measured4.4590229s wall,0.4942611s EE,0.1860953s IOP,0.0537283s SIF. Timestamp overhead unknown; instrumented FPS invalid. Header restored byte-exact, clean game rebuilt with provenance. Full jobs/hashes in PROGRESS.

## HG-DIAG-075 completed, uninformative, removed

Temporary host-only31M..33M phase snapshots in runtime/include/hg/diagnostic_profile.hpp; default `--profile` only, no guest-state/clock/fault/rendering effect. Original replay5606cb4f reached33M/864 writer events.1/256 sampling captured7,825 slices, scaled all-phase sum1.3766912s versus4.3229158s heavy wall; bursty work was undersampled by ~2.946s, so do not interpret phase proportions as complete attribution. Instrumented FPS invalid. Header restored byte-exact, clean game rebuilt with provenance. Full job/backup in PROGRESS.

## HG-DIAG-074 completed and removed

Temporary host-only program sampler in runtime/include/hg/vu_matrix.hpp, runtime/vu_matrix.cpp, tools/hgtool/vu_emit.py and runtime/system_diagnostic.cpp; default disabled, armed solely by unpaced `--profile` at31M..33M. It sampled1/16 completed verified AOT VU1 program calls, with no guest-state, clock, ordering, rendering or fault effects. Replaycbaf2bae reached33M/864 original writers/no fault; sampled116,284,000ns across13 programs (~1.860544s fixed16x), of which program1 ~0.703173s and all others ~1.157371s. Host sample cost not isolated; instrumented FPS invalid. Python tests and native build passed. All four source files restored byte-exact; generated source and clean executable rebuilt with provenance. Retain only evidence and exact details in PROGRESS.

## HG-DIAG-073 completed and removed

Sampled host-only timing in runtime/include/hg/vu_matrix.hpp, runtime/vu_matrix.cpp, tools/hgtool/vu_emit.py and runtime/system_diagnostic.cpp; armed only by unpaced `--profile` at31M..33M, default disabled. No guest-state, clock, packet, pixel, ordering or fault effect. Sampled43,722 completed of2,798,220 original fast-loop attempts:10,426,500ns whole-loop,4,380,700ns nested exact transform. Estimated fixed64x host cost0.667296s/0.280365s, with sampling/timer caveats; instrumented FPS invalid. Python tests, generation, build and original33M replay passed; all four source edits restored exactly, generated source and executable rebuilt clean. Retain evidence only. Full jobs and hashes in PROGRESS.

## HG-DIAG-072 rejected and removed; HG-DIAG-073 planned

072 preflight/direct-memory candidate qualified12.91581 and12.60928FPS around adjacent exact-clean12.58608; later candidate is flat. All captured guest/rendering state and864 original writers match. Emitter/test restored byte-exact, generated C++ and game rebuilt clean. No072 runtime code remains; archives/jobs in PROGRESS.

073 temporary sampled host timing: runtime/include/hg/vu_matrix.hpp + runtime/vu_matrix.cpp hold counters (default disabled), tools/hgtool/vu_emit.py samples1/64 fast-loop iterations and nested fused transform elapsed ns, runtime/system_diagnostic.cpp arms only --profile/unpaced31M..33M and reports at budget. No guest-state or timing-model effects; host diagnostic cost unknown until replay and instrumented FPS invalid. Remove all code, regenerate and rebuild clean after one attribution replay. Retain evidence only; use it to choose a measured larger optimization rather than another unchecked hot-call guess.

## HG-DIAG-072 qualified candidate; reversal pending

Candidatea6892685 preserved all retained captures/864 writers and measured12.91581FPS. Exact-restored adjacent clean d97629a6 measured12.58608FPS, whereas earlier clean e3442936 reached13.72106. Host variation prevents deciding from one pair. Emitter/test and generated output currently restored; no072 code is in the production build. A close candidate replay is planned to decide retention; no diagnostic counter/timer exists. Exact jobs/backups in PROGRESS.

## HG-DIAG-072 - whole-loop VU memory proof hoist - ACTIVE PRODUCTION CANDIDATE

Emitter-only trial on the retained exact15-pair transform loop. Additional static recognition proves the loop counter decrements by1 to VI0 termination and the unique LQ/write bases advance by the same positive stride. Before any guest mutation, a conservative runtime preflight proves the complete counter-bounded base ranges cannot wrap or leave VU data memory, checks invariant same-iteration read/write non-alias once, and scans all future LQ selected definition masks. Failure rejects the optimized loop from its first pair and uses the unchanged checked CFG/fault route. Success removes the repeated per-iteration safety network and uses the exact direct selected-lane memory effects already validated by071; same-pair SQ retains the full pre-upper state snapshot. No timers/logging/runtime header/renderer/thread/guest-clock change. Pre-edit manifest `artifacts/vu-static-loop-preflight-preedit-20260926.txt`; clean control `e3442936...` qualifies13.72106FPS. Remove on mismatch/flat/slower reversal.

## HG-DIAG-071 - direct guarded VU-memory access fast loop - REJECTED AND REMOVED

Exact emitter-only fast-loop trial. Existing guards proved every direct LQ/SQ/ISW address, selected LQ definitions and read/write non-aliasing before mutation; failed guards retained the original checked CFG/fault path, and same-pair SQ retained the pre-upper state snapshot. Python/runtime/translation plus262144 arithmetic and81920 full VU state/fault comparisons passed. Candidate `5078e954...` qualified13.34493FPS with exact retained state/864 writers. Exact-restored/regenerated clean `e3442936...` then qualified13.72106FPS in adjacent reversal. Emitter/test/generated output are back to pre-071 hashes; no071 code or runtime cost remains. Do not repeat unchanged direct-memory emission from helper-call count alone. Full jobs/evidence in PROGRESS.

## HG-DIAG-070 - sampled triangle coverage-search cost - COMPLETED AND REMOVED

One heavy31M..33M attribution replay sampled1/64 triangle first-covered searches:9,257/592,835 calls sampled,5,155 sampled empty,174,872 tested pixel centers across3,973,809 bounding-box positions,664,400ns sampled scan time = ~42.52ms fixed64x extrapolation. Too small to justify a triangle-admission/raster rewrite by itself. All probe files were byte-restored and clean rebuild completed; instrumented timing was never FPS evidence. Full job/evidence in PROGRESS. No070 code remains.

## HG-DIAG-069 - asynchronous GPU elapsed-time census - COMPLETED AND REMOVED

Heavy31M..33M diagnostic replay collected2045/2046 nonblocking GL_TIME_ELAPSED queries with0 skips/1 unresolved: sprite122,800,640ns, triangle245,050,592ns, masked3,759,104ns; total371,610,336ns. No glFinish/unavailable-result read occurred and instrumented timing is not FPS. All three probe files were byte-restored exactly and clean game `c4b0299d...` requalified13.36916FPS with matching retained state/864 writers. GPU execution is only ~8.3% of the adjacent4.48794s clean heavy host span, so raw compute/shader work is not the principal30FPS limiter. Full evidence/jobs and pre-edit manifest are in PROGRESS. No069 code remains.

## HG-DIAG-068 - VU AOT program identity cache - REJECTED AND REMOVED

Host-only cached program-id dispatch was exact but slower. Candidate game `ea4f0575...` qualified twice at13.37745/13.38073FPS with exact retained state and864 original writers; exact-restored/regenerated clean game `28338556...` qualified13.63652FPS in adjacent reversal. All cache/generation production and focused test edits were byte-restored; original ordered full MicroMem matchers are active again. The scan-count opportunity (126,180 activations vs3,960 MPG commands) did not translate into throughput on this host. Do not repeat unchanged. Pre-edit manifest/evidence/jobs are recorded in PROGRESS.

## HG-DIAG-067 rejected and removed; HG-DIAG-066 removed

067 whole-transform AVX-512 ER candidate preserved exact arithmetic/state/fault behavior in262144 arithmetic +81920 CFG comparisons and qualified original replay. Corrected candidate game70fc003a measured13.39402 then13.19900FPS; exact-restored clean game1f9bc032 measured13.62447FPS in adjacent reversal with matching retained captures and864 writers. Both production files are byte-restored to pre-067 hashes (`fpu_madd4_er.cpp`11e89dcc..., `fpu_add4.cpp`c5f6d80d...). No067 runtime branch/counter/default cost remains; standalone per-instruction ER route stays disabled. Do not repeat unchanged. Evidence/jobs in PROGRESS.

066 batch restoration preserved state but candidate13.01247 versus adjacent clean12.99300 is flat. Source/tests byte-restored; no remaining probe or cost. Archives in PROGRESS.

## HG-DIAG-066 - batch register restoration candidate

runtime/gs.cpp; default-on experimental host copy reduction, no counters or switches. Save/restore live GS register bank once per pending batch while preserving captured/uncaptured draw environments, completion and faults. tests/runtime_tests.cpp differential reference coverage. No intended guest-state/pixel/clock changes. Host cost/saving unknown until qualified A/B; retain only meaningful gain with matching state, otherwise byte-restore source/tests. See PROGRESS plan and automatic backups.

## Current 063-065 inventory - all removed, 2026-09-26

Final clean buildfe5f7a2f /game7dd58858 /verifiera03d48e7 qualifies13.49206FPS with matching retained captures.06313.49534 is flat,06510.68711 is slower.064/065 counters, packet hook, worker integration and new headers are removed; runtime/GS/emitter/test sources restored exactly. No default or disabled probe cost remains for these entries. External archives/evidence and exact recovery paths are in PROGRESS. No active jobs.

## HG-DIAG-065 coarse VIF producer / GS consumer - rejected and removed

Final expanded runtime test442615cd passes3072 classifications and80 synchronous/threaded ordering/event/partial-fault comparisons. Qualified candidate10.68711FPS preserves all relevant captures. Census95,409 asynchronous/56,877 synchronous packets explains why60 coarse launches are insufficient. Hook/runner/tests restored byte-exact and new headers archived externally then removed. No065 thread, hook, counter or default overhead remains. Clean rebuildfe5f7a2f produces7dd58858...; final clean qualification pending. Archives: artifacts/rejected065-runtime-include-hg-gif_packet_schedule.hpp.txt, rejected065-runtime-include-hg-vif_gif_worker.hpp.txt, rejected065-tests-vif_gif_worker_regression.hpp.txt.

### Historical candidate scope

First candidate is correct but slower:10.68711FPS, all retained captures match; not retained. Temporary schedule counters in worker header/runner arm only --profile unpaced31M..33M, counting safe/synchronous packets, transfers and synchronous IMAGE/CLUT/event/transfer commands. No timers/guest effects; disabled branch per packet; host cost unknown and instrumented FPS invalid. Remove after one explanation replay. See PROGRESS for backups and exact active job.

New runtime/include/hg/gif_packet_schedule.hpp and vif_gif_worker.hpp, gs.cpp thread-local packet hook, system_diagnostic.cpp drain integration, runtime synthetic regressions. Persistent producer executes original VIF/VU; main native GL thread applies ordered immutable GIF packets. Only prevalidated non-event/non-transfer/non-CLUT register/vertex packets defer; all possibly faulting/event packets synchronously join before returning to producer. Default candidate enabled only during VIF DMA drains without VU GS-observation diagnostics. No intended guest-state, clock, input, pixel, exception-order changes. Disabled hook is one null check per complete packet. Host cost unknown until A/B; remove on any mismatch or nonrepeatable gain.064 measured60 heavy drains, supporting this coarse ownership scope; producer source-state and GS ownership requirements detailed in PROGRESS.

## HG-DIAG-064 coarse VIF drain census - removed after evidence

Result:60 active drains,14,457,736 pump iterations,635,882 draws,49,564 CLUT loads in heavy window; main runner restored12d9ef7c byte-exact. Diagnostic timing not FPS evidence.

### Original census scope

runtime/system_diagnostic.cpp only. Counts actual nonempty VIF drains, pump iterations, VU-active drains/cycles, GS draw and CLUT load changes with size histogram/maxima, only --profile plus unpaced31M..33M. No clocks/timers, synthetic input or guest-state changes; host counting cost unknown, instrumented FPS invalid. One replay answers whether coarse drains amortize the previously rejected per-activation handoff cost; remove byte-exact immediately after evidence.

## HG-DIAG-063 prepared invariant transform operands - archived, not retained

Candidate13.49534 versus adjacent restored13.16456/13.31100 and earlier retained13.39..13.42; all relevant captures and tests pass. Insufficient evidence of a repeatable material gain, not a universal failure of operand preparation. All six source/test files restored byte-exact and candidate archived. No active063 code.

### Original candidate scope

Locations: runtime/include/hg/vu_matrix.hpp, runtime/fpu_add4.cpp, runtime/fpu_add4_avx2.cpp and tools/hgtool/vu_emit.py. Host-private sign/exponent/mantissa/active masks prepared once per statically proved invariant coefficient loop. Candidate enabled only at existing061 proved sites with original fallback. No logging, timers, memoization or intended guest-state/clock/fault effects. Host cost unknown pending qualified comparison; tests cover exact arithmetic and complete CFG/fault state. Retain only repeatable material FPS improvement, otherwise restore. Pre-edit plan and external automatic backups in PROGRESS.

## HG-DIAG-062 paired eight-lane transform arithmetic - rejected and removed

Exact arithmetic/full-state/runtime/translation and original captures pass, but candidate cf7ace95 /482171bd measures13.28230FPS against retained06113.42153. Restored source8d81eb8e via b93e7eae; rejected bytes archivedc535876a. Clean buildc79375b2 producesb4418029. Its first verifier91d45e3d measured an outlying12.52048FPS, immediate identical-executable repeatc6b9bd9d gives13.38861 with exact captures. Retain061's modest reversal-supported result while recording real host variation; no universal speed guarantee.

Location runtime/fpu_add4_avx2.cpp, behind existing CPU/OS AVX2 dispatch and061 static proof. Default candidate combines independent transforms into eight-lane exact integer additions, with no logging/timing or intended guest-state effect. Scalar fallback unchanged. Host cost unknown until qualified A/B against retained061. Differential arithmetic/full-state tests required; remove if slower/flat or incorrect. See PROGRESS for pre-edit plan and evidence.

## HG-DIAG-061 fused exact static transform block - retained optimization

262144 arithmetic/scalar and81920 full CFG/fault comparisons pass, plus132 Python/runtime/translation. Qualified candidate13.39465/13.40776, adjacent disabled12.95555, re-enabled13.42153FPS with all retained captures exact. Retain this modest structural win;30FPS remains unmet. No instrumentation. Detailed jobs/hashes in PROGRESS.

AOT guarded15-pair loop plus exact two-transform helper; no guest-visible arithmetic/order/clock changes intended. Runtime helper publishes only live boundary state while retaining all sticky flags. Enabled only at statically proved sites with checked fallback; no timers/logging/cache. Measured modest benefit in adjacent reversal, with recorded host variation. Full differential state/fault tests and original qualified A/B required; remove on mismatch/flat result. Files and pre-edit plan in PROGRESS.

## HG-DIAG-060 complete heavy-window outer phases - captured and removed

Replay8510bd08a31d4ce1b83767777c53f512: VIF/GIF3430.6ms dominates, EE495.2ms, other phases each<=187.3ms. Measures2M slices with substantial timestamp overhead; no FPS inference. Header restored exactly4efc1994, archived753cf440cccb4249bc6586ec7a57ae7b.

Location runtime/include/hg/diagnostic_profile.hpp; --profile only, every slice31M..33M, existing11 phase boundaries, omit PC maps. No guest-state/order/clocks effects. ~22M timestamp calls incur unknown host cost; measurements include timer overhead and cannot certify FPS. Purpose: resolve remaining runtime cost without rare-burst sampling bias. Retain one capture then exact header restore; no generated code changes.

## HG-DIAG-059 VU entry inclusive time - captured and removed

Replay0d0dcdb1413047c0b38690a9692d8794 records126180 calls, with biggest buckets0x288578.4494ms and0x60367.3810ms; several other entry groups each129-221ms. Counts and measured issue-cycle deltas retained in native.log. No program identity inferred; nested GIF/GS is included. Runner byte-restored12d9ef7c; diagnostic archivedc421006585f740c096fbf71ade3e1eee.

Runner wraps retained AOT executor at31M under --profile;1024 entry buckets count calls, issue cycles and elapsed host nanoseconds. Includes nested GIF/GS, not exclusive arithmetic; no program-identity claim from entry alone. Two timestamps per activation, host-only arrays, no guest changes; active cost unknown. Restore original executor before final captures and byte-restore runner after one replay.

HG-DIAG-058 captured and removed: heavy downloads154,011,700ns and backend flush154,759,400ns, evidence replayf4d3ea6142a54f21964a095c5058cefe. Both sources restored byte-exact. Whole replay readback is not heavy-window cost.

## HG-DIAG-058 heavy-window backend synchronization attribution - captured and removed

Locations gl_gs.cpp existing download/flush counters and system_diagnostic.cpp31M start/33M end. Default inactive, host-only category counters reuse existing operation timestamps; no guest effects. Active cost unknown; instrumented FPS invalid. Separate raster bulk scopes, ordinary CPU reads/writes and other downloads; report heavy-only bytes/calls/time and flush time. Remove after one capture and retain evidence only.

## HG-DIAG-057 actual GPU-dirty bulk CPU-scope origin census - captured and removed

Replay1eb7f6c56f454dadbcdaba92f9a32c9e: heavy window self-feedback180 scopes/720 pages; other textured triangles159/620; total339/1340. No sprite dirty scope. Whole-run3111/75809 must not be confused with this window. All four sources restored byte-exact; archived probe backups b75a90c834e04f6bbfcea3166f82b454,8df4acaf2c834835aab16e5400c9c45d,960dc5da7fc34c1daa1356aaf26ac22d,49a655cfe7694dc2a749be7c573fdce4.

Counts only retained bulk `GsCpuMemoryScope` calls that, after any required queued-GPU flush, actually intersect `gpu_dirty` resident VRAM during31M..33M. Six host-only tags distinguish CT32-special sprite fallback, generic textured/untextured sprite fallback, proved PSMT8H triangle self-feedback, and other textured/untextured triangle fallback. Records dirty scope count and dirty page count by class; clean scopes are ignored. No timers, masks, downloads, rendering decisions or guest behavior change; instrumented timing is invalid. Pre-edit manifest `artifacts/dirty-cpu-scope-origin-probe-preedit-20260925.txt`. Capture once, restore all probe source byte-exact, then choose the next production mechanism from the dominant real dirty class.

## HG-DIAG-056 PSMT8H self-feedback page-overreach census - captured and removed

Replay `d38ded3dfdd24a93b56a4d0cd2929799` measured the retained proved self-feedback class over31M..33M: **18,378 draws / 794,236 covered samples**. The existing clipped-bounding-rectangle frame+depth mask totals **98,312 page references**, while pages actually touched by covered samples total **66,374**, a **31,938 / 32.49%** reduction opportunity. **8,376 draws (45.58%)** have a smaller exact footprint; maximum conservative mask is12 pages versus maximum actual6. Timing is invalid; coverage/rendering were unchanged. `runtime/gs.cpp` and `runtime/system_diagnostic.cpp` were byte-restored to `050855fa...` / `12d9ef7c...`. Pre-edit manifest `artifacts/feedback-page-overreach-probe-preedit-20260925.txt`. This is large enough for one bounded production trial that computes the exact covered-page mask before the existing CPU feedback raster; reject if the coverage-prepass CPU cost outweighs reduced synchronization.

## HG-DIAG-055 PSMT8H self-feedback depth-state census - captured and removed

Replay `d71d4b009e114fc8a214e77102009c50` / `project-link-runtime/20260925T203752Z-4c1b86d480eb4133b5d26b7ea1c1b212` classified exactly **18,378** already-proved self-feedback triangles in31M..33M. All18,378 are **ZTST=2 (GEQUAL), ZBUF.ZMSK=1, ZBUF.PSM=1 (PSMZ24)**; no other bucket occurs. Therefore old Z contents are semantically required for this class even though Z writes are masked: skipping `read_z_pixel()` or omitting depth visibility would be incorrect. Instrumented timing is invalid. `runtime/gs.cpp` and `runtime/system_diagnostic.cpp` were byte-restored immediately to `050855fa...` / `12d9ef7c...`. Pre-edit manifest `artifacts/feedback-depth-state-probe-preedit-20260925.txt`. Next readback lead is exact page-footprint overreach within these thin triangles, not removal of the depth dependency itself.

## HG-DIAG-054 CLUT reuse activation audit - captured and removed

Replay `021572796c9d4e9eaf3c8a19a8665684` / `project-link-runtime/20260924T184234Z-fbcb3f9a33e644ee8d921dfa0110138d` resets only the candidate's host-private reuse counter at31M and reports at33M: **24,305 reuse hits / 49,564 loads**. This exactly matches HG-DIAG-053's same-key+unchanged+no-pending opportunity count. Therefore the candidate's global VRAM generation is not over-invalidating eligible no-draw reloads; source-page generations would not expand the safe no-pending set. No timers or guest state changes. Runner was restored byte-exact to12d9ef7c....

## HG-DIAG-053 CLUT reload redundancy census - captured and removed

Replay `65821f629e2b49849e450fa2190c08f9` / `project-link-runtime/20260924T174900Z-660cb9ecd7c74f51a7e140c9343dc6f5` reports **49,564** actual successful CLUT loads in31M..33M:39,946 unchanged-result,40,006 same normalized source/load key as the preceding load,39,886 same-key+unchanged, and24,305 same-key+unchanged with no pending draws before the mandatory flush. Thus **80.47%** of all loads repeat the same key and produce exactly the same destination CLUT; **49.04%** do so while the draw queue is already empty. No result fed execution and timing is invalid. The replay/task infrastructure restored `runtime/include/hg/gs.hpp` and `runtime/system_diagnostic.cpp` byte-exact to23f36c56.../12d9ef7c.... This is strong evidence for an exact source-change proof around CLUT reloads, not permission to skip a load based only on key equality.

## HG-DIAG-052 exact transform temporal-locality census - captured and removed

Replay `abca19e41b804f2586664fe89a5ddac2` / `project-link-runtime/20260924T173527Z-5eac56a4e8eb46f784c7e84c3dd209fe` reports **2,957,520** exact keys: hit1=45,120, hit2=71,460, hit3=132,840, hit4=88,980, miss=2,619,120. Only **338,400 / 2,957,520 = 11.44%** recur within the last four distinct keys, versus87.37% reuse in HG-DIAG-051's large direct-map working set. Immediate repeat is only1.53%. This disproves a tiny1..4-entry transform cache as a route to recover most arithmetic savings. No cached result was supplied; instrumented timing is invalid. `runtime/vu_products.cpp` and `runtime/system_diagnostic.cpp` were byte-restored immediately to SHAs1e53d7e7... /12d9ef7c.... Transform memoization is closed unless materially new locality evidence appears.

## HG-DIAG-051 exact transform repeat census - captured and removed

Replay386824fc4c46408687749b733796286a / project-link-runtime/20260924T042524Z-9ddde7cce7974743b8a4eea837f60093:2957520 calls,2583893 exact hits (87.3669%),354607 replacements,19020 occupied slots. Probe never supplied cached results. Both files restored exactly after capture; no census retained. This establishes input reuse opportunity only; production throughput remains unproved.

runtime/vu_products.cpp first full-vector MULA source1/other11/X is observed under runner --profile31M..33M. Key is all32 coefficient words (VF1..8) and input VF11 XYZ; bounded65536-entry direct-map storage compares full35-word identity after hashing, records hits/misses/replacements only. No cached result is used and no guest state/order/clocks/fault changes occur. Default inactive branch; active hashing/copy cost unknown, about9MiB temporary host storage; instrumented FPS invalid. Capture once then byte-restore both runtime source files. Purpose: discriminate whole pure-transform memoization, not approximate arithmetic.

## HG-DIAG-050 static-loop activation observation - captured and removed

Replay4c6f7fc3bed94c8a94c2df80e6247299 / evidence20260924T042230Z-85baf60ba94a45a9922f3f1c339b0683 counts27863040 full_result calls and16789320 repeated cycles =6*2798220 hot-loop iterations. This proves full activation of the guarded loop, not a speed gain. Exact source restored after capture; no probe remains.

runtime/vu_products.cpp full_result entry counts total calls/repeated issue_cycle, enabled/reset by runtime/system_diagnostic.cpp --profile at31M and reported at33M. One generated upper operation calls full_result once; original pair scheduling advances between calls, while the whole-loop candidate deliberately materializes time at Q boundaries. Six equal-clock transitions per accepted15-pair loop identify its active arithmetic group. No guest state/clocks/order/fault changes, no timers per operation. Disabled cost one boolean branch; active integer-counter cost unmeasured, instrumented throughput invalid. Capture once then restore both files exactly. The observation checks activation, not performance or universal attribution of equal clocks outside this bounded workload.

## HG-DIAG-049 exclusive heavy graphics pipeline timing - captured and removed

Replayfaffeec91c8344d08a14db75e96ce3d6 / project-link-runtime/20260924T040408Z-a7ac5e72ef204c57b247f49fbf2acd1d. Heavy wall4662309900ns: VU1907775100, DIRECTtransport70216800, UNPACK126295600, MPG1427600, completeGIFpacket740628400, wholependingraster505807000. Disjoint categories; remaining1310159400ns is other work plus profiling overhead. Raster2052864calls includes empty service. All three files restored byte-exact; disabled cost now zero. Current executable must be rebuilt before speed comparisons.

Locations runtime/vif.cpp (host-only category stack and completed command scopes), runtime/gs.cpp (complete packet and whole pending raster scopes), runtime/system_diagnostic.cpp (31M activation/report under --profile). No guest effects, clock/input/order/fault changes. Default inactive; one boolean check per instrumented scope, no timestamp on inactive path. Active cost unmeasured; two clock reads per completed scoped operation, no per-instruction/qword timestamps. Exclusive time subtracts nested packet/raster work; VU remainder includes XGKICK transport/validation. Instrumented throughput is not FPS evidence. Retention criterion one capture followed by exact three-file restore; no header or generated source changes.

## HG-DIAG-048 current heavy raster timing - captured and removed, 2026-09-24

Replay04f386d7930a499196383849ab73f836 / evidenceproject-link-runtime/20260924T033513Z-66f924fa35ff441789c43b3d69230517 reaches33M/no fault. Heavy calls: triangle lists240/14290600ns, strips601922/265329700ns, sprites33900/163547300ns; other kinds zero. Inclusive measured raster total443167600ns. Scope excludes environment setup outside primitive dispatch, batch-final flushes and unrelated VIF/VU/GIF work; not a complete renderer ceiling or GPU-time measure. Runner restored exactly12d9ef7c... via backup080bc1b8..., diagnostic archived6b148e7b....

Location runtime/system_diagnostic.cpp31M activation/reset of existing HG-DIAG-005 GS raster counters. Enabled by --profile only in temporary source, captures31M..33M then report; default unprofiled path unchanged. No guest state, scheduling, clocks or rendering changes. Two host timestamps per raster call plus counters have unmeasured active cost; instrumented throughput invalid. Reports inclusive CPU raster cost, including nested GPU coherence stalls, not GPU execution duration. Retention criterion one completed capture followed by byte-exact runner restore; disabled cost zero after removal.

## HG-DIAG-047 sprite dispatch dependency waves - captured and removed, 2026-09-24

Replay dbd1ced8a6994345ae5a5c67c2bc6ab0 / evidence3f2311a748ab472796890295b7fe81c5:52398 sprites,22389 predicted conflict-free waves,1494 sprite flush groups; all retained captures exact. Diagnostic removed byte-exact after capture. This establishes dispatch-reduction opportunity, not a speed result.

Location runtime/gl_gs.cpp accepted-sprite queue path and backend summary. Temporarily unconditional in the diagnostic build only; two512-bit masks and integer counters predict conflict-free waves within unchanged flush boundaries. New wave on read-after-write, write-after-read or write-after-write page overlap. No guest-state, clocks, ordering or rendering effects. Active host cost unmeasured; timing invalid. Remove after one33M capture, before any production measurement; disabled cost zero after exact restore. Purpose: discriminate whether safe multi-sprite dispatch is structurally worthwhile using existing conservative page bounds.

## HG-DIAG-046 accepted GPU sprite classes - captured and removed, 2026-09-24

Temporary source: runtime/gs.cpp class counters/record helpers, runtime/system_diagnostic.cpp begin at31M/report at33M when --profile enabled; normally inactive. One host integer update per accepted draw; no guest state, clock, admission or rendering changes. Disabled cost was a conditional call/branch; active host cost unmeasured and instrumented FPS invalid. Retention criterion was one complete census; remove after capture. Replay40c0559ada4a4461a98c833bf3227b18 / project-link-runtime/20260923T195005Z-71f1a714515a4af1b37f309f512af94c completed the census. CT32->CT32 blended nearest136302720pixels and linear111705840 dominate, with ATE off/ZTST ALWAYS/no Z writes. Source class IDs are0 untextured,1 CT32,2 CT24,3 CT16,4 CT16S,5 PSMT8H,6 PSMZ24,7 other. Cleanup restored exact pre-census source hashes050855fa.../12d9ef7c...; no probe remains. Clean verifier5c135a0a... preserves all retained hashes. See PROGRESS for final verification.

## HG-DIAG-043 heavy triangle GPU admission classification - captured and removed, 2026-09-23

Replay `88095b71d7504a669fdd665f1a30b2e0` / evidence `project-link-runtime/20260923T091510Z-04ec5d9f442048f4b1ead12e1597b1ee` arms integer-only triangle counters only for31M..33M. Result: calls602162, area_zero9327, no_sample328578, post_sample264257, fog0, textured_fst0, stq_not_ready0, pre_submit264257, submit_reject18641, gpu_accept245616. Thus337905/602162 (56.12%) are degenerate or cover no pixel; every sampled triangle is eligible for the current high-level triangle path, and only18641/264257 (7.05%) sampled triangles fail inside the bounded submit/state/backend checks. FST/fog/STQ readiness are not the heavy-scene blocker. Instrumented timing is invalid as FPS evidence. Temporary `gs.hpp`/`gs.cpp`/`system_diagnostic.cpp` are byte-restored after capture. Next diagnostic splits the18641 first-failing submit guards; do not implement native FST triangles from this scene.

## HG-DIAG-044 triangle submit first-failure classification - planned, 2026-09-23

Heavy gameplay has18641 sampled triangles that reach `submit()` but fall back. Temporarily count the first exact false-return condition in submit order: extent/bounds; frame/depth/state-format; depth/test; alpha-test form; texture format; texture-width/CLUT config; clamp; blend selector; coefficient-scale; Q bounds; numerator bounds; vertex-ratio bounds; CLUT-valid; backend rejection, plus attempt/accept totals. Integer-only,31M..33M, no clocks or render changes. Restore immediately after one replay; the dominant bucket selects the production renderer extension.

## HG-DIAG-042 CPU raster primitive mix - captured and removed, 2026-09-23

Temporary count-only replay `57724dd55341427d99b481efa6eeba3d` / evidence `project-link-runtime/20260923T085946Z-a7f8959b18064337be7205881d273b8d` completed33M/no fault with normal48195 GPU sprites/305856 GPU triangles/842 masked batches. Existing raster counters were temporarily enabled under normal `--profile`; per-draw chrono timing was removed, so only integer counts were added. Final totals exactly sum to861346 rasterized draws: primitive0/1/2=0, primitive3=292, primitive4=794702, primitive5=1920, primitive6=64432. Thus triangles total796914 and sprites64432. Relative to backend accepted counts,491058 triangles and16237 sprites fall back to CPU; triangles are96.7993% of the507295 unaccelerated draws. Triangle strips alone are92.2628% of all rasterized draws. This run is attribution only, not FPS evidence. Temporary `runtime/gs.cpp` and `runtime/system_diagnostic.cpp` edits are restored immediately after capture.

Next discriminator: count why those491058 triangle draws fail GPU admission. The current triangle gate excludes textured FST triangles entirely, fogged triangles, non-ready perspective STQ, and then bounded submit/state/format cases. Do not assume FST is dominant until counted. Use integer-only reason buckets, no hot-path clocks; restore after one capture.

## HG-DIAG-041 GPU-direct native scanout - rejected and removed, 2026-09-23

Active-root only; `v1/` remains frozen. Candidate bypasses host-only CPU scanout materialization during normal native presentation. It reuses the existing resident GPU VRAM buffer, commits only already-rasterized accelerator work, uploads CPU-dirty displayed pages when needed, converts supported DISPFB PSMCT32/24/16/16S pixels in an independent compute shader into bounded shared RGBA textures, then presents those textures directly in the host context. Guest pending draws are not flushed merely for presentation. Guest-visible VRAM/readback, transfer ordering, final diagnostic `display_image()` capture and CPU fallback remain unchanged. Shared texture ownership must be synchronized so the worker never rewrites a texture still sampled by the UI. Pre-edit manifest `artifacts/gpu-direct-scanout-preedit-20260923.txt`.

Purpose/result: remove per-preview GPU->CPU resident-page materialization, CPU GS-swizzle/format conversion/vector allocation, and immediate CPU->GPU texture upload. It is state/visual correct under the relaxed renderer but slower overall. Qualified direct runs:11.4898726/11.6583860FPS with1064/1058 scanouts; activation-off control under the identical final-only verifier:12.2141880FPS with0 scanouts and identical relaxed frame/VRAM plus EE/VU/IOP/GS state. Direct path lowers download bytes/time (~2.327GB/~1.02s versus2.724GB/1.524s control) but raises flush time (~0.78-0.79s versus0.497s) and requires cross-context glFinish/texture ownership handoff. Remove HG-DIAG-041 and do not repeat this synchronous shared-texture design unchanged. The final-only verifier capture is a separate measurement-harness improvement and remains.

## HG-DIAG-040 native relaxed float barycentric triangle candidate - rejected, 2026-09-23

Candidate2 extended retained HG-DIAG-039 only in the active game host's native-relaxed textured-triangle path. Strict factory mode remained exact. For already-admitted jobs it uploaded float area and used float edge/top-left coverage, Gouraud and Z interpolation in addition to native STQ; texture addressing/sampling, alpha/depth tests, blending, masks, draw/tile order, page coherence and guest-visible materialization were unchanged. Two fixed qualified verifiers on the exact same executable/output disagree materially: `6f20053d...` =11.622501FPS while repeat `3d9bf9ba...` =11.225003FPS. Both reproduce candidate2 frame SHA5318ff66... / GS-VRAM SHA43079f83... and exact EE/VU/normalized-IOP/GS-register state. Therefore no repeatable speed benefit exists; restore candidate2-only backend/shader changes and retain HG-DIAG-039 native STQ. Do not repeat this full float coverage/Gouraud/Z route unchanged.

## HG-DIAG-039 native relaxed STQ triangle interpolation - retained, 2026-09-23

Active-root renderer experiment authorized by the user after freezing strict pre-rule-loosen state under `v1/`. The game GPU path may replace renderer-private exact PS2 GS arithmetic when game-visible behavior and visual output remain correct; CPU/EE/VU/game logic and guest-visible render readback/order remain strict. First candidate changes only textured STQ interpolation inside the existing OpenGL triangle accelerator: keep exact integer coverage edges, existing job admission, texture addressing, draw ordering, blend/depth/frame-mask behavior and resident-VRAM coherence, but upload float approximations of the already-validated S/T/Q coefficients and use native float dot/divide/floor in the shader. Strict coefficient data/path remains the default backend mode used by existing differential tests; only the active game host opts into the relaxed mode.

Purpose/result: remove per-fragment GL_ARB_gpu_shader_int64 weighted S/T/Q math and two signed64 divisions from the existing triangle workload before a larger native-renderer rewrite. It activates on302650 triangles. Strict verifier controls:11.36245,11.40322,11.33980FPS with exact historical frame/GS-VRAM hashes. Relaxed verifiers:11.46176,11.49945,11.43419FPS with identical EE/VU/normalized-IOP/GS-register state and one stable relaxed framebuffer/VRAM result. Offline exact PPM comparison:65/286720 pixels differ, max channel delta2/255, none>=4, PSNR87.56dB. Retain under the user-authorized native-renderer contract; guest-visible readback/order and all CPU/EE/VU behavior remain strict. Pre-edit manifest: artifacts/native-renderer-preedit-20260923-a1.txt.

## HG-DIAG-038 heavy VU AOT static-pair mix - captured and removed, 2026-09-23

Configured replay f3de50c86e644776bfe982d37de307ee from diagnostic build0da99b75e9754ff5808ea4a78307d207 reached the normal33M budget/no runtime fault with48195 GPU sprites,305856 triangles and842 masked-transfer batches. The profiler was enabled only for31M..33M and used integer counters at statically emitted AOT program/entry/pair sites; it added no clocks, decoder, interpreter or dynamic VU dispatch. Instrumented elapsed time and presentation rates are invalid as FPS evidence.

The heavy window contains126180 VU activations, exactly the previously observed91140 MSCNT+35040 MSCAL total. Program1(post_opening_mscal0) accounts for111840 activations (~88.6%): entry0x0=33120,0x60=32580,0x288=46140. Across all programs the window executes84835200 static VU pairs; program1 contributes46003620 (~54.2%). Its contiguous0x0158..0x01c8 15-pair loop executes2798220 times per pair,41973300 pair executions total (~49.5% of the whole measured VU pair stream). The next-highest individual static pair count is91740 in program3, so this is a strong dynamic concentration rather than a static-call-site inference.

Inspection of that exact generated loop identifies a focused host-overhead lead: program1:0x0170 still copies the whole Vu1State every iteration solely to preserve the lower SQ source/address across the same-pair upper arithmetic_q write. The retained nonconflicting-SQ optimization had intentionally left whole-state snapshots at20 overlapping SQ sites; this hot overlapping site therefore executes about2.798M whole-state copies in the measured window. This diagnostic proves execution concentration, not that replacing the copy is faster. A bounded follow-up may capture only the exact pre-upper SQ operands/address/defined mask while retaining the lower store's original post-upper memory-bound fault point.

Original-frame diagnostic summary at budget still reports864 writes at0x001bef84, unexpected_writers=0,lost_trace=0,caller_mismatches=0,invalid_stack=0 with caller0x002d1c50/object0x004f14c0. Fixed verifier f231b0e85d504238bb5678a9e110eecc remains the qualified correctness/FPS authority; this count-only replay is attribution only. Diagnostic executable SHA8d00cbaa6a7c26494d6a391f13d1bdc8c65ab64a44f4ff1c34d64be1f3802a74; generated translated.cpp SHAb4b24da059441ee8de557826e77e6bda2f3a2a3bcc81d4087371fcf29beecde9. Pre-edit manifest is artifacts/vu-dynamic-mix-preedit-20260923-rc15.txt. Temporary VIF/emitter/runner profiling edits are restored byte-exact and clean AOT output rebuilt immediately after this capture; no VU mix counters remain in production.

## HG-DIAG-013/014 gameplay audio starvation observed; no audio fix applied, 2026-09-22

User's Select-menu session93c3705f... (project-link-visible/20260922T060315Z-2b6cfc946afd44cfb159f18fef315246) reports354/384/414 speaker-queue underruns at51M/52M/53M, BEFORE explicit missing-static-target396900. Consecutive source-frame counts2443627/2491627/2539627 rise by48000 per about2.86..2.96host seconds in that heavy route. The host device is configured48kHz, so available queued audio cannot sustain normal wall-time playback. Peak14244input/10683output does not indicate hard clipping. This is observed queue starvation; it does not identify every audible defect or prove the cause of all CPU cost.

After config-only menu repair, configured run_audio_replay929ee941048344b6860a0667be7a53b9 uses gameplay33m,current,speakers and the pinned12 events. Evidenceproject-link-runtime/20260922T061958Z-47460baf9a3e429cb0dd39d7a0b019d6 reaches normal33M budget, no fault lines, matched delivered-input prefix. Raw core0 WAV6318552bytes/SHA c2a6162a39bd41d0438adcb9df7c92d2e599802da04a0f87bb131717f99b35e3 holds1579627stereo frames at48000Hz. Speaker count1579627,73underruns,maxqueued7,inputpeak16966/output12724. Expected frame/EE RAM/GS VRAM/state/VU and state-metadata hashes match the fixed qualifier; raw IOP RAM differs and was not separately normalized for this audio run, so no full IOP-equivalence claim for it. Exact saved2s clip at frame1480000 has PCM SHAb6176ab3f720e01d227a535ed573c32d64e46464ac074abac61f4c3b2b7f028c; tool response supplied metadata, not an independently assessed audible-quality result.

Source verified: system_diagnostic.cpp318..329 sends the SAME raw observer samples to WAV and HostAudioOutput;1066..1073 attaches that observer to iop.advance_spu2. iop.hpp1222..1227 advances Spu2Input, not a final voice/effects mixer. host_audio.hpp retains eight1024-frame48kHz buffers and3/4gain. Existing DIAG014 explicitly bypasses unfinished voice synthesis, volume/effects and cross-core mixing. Therefore neither selecting speakers nor a nonempty raw WAV demonstrates complete game audio; a larger finite queue cannot correct sustained sample-production deficit. No audio buffer/clock/sample/renderer/source changes were made here; no new probe remains. Captures use already-existing opt-in013/014. Follow-up needs native-throughput work for dropouts and independently derived final SPU2 synthesis/mix implementation for completeness, not treating either as already solved.

## HG-DIAG-037 own-worker native RIP sampling - captured and removed, 2026-09-21

Capture project-link-runtime/20260922T043923Z-777a3f5a342d4ff2b8b420755c5e06d3, replay3dc6017703934d2fb276fd5b094f02d2 completed33M/no faults and normal original work.330 samples,290 main-image/40 outside-image,0 suspend/context errors,0 unexpected counts, no cap;13,465 map symbols. Top native samples include fpu_madd4_avx2(15),Vu1State::multiply_vector(12),Vif1Path::process_pending(11),convert_fixed(10),VU program1(8)/8(7),sprite raster(8) and multiple smaller helpers.40 outside-image samples lack caller attribution. This small sample establishes dispersed native cost, not exact percentages, inclusive call time or a single culprit. Nominal1..4ms waits actually yield about330 samples in5.06921s; do not claim a precise high-frequency sampler.

Image/EE RAM/GS VRAM/state/VU hashes match the established captures. A temporary offline case in existing tests/test_runtime_verifier.py compared pinned complete IOP files against contemporaneous control1b05c22...: only bfcd1 and bfcd2 differ; existing normalized digest bdbf3a1117269518e1a88176af2fa387cf8d35b57e77ee41e4af955ee2c626b1 matches. Configured python_tests df58fe42022548ffbeedc6829ce2f61d passed129 tests. Both temporary runner and offline test were byte-restored (c8c2d536.../95054f79...). Runner diagnostic source backup7ca4761b5bad4725b3c6e029b48306cb; offline case backup0c55820dfd4c4a75b11c399e3380f33b. Clean game rebuildb94d2b4dc6bc41f4a34f3bdf73536073 producedd47ee0b1... with147 inputs. Protected host/parser remain. No active sampler or temporary capture-specific test; no FPS inferred from this run.

Historical implementation/activation (removed):

Source only runtime/system_diagnostic.cpp; profile-enabled unpaced33M replay activates inside existing31M checkpoint. Duplicate the current worker pseudo-handle into a real same-process handle, use one sampler thread and4096 preallocated RIP slots with jittered nominal1..4ms sleeps (no timer-resolution, priority or affinity changes). Each successful SuspendThread is paired with CONTEXT_CONTROL GetThreadContext and immediate ResumeThread; no allocations, locks, IO, symbolization, stack walk, guest-memory reads or callbacks during suspension. Report failures/unexpected suspend counts; a failed resume fails fast rather than silently retaining a suspended native worker. Stop/join and close the handle on normal/exception paths. Symbolize after the window against the exact copied map; outside-image addresses stay unclassified, nearest-symbol attribution is not an inclusive stack proof.

Purpose: resolve current host CPU hot-code distribution after host fix, rather than extrapolate pre-fix DLL stalls or repeat qword transport tweaks. Guest instructions, clocks, input schedule, rendering, faults and PC unchanged. Host suspension and sampling alter wall scheduling and incur nonzero unmeasured cost: this capture is NOT qualified FPS. Windows x64 only; unsupported hosts explicitly report diagnostic unavailability. Remove by exact runner restoration c8c2d5364eaa64ced38bbde901875f4e4b0b0d50368d55937000706ac029036f after one useful configured replay, verify expected work/image/state, and rebuild clean before production performance. Protected host1a122b9f... and parser5ab22f5b... stay unchanged. External plan/manifest native-rip-heavy-profile-preedit.txt.

Primary Microsoft API contracts: GetThreadContext requires suspending the other thread; DuplicateHandle converts a current-thread pseudo-handle into a real handle; SuspendThread must not lead to waiting for locks owned by the suspended thread; ResumeThread decrements only our added suspend count. Documentation at learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getthreadcontext, -suspendthread, -resumethread and handleapi/nf-handleapi-duplicatehandle. This is independent host observation infrastructure, not emulator/runtime instruction decoding.

## HG-DIAG-036 inert DIRECT input batching coverage - captured and removed, 2026-09-21

Evidence project-link-runtime/20260922T024554Z-c6cc6abda3b043de9a8e92cd6fb3153f:7306154 helper calls,25986 accepted batches and7151582 buffered qwords during31M..33M. This confirms substantial actual batching, but millions of rejected calls motivated a caller-side DIRECT-byte prefilter. Normal work counts/GPU timing and no faults. Runner-local counters were removed by exact restoration before the refined production trial; no timing probes remain. Instrumented run is not FPS evidence.

Historical probe description:

Runner-only local counters around the candidate helper, enabled for31M..33M in profiled non-realtime33M. Counts helper attempts, accepted batches and qwords actually appended. No clocks, new VIF/State fields, guest/input/render/timing changes or altered completion/fault paths. Report once at budget; overhead is diagnostic, not FPS evidence. Restore exact candidate runner after one useful capture and rebuild before qualified performance. Goal is to relate the small apparent performance difference to real saved input work and rejected-call overhead, not accept the first slow control as proof.

## HG-DIAG-035 DIRECT complete-packet batching coverage - captured and removed, 2026-09-21

Evidence project-link-runtime/20260922T022141Z-10b6ddd9ec4a48c7b6544089db0ae5bd:61146 completed heavy DIRECT commands,7306814qwords,3240 starting with prior GIF suffix,57906 eligible whole packets/564374qwords. Only7.724% of payload is covered despite many packets; all eligible first-tag formats are PACKED, none IMAGE. Original serial transport still executed every qword. Normal48195 sprite/305856 triangle/842 masked-batch work and GPU timings. Both temporary source files restored byte-exact (vif.cpp5ab22f5b..., runnerc8c2d536...), before further production work. No FPS claim from probe. HG-FAIL-031 records why the complete-packet candidate is not retained.

Historical probe description:

Two files only: runtime/vif.cpp and runtime/system_diagnostic.cpp. Enable fixed thread-local counters at31M only in profiled non-realtime33M replay. Count commands/qwords, prior GIF suffixes and hypothetical whole EOP packets inside already-validated DIRECT payloads; still execute ORIGINAL serial qword loop unchanged. No timer, alternate decoder, earlier commit, altered fault boundary, packet data, guest state, input or clock change. Local lookahead counter avoids counting the same eligible packet repeatedly. Report once at worker teardown; counter/scan overhead is attribution only and never FPS evidence. Restore both sources exactly after one useful capture. Purpose is to quantify actual coverage of the rejected complete-packet candidate, not assume its predicate is frequently true.

## HG-DIAG-034 completed VIF command cost - captured and removed, 2026-09-21

Evidence project-link-runtime/20260922T015200Z-c79db1b4e97b4a43806f598f36fc5af7:60 heavy VIF DMA bursts total4.6822539s; MSCNT91140 calls2.5603861s and MSCAL35040 calls0.0058286s; DIRECT61146 calls/7306814 qwords/116909024bytes1.6075108s; completed UNPACK commands total~0.176s; MPG3960 calls0.0016023s. DIRECT retried7306814 incomplete headers before whole-command processing. Nested VU/GIF/GS and enclosing burst times overlap. Host timers/attempt counters affect cost; this is attribution, not FPS. Normal work counts preserved; full-run GPUflush0.5238009s/readback1.1283055s do not show the old15s redundant-redraw pathology. Both parser and runner probes were byte-restored (vif.cpp5ab22f5b..., runnerc8c2d536...) after the capture. The latest built binary remains diagnostic until rebuilding restored/candidate sources.

This explains why narrow word/index UNPACK tweaks produced no reliable gain; completed unpack work is small relative to VU and DIRECT. A bulk already-complete DIRECT transport candidate is justified only to reduce its repeated word reconstruction/packet buffering calls, not to assume that all1.608s of nested GS work can be removed.

Historical probe description:

Source runtime/vif.cpp and system_diagnostic.cpp only. Enable at31M for profile-enabled non-realtime33M replay. Fixed thread-local attempt/retry counters have no per-qword timers; one clock pair per completed MPG/UNPACK/DIRECT/MSCAL/MSCNT records inclusive command time, units and bytes. UNPACK detail classifies format/mask/mode. Runner times only active VIF DMA bursts for context. Completed-command times overlap enclosing burst times and may include nested VU/GIF/GS work; do not sum them as exclusive system cost or call them FPS. No guest memory/state, command order/partial writes/faults, input, clock or renderer change. Remove after one useful capture by exact restores (baseline vif.cpp5ab22f5b..., runnerc8c2d536...) and rebuild before performance measurement. Purpose is to establish whether bulk DIRECT transport or VU execution warrants the next optimization, not assume either dominates.

## HG-DIAG-033 current worker CPU-accounted versus wall time - captured and removed, 2026-09-21

Unpaced evidence project-link-runtime/20260922T000338Z-20d93d6e4ad940e49e73a065dc1f4cac: heavy31M..33M wall5.0162412s, worker user4.515625s + kernel0.34375s =4.859375s (~96.9% CPU-accounted). Same diagnostic executable under fixed paced verifier adee07abe7974ec6964b41954cbf52c7: snapshot wall5.8396614s, worker user4.921875s + kernel0.390625s =5.3125s (~91.0%). Raw cycles recorded without conversion. This shows CPU work dominates both observed intervals, with differing CPU-accounted and unaccounted wall durations; it does not identify a particular competing process, clock, cache or scheduler cause. The paced diagnostic retains original writer qualification and all frame/EE/GS/VU hashes; same-date normalized IOP hash bdbf3a11... matches the after-midnight control. Its10.272319FPS is instrumented attribution, not a new accepted baseline. Runner restored byte-exact to c8c2d536... after capture; no snapshots remain in production.

Historical probe description:

Only runtime/system_diagnostic.cpp; profile-enabled33M captures snapshots at31M,32M and33M. GetThreadTimes uses the current native worker pseudo-handle, GetProcessTimes only the current process, QueryThreadCycleTime reports raw cycles without conversion, steady_clock supplies wall duration. No unrelated process querying, changing affinity/priority or settings. Three boundary snapshots only, no guest instructions/timing/input/render/fault changes. Invalid/missing APIs report diagnostic unavailability instead of inventing measurements. Compare CPU-accounted versus elapsed time as a lead; their difference does not identify a particular cause. Remove after two useful unpaced replays and rebuild before production validation. Manifest artifacts/worker-cpu-wall-profile-20260921-preedit.txt. Primary Microsoft API documentation: GetThreadTimes timing units100ns; QueryThreadCycleTime warns not to convert raw cycles to elapsed time.

## HG-DIAG-032 recovered-host whole DMA burst/raster profile - captured and removed, 2026-09-21

Capture project-link-runtime/20260921T234321Z-549f1512122044dbbb449017c113e666 completed33M. Heavy31M..33M instrumented loop4.8610742s; VIF service3.5851216s in60 active bursts (max59.9937ms), GIF service0.0400684s in180 active bursts. Explicit raster0.306112s; all GS primitive timing including nested work: sprites0.6324601s/33900 calls, triangle strips0.3381137s/601922 calls, triangles0.0032072s/240 calls. These costs overlap where GS is called inside VIF and must not be summed as exclusive phases. Driver costs stayed near recovered baseline: full-run flush0.4318462s/readback0.9447543s, unchanged draw/work counts. Results point to VIF/VU CPU work, not renewed15s GPU stalls. Instrumentation overhead is included; no FPS claim. Runner byte-restored to c8c2d5364eaa64ced38bbde901875f4e4b0b0d50368d55937000706ac029036f. All probe code removed before subsequent production comparison.

Historical probe description:

runtime/system_diagnostic.cpp only. At31M of configured profiled unpaced33M, enable existing GS primitive counters and reset only host counter arrays/raster totals. Three host clock reads around unchanged VIF/GIF service loops per outer slice capture rare long bursts that sparse sampling can miss. Record sums/maxima, active-channel counts and total heavy-loop time before budget logging. No per-instruction/qword hooks, no Vif1Path layout change, no guest memory/clock/input/render/fault or host-presentation change. Attribution includes instrumentation overhead and is NOT qualified FPS. Remove after one capture and byte-restore runner before production timing. Protected host1a122b9f... and baseline11.401186864FPS remain unchanged. Manifest artifacts/recovered-host-burst-profile-20260921-preedit.txt.

## HG-DIAG-031 duplicate host-swap count - captured and removed, 2026-09-21

Replay project-link-runtime/20260921T201812Z-ca7efefa54de4b79a56614944ada17ea:7097 swaps of which6235 had no fresh image;862 fresh presentations,42.9934s. Normal guest rendering counts; GPU flush15.2648s/readback3.8024s. Source was byte-restored after capture. The candidate following this measurement is event/refresh-aware native host presentation, with no diagnostic counters retained. Actual swaps are explicitly NOT original-game FPS.

Historical probe:

runtime/game_host.cpp adds two UI-thread integer counters (actual swaps and swaps with no new pixel mailbox frame), reported once at the existing final log with elapsed time. Existing presented means fresh frames, not actual swaps. Activation: this temporary build only, no per-loop extra timing calls. No GL, guest GS/EE/VU operations, memory, faults, input or clocks change. Capture one fixed hidden replay, remove, and use only to test whether redundant presentation load is a plausible source of driver contention. Counts are NOT gameplay FPS. Backup/hash manifest artifacts/host-swap-audit-20260921-preedit.txt.

## HG-DIAG-030 validated GL error-notification startup check - rejected and removed, 2026-09-21

Actual NVIDIA INVALID_ENUM self-test passed and qualified verifier a8086a1ce14a41ebae73903e79a5fa87 preserved all established state/image hashes, but throughput3.595859FPS regressed versus the current polling control4.040570FPS. The callback/debug-context setup, all backend changes and latch-test activation have been byte-restored; production keeps every original glGetError check. The rejected helper is archived externally as gl-error-state-rejected-20260921.hpp; runtime/include/hg/gl_error_state.hpp is an empty unreferenced placeholder because the connector cannot delete new files. No debug callback or startup injected-error test remains active. See HG-FAIL-028.

Historical trial description only:

runtime/gl_gs.cpp GlErrorMonitor verifies debug-context flags, available callback functions, absence of an existing callback, enabled API-error notifications and synchronous mode. One defined glEnable(0) invalid-enum call tests actual driver notification before it returns; only that expected startup error is consumed/reset. No GL object, game data, input or timing state is changed. Unexpected setup/self-test errors fault explicitly; lack of callback delivery retains ordinary polling. Portable first-error latch tests cover filtering, all513 message lengths0..512, reset, empty/null and negative-length safety. The GL callback allocates nothing, calls no GL function and never throws; original host check locations remain and throw upon a recorded error. Ordinary/debug-unavailable contexts continue glGetError. Startup calls/logging occur before native execution; there are no per-instruction/qword timestamps. Retain this guard only with successful actual-driver validation and repeatable qualified performance without state/image drift. Primary KHR_debug sections2.5/5.5/5.5.2/5.5.7; debug flag is essential because ordinary contexts may legally emit zero messages.

## HG-DIAG-029 GL flush API-stage attribution - captured and removed, 2026-09-21

Results: coarse capture project-link-runtime/20260921T193154Z-4ee147f04473426fa031c6fcaf4cdf53 put99.38% of15.283s flush time in cleanup/error-check. Refinement project-link-runtime/20260921T193358Z-e1b5664fb18a4a06a4837c8e5a20c198 measured glGetError alone15.115755s of15.224291s total (sprite6.9420725s, triangle4.2034801s, masked3.9702024s). Ownership/bind and CPU clears were minor. This attributes host latency at the API call, not the reason for it. Normal work counts and no faults; no diagnostic FPS claim. Source restored byte-exact to gl_gs.cpp e5069d2b96f77d573fd92b7038632f9c28c30b4f8742bf3405ae56f047d602ae; all timing additions removed before the independent-context production candidate. Existing correctness error checks remain enabled.

Historical probe description:

Source runtime/gl_gs.cpp only. Temporary aggregate host timers divide each existing flush span into seven stages: VRAM preparation/upload, command upload/bind, tile construction, tile upload/bind, program/dispatch/shader barriers, final update barrier, ownership/cleanup/error-check. Group by sprite/triangle/masked batch and report sums/maxima at destruction. Active in this diagnostic build only, removed after one hidden33M replay. No guest mutation or altered GL commands, ordering, barriers, ownership, faults, input or clocks. Instrumented rates are not FPS evidence. The preceding clean control independently reproduced15.166s flush time, so this investigates a current non-probe-only problem rather than presuming all elevated GPU timings are instrumentation artifacts.

## HG-DIAG-028 post-link build freshness gate - verification infrastructure, 2026-09-21

CMake hg_game POST_BUILD runs tools/hg_build_provenance.py to bind the linked executable hash to a sorted hash snapshot of first-party runtime, generated native sources, config, generator sources, CMakeCache and generated hg project files. The fixed frames verifier rejects missing/stale manifests or a different isolated executable before launching and copies the checked manifest into its evidence directory. Tests include source edits with preserved timestamps, added/deleted sources, non-Release configuration and stale-binary launch prevention. No native/game instruction, input, renderer, clock or frame-metric change; all hashing occurs outside measured native execution. Retain to prevent source-only reverts or test-only builds from being presented as current game measurements. It relies on the configured build dependency graph and is not an independent correctness or fresh-emission proof. CMake post-build semantics verified in official add_custom_command documentation.

Correction to HG-DIAG-026/027 below: abnormal GPU timings were observed during these probes, but instrumentation causality was NOT established with contemporaneous clean controls. Treat the cause as unresolved; do not infer that removing a hook alone restores normal GPU latency.

## HG-DIAG-026 exclusive VU/XGKICK timing split - captured, contaminated, removed, 2026-09-21

Evidence project-link-runtime/20260921T174657Z-425b8f33245740069045fe041d672f82 completed33M and preserved the expected guest work counts, but the run had unexplained host/GPU scheduling latency: full-run GPU flush_ns rose to15162124800 and readback_ns to3781902100, far above normal ~0.57s/~1.0s. Therefore the run is not representative host timing and must not be used as FPS or absolute subsystem cost evidence. Its internal lead was126180 VU calls,91140 XGKICK calls/10539780 qwords,2.2394372s measured VU total,0.2776441s nested XGKICK and1.9617931s remainder; treat only as a hypothesis that VU-core work may dominate nested XGKICK. Both runtime/include/hg/vif.hpp and runtime/system_diagnostic.cpp were byte-restored immediately after capture.

## HG-DIAG-027 XGKICK-only hook timing - captured, contaminated, removed, 2026-09-21

Evidence project-link-runtime/20260921T180839Z-1fd269d9aed0439583ee3ba6a01b11a2 measured91140 whole XGKICK calls at248286800ns during31M..33M using only begin/end hook clocks and no per-qword instrumentation or Vif1Path instance fields. The measurement still fails its acceptance check: full-run GPU flush_ns reached15082715800 and readback_ns3428193800, again far above the established roughly0.57s/~1.0s range. These runs do not establish that timing XGKICK caused the abnormal latency. A later clean control and a byte-identical archived executable reproduce it without the hooks; host/driver conditions remain unresolved. Both runtime files were byte-restored immediately. Do not repeat hot-path clock hooks here; use static/dynamic call-count evidence or end-to-end candidate timing instead.

## HG-DIAG-005 heavy31M..33M GPU readback profile - captured and removed, 2026-09-21

Completed evidence project-link-runtime/20260921T110234Z-2c636777143542a7bb347d2f3992ecca. A temporary host-only reset at slice31,000,000 left VRAM/ownership/queues/GL state/guest state/input/clocks untouched and isolated the final2M slices. Results: sprites31620/pixels494505840, flushes1866/270618100ns, upload38772736bytes, download650149888bytes/readback336099600ns, triangles245616/723 batches, masked35389440/600 batches. Multi-page CPU reads account for420 dirty events,900 GL ranges,79140 pages,648314880bytes and335193200ns; one dirty multi-page write adds224pages/1835008bytes/906400ns. Thus synchronous readback is a real secondary cost but not sufficient by itself to explain the roughly6+ second heavy qualified interval. All temporary interface/backend/runner changes were byte-restored after capture. Do not use the instrumented run as FPS evidence.

## HG-DIAG-005 synchronous GPU readback distribution/coalescing footprint - captured and removed, 2026-09-21

Two observation-only runtime/gl_gs.cpp probes completed and were byte-restored. The first capture, project-link-runtime/20260921T103855Z-dbe0e619b99a418ab34b14ee61f1eada, kept requested pages/order/barriers/transfer APIs unchanged and classified dirty downloads by trigger. Multi-page CPU reads accounted for1731 dirty events,3537 glGetBufferSubData ranges,326778 pages,2676965376bytes and1006640500ns:98.93% of measured readback time and99.83% of downloaded bytes. Single-page CPU writes were336 events/2.75MB/10.08ms; only one dirty multi-page write occurred.

The second capture, project-link-runtime/20260921T104328Z-311b52eb2a58411eb94a3697ae3b1591, measured a hypothetical first-to-last staging span without changing actual transfers:1735 dirty multi-read events,327338 dirty pages,351042 bounding pages,23704 gaps (+7.241% volume), max bound496 pages; range-event counts were1083 one-range,458 two-range and194 five-to-eight-range. These probes were attribution only, not FPS measurements. The resulting staged-bounding production experiment preserved state but failed throughput and is recorded as HG-FAIL-025; all temporary counters are removed.

## HG-DIAG-005 triangle framebuffer-feedback mapping/proof - captured and removed, 2026-09-21

Observation-only31M..33M probes derived from existing CPU/GPU job data classified all18378 measured alias rejects as nearest PSMT8H with texture/frame base and stride equal, then independently checked794236/794236 covered samples mapping to their own destination pixel/CT32 word. A final per-draw proof measured Q equal and exactly1.0 at every vertex plus exact4-bit source-minus-screen vertex deltas; all18378 draws passed and every delta was dx=dy=8, establishing the positive-subtexel convex bound for this captured class. Evidence project-link-runtime/20260921T083235Z-0a4b130471b5400693210ae98b0b9330, project-link-runtime/20260921T083540Z-16227b5d09bc4cef88f6fe9c5aee1c54, and project-link-runtime/20260921T084132Z-1247e861480f4e5a9833ce0ee51da2b4. No backend decision, memory, guest state, input, clock or raster result changed during the probes. Temporary counters and the31M hidden activation are now removed. The later specialization based on this proof was rejected for throughput in HG-FAIL-024; the proof itself remains valid only for the observed guarded class.

## HG-DIAG-005 triangle semantic-no-op output hazard split - captured and removed, 2026-09-21

Completed capture project-link-runtime/20260921T081347Z-cf5afc140bad4634a1c56ca8265a5f8c. All18378 measured GPU-triangle alias rejects overlap framebuffer pages; none overlap depth pages, and none have a fully masked/no-effect framebuffer write. The unchanged backend rejected every case and CPU fallback executed as before. Conclusion: this measured class is genuine framebuffer feedback at page/output-semantics level, not a false dependency from FBMSK. Temporary counters and31M activation are removed after this capture.

## HG-DIAG-005 CLUT pending-draw overlap potential - captured and removed, 2026-09-21

Completed capture: project-link-runtime/20260921T064340Z-f8f2c6d380b2427d87b8dc9b60833544. During31M..33M the unchanged CLUT barrier saw49564 loads;23399 had pending work, summing586858 queued draws with max1164.585418 pending draws used indexed textures. Conservative exact CLUT-source-page versus captured full-scissor FRAME/ZBUF-page analysis found zero overlapping loads and all586858 pending draws in a theoretically deferrable tail. The original full barrier still executed; this is attribution only, not an FPS result. Temporary counter/helper and system_diagnostic activation were removed before the palette-version candidate. The conclusion is retained in PROGRESS; do not treat zero overlap in this captured scene as a universal proof for all game states.

## HG-DIAG-005 temporary steady-gameplay IMAGE format split - 2026-09-21

With the streaming candidate disabled, runtime/gs.cpp wraps the unchanged original
packet application only when existing GS profiling is enabled. Whole IMAGE-only
packets are grouped by initial BITBLTBUF destination format; mixed/register/empty
packets are explicitly bucket64, not guessed into an IMAGE format. Successful
packet/transfer counts and application durations are printed once by a bounded
thread-local destructor. Classification happens before the timer; it still
perturbs cache/host cost. Existing per-draw GS timers are enabled too. This is
attribution, not performance evidence. Exceptions propagate with original state.

Activation: system_diagnostic.cpp turns on profile_gs only at31M for the configured
profiled unpaced33M replay; no guest input/clock/render operation is changed.
Default live/realtime runs do not activate it. Remove after one useful capture.
Source byte backups: gs.cpp898a844947e54e7b90ffbb8082c6b526 (includes disabled-trial
streaming source to archive), runner7d8915155d8b4919b50ca990678748f6. Restore the
runner exactly; restore GS then explicitly keep streaming off before production.

## HG-DIAG-005 gameplay GIF command split - captured and removed, 2026-09-21

Completed capture: project-link-runtime/20260921T043204Z-a41c1884534f4d96bc61a2fbd033baef.
The31M..33M deltas count17,850,014 qwords and1.4821181s GS application,
0.1863614s decode/materialization,0.3895350s append and0.3689151s scan bucket.
Per-qword timing overhead is included; scan bucket also includes destruction of
the previous packet. GS application includes nested raster/transfer work. These
are attribution only, not a sum of exclusive whole-system costs or an FPS claim.
Temporary counters, hooks and conditional activation have been removed from
source. system_diagnostic.cpp is byte-restored to bcb9f289; old instrumented
binary must not be labelled production until a clean build completes. Only the
out-of-line GifPath boundary remains for the explicitly tested packet candidate.

Reporting: fixed verifier now retains its full five-window audit trail but uses
only final two steady-gameplay intervals for gameplay_fps and the30FPS verdict.
It divides total original boundary count by total host seconds, and requires
both intervals to meet30. After-run capture digests cost no measured game time;
IOP normalization masks only documented RTC seconds/minutes/hours and validates
BCD ranges. Original captures remain untouched. No guest clocks/input changed.

### Historical probe implementation (no longer active)

GifPath::submit_qword is moved without transport changes into runtime/gs.cpp.
Temporary thread-local counters split append, packet completeness scan, decoded
transfer materialization, GS application (including nested raster/transfer waits),
and suffix erasure. Activation is only profile-enabled/unpaced33M at30M in
system_diagnostic.cpp. Checkpoint/budget output is therefore gameplay-only, with
no menu time. Clock-query overhead and out-of-line code placement perturb timing;
this single capture is attribution, never an FPS benchmark. Guest bytes, input,
qword/commit order and faults remain unchanged. Restore activation backup
d816454d55b44f679559a750eeffe8af and remove timers after capture. Header backup
08920fc4f73444c392d6705e61721e5b and GS backup b83e4d2b72b6472ab81d71f30b6c8455
preserve the exact pre-refactor files. Compare original state/image evidence.
All three preceding conditional/dense/VU timer activations have already been
restored in source; their historical notes below are not active probes.

## HG-DIAG-010 temporary VU entry timer - 2026-09-21

system_diagnostic.cpp temporarily wraps its existing AOT executor at30M only in
profile-enabled unpaced33M runs without explicit VU capture. Counts/time per original
entry are printed at checkpoints and budget. Original execution and exception
propagation remain; no guest writes or changed inputs. Timer overhead is unknown;
exclude from FPS/timing comparisons. Restore backup06f01c8a7d7c416a94bc0fe34f04d996
after one capture. Dense phase header was restored byte-exactly after evidence
27078e0ee0664d6885448a2c590e257e showed VIF/GIF4.857s as the dominant phase.

## HG-DIAG-010 temporary dense tail attribution - 2026-09-21

Temporary diagnostic_profile.hpp selection samples every enabled-profile slice in
30M..33M; ordinary sampling elsewhere. No guest changes. Purpose: avoid missing
rare long execution bursts when attributing the current slow segment. Map/timer
cost perturbs the run; no timing/FPS claim from it. Restore backup
015d894507f0403d8b933086fb29ca12 after one fixed replay, then rebuild.
The preceding HG-DIAG-005 conditional activation was removed byte-exactly after
capture d1c61ba280c946609521871303ef07b4; ordinary --profile-gs remains unchanged.

## HG-DIAG-005 temporary current-build attribution - 2026-09-21

Temporary activation in system_diagnostic.cpp: existing per-primitive timing is
enabled for profile_enabled && !realtime && slices==33000000, with a startup banner.
Purpose: current30M..33M cost attribution through the fixed Project Link replay.
No guest-state/clock/draw/input changes. Host timing/logging cost is nonzero and
unmeasured; exclude the diagnostic run from performance claims. Restore the exact
source backup and rebuild after one completed capture. Normal realtime/manual
sessions do not enable this conditional observer. Keyboard controls are production
host-input mappings, not diagnostic injection; see CONTROLS.md.

## HG-DIAG-001 VU0 fault operands - 2026-09-20

Existing system_diagnostic.cpp EE fault output now includes a2/sp, all32 backing
VF registers and ACC/MAC/status. Trigger is the existing caught EE Fault; no
extra instruction polling, guest writes or normal execution work. Retain bounded
exit report to diagnose original operand-dependent stops; fault-output cost is
unmeasured and excluded from gameplay timings. First target: live10dac8 VMADDw.

## HG-DIAG-010 post-masked profile - 2026-09-20

Accepted masked-transfer binary/map:3265 samples, zero errors. Main2725, ntdll430,
VCRUNTIME91; full multiply144, VIF parser126, XYZ add prepare100, scalar FPU add85,
plain multiply69, masked writes54. Profile replay matches original images/EE/GS/VU
and verified RTC-only IOP. Sampling excluded from timing results. Saved artifacts
post-masked-gameplay-*. Hidden benchmark changed-image intervals were11.163-11.166/s
for candidates versus10.833/s controls; these are not visible or internal-game FPS.

## HG-DIAG-017 masked transfers retained - 2026-09-20

1042 GPU/CPU cases per mode and four original state/image comparisons pass (RTC
only). Scene mean7.53305 ->7.33985s,2.565% lower, both pairs faster. Retain optional
resident masked-write batching. This defers host materialization, not guest writes;
reads, draws, copy and accelerator teardown preserve ordering. Source/activation
remain in candidate entry below, now validated/retained; no new timing substitute.

## HG-DIAG-017 masked resident transfer candidate - 2026-09-20

Optional memory observer accepts masked word writes only for resident GPU-owned
pages. Unique-word jobs merge repeated masks before integer GPU RMW; no readback
is needed until a CPU consumer. Existing draw/CPU hazards flush this batch, and
copy/teardown use ordinary materialization. CPU-owned and nonresident memory keep
the scalar path. Enabled only by existing opt-in resident acceleration; no guest
state substitute or clock change. Counters report queued word updates/batches.
Performance and original-state trials pending; remove if no repeatable gain.

## CURRENT - stack profile isolates GS transfer waits, 2026-09-20

External StackWalk64 sample:325 contexts, zero context failures, depths6..20;
276 walks end with API false below20,49 reach the20-frame cap. Do not claim every
stack fully unwound. Among62 top-ntdll samples,39 unwind through Accelerator::
download and15 through flush. Many download chains directly reach PSMT8 host
transfers or CPU raster scopes. This uses unwind metadata, not stack-word guesses.
Original images/EE/GS/VU match; only verified IOP RTC offsets differ. Replay exit2
native-iop-budget, sampler/launcher exit0. No replay/build/profile remains active.
Evidence post-stackwalk-gameplay-native-sample.log, summary and equivalence JSON.

Next candidate lead: resident masked host-transfer writes can preserve untouched
GPU word bits without downloading their pages first. Scope PSMT8 fast16-byte
columns, queued unique-word masked updates; must preserve CPU reads, queued draw
order, partial transfers, overwritten lanes and memory lifecycle. Need written
implementation plan/backups and GPU/CPU dependency tests before changes. Unlike
rejected read-footprint narrowing, this removes a demonstrated readback boundary.
No such production transfer change implemented yet. Accepted game/map remain
AVX2 product + untextured triangle build restored from vu-full-vector-baseline.
Library objects/headless/tests require rebuild after rejected full-vector revert.
30FPS remains unmet; latest visible changed-image measurement still~9/s.

## HG-DIAG-010 post-product4 profile - 2026-09-20

Fresh accepted binary/map sampled five seconds after30M in a hidden33M replay:
3254 samples, zero context errors. Main image2517, ntdll618, VCRUNTIME102.
Full multiply132, VIF parser122, XYZ prepare84, scalar add65, plain vector
multiply63 and vector add62 samples. No full stack unwind or specific ntdll wait
cause inferred. Original images/EE/GS/VU match; only verified IOP RTC bytes differ.
Artifacts post-product4-gameplay-native-profile.json and corresponding completion/
equivalence. Sampling perturbs timing and is excluded from speed benchmarks.

## Retained untextured triangle coverage - 2026-09-20

HG-DIAG-018 opt-in --gpu-triangles now also accepts bounded untextured flat/Gouraud
triangles with the existing CT32 frame, Z32/Z24, depth/alpha/blend restrictions.
Job flag512 suppresses STQ/texture sampling and texture-page dependencies, preserving
frame/depth ordering. Textured path unchanged; unsupported states retain CPU/faults.
1018 GPU/CPU cases pass both modes, including actual untextured GPU acceptance,
invalid unused texture state and mixed CPU/GPU ordering. Original captures pass; matched hidden scene mean8.2572 ->7.73375s (6.339% lower).
No visible30FPS or complete physical-console fidelity claim.

## Independent-triangle fallback observation - removed after identification, 2026-09-20

HG-DIAG-005 temporary gs.cpp logging (now removed), only --profile-gs, printed PRIM/frame/Z/test/texture/alpha and
clipped bounds for independent triangles (primitive3) that reach CPU fallback.
Motivated by292 calls consuming0.364s of profiled scene time. No changed rendering,
submission, guest state, or memory ownership. Default off; logging cost unknown,
exclude these runs from speed comparisons. All292 observed cases were PRIM0x4b untextured blended triangles; detailed logging
was removed before candidate measurements. External triangle-fallback-profile.log
retains the evidence. Existing aggregate raster profiling remains unchanged.

## Hidden correctness replay host - 2026-09-20

HG-DIAG-011: game_host.cpp --hidden-host leaves the created GLFW window hidden,
skips taskbar/show calls, and disables host controller polling. Default off. GL
contexts, guest work and committed-frame presentation code remain. Intended for
unattended exact-state/controller-log replays so they cannot be mistaken for a
live-input window. Hidden-window driver scheduling may differ; do not compare its
presentation/performance measurements with visible-window runs. No guest timing or
state change beyond the explicitly selected deterministic input source.

## 2026-09-20 recorded controller replay and audio observation

HG-DIAG-006: system_diagnostic.cpp accepts --input-at slice
pad:buttons,axis0,axis1,axis2,axis3,pressure0,...,pressure11. Strict decimal values,
16-bit buttons and byte axes/pressures, exactly17 fields. Applies the same complete
controller state logged by --profile at the specified slice. Default off; guest
input intentionally changes only when selected. Legacy input commands unchanged.
Retain for deterministic user-fault reproduction; parse only at startup/events.

HG-DIAG-014: with --profile and --spu2-output-core, report existing speaker frame,
underrun and peak counters at checkpoints and fault exit without flushing the queue.
Host-only output, default off, no guest changes. Logging cost unknown; exclude from
performance comparisons unless identically enabled. Retain while diagnosing reported
intro sound gaps; no changed gain, buffering, PCM production or guest timing.

# Diagnostic and experimental-code inventory

Current audit: 2026-09-16. Stable identifiers make these items searchable and
accountable during optimization and handoff. This inventory covers the identified
active families; it does not certify that every historical diagnostic is removed.
Correctness validation and explicit unsupported-hardware faults are required
runtime behavior, not debug code to delete for speed.

| ID | Source / purpose | Activation and state effects | Cost / disposition |
|---|---|---|---|
| HG-DIAG-001 | `runtime.hpp::trace_pc`, EE emitter: instruction/register history | HG_EE_INSTRUCTION_TRACE defaults ON; history records only with EE watches. OFF removes calls and rejects PC watches explicitly. No guest writes. | Native profile39:161/3254 samples in the function with no watches. Combined candidate40 (call-site guard plus forced register inlining) preserved state but slowed playback and was reverted. Isolated OFF replay42 takes50.2381s vs51.8379s37 with matching guest times, EE/GS/machine state and1311images. Synthetic branch/watch/fault checks pass. Retain opt-in traces. |
| HG-DIAG-002 | `iop.hpp::trace_pc` / `trace_sifman_pc`, IOP emitter: instruction history and SIFMAN event snapshots | HG_IOP_INSTRUCTION_TRACE defaults ON. Candidate49 local build OFF removes per-instruction history and explicitly rejects PC watches; statically placed SIFMAN entry/return snapshots remain active. No guest writes or load-hazard changes. | Profile48:76/3321 samples before gating. Audited history consumers: worker return-time reporting, IRQ/error history copies, final reports and explicit watches. Those history reports are unavailable when OFF; SIF snapshots and their downstream capture triggers remain. All23 CTest entries pass, including the full IOP suite in both configurations. Replay49fixed takes35.2681s vs36.0054s47fixed with equal guest times, EE/GS/machine state,1316 images and retained SIF/SNDDRV reports. Retain opt-in history and explicit correctness faults. |
| HG-DIAG-003 | `runtime.hpp::store`: fixed EE write watch at0198cbc8 | Always checks overlap; records writes to the startup semaphore word. No substituted value. | Cost not isolated; scalar/quad RAM paths preserve watch behavior. Replace with an explicit diagnostic option after startup regressions no longer require its history. |
| HG-DIAG-004 | `system_diagnostic.cpp`: SIF/RPC/worker histories, `iop_dmac.hpp` histories | Automatic startup provenance; several host scans occur each quantum. Reads original state, records host history. | Dense profile35 isolates `diagnostic_history`; native profile39 is available. Candidate54 gates unchanged completed rings after auditing insertion/return/reset sites. All24 tests pass after explicitly building the newly generated test target. OPENING34.6978s versus36.1589s53 (4.04% less elapsed);1066 guest times, EE/GS/machine state,1315 shared images and49 retained SIFMAN/SNDDRV report lines agree. Cache affects observation work only; retain report equivalence checks. |
| HG-DIAG-005 | `diagnostic_profile.hpp`, system diagnostic phase marks and CSC/disc event output | `--profile`, default off. Host timestamps, sampled PC costs and event logs; no guest time substitution. | Sampling1/256,11phases. Needed for current evidence; compare runs with identical options. Disable for final presentation measurements and report that setting. |
| HG-DIAG-006 | `system_diagnostic.cpp` preview/dumps and external replay helpers | Explicit `--preview-file`, `--dump-iop`, input file, ordered `--input-at slice command`, and checkpoint options. Preview copies committed scanout; dumps are diagnostic artifacts. `--dump-iop` also writes exact EE RAM, GS state/VRAM, the current 16KiB VU1 MicroMem image, VU1 data memory/defined masks, and compact VU1 register state for AOT identity and packet-source analysis. Input scripts intentionally change button/pressure state. Optional --capture-vu1 external-prefix with --capture-vu1-tex0 selects the first64 matching AOT VU1 calls, saving before/after memory/registers and entry metadata. The existing AOT executor runs unchanged; the observer detaches after64 calls. Default has no scheduled events or VU observer. | Filesystem/image sampling adds host cost; no renderer flush allowed. VU1 captures are host-only reads at dump time and do not alter VIF/VU state. Candidate62 cache/counters rejected and removed:394 unchanged writes avoided, but35.3766s vs34.9217s58 shows no playback gain. Exact guest times, EE/GS/machine state,1315 shared images in order and49 retained reports agree. Exploratory post-opening helper adds normal Start at27M/none27.1M to the fixed schedule; this intentionally skips OPENING and cannot establish full-movie behavior or timing. Host-timed pulses caused different end states in repeats42fresh/43fresh. Exact input-at events replay recorded guest slices; seven malformed/conflicting argument cases are rejected. Scheduled-event host cost has not been isolated. VU call capture writes bounded external files.32M capture on/off checks preserve EE RAM, GS memory/draws, VU memory/state and image exactly; two IOP RTC bytes differ consistently with launch time. Enabled cost is not isolated; disabled mode keeps the original executor pointer and adds no VU call overhead. Retain only while resolving character geometry provenance. Retain for repeatable validation and compare identical schedules/capture settings. |
| HG-DIAG-007 | `system_diagnostic.cpp --toc-record`, IOP TOC input | Explicit external TOC record. Current replay helpers supply a synthetic index-pattern record. **Guest-visible experimental input.** | Quarantined startup probe, not physical-drive correctness. Never promote this input to a normal production default; replace only with independently verified disc behavior. |
| HG-DIAG-008 | `execution_clock.hpp`, `--clock-profile issue-slots`, worker budgets | Explicit experimental clock profile; legacy is default. **Changes guest work/time allocation**, including bounded polling yields and DMA pacing. | Not a PS2 cycle/cache model. Preserve and disclose exact profile in comparisons; retire the approximation only with independent timing evidence. |
| HG-DIAG-009 | `iop.hpp` event/CDVD/RAM traces; kernel semaphore/SIF histories | Event-driven bounded history, some default-on, optional address/PC watches. Host records of native services. | Aggregate cost not isolated. Keep until consumers audited; new instrumentation must state its activation and cleanup criterion. |
| HG-DIAG-010 | External native sampler and ignored Release linker map | Explicit profile-only replays39/45/48: bounded suspend/GetThreadContext/resume sampling for five seconds. No guest-memory edits. | Latest48:3321 samples,0context errors; CT32 addressing181, raster126, rgba106, IDCT99, IOP history76. Current47 PE/map timestamps were verified equal before sampling. Perturbs host timing; never use sampling replays as speed comparisons. Map enabled only in ignored vcxproj; regeneration removes it. |
| HG-DIAG-011 | `opengl_host.cpp --presentation-log`: changed-image submission timestamps and live-preview presentation validation | Explicit external CSV path, default off. Records only full-image changes after a nonempty framebuffer swap; repeated file contents do not increment the changed-image count. The live-preview readback check accepts the one-texel neighborhood around the analytically selected source texel to account for legal nearest-filter half-texel selection; the separate synthetic exact-color OpenGL probe is unchanged. No guest access or clock changes. | Optional timestamp/CSV cost is not yet isolated. The prior exact live-preview sample could false-fail before the first swap and terminate the viewer with `Diagnostic preview readback mismatch`; the corrected viewer survives direct/Explorer launch with advancing live preview. Buffer swaps, complete file reads and changed images are separate counters. Measures host submission, not physical display scanout. Retain for presentation validation; log off for uninstrumented use. |
| HG-DIAG-012 | `host_pacing.hpp`, `host_wait.hpp`, `system_diagnostic.cpp --realtime`: host playback limiter | Explicit `--realtime`, default off. Steady-clock deadlines every10ms,100ms lag bound; Windows high-resolution waitable timer. No guest clocks, budgets, device order or input changes; disabled mode creates no timer. | Waits excluded from sampled execution phases; reports requested/actual waits, maximum overshoot and rebases. Candidate60 adds profile-only per-checkpoint counters to localize overhead.25 CTests pass.59paced OPENING100.175% and CAPCOM100.593%,matching58 guest timestamps, EE/GS/machine state and49 retained reports.57 was slower on the same binary; host variance remains unresolved. Timing/check cost not isolated. Retain opt-in; no physical-console parity claim. |
| HG-DIAG-013 | `runtime/include/hg/diagnostic_wav.hpp`, `system_diagnostic.cpp --spu2-input-wav/--spu2-input-core` and `--spu2-sync-log`: raw SPU2 sound-input and movie-sync provenance | Explicit options, default off. WAV capture writes only the selected core's already-modeled 48kHz stereo input frames. Sync logging watches the original EE movie formula/callbacks, callback-A queue root/live node length, callback-B buffered-byte count, the six verified queue/ring methods, and core-0 AutoDMA MADR/BCR/CHCR/remaining/count/bytes. It writes the requested sync CSV plus `<path>.events.csv`, draining watched-write history every guest slice so writer events are not lost. These are host observations only; no guest memory, device state, clocks, budgets, or scheduling are changed. | File I/O, trace watches, write-history scans and per-sample statistics occur only while enabled. The focused 19.4M replay captured 1024-byte half transfers every 256 frames and 2048-byte descriptor completion every 512 frames without underrun/fault. Retain until streamed-audio timing no longer needs provenance tracing; WAV capture may remain useful until final SPU2 synthesis/mix/output exists. Neither output is final speaker audio or physical-console timing proof. |
| HG-DIAG-014 | `runtime/include/hg/host_audio.hpp`, `system_diagnostic.cpp --spu2-output-core`: Windows host presentation of the already-modeled raw SPU2 input stream | Explicit option, default off. Sends the selected core's 48kHz stereo16 frames to the default WinMM `waveOut` device using eight 1024-frame host buffers. Playback is paused until four buffers (~85ms) are queued; total queue capacity is ~171ms. The speaker path applies 3/4 (~-2.5dB) host-only headroom. Host-only underrun, queue-depth and input/output peak counters are reported. No guest memory, source PCM, clocks, DMA state, budgets, scheduling or callback results are changed. | Host audio-device latency and blocking affect wall-clock execution only while enabled. A focused 26M realtime movie probe produced 1,243,627 speaker frames with 0 underruns and max queue depth 7 after replacing the audibly scratchy 4x256-frame version. The retained long movie capture peaks at 31944/32767 with no hard-clipped samples, motivating presentation headroom without altering guest audio. This remains a presentation aid that bypasses unfinished SPU2 voice synthesis, volume/effects and cross-core mixing. Retain until the final SPU2 mix/output path supersedes it. |
| HG-DIAG-015 | `runtime/opengl_host.cpp::configure_taskbar_window/register_taskbar_window`, `tools/run_movie.ps1`: Windows viewer presentation | Windows viewer configures the real GLFW window as an unowned app window with shell identity and taskbar registration; no proxy exists in current source. Launcher selects WinSta0\Default explicitly. No guest-state or input effects. | Audited 2026-09-19. Host shell cost unmeasured. Earlier minimized-proxy description was stale. A visible HWND on the private Codex desktop does not prove user visibility. Retain desktop selection for diagnostic viewing; reassess shell helpers with the final host. |

| HG-DIAG-016 | `gl_gs.cpp`, `gs.cpp`, `game_host.cpp --gpu-sprites`: compute sprite experiment | Explicit option, default off. OpenGL4.3 worker context; checked CT32 FST sprites use CPU-prepared exact addresses/fractions. Destination mapping must be injective, tests always pass, depth masked, and texture/frame pages disjoint within each draw. Ordered dispatch batches finish at existing pending-draw/CPU boundaries. | 232 GPU/CPU wrap/blend/feedback/mixed/fault cases and original33M equality passed. Native scene31.6796 ->22.9379s in matched comparison;32848 draws,443M pixels,721 flushes. Per-draw profile can charge deferred GPU work to a later primitive or pending-list completion; do not blindly add its timings to GPU flush time. Whole-console fidelity/target speed unproved. |

| HG-DIAG-017 | `gs_memory.hpp`, `gl_gs.cpp`, `game_host.cpp --gpu-resident`: GPU-resident local memory | Explicit option implies GPU sprites; default off. Observer tracks CPU/GPU ownership by8KiB pages. Ordered uploads precede dispatch, CPU reads materialize dirty GPU pages, CPU writes submit preceding draws first. CPU raster scopes synchronize once and suppress per-pixel callbacks. Copy/move/reset/teardown preserve memory or discard only destroyed storage. | 241 GPU differential checks and original33M state/image comparisons pass (RTC excepted). Initial matched sequence: pre-wrapper24.7895s, wrapper29.5581s, resident30.2274s; later nonresident comparison narrows overhead to about5%, so variance is material. CPU overhead remains under investigation; reduced upload bytes are not an accepted speed win. Extra fields are host-only. Retain only with guest/image equivalence and acceptable host cost; broader GPU coverage is the intended next use. |

The current successful opening still runs in a diagnostic host. Synthetic TOC
input and experimental timing are meaningful limitations, not evidence that
missing translated instructions are being silently skipped. The fresh complete
scan and unresolved-site classification are in `DECODING_SCAN.md`.

Rejected experiments: sparse IDCT26, alternate bit reader34 and byte-copy
batching36, combined trace/inlining40 and framebuffer-address41 showed no connected speed benefit and were reverted. Their external
evidence remains for accounting; do not reintroduce them without new evidence.
### HG-DIAG-006 preview-rate measurement (2026-09-19)

External TEMP/hg-linear-fps.py and hg-linear-center-fps.py optionally poll the
published PPM every2ms, hash each new file and record host timestamps. They are
not runtime defaults and do not modify guest state. Host file I/O/hash/polling
cost is included in observed wall time and not separately isolated. The report
distinguishes publications from changed images; neither is an internal game
frame count or physical scanout FPS. Retain the external measurement until a
verified original frame/presentation counter can replace this limited metric.

### HG-DIAG-011 native host validation extension (2026-09-19)

`runtime/game_host.cpp`: opt-in `--verify-presentation` checks OpenGL readback
against a source pixel, requiring a nonblack match before success. Readback can
stall the GPU; cost not isolated. `--host-frames N` closes after N fresh presented
images (or runtime termination); it bounds execution through the normal close
signal. `--no-host-input` suppresses live input for deterministic comparisons;
scripted diagnostic input remains enabled. All three default off. Retain until
native presentation/input regression coverage replaces these probes. The normal
title counts published video snapshots per host second, not game frames or swaps.

### Optimization evidence audit (2026-09-19)

HG-DIAG-005/006/010/011 remain the measurement mechanisms; no new runtime probe
was added for the speed change. Native samples identify at least70.8% named GS
work. Matched replay options, no competing live game, and state/image comparisons
gate the retained pixel-stage factoring (11.75% mean scene-time reduction).
Texture-setup candidate lacked a measurable scene benefit and was reverted.
The live viewer remains separate from timed replays. Evidence and exact RTC
exceptions are recorded in PERFORMANCE.md; reusable lessons in learned skills.md.

### HG-DIAG-005 full raster-call timing (2026-09-19)

`system_diagnostic.cpp --profile-gs` enables GS-side per-primitive call counts and
cumulative host nanoseconds in `gs.cpp::rasterize_draw`, including implicit FINISH,
CLUT and transfer flushes. Default off: no per-draw clock queries. Host-only arrays
are excluded from guest dumps; guest clocks, draws and scheduling are unchanged.
Two clock queries per completed primitive while enabled; cost to be measured.
Faulting primitives retain original propagation and are not counted as completed.
Retain until CPU/GPU workload attribution no longer needs this diagnostic.

The runner re-arms --profile-gs at each quantum after GS reset; counters are since
reset, so a reset-quantum call could be omitted. In the isolated33M validation,
all861346 completed draws were counted exactly and matched the dumped total.
Host-only profiling preserved compared image/EE/GS/VU state. Raster time split:
74.24% sprites,25.19% triangle strips. Never infer speed gain from this instrumented
run alone; compare identical profiling options in before/after replays.

HG-DIAG-011 native --video-rate-log (2026-09-19): opt-in once-per-second host
interval reports produced, newly presented, and changed presented images. Compares
complete incoming image to previous presented image and retains it only when
enabled. No guest writes, draw flushes or clock changes. Disabled path does not
compare/retain images or write rate logs. Comparison and logging cost unisolated;
use same option for matched instrumented runs. Changes include any pixel change,
not proof of original simulation-frame boundaries. Retain for 30-update/s evidence.

| HG-DIAG-018 | gs.cpp, gs_triangle_acceleration.hpp, gl_gs.cpp, gl_triangle_shader.hpp, game_host --gpu-triangles | Explicit opt-in OpenGL integer triangle tile batches. CT32 frame, Z32/Z24, CT32/PSMT8/PSMT8H with CT32 palette, bounded positive-Q coefficients, checked tests. Per-pixel lists retain draw order; feedback and mapping hazards flush or use CPU. No guest clocks changed. |345 GPU/CPU cases pass resident; revised original replay and four matched comparisons preserve state/images (RTC excepted). Scene mean22.45325 ->17.62155s,21.52% reduction. Profile962triangle batches/1683totalflushes. Default off. Retain checked bounds/fallback; whole-console fidelity remains unproved. |

HG-DIAG-010 fresh native sample: sprite-format-native-sample.log contains3266
samples/zero context errors. product265, add126, VU flags119, require_vf96,
read_vf61, multiply_acc61;554 outside-main-image samples are unclassified.
Sampling replay is separate from all timing comparisons. Current generated map
matches the sampled executable; source/binary retained in vu-inline-baseline.

HG-DIAG-016 sprite-format expansion: same opt-in activation, independently ports
existing CPU arithmetic for solid and FST CT32/24/16/16S/Z24/8H textures into
CT32/24/16/16S frames. Checked DATE and Z32/Z24 tests/writes, TEXA, dither,
blend and masks. General path only inside pending batches where per-draw bool
is unobserved; small/unsupported draws and conservative feedback aliases use CPU.
One invocation owns both16-bit lanes of a destination word.853 GPU/CPU checks
pass both memory modes, original33M state/images agree. First scene13.5048s;
paired mean18.65525 ->14.22910s (23.73% reduction). Retained. Extra coverage raises
transfers/readback. Movement changed-image rate5.52-6.22/s. No clocks or
guest instructions changed. Retain only with repeatable speed and equality.

HG-DIAG-017 disjoint-write guard: CPU writes now submit queued work only if the
affected pages intersect queued reads/writes. Prior GPU dirty data is still read
back before modification; CPU dirty tracking is unchanged.856 cases pass both
modes and four original replays agree (RTC excepted). Mean scene12.7671 ->12.55475s
(-1.66%); flushes3007 ->2415 and triangle batches962 ->444. Retained host ordering
optimization; does not change guest clocks or claim physical cache equivalence.

HG-DIAG-016 opposite-lane candidate: permits an otherwise rejected16-bit sprite
only with<=8 distinct destination columns, zero filter fractions, identical source
and destination physical words and opposite16-bit lanes. Separable address checks
prove all sampled bits remain unchanged; depth/frame alias rejection remains.
 896 GPU/CPU cases pass both modes; original replay/speed pending. No guest-state
 substitute. Retain only after original comparisons and measurable benefit.

HG-DIAG-016 opposite-lane result: retained after four original comparisons.
Scene mean12.58175 ->11.18555s (11.10% reduction), images and compared guest
state equal except RTC.896 GPU/CPU checks pass both modes. All5888 observed alias
rejections now accelerated with the bounded independent-halfword proof above.
No guest-state substitute; flags remain opt-in/default off. Readback still1.894s;
whole-console fidelity and30updates/s unproved. This supersedes pending status.

HG-DIAG-017 exact upload suppression: gl_gs.cpp retains4MiB host mirror with
per-page validity after transfer, invalidated by GPU writes or owner changes.
Resident opt-in only; exact memcmp suppresses unchanged uploads, no guest effect.
896 cases both modes and four original comparisons pass (RTC only). Measured
mean scene1.93% lower; copies/comparisons included. Retain while benefit persists.


HG-DIAG-017 bulk scanout: gs.cpp::display_image synchronizes committed VRAM once
read-only, then performs existing address/format conversion. No guest pending draw
flush, CPU dirty marks or guest-state mutation.901 GPU/CPU cases both modes and
four original replays pass. Full replay mean4.38% lower; scene only0.888% lower.
Additional pages can be downloaded; measured~2.90GB,readback1.33-1.35s. Retain
while full-run benefit persists; no full-console or30fps claim.


HG-DIAG-017 pending PSMT8 bulk candidate: gs.cpp::write_image_psmt8_fast uses a checked16-word mutable span for aligned16-pixel uploads; gl_gs.cpp handles any same-page span through the existing dependency guard. Guest byte lanes/completion are unchanged; scalar fallback remains. Active with normal PSMT8 transfers, GPU observer only when enabled. Host cost unmeasured pending original replay; retain only after scalar/GPU/state equivalence and matched benefit.

HG-DIAG-017 PSMT8 span candidate retained:910cases both modes and4original replays match RTC excepted;9.10% lower scene time. No guest-state substitute,new toggle or timing override. Checked span remains general memory API; keep scalar fallback and dependency tests. See PERFORMANCE and HG-LEARN-019.

HG-DIAG-017 pending scanout-page candidate: checked const spans in gs_memory.hpp and page selection in gs.cpp::display_image synchronize only displayed physical pages; coalesced runs and page pointers replace whole4MiB read. No dirty marks or guest draws executed. Enabled for existing display capture;host cost pending.216scalar rectangle/format/stride/base cases check exact observed page set and pixel bytes; retain only with GPU/original equivalence and matched benefit.

HG-DIAG-017 displayed-page scanout retained:216footprint/pixel cases,910GPU both modes and4original captures pass RTC excepted. Full replay1.02%faster;scene0.74%mean uncertain. Downloads~130MBless but readback time higher;keep measurement caveat. No guest effects or new flag.


HG-DIAG-017 readback-origin probe: runtime/gl_gs.cpp optional environment
HG_PROFILE_GS_READBACK enables aggregate actual readback bytes/calls/existing
elapsed timer by CPU writable whole-range/partial/read-only/other origins.
Disabled by default;no guest state,ordering,or extra synchronization. Host cost
unmeasured;one conditional and counters per actual download when enabled. Remove
after attributing the dominant readback source;profile run excluded from timings.

HG-DIAG-017 sparse-scope candidate: gs_memory.hpp optional sparse CPU scope mode,
gs.cpp raster loops opt in after existing flush. Synchronizes each actual8KiB page
once forread andupgrades forwrite;no GPU submission orowner/size mutation in scope.
No guest timing/arithmetic changes. Host cost pending;retain only afterownership/
fault tests,GPU/original equivalence andmatched timing. Readbackprobe foundwhole
CPU scopes2.335GB/1.099s outof2.779GB/1.767s;diagnostic runstateequalRTC-only.


HG-DIAG-017 2026-09-20 disposition:readback-originprobe REMOVED afterattribution;
no HG_PROFILE_GS_READBACK handling remains. SparseCPUscope and128KiBprefetch
candidates REVERTED despitecorrectness,for7.36%/1.32%gameplayslowdowns. Existing
fullCPUscope remains;preparedPSMT4sampler retained. Earlierpendingentries historical.
See PERFORMANCE/PROGRESS forwhole-runvsphasecost distinction andrestoredbinary.


HG-DIAG-017 command-storage candidate: gl_gs.cpp::flush replaces only scratch
command/tilebuffer stores with exactnewpayloads perbatch. GPUVRAM remains intact;
GLordering,shaderarithmetic anddrawcoverage unchanged. Activeonlyopt-inGPU path;
hostcostpendingmatchedtiming. Retain onlyifGPU/originalequivalence andspeedpass.
External gameplaystack sampling istemporary/outside repo;no gueststate effects.


2026-09-20 disposition: fresh command/tile glBufferData experiment reverted after
0.59% scene regression; no shader or guest changes retained. External gameplay
samplers briefly suspended/resumed the main thread; sampled runs excluded from
benchmarks. Stack candidates are heuristic, not an unwound call stack. Combined
VU dependency clock and packed32 product candidates also reverted; no new runtime
probe or guest timing override retained. See PERFORMANCE and HG-FAIL-014..016.


2026-09-20 active HG-DIAG-017 refinement: CPU sprite/triangle scopes retain full
read synchronization and use conservative clipped framebuffer/depth page masks
for write ownership. Source gs_raster_pages.hpp, gs_memory.hpp and gs.cpp. GPU
acceleration remains opt-in; no runtime timing/logging probe added. Expected guest
state effect none; performance unknown until matched replay. Retain only after
coverage/fault checks, original state equality and measured benefit. Selective
LTCG build experiment reverted after gameplay regression (HG-FAIL-017).


2026-09-20 disposition: bounded CPU write ownership retained (HG-LEARN-026).
Texture read-page narrowing and deferred CPU-fallback flush reverted after warmed
scene regressions, despite exact captured state. Current scopes read all VRAM once
and mark only conservative clipped frame/depth pages writable; original eager GPU
flush remains. No new timing override or runtime probe retained. HG-FAIL-018.


Handoff2026-09-20: current GPU regression count914 in both modes. Only bounded
write ownership retained from latest GS experiments. Read scopes and deferred
flush reverted. Full-lane exact packed arithmetic is ordinary native implementation,
not a guest timing override; XYZ extension reverted. No new runtime probe remains.
Post-add4 native profile has not run. See authoritative PROGRESS header.


## HG-DIAG-010 update - completed post-add4 profile, 2026-09-20

External %TEMP%/hg-post-add4-gameplay-profile.py and -sample.py, explicitly invoked
only, no runtime default or runtime-source probe. Waits for slice30000001, selects
busiest native thread and samples for5seconds with suspend/context/resume in
try/finally.3295hits/0errors; sampler0, replay2 with native-iop-budget. Guest memory
and clocks are not edited; host scheduling/elapsed time are perturbed and overhead
is not separately measured. Therefore excluded from benchmarks. Original image/
EE/GS/VU state matches; IOP differs only verified RTC fields. Full symbol/module
counts retained in external post-add4-gameplay-native-profile.json. Module identity
is not a wait-cause diagnosis; no unwind was performed. Retain external helpers for
reprofiling only after relevant changes or new evidence; nothing runs when disabled.
Supersedes the preceding handoff statement that this profile has not run.
HG-DIAG-007 synthetic TOC and HG-DIAG-008 issue-slot timing remain active limitations.


2026-09-20 HG-DIAG-006/011/012 live timing check: external
hg-live-paced-input-check.py uses --realtime, live controls, scheduled startup
inputs and checkpoint/video/fault logs. User confirms normal perceived speed and
working controls; unpaced deterministic benchmark controls were intentionally off.
Observed explicit missing AOT target202d3c, saved full fault dump. Next external
hg-live-paced-audio-check.py additionally enables existing HG-DIAG-014 core0 PCM
speaker output. This bypasses unfinished voice synthesis/mixing; no full gameplay
audio claim. Live tests are excluded from deterministic performance comparisons.


HG-DIAG-006 live input capture: system_diagnostic.cpp logs delivered host-controller
changes with exact slice,buttons,4axes and12pressurebytes only under --profile.
No extra input polling or guest writes beyond existing assignment; disabled cost
is one flag check per delivered change, not per instruction. Enabled stream cost
is unmeasured; live sessions excluded from deterministic speed measurements. Retain
while capturing reproducible interactive faults; no system-wide keyboard capture.

