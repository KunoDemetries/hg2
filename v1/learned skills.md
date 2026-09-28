# Learned skills

Project memory for implementation decisions. Read before changes together with
`rules.md` and `AGENTS.md`. Entries describe evidence-backed techniques and their
limits; they are not permission to substitute guessed behavior or skip validation.
Update an existing entry when possible. Do not promote a hypothesis to a fact.

## Successful techniques

### HG-LEARN-043 - Keep a user controls reference outside guest rendering and test input aliases

The user-requested F1 panel is independently drawn host UI in controls_panel.hpp, using only the existing presentation shader/VAO/texture path. Build/upload its fixed bitmap lazily once, then render only inside the retained fresh/refresh/resize boundary; closing help requests one refresh and must not restore unconditional swaps. Do not alter guest framebuffers or auto-pause clocks to show help. While help is open, release live input and explicitly label that the original game keeps running. Edge-latch Escape so dismissing the panel cannot also quit on the next poll while the key remains held. Keep Backspace and add Tab as an OR-alias for Select; never replace the old mapping unintentionally.

Runtime33980605f16646659c29df526c7500a6 passes262144 complete input/focus/alias comparisons, hotkey and held-Escape checks, bitmap bounds and the existing regressions. Synthetic740x490 preview SHA28bd69d434f39397c7d06304667fad6db4e6d905e6ce3c1a6ed2205862e31e82 shows all rows without clipping; this is NOT a capture of the panel inside a live game. Translation/readiness tests also pass. Full game-state/throughput and physical F1/Select/pause interaction are separate checks in PROGRESS. Authorized host UI changes replace the historical1a122b9f... hash with9704ed0bbf59c9ede823a03821f224aa7aa42512978dc22d153f0439de9b6d0f while preserving its redraw fix. Exact pre-panel backup78fec5d1ba344f79892d0a2cda4f5510; protect both the fix and new controls during later unrelated rollbacks. No audio fix is implied.

### HG-LEARN-042 - Separate an explicit coverage stop and muted launch from performance/audio regressions

Bounded-family follow-up20260922: repeated staircase stops justified checking the ORIGINAL object installer rather than another isolated getter. Latest2ac570/caller209624 uses the same live46dfc0 receiver. Original20a6f8..704 installs46dfc0;20a708..714 begins distinct46e000, with saved following receiver words and original/live64-byte table agreement. This matches the existing documented46e4c0-family technique. After independently decoding every missing body, add the eight absent initialized methods only within this bounded family; retain six existing entries and all older coverage. Exact50 additional words in shards0010/0015, matching original/live bytes and fixed verifier5d147ab1..., support static coverage, not dynamic execution of every method or a successful whole stair route. Do not generalize from one observed pointer to an unbounded table, adjacent object families or256 BSS selectors. Getter callsites2095dc/209600/209624 were independently decoded; two default methods are original zero-return code, not substitutes for unsupported behavior. Current proof/backups are in SOURCES and stairs-object-46dfc0-preedit-20260922-g1.txt. Required next evidence remains the user's actual staircase traversal; original33M checks alone cannot provide it.

Staircase slot+30 follow-up20260922: latest444f9702... stops at2ac540/ra2091a0, not previously repaired2ac530/2ac5d0. Original/live receiver and vtable plus caller209198 prove the getter; paired2091c4 in the same original function consumes the same slot, so root both callsites without claiming both were observed. Original2ac540 is only LUI/JR/delay ADDIU returning3f92e0; emit those three instructions, not a host shortcut. No nested function chain exists inside this getter, and nearby getters must not be assumed live merely because they share a table. Config81772f41... and generation57806f8b... add exactly3 words in one shard; proof/backups in SOURCES and stairs-2ac540-preedit-20260922-f1.txt. Exposed controlled pulses support only left/cross/start: they cannot replay this moving staircase route, and standard33M completion cannot be described as route success. Do not alter pinned recordings to hide this verification gap.

Second staircase stop20260922: same scene/object is not proof of regression. Old2ac530 was slot+0c; new fault2ac5d0 is original2032bc slot+28 on the same vptr46dfc0. Its original40-byte dispatcher selects BSS01990900+12*(initial a1&255). Captured a1=0 and descriptor{0,-1,2ac600} prove one next target; do not generalize to every byte index or root adjacent descriptors. Original/live dispatcher and104-byte callback match; next virtual target2c91d0 is already compiled. Add both required roots, preserve all old roots/host controls, regenerate and qualify. Exactly36 new original words and verifiera2251c6a... preserve expected state; actual staircase traversal still needs live evidence. See SOURCES/PROGRESS and stairs-2ac5d0-preedit-20260922-e1.txt. No audio/performance fix implied.

Menu-close extension20260922: opening-menu coverage at object+1708 did not cover its outer close-state callback at+16fc. Original caller3999d0 and live01810d7c descriptor prove3977d0; follow precise original stores397924/28/2c from44b1d0 to obtain3974a0, then397714/18/20 from44b1e0 reaches already compiled398850. Full original/live724/812-byte bodies and32-byte descriptor pair agree. Add only the two missing roots, not neighboring functions. This prevents assuming every menu dispatch shares one callback field. Configb8d3101f.../one-shard generation35b82014... and qualified verifierfa260aa4... preserve expected state; actual later open/close success still requires a live test. Same permitted inspection retry succeeded after the user asked to retry a prior platform block; no alternate route was used. Source proof, backups and latest10.297336FPS are in SOURCES/PROGRESS/PERFORMANCE; no audio fix or dependable speed gain is inferred.

Select-menu extension (2026-09-22): manual93c3705f... proves396900 via object180f680+1708 descriptor and397f5c/helper100b6c. Follow the ORIGINAL reached routine's precise descriptor loads/stores into that SAME consumed field:396e3c/40/48,396ed0/d4/d8 and396fdc/e0/e8 establish three successor roots394e40/395630/394670 from original/live44b200/210/220. Original100bd0 checks descriptor nonzero; all pass. This is stronger than rooting adjacent functions or every apparent pointer run, and reduces repeated single-callback stops without weakening faults. Full initial1996-byte body/original descriptors agree with saved RAM. New roots build and standard33M qualification53369591... preserves expected state; actual Select operation/successor traversal remains a live-test requirement. Details and exact hashes/backups: SOURCES and select-396900-preedit.txt. Preserve current F1/Tab host9704ed0b..., not the historical pre-panel hash.

Audio distinction: manual414 queue underruns precede the Select coverage stop; configured33M audio replay929ee941... separately records73 underruns and1579627 raw48kHz frames. The speaker path still consumes raw AutoDMA input, not completed synthesis/effects/cross-core mixing (DIAG013/014 and iop.hpp1222). Queue starvation supports the dropout diagnosis, while raw-WAV activity does not prove source/mix fidelity. Do not promise that speakers=on, a larger queue or achieving30FPS alone completes audio. No audio code was changed in this repair.

Manual sessionc1fa9cfc... appeared frozen but native.log gave EE2ac530/no static translation with saved memory. Read the explicit fault and saved call target before adding timing probes. Original/live vtable46dfc0 slot+0c and caller209574 independently prove the three-instruction target; root only that verified target, regenerate, confirm linked page_2ac and fresh manifest, and preserve unsupported-address faults. Original/live16-byte code slices match. Rebuilt original33M regression qualifies all expected hashes and864 writers (50aa2939...), but it does not cross the later79M manual staircase stop: retest the actual route before declaring it fixed. Preserve newly verified coverage on BOTH sides of later optimization reversals. See SOURCES/PROGRESS and manual-freeze-2ac530-preedit.txt.

That launch explicitly used audio=off, explaining missing speaker output without evidence of an audio-engine regression. Use existing audio=speakers for manual sound tests; enabling output alone is not proof of complete mixing or sync. PrintWindow was all-white while saved framebuffer showed the staircase, so use saved pixels for diagnostic inspection and do not claim the actual window was blank.

Later manual unpause sessionecd7f4c9... independently reaches a different direct member target2f55c0, not the already-compiled pause callbacks2f5c80/2f59a0. The same caller2f6094/helper100b6c and live {0,-1,target} descriptorfc70f0 prove the new root; full992-byte original/live code comparison agrees. Add this observed target and regenerate rather than guessing neighboring routines or treating pause success as unpause coverage. Its built33M regression preserves expected state, but actual unpause success still needs a live retest. Speaker-enabled sessions4594260a... andecd7f4c9... record151/426 queue underruns, respectively: sound is genuinely still faulty, separate from the earlier muted launch and coverage faults. Default L1Q/L2top1/R1E/R2top3 already exist; preserve the protected host when the user merely needs those bindings identified.

### HG-LEARN-041 - Sample native instruction pointers without per-guest-operation timers

A bounded own-worker Windows-x64 sampler (HG-DIAG-037) took330 contexts/290 native samples during31M..33M, with zero suspend/context errors or unexpected suspend counts. Duplicate only the current native worker handle; preallocate sample/context storage and never acquire locks, allocate, symbolize, read guest state or print while it is suspended. Always pair suspension with immediate resume, join on exits, and resolve main-image RVAs against the exact copied map after the window. Microsoft GetThreadContext/SuspendThread/ResumeThread/DuplicateHandle contracts are recorded in DIAGNOSTICS.330 samples over5.06921s shows nominal1..4ms Sleep is not a precise high-frequency sampler; do not change host timer settings just to claim a requested rate.

Observed work is dispersed: fpu_madd4_avx2(15),multiply_vector(12),VIF process_pending(11),convert_fixed(10),several generated VU bodies and GS helpers.40 outside-image samples are explicitly unattributed, not proof of GPU waits or allocation. Nearest-symbol/self samples do not provide inclusive stacks or exact per-function costs. Guest frame/EE/GS/VU match; pinned complete IOP comparison through temporary offline test shows only bfcd1/bfcd2 differences and identical existing RTC-normalized digest. Probe timing is not FPS. Replay20260922T043923Z-777a3f5a342d4ff2b8b420755c5e06d3 and129-test jobdf58fe42022548ffbeedc6829ce2f61d retain evidence. Runner and temporary test restored exactly, clean game rebuilt. Reuse results before another instrumentation pass; a sampled small helper is not automatically the best optimization target.

### HG-LEARN-040 - Count actual host swaps, then eliminate only redundant presentation work

The old UI called clear/draw/swap without a fresh mailbox frame: a count-only probe measured6235 duplicates among7097 actual swaps. Event/refresh-aware drawing preserves every fresh mailbox image (even identical pixels), initial paint, resize/refresh requests, input polling and all guest GS/EE/VU work, while idle iterations use a bounded event wait. Four interleaved qualified observations reversed4.08226 ->11.32616 ->4.05805 ->11.37003FPS with matching input/frame/EE/GS/VU/normalized-IOP hashes and864 original writer events. This is retained and must not be lost when restoring unrelated renderer trials. A later controlled input smoke moved original no-card NO->YES and Cross advanced to CAPCOM, with delivered press/release logs; PrintWindow yielded all-white despite valid saved game pixels, so physical-screen and resize behavior remain unverified by that capture method. Evidence PROGRESS top retained section; verifiers6307f56d...,c6a43b6f...,7f64ea2a...,f7e7e60d... and visible20260921T203204Z-0baaca1564d54a90a3572e41a034b953. glGetError was where contention surfaced, not necessarily its cause. The unchanged archived-binary test ruled out recent recompilation as necessary, not an interaction involving the unchanged old host loop. Exact driver-internal cause of the earlier shift remains unknown.

### HG-LEARN-038 - Tie qualified measurements to post-link input and executable hashes

A restored source tree or successful test build does not imply hg_game.exe is current. A real mismatch left the HG-DIAG-027 executable active after probe removal and a later VU header edit. The hg_game post-link manifest now hashes first-party native/generated/config/generator inputs plus build configuration and executable; the fixed verifier rejects missing/stale/wrong binaries before native launch and pins the checked manifest. Twelve synthetic cases plus real missing-manifest and modified-vif.hpp rejections pass (128 Python tests total). Both fresh candidate and control then passed manifest checks. Inspect native function bytes for the intended change too; neither changed whole-file SHA nor equal executable size proves intended code or identical layout. This guard relies on configured build dependencies and does not independently prove correct compilation or fresh generation. Evidence: HG-DIAG-028, tools/hg_build_provenance.py, verifiers a2f951df1cf246a7bd894393d47f9f25 and3b66612128714d64aa2aac308679a940, current candidate/control e11b510d1a094cbf8e911e887bdf7c15/0321d8e9233f4e2a921d3812a72c8bb3.

### HG-LEARN-039 - Replay an unchanged archived executable before blaming a source edit for timing drift

Pinned archived executable SHA2608296b... is byte-identical in old and new captures; input-events, final frame, full EE RAM and GS VRAM match, and the same RTX5070/NVIDIA616.92 string and draw counts are reported. Yet full-run flush0.9963266s becomes15.1869414s and readback1.5088076s becomes3.5050519s. This shows the current latency shift can occur without recompilation or any new code. Runtime/driver/host conditions remain to be isolated; it does not identify a specific driver bug, clock limit or competing process. A narrow GL-only probe attributes15.115755s to glGetError, but replacing polling with synchronous notifications fails to remove the cost: API wait location is not necessarily the underlying cause. Do not reject useful compiler/algorithm ideas universally from a drifting host baseline. Evidence old project-link-runtime/20260921T023355Z-e962461879604a5e9776d9d8db125a5b and new20260921T195021Z-5c12f01e72cc4234bae8c012c45afcdd, HG-DIAG-029.


### HG-LEARN-001 — Isolate CPU cost without changing guest work

Use a current executable/linker map, bounded host samples and phase profiles to
locate hot paths. Compare alternating deterministic original-program replays after
the change. Keep input, clocks, diagnostics and competing load comparable; check
EE/IOP/GS/VU state and images. Ignore only independently explained differences
(IOP RTC seconds/minutes, and hour when the run crosses an hour boundary), never arbitrary mismatches. Exact STQ division
reduced the measured scene time 4.70% while preserving compared state/output.
This proves equivalence for that scene, not whole-console fidelity.
Evidence: `docs/PERFORMANCE.md`, external `divide-times.json` and
`divide-equivalence.json`.

### HG-LEARN-002 — Factor exact address arithmetic

Small compile-time tables can factor the independently derived GS X/Y address
formula without caching mutable VRAM. Exhaustively compare coordinate/stride
coverage and replay the scene. CT32 factoring improved the measured nearest-filter
scene by 6.44%; do not extrapolate that number to other filters/workloads.
Evidence: `docs/PERFORMANCE.md` (scene address audit).

### HG-LEARN-003 — Trace corrupt geometry before changing the rasterizer

Capture bounded before/after original AOT VU execution and independently calculate
the expected projection. VF/VI dependencies also advance pending Q/P operations;
omitting those stalls produced Fiona's long triangles. Correct readiness tracking
matched the matrix calculation and removed the spikes. No runtime decoding.
Evidence: `docs/SOURCES.md` (VU character projection), `docs/PROGRESS.md`.

### HG-LEARN-004 — Validate pixel math against register semantics

Derive packing, saturation, filtering and wrap behavior from hardware manuals;
use synthetic edge cases and permitted independent observations. Correct texture
product saturation removed Fiona's green artifacts without changing EE/VU state.
Bilinear UV tests matched 56 external oracle samples across four wrap modes.
Physical-console STQ precision and full rendering fidelity remain unproved.
Evidence: `docs/SOURCES.md`, `docs/ORACLE.md`, GIF tests.

### HG-LEARN-005 — Separate presentation from guest execution

Use committed scanout pixels in a bounded mailbox; keep GLFW/OpenGL ownership on
the UI thread and AOT execution on its worker. Never flush guest draws to produce
a preview. Controlled33M native/headless state and images matched (RTC excepted);
nonblack GL readback and cooperative close passed; user confirmed keyboard movement.
This is direct presentation, not a GPU GS renderer. On Windows select
`WinSta0\Default` for the visible launcher; a private-desktop HWND is not visibility.
Evidence: `docs/PROGRESS.md`, HG-DIAG-011/015.

### HG-LEARN-006 - Reuse validated pixel state across adjacent stages

When no guest state changes between depth testing and frame commit, reuse the
already-read destination and passed alpha/DATE results. Share the final blend/
write helper with the standalone frame path; retain validation, masks, AFAIL,
SCANMSK, depth-test order and frame/depth alias behavior. Four controlled33M runs
kept compared EE/GS/VU state and images identical (only verified IOP RTC fields
differed). Scene means47.56255s ->41.97535s,11.75% less elapsed; individual pairs
13.54% and9.80%. Relevant GIF tests pass. This is exact factoring of the existing
pixel pipeline, not independent proof that the entire renderer is console-perfect.
Evidence: `runtime/gs.cpp`, `docs/PERFORMANCE.md`, external commit-times.json,
commit-equivalence.json and commit-iop-equivalence.json.

### HG-LEARN-007 - Specialize measured draw classes, retaining live memory order

After per-primitive timing identified sprites as the largest GS cost, a checked
CT32 sprite path prepared address components once per column/row. Reuse decoded
draw state; keep live texel reads and row-major writes, all applicable blending/
mask/alpha/clamp behavior, and the checked general path outside the predicate.
72 scalar-reference cases exercise wrap/filter/blend/masks and overlapping texture/
frame feedback. Four33M replays match images and EE/GS/VU state, with27.04% lower
mean scene time. Scope is this supported sprite class, not all GS behavior.
Moving raster bodies into gs.cpp also avoids rebuilding translated shards for
renderer-only edits. Evidence: PERFORMANCE.md and sprite-equivalence.json.

### HG-LEARN-008 - Correct estimates back to exact integer results

A floating reciprocal can seed a quotient when integer product bounds are proved;
correct the estimate using integer comparisons before returning. Keep the checked
wide path for values outside the proof. Preparing bounded STQ coefficients once
per triangle and sharing Q reduced scene time7.81%; 300000 integer checks, six
triangle/fault cases and four33M state/image comparisons passed (RTC excepted).
This is exact arithmetic factoring, not a license for approximate guest math.

### HG-LEARN-009 - Amortize GPU transfers at existing ordering boundaries

A per-draw synchronous sprite experiment was8.96% slower in one matched native
comparison. Grouping ordered compute dispatches inside existing pending-draw
flushes and including medium strips reduced the same scene27.59%. Read back before
CPU fallback, guest return or fault; use shader-storage barriers between dispatches
and a buffer-update barrier before CPU reads. Reject per-draw feedback alias.
232 GPU/CPU cases cover wrap, blending, feedback, mixed CPU draws and faults;
original33M images/EE/GS/VU match (RTC excepted). Broader console fidelity remains
unproved. Do not assume GPU execution alone eliminates transfer/dispatch cost.
Evidence: PERFORMANCE.md, gpu-sprite-batch-equivalence.json, gl_gs.cpp.

### HG-LEARN-010 - Hoist immutable texture setup per draw

Unlike the inconclusive fragment-local setup experiment, specializing once per
triangle for CT32 and CT32-palette PSMT8/PSMT8H removed repeated validation and
format decoding. Keep live memory reads and validate only accessed palette entries;
retain checked fallback.864 scalar comparisons and four original-scene replays
preserve state/images (RTC excepted), with18.71% less mean scene time. This is
workload-scoped, not full console parity. See PERFORMANCE.md and gpu-texture-times.json.

### HG-LEARN-011 - Batch GPU work across safe ownership boundaries

Triangle compute initially regressed the original scene to71.4774s with49814flushes
and208.935GB uploads. Cull empty coverage, retain exact bounded coefficients and
defer resident submission until checked consumers require visibility. Revised
profile has1683flushes,1.110GB uploads; matched pairs reduce scene mean22.45325
to17.62155s (21.52%).345 GPU/CPU cases and original state/image comparisons pass
(RTC excepted). Host ownership coherence does not prove complete console cache or
timing fidelity. Preserve faults and CPU fallback; fewer bytes alone is not success.

### HG-LEARN-012 - Expand GPU coverage only with exact word ownership

The remaining sprite cost justified revisiting HG-FAIL-006 after resident batching.
Port existing integer texture/frame stages; for16-bit frames assign both lanes of
each word to one invocation to avoid read/modify/write races. Keep conservative
feedback/depth aliases on CPU and retain palette faults.853 GPU/CPU cases and five
original replays preserve compared state/images (RTC excepted). Matched scene mean
18.65525 ->14.22910s (23.73% reduction), despite more transfer/readback cost.
This is bounded CPU-reference equivalence, not full console fidelity. See SOURCES
and PERFORMANCE; sprite-format-pair-times/equivalence artifacts remain external.

### HG-LEARN-013 - Keep AOT helper specialization bounded

For this scene, small VU register/readiness/flag helpers and integer multiplication
benefit from inlining; compound arithmetic forced into the giant CFG does not build
economically (HG-FAIL-007). Exact MAC bit placement replaces its inner branch loop.
Eight focused tests and six original state/image comparisons pass, RTC excepted.
Scene mean13.36263 ->12.97297s (-2.92%), one neutral pair; executable grows4.69%.
Retain this bounded version, but expect workload/compiler variation. This establishes
unchanged checked arithmetic, not previously unverified console precision or timing.

### Reference-review practice

Compare architecture at the measured bottleneck. PCSX2 documents transfer/ordering
costs; OpenGOAL documents a game-specific port with offline preparation; PS2Recomp
documents literal AOT translation and runtime services. These are leads, not proof
that a borrowed replacement preserves HG behavior. Keep independently derived
implementation and state/performance evidence. See SOURCES.md for exact documents.

## Failed skills / approaches to avoid

### HG-FAIL-041 - Fine-grained host-thread VIF1/VU1 overlap costs more than HG can overlap

The architecture premise is real: captured Haunting Ground VIF1 traffic uses BASE0/OFFSET0x200, TOPS-relative UNPACK and MSCAL/MSCNT, matching Sony's double-buffer producer/consumer mechanism. Multiple host implementations preserved exact frame/EE/GS/VU/current-date normalized IOP state, but all lost throughput versus synchronous production: full-snapshot overlap8.746445FPS, dirty VU/VIF tracking8.584612FPS, VIF-only tracking/bulk merge8.749468FPS; atomic spin/yield handoff was even worse in whole-run time. Retained diagnostics show only~0.18s completed UNPACK work available to overlap with~2.56s VU execution in the final2M region, while ~126180 MSCAL/MSCNT activations create frequent host scheduling boundaries. Do not retry fine-grained one-host-job-per-activation designs unchanged. Split-tag/TTE physical phase tracking discovered by this work is independently correct and remains. Clean synchronous production verifier98f8f5f6... is11.428331978FPS with exact state and becomes the post-audit baseline. Future work should target actual VU AOT/EE/GS cost or a much coarser architecture mechanism only if its overlap budget is independently proven.

### HG-FAIL-040 - Hoisting most VU pair clock updates into a function-level deferred scheduler regressed gameplay

The root-cause audit correctly identified runtime VU scoreboard traffic as substantial host work, but the tested fix was counterproductive. An overflow-safe scheduled-clock design moved3194/3477 static pair advances off direct architectural mutation while preserving Q/P timing, readiness, exceptions and uint64 wrap. It passed132 Python tests,65536 compiled full-state/memory readiness comparisons, runtime/translation, replay and all established frame/EE/GS/VU checks. Qualified candidate9.622339223FPS versus exact restored same-date control10.079036444FPS is~4.7% slower; both share post-date-rollover normalized IOPb29aec44.... Candidate executable grew~348KiB, so extra generated control/sync code and I-cache pressure are plausible contributors. Restore original per-pair route; do not repeat this deferred scheduler unchanged. The failure does NOT invalidate the audit's independent VIF1/VU1 double-buffer serialization finding; move to actual device overlap rather than more scoreboard bookkeeping.

### HG-FAIL-037 - Single-tag EOP IMAGE materialization bypass was within qualified noise

A narrow gs.cpp path recognized only complete packets consisting of one EOP IMAGE/IMAGE2 tag, then sent its qwords directly to the existing write_image_qword endpoint. Mixed/register/multi-tag GIF packets retained the exact decoder/application route; zero-loop IMAGE, capacity, pending bytes, partial GS effects and fault checkpoints matched the frozen reference in5825 runtime checkpoints. Candidate11.228385358FPS versus exact whole-file reversed control11.173891829FPS differs by only~0.49% host time, with overlapping heavy windows (candidate11.2396/11.2172; control11.2406/11.1080). All149-input/864-writer/frame/EE/GS/VU/RTC-normalized IOP checks pass. This does not show a dependable gameplay gain; production gs.cpp is restored exactly e03185c7.... Keep the strengthened test cases, not the fast path. Do not repeat unchanged by appealing to the older perturbed GIF subphase timing.

### HG-FAIL-036 - Compact Mask/Broadcast-only VU MUL specialization had no demonstrated gain

After full five-operand templating failed, a materially smaller helper kept only Mask/Broadcast static and destination/source/other dynamic. It passed786432 exact complete-state comparisons and427520 matching faults across16 MXCSR modes, linked only four mask14 variants, and did not enlarge the executable. Candidate10.608447507FPS versus fresh reversed original multiply_vector control10.715204160FPS is about1% slower; an immediately earlier original-route sample was11.145786893FPS, demonstrating larger host drift. Retain original multiply_vector gameplay route and the compact helper only as a test fixture. This result is workload-specific and does not invalidate all compile-time constants, but there is no basis to re-enable this route unchanged.

### HG-FAIL-035 - Full static-operand broadcast VU MUL specialization regressed the current qualified build

The AOT route templated Destination/Source/Other/Mask/Broadcast for broadcast MUL masks14/15 and used a noinline exact-order helper. Its104-case fixture passes851968 complete-state comparisons and466304 matching faults across16 MXCSR modes, but correctness tests alone did not establish speed. With identical current gameplay coverage/config and host, fresh candidate5fddf7c... measured9.449696901FPS (60/6.34941s); switching only generated routing back to existing multiply_vector and removing the helper include produced control3be4caa8... at11.145786893FPS (60/5.3832s), with near-identical11.1431/11.1484 heavy windows. Both pass149-input/864-writer and exact frame/EE/GS/VU/RTC-normalized IOP qualification. Reject full five-operand templating; keep the helper/fixture test-only. Candidate executable was~306688bytes larger, which may contribute but is not established as the sole cause. Do not repeat unchanged or infer that all compile-time specialization is bad: a compact Mask/Broadcast-only helper with dynamic register indices is materially different and may be tested separately.

### HG-FAIL-034 - More aggressive automatic VU-unit inlining regressed this qualified comparison

Adding only /Ob3 beside existing /O2 on the generated main/VU source built in106.35s and passed original replay plus147-input/864-writer/state qualification. Candidate6a179db53b724cbcab73d34af82e0b94 measured60/6.5205s=9.201748332FPS. Exact CMake reversal and rebuild restored60/5.39847s=11.114260151FPS (bff1d79d44d0454e870c25316cd62076), with unchanged image/EE/GS/VU/RTC-normalized IOP and preserved host1a122b9f.... Reject /Ob3 on this evidence; do not keep a slower build while describing only its passing tests. One pair shows a substantial reversible loss, not its compiler microarchitectural cause or a universal /Ob3 verdict. No forced-inline annotations or /O1 large EE shard settings changed. CMake restored2b9e0b6b..., current control game2dc87790.... Candidate CMake backupfa1333528bde47f7a2b369a33aa1841f; source/build plan vu-auto-inline-ob3-preedit.txt. Do not repeat unchanged. Selective main+VU /GL/LTCG was separately rejected already (HG-FAIL-017); that is not an untested follow-up.

### HG-FAIL-033 - Batching still-incomplete GIF payload bytes did not establish a dependable gain

The private outlined-VIF helper batches opaque payload bytes within an already-complete DIRECT command, leaving tags, completion/application, max_pending overflow and first-fault bytes on original submit_qword. This is distinct from whole-EOP coverage failure031 and DMA-arrival mechanism032.936 serial-reference cases/7873 checkpoints passed, the native linked helper/callsite was inspected, and the original recorded replay and all four147-input qualified runs preserve864 writer events plus exact frame/EE/GS/VU/same-date normalized IOP hashes. Candidate11.021045/10.911730FPS versus interleaved exact controls10.904967/10.861439 gives only1.053%/0.461% less host time, with downward sequence drift and the second pair's individual windows disagreeing. This is not strong evidence for retention, nor proof the host-only byte-copy equivalence is wrong or universally useless. Original parser5ab22f5b... rebuilt; protected host1a122b9f... and original runnerc8c2d536... remain. Useful fixture retained. Do not repeat unchanged or advertise comparison against older isolated10.476FPS as a gain.

Evidence verifier directories fa3283542166432f8058a0e932dcd847,f048f775d8754a55b7c49772e64335c2,4f708348e1c243b39043e861a449f87b,1b05c22d19ff484581c6d713951f95f5. Restored control game37f7d62d3dc9acfa7aeed8009f97c3853cadc287c16c1e2bacd35e51787f6cdc;60/5.52413s=10.861438815FPS. Candidate backups bffa0a620e0a4400be7203efcf2025c3 and ab4f8269661946a0bcf6ce570345470e. Next investigate sampled native hot code instead of assuming source-level qword-copy savings dominate the1.6s inclusive DIRECT measurement.

### HG-FAIL-032 - Inert DIRECT DMA arrival batching had no stable end-to-end gain

A guarded helper concatenated only known-incomplete DIRECT/DIRECTHL RAM/scratch input inside the existing synchronous pump loop, leaving both command and DMA completion to the original final qword.2338 state/fault cases including256 caller-command guard cases passed; original gameplay image/EE/GS/VU/same-date IOP hashes and864 writer events matched. Qualified unfiltered candidates10.943853/11.237597FPS versus original controls10.273726/11.125079 had one slower first control and only~1% advantage in the second pair. A DIRECT-only caller prefilter also measured10.956803FPS, not a demonstrated advantage. Keep production runner original; preserve the helper only in runtime_tests, not the game library.

HG-DIAG-036 confirmed real coverage:25986 accepted batches buffered7151582qwords among7306154 helper attempts in heavy gameplay. This is distinct from HG-FAIL-031's low-coverage complete-EOP batching; large operation-count savings still did not establish a reliable qualified gain, and the earlier6% pair must not be advertised. Future work should attack actual completed DIRECT/GIF application rather than repeat the same incomplete-input helper. Evidence7c07aef8...,ccd2a496...,e9e0f1d0...,7eae15ea...,ee470df3566642e6ac6ea86ca68a2130 and project-link-runtime/20260922T024554Z-c6cc6abda3b043de9a8e92cd6fb3153f. No semantic error was observed within the tested scope; this does not claim the mechanism can never help another workload.

### HG-FAIL-031 - Complete DIRECT packet batching covered little of the actual payload

The candidate kept the original materializing decoder and complete-EOP commit/fault semantics;791 cases/5124 exact checkpoints and original frame/EE/GS/VU/same-date IOP hashes passed. Qualified enabled10.983700/10.638468FPS did not improve over the exact original parser11.020640FPS (prior11.038888). A flag-disabled outlined helper gave drifting7.564354/8.917942FPS and must not be treated as proof of improvement versus the original pre-helper implementation. Restore exact pre-experiment code when control code placement differs.

Counter-only HG-DIAG-035 on the ORIGINAL serial path showed57906 eligible packets but only564374 of7306814 heavy DIRECT qwords (7.724%) fit the complete-EOP guard; every eligible packet began with PACKED format, none IMAGE.3240 commands began with a prior GIF suffix. Thus high packet count did not mean coverage of the large payload; reject/restore this candidate, retain fixtures, and do not repeat unchanged expecting the1.6s inclusive DIRECT cost to disappear. Evidence candidate80d93dc0...,7f89e3b7..., exact original1945b31d1f4d440393e22906ae0a9d37 and coverage project-link-runtime/20260922T022141Z-10b6ddd9ec4a48c7b6544089db0ae5bd. Probe sources restored exactly; host1a122b9f... protected.

### HG-FAIL-030 - Whole-span VIF word bounds check showed no useful qualified gain

The overflow-safe4-byte span guard preserved535050 value/exception comparisons, generated game state and the original invalid-range .at path. Candidate88bac4c386484cf2b68bb33f16da4e7b measured10.92770775FPS (60/5.49063s); contemporaneous restored control4b1d9bef5834489fb798137145ba3893 measured10.94219603FPS (60/5.48336s). Both143-input manifests,864 frame-writer events and all established frame/EE/GS/VU plus same-date normalized IOP bdbf3a11... hashes match. An earlier unchanged candidate was10.33145014FPS, illustrating host variance. No repeatable gain is established; the shortcut is byte-restored and tests retained. Do not repeat this word-helper guard unchanged or treat fewer source-level bounds checks as proof of faster gameplay. Protected event/refresh host1a122b9f... remains unchanged. This does not rule out distinct loop-level UNPACK indexing changes.


### HG-FAIL-029 - Guarded embedded-rounding MADD did not demonstrate a gameplay gain

The isolated guarded helper passed2113152 direct vectors/8452608 scalar lanes,16 MXCSR modes, boundary/exponent/signed-zero checks, exact flags and rejection atomicity. Original gameplay with the route enabled passes all established state/image hashes and864 frame-writer events, but candidate11.388375885FPS (60/5.26853s) versus subsequent disabled-route control11.401186864FPS (60/5.26261s) is essentially flat (~0.11%). This is an inconclusive performance result, not proof the arithmetic is wrong or universally slower. Keep use_embedded_rounding_madd=false; preserve helper/direct fixtures for reproducibility and the original integer/AVX2 route in gameplay. Do not re-enable unchanged on the assumption that ISA-level simplification implies end-to-end improvement. Both runs used the retained event/refresh host and143-input freshness. Evidence project-link-verifier/9e7d985c63b6448c8312d3d912fa8039 and47eac334b33a4a2b9f0a5b3acd8fce84; current exe78a0c91d.... Host fix1a122b9f... remains protected and11.4FPS qualified. This is not the rejected prepared-matrix path or a loss of the host recovery.


### HG-FAIL-026 - Two small VU shortcuts had no demonstrated qualified gain

Validated MicroMem-generation caching passed state/regressions but cache-on9.3733/9.3863FPS versus controls9.5073/9.3338FPS overlapped variance; equal executable sizes were not identical machine-code layouts. The later no-Q/no-P advance_pipeline early-return passed65536 compiled readiness cases and exact original state, but fresh candidate4.050141FPS versus restored4.040570FPS is effectively flat while both have the large GL stalls. Both changes are removed. Their correctness or usefulness in other workloads is not disproved; do not repeat either unchanged expecting to explain the current GPU latency. See PROGRESS and verifiers9394004a...,48240a60...,9798d9a8...,37fc1136...,e11b510d... and0321d8e9....

### HG-FAIL-027 - Independent worker GL context did not reduce present driver stalls

Only the worker context share pointer was changed to nullptr after proving the CPU pixel mailbox is the only cross-thread image transfer. All GL error checks/commands remained. Replay20260921T193643Z-94beeb0eb8514953b68accb9143a01cf still spent15.1996s flushing and3.6567s reading back, essentially unchanged. No qualified improvement was established; game_host.cpp restored exactly. Do not repeat unsharing alone unchanged for this observed condition.

### HG-FAIL-028 - Guaranteed synchronous GL error notifications moved cost rather than improving throughput

A requested DEBUG worker context, synchronous KHR_debug API-error callback and actual-driver INVALID_ENUM self-test preserved fault detection; ordinary contexts retained polling. Portable513-length/first-error tests and original state hashes passed. Nonetheless replay20260921T194610Z-a12e18b8014d4cee870168eaafd6daa1 had18.3585s flush/2.7917s readback, and qualified verifier a8086a1ce14a41ebae73903e79a5fa87 measured3.595859FPS versus4.040570 control. Backend/host/tests restored; helper implementation archived outside project, empty unreferenced placeholder remains because connector deletion is unavailable. Do not disable errors to make glGetError timing vanish. Ordinary non-debug contexts may legally emit zero notifications and cannot safely replace polling; synchronous debug mode itself can reduce driver performance. Primary specification KHR_debug sections2.5,5.5 and5.5.7; see HG-DIAG-030.


### HG-FAIL-001 — Treating preview rate as game FPS

File changes, generated snapshots and repeated GL swaps count different things.
None proves how many original game frames executed. Never claim console-speed
parity from those counters. Establish an original frame boundary and host elapsed
time; retain explicitly labeled video-update measurements until then.

### HG-FAIL-002 — Comparing uncontrolled input as deterministic state

Wall-clock button pulses and a live-input-enabled window can diverge with host
timing. A native-host test diverged after31M; the controlled-input repeat matched.
Use identical guest-slice input events and disable live controls during equivalence
tests. This does not imply live controls are incorrect.

### HG-FAIL-003 — Retaining an optimization merely because it sounds faster

The earlier preview cache avoided394 writes but increased elapsed time
(35.3766s vs34.9217s); it was removed. Sparse IDCT26, bit reader34, byte batching36,
combined trace/inlining40 and address41 also lacked connected speed benefit.
These are workload-specific failed experiments, not universal bans on caching,
inlining or lookup tables. Revisit only with a new measured bottleneck and a
different, explicit hypothesis. Evidence: `docs/DIAGNOSTICS.md` and performance log.

### HG-FAIL-004 — Trading fidelity for apparent throughput

Skipping required draws, forcing initialization calls, changing guest clocks to
inflate FPS, silent unsupported operations, and interpreter/JIT fallbacks cannot
satisfy this project's 1:1 independent AOT goal. Diagnose the original path and
optimize host work instead. This is a project constraint, not a benchmark result.

### HG-FAIL-005 - Fragment-local texture setup factoring did not speed this scene

Candidate A shared TEX0/TEXA decoding and per-axis wrapping across bilinear
neighbors. Four controlled33M replays preserved all compared images/EE/GS/VU state,
but scene times before42.9333/44.1434s and after42.7344/44.1626s differ by only
0.21% on average, below the observed variation. Reverted this candidate; retained
its external source/binary for reference. This rejects that implementation as a
useful scene optimization, not the general idea of hoisting texture setup.
Revisit only with a different scope (e.g. per draw) and evidence of saved cost.
Evidence: external setup-times.json and setup-equivalence.json.

### HG-FAIL-006 - More GPU coverage can lose its benefit to extra flushes

Adding solid CT32 fills offloaded116.5M more pixels but raised flush count721 to
1227 and host flush time~0.89 to1.49s. Four matched native runs preserved state,
but scene mean improved only2.09%, with one pair slower and one faster. Reverted
that candidate rather than retain an unproven optimization. This is inconclusive
for solid fills generally; revisit with less synchronization or new cost evidence.
External gpu-solid-times.json and solid-*-equivalence.json retain the measurements.

### HG-FAIL-007 - Do not force-inline compound VU arithmetic into the giant CFG

Forcing Fpu::add plus multiply_acc/madd_vector/multiply_vector into the generated
VU unit passed focused tests but drove MSVC to36.75GB private/14.12GB resident
memory during code generation; the build was stopped. No native speed result exists.
Preserved candidate/log in external vu-inline-aggressive. Narrow to small access,
readiness, multiplication and flag helpers, then measure. This rejects that combined
aggressive configuration, not all inlining or the exact arithmetic it implements.

## Open questions (not learned facts)

- User corrected the map-entry text report: no text should appear there. Do not
  infer missing initialization calls from that withdrawn report.
- Current scene speed is inadequate. CPU raster/texture/address work is prominent
  in host samples; quantify sustained phase costs before asserting a sole cause.
- Experimental issue-slot timing and synthetic TOC input remain fidelity gaps.

## Entry template

ID / technique; when applicable; observed outcome; correctness source and limits;
performance evidence; files/artifacts; retention or retry condition.

### Pending investigation — CPU cost of GPU coherence

A page-ownership wrapper passed241 GPU/CPU cases and original-scene state/image
comparisons, but slowed the scene24.7895 ->29.5581s even with residency off.
Resident mode took30.2274s. Do not count reduced upload bytes as a speed win or
accept this wrapper as performance-neutral. Profile the disabled observer path and
verify generated access code before expansion. This is not yet a generally failed
technique: the cause remains unproved. See docs/PERFORMANCE.md.

Disjoint CPU-write follow-up: test dependencies against both queued read and write
pages before submitting GPU work. Unrelated writes commute, but writes to queued
texture inputs must still submit first.856 cases and four original replays preserve
state/images; flushes3007 ->2415, scene mean1.66% less. Synchronous tests must end
the explicit service batch before CPU mutation; resident tests must exercise queued
work surviving that boundary. See HG-DIAG-017 and disjoint-flush-pair-times.json.

### HG-LEARN-014 — Prove disjoint bits inside aliased VRAM words

Page overlap alone can reject safe in-place work. For bounded CT16/16S sprites,
prove each pixel reads the opposite16-bit lane of its own unique destination
word, with zero filter fractions and distinct destination columns. Separable
row/column deltas establish word equality including wrap; <=8 columns bounds
ownership. All sampled bits remain unchanged during the draw. Keep other hazards
and fall back when any proof fails.896 GPU/CPU checks and four original replays
passed, scene time11.10% lower; no external rendering algorithm copied.
Scope: these proven sprites only, not arbitrary feedback or console cache fidelity.
Evidence: docs/PERFORMANCE.md; external lane-copy-baseline and lane-* artifacts.

### HG-LEARN-015 — Suppress conservatively dirtied uploads by exact bytes

A CPU scope may mark untouched pages dirty. Compare only against a known GPU
copy, invalidate after GPU writes and memory-owner changes, refresh on transfers.
Full memcmp, not hash equality, preserves correctness.896 GPU cases both modes
and four original replays agree; uploads2.950GB ->0.678GB but scene only1.93%
faster. Retain modest benefit; bytes saved do not equal elapsed savings. Extra
4MiB host storage and compare/copy work included. See PERFORMANCE.md.


### HG-LEARN-016 — Synchronize committed scanout before its pixel loop

A const bulk VRAM read removes per-pixel observer overhead without flushing
uncommitted guest draws.901 GPU/CPU cases both modes and four original replays
match (RTC only). Full replay4.38% faster, scene only0.888%; downloads increased.
Measure both scope and phase: do not turn the full-run benefit into a gameplay
FPS claim. Existing conversion/wrap/fault behavior retained; see PERFORMANCE.md.


### HG-LEARN-017 — Gather VU flags while lane results are live

Fuse temporary flag collection into multiply_acc/madd_vector computation, but
commit flags/results at the original boundaries so faults and aliasing match.
49152 frozen-reference cases and four original replays agree (RTC only); scene
mean1.375% lower. No compound force-inlining; arithmetic/timing unchanged.
Evidence PERFORMANCE.md and vu-flags-fused artifacts. Benefit is modest/variable.


### HG-FAIL-008 — Runtime /O2 placement alone did not improve VU arithmetic

Moving Fpu::add and five compound VU helpers from the /O1 AOT unit into one /O2
runtime unit preserved state but slowed two matched pairs by1.65% on average.
Reverted; retained fused flags. This combined placement/inline-boundary experiment
does not prove /O2 generally slower. Revisit only with a changed hypothesis and
measurement. Candidate/source/map in vu-o2-rejected; PERFORMANCE.md has evidence.


### HG-FAIL-009 — PSMT8 table relocation lacked a measured scene benefit

Moving psmt8_word to gs.cpp with proven X/Y table factoring passed exhaustive
address checks and original equality, but two pairs disagree and mean is0.35%
slower. Reverted the combined change. Tables remain valid in existing DrawTexture;
revisit only with new measured evidence/scope. Artifacts psmt8-table-rejected and
psmt8-table-pair-times.json. Refresh restored build-input timestamps so CMake/MSBuild
reconfigure/recompile rather than reuse stale candidate dependencies.


### HG-LEARN-018 — Prototype exact SIMD at the complete helper boundary

SSE2 integer broadcast products preserve the scalar finite-exponent profile.
Product-only timing overstates comparison if compiled scalar settings differ;
compare against the actual /O1 scalar helper, including checks,flags and addition.
Full helper synthetic23-53% benefit becomes only1.42% mean scene gain; four
original states/images match (RTC only),245760 integrated cases pass. Inspect
AOT masks before widening scope: XYZ sites exceed XYZW here, but static counts
are not runtime frequencies. Source/bench evidence in PERFORMANCE.md; other-host
scalar fallback remains and cross-platform performance is not verified.


### HG-FAIL-010 — More SIMD mask coverage did not improve the matched scene

XYZ extension preserved all checked state and inactive W behavior but paired
timings disagreed; mean0.19% slower. Reverted extension, retained full-lane path.
Static AOT site counts are useful leads but cannot substitute for dynamic cost
and matched timing. See vu-xyz-rejected and PERFORMANCE.md.


### HG-LEARN-019 — Batch proven packed IMAGE word ownership

PSMT8 aligned16-pixel rows pair opposite byte lanes in8words within one16-word
column. Compute address/row once and synchronize an explicitly checked mutable
span; avoid16independent observer/address operations. Retain scalar fallbacks for
unaligned,partial or row-crossing qwords and exact completion/padding semantics.
3168scalar transfer scenarios,910GPU cases both modes and4original states match
(RTC excepted);scene mean9.10% lower. Unlike HG-FAIL-009,address-only relocation,
this removes repeated coherence and per-pixel division together. See PERFORMANCE.

### HG-FAIL-011 — Synthetic cancellation gains did not establish scene benefit

FPU add normalization bit scan preserved3M+12240results/flags and original states,
but original paired timings disagreed (one6.73%faster,one2.44%slower). Mean2.33%
gain was dominated by slower first control;reverted instead of assuming synthetic
5-52%gains transfer to this game. Technique valid but workload benefit unproven.
Evidence fpu-normalize-rejected and PERFORMANCE. Retain original add loop.

### HG-LEARN-020 — Bound scanout ownership by physical pages

Use original format page geometry to select displayed VRAM pages,coalesce runs,
and retain exact pixel conversion via const pointers.216cases check both bytes
and exact observer footprint;910GPU cases and4original captures agree (RTC only).
Readbacks decrease~130MB but readback time rises;full replay1.02%faster and scene
0.74%mean benefit uncertain. Retain narrower ordering;do not equate bytes saved
with speed. See PERFORMANCE and scanout-pages artifacts.

### HG-LEARN-021 — Tune optimization per generated translation unit

MSVC /O1 remains appropriate for huge EE shards during development,but the1.19MB
main containing VU program bodies benefits from /O2. Source-level override retains
shard compile settings;observed~10GB compile memory,completed normally. Four original
states match RTC excepted,scene mean5.89%faster;whole replay only0.90%. Rejected
helper-only relocation does not predict complete-call-site optimization. Verify
actual generated build options and matching map;other-host performance unverified.

### HG-FAIL-012 — Contiguous VIF fast path lacked reliable workload benefit

V4-32 unmasked mode0/CL==WL memcpy is equivalent under checked bounds,but original
second pair is flat(+0.13%)and mean gain comes from slow first control. Reverted;
600scalar cases and4original equality checks show correctness,not value on this
scene. Actual fast-path frequency was not measured. Future timing helpers warm
both binaries before measured pairs due recurring first-control variance.

### HG-LEARN-022 — Pack full-vector flags together, and warm both binaries

Exact SIMD products/additions already produce lane arrays. Gather zero/sign masks
once,retain UF/OF masks,and reverse lane nibbles for MAC rather than packing each
lane separately.245760+256cases and4original states agree RTC excepted. After
warming both binaries,scene means9.08285->8.87205s(-2.32%),both pairs faster.
Keep warm-ups excluded;changed-image interval counts still fluctuate and are not
internal FPS. Scope full-lane native helper only,not arbitrary VU flag behavior.


### HG-LEARN-023 — Profile startup and gameplay separately; distinguish call boundaries

Early5M-start sample is dominated among identified functions by scalar PSMT4/
bilinear texture work,while30M-start sample emphasizes FPU/VU. Selecting fixes
from gameplay alone misses startup costs. Unresolved indirect-site counts are not
counts of missing or skipped functions:4598EE indirect boundaries coexist with
successful bounded replays because runtime targets may already be compiled.
Verify original callback extent/live use before roots;explicit fault behavior
checks cannot prove native service fidelity or cover unvisited paths. Evidence:
startup-coverage-audit/summary.json and PERFORMANCE.md. No runtime changes.


### HG-LEARN-024 — Prepare indexed sprite state without caching sampled pixels

PSMT4/CT32CLUT sprites reuse immutable TEX0/wrap/CSA state but read texture words
and CLUT validity live in original row-major order.128scalar alias/fault fixtures,
910GPUbothmodes and4original states agree RTCexcepted. Startup3.58%faster,whole
replay2.34%;gameplayflat. Preserve this phase distinction. See PERFORMANCE.


### HG-FAIL-013 — Sparse raster readbacks saved bytes but lost throughput

Demand8KiBreadbacks plusper-pixelownershipcache reducedtransferbytesbut increased
GPUwait/CPUcost;gameplayscene7.36%slower.128KiBdirty-pageprefetchreducedpenaltybut
still1.32%slower. Bothpreservedtestedstate;bothreverted. Whole-replayreadbacktime
cannotbe assigned togameplayscene withoutphaseattribution. Nexttransferworkneeds
coalescedprovenfootprints orfewerCPUfallbacks,notrepeatdemandpagevariant. Evidence
PERFORMANCE.md,sparse-single-page-rejected,sparse-grouped-rejected.

### HG-LEARN-025 — Narrow XYZ AOT specialization has a modest measured benefit

Newruntimevu_xyz helpersselectedonlyfororiginalbroadcastXYZ MULAbc/MADDAbc/MADDbc
keep scalarproduct/add arithmetic andoriginalfaultfallbacks.872AOTcalls changed;
295168referencecases,15emittertests and4originalstates agreeRTCexcepted. Warmed
scene0.67%lower,bothpairsfaster;retainwithsmall-effect caveat. Not evidenceforbroad
SIMDXYZ orcompoundforce-inlining. See vu-xyz-scalar artifacts andPERFORMANCE.

### HG-FAIL-014 � Replacing command storage did not reduce scene wait

Fresh glBufferData storage for each immutable command/tile batch preserved all
910 GPU cases in both modes and four original captures, but warmed scene mean
8.8928->8.9454s (+0.59%). Reverted. Driver wait attribution alone does not show
that overwriting command storage causes the wait. Keep existing sub_data path.
Artifacts command-storage-baseline/rejected and command-storage-pair-times.json.

### HG-FAIL-015 � Combining dependency clocks increased native cost

AOT max of next issue cycle and all operand readiness timestamps preserved
131072 randomized clock/Q/P cases and original state, but scene mean
8.86385->9.61985s (+8.53%). Reverted emitter/generated prologues. Equivalent
fewer helper calls do not guarantee cheaper generated code; keep original sequence.
Artifacts vu-pair-clock-baseline/rejected and vu-pair-clock-pair-times.json.

### HG-FAIL-016 � Packed32 product normalization lacked repeatable benefit

Four packed32 normalization lanes after exact SSE2 24x24 multiplication passed
295168 arithmetic cases and four original state comparisons. Warmed scene
9.5522->9.48645s, but pairs disagree (1.42% better then effectively flat/slower)
and whole process was slower. Reverted; keep original two64-lane normalization.
Artifacts vu-packed-products-baseline/rejected and corresponding pair-times.json.

### HG-FAIL-017 � Selective MSVC LTCG hurt gameplay throughput

/GL on generated main and VU helper sources with /LTCG preserved all four original
states (RTC only), but warmed scene8.95825->10.5671s (+17.96%). Startup improved
18.250895->17.76108s, illustrating phase-specific tradeoffs. Reverted CMake and
binary/map. Linker used about16GB private memory and completed; no memory failure.
Do not infer faster execution from broader compiler visibility. Artifacts
vu-ltcg-baseline/rejected, vu-ltcg-pair-times.json and vultcg-*.

### HG-LEARN-026 � Separate bulk read visibility from raster write ownership

Keep original full readback but mark only conservative clipped frame/depth pages
writable. This avoids false dirty state without per-pixel checks.1920 page cases,
910 GPU cases both modes and4original states agree RTC-only; warmed scene0.68%
faster, both pairs better. Small gain; do not equate write-footprint reduction with
readback savings. Inputs need an independent footprint proof before narrowing.

### HG-FAIL-018 � Bounded raster reads saved bytes, not gameplay time

Whole wrapped-texture/output page bounds passed5760 read/1920 write coverage cases,
910 GPU comparisons and4original states (RTC only). Downloads fell~31%, but scene
8.8659->8.90155s (+0.40%). Deferring eager CPU-fallback flush through resident
hazards passed914 cases and4originals but8.85645->8.8966s (+0.45%). Both reverted;
write-only scope remains. Coalesced ranges still expose readback latency; bytes or
flush counts alone are not the optimization objective. Evidence raster-read-pages,
raster-deferred-flush and raster-read-deferred-rejected artifacts. Retain four new
GPU/CPU dependency fixtures; no deferred-flush API or texture-read helper remains.

### HG-LEARN-027 � Exact small-integer conversion can supply packed bit length

For post-carry addition mantissas0..2^24-1, binary32 integer conversion is exact.
Its exponent replaces five packed normalization steps without rounding guest sums.
16M scalar comparisons across4host rounding modes plusVU/GPU/original-state checks
pass; scene0.83% lower, both pairs faster. AVX2 runtime CPU/OS guard and scalar
fallback retained. Scope full-lane validVU MADD; not general host-float replacement.
General packed binary-normalization variant was0.65% slower, so SIMD alone was
insufficient. Evidence PERFORMANCE and vu-add4-exact artifacts.

### HG-FAIL-019 � Padding XYZ to reuse add4 did not improve the original scene

Three scalar products plus padded packed addition preserved VU tests and4original
states (RTC only), but scene9.4156->9.47435s (+0.62%), both pairs slower. Reverted
XYZ only; full-lane exact add4 remains. Packing/setup overhead can outweigh one
shared SIMD operation for three lanes. Artifacts vu-xyz-add4-rejected and
vu-xyz-add4-pair-times.json. Do not generalize full-lane gains to partial lanes.

### HG-LEARN-028 - Profile costly fallback cases before extending GPU coverage

A small count can dominate a primitive class:292 untextured blended triangles
cost0.364s in the measured scene. Extending the existing exact integer tile path
to TME-off removed STQ/texture work and corresponding texture dependencies while
retaining conservative frame/depth/alpha checks.1018 cases per GPU memory mode,
actual acceleration checks and original snapshots pass. Matched hidden scene
mean8.2572 ->7.73375s (6.339% lower), both pairs faster; no visible30FPS claim.
Remove detailed diagnostic logging once the case is identified. Scope limited to
validated original/CPU behavior; physical-console completeness remains unproved.
Evidence: PERFORMANCE and external untextured-triangle-summary.json.

### HG-LEARN-029 - Keep exact wide product lanes through normalization

Four AVX2 unsigned24x24 products retain64-bit lanes for carry/exponent selection
and saturation. Unlike rejected packed32 normalization, this reduced matched
scene time2.476%, both pairs faster.16M scalar comparisons across four rounding
modes and four original image/state captures pass. Keep CPU/OS dispatch and
SSE2/scalar fallback; SIMD instruction counts alone do not establish a speed gain.
Evidence vu-product4-avx2-summary.json. No clock or guest arithmetic changes.

### HG-FAIL-020 - Extending packed helpers to plain full vectors did not help

Full-lane multiply/add/subtract reuse passed49152 new frozen-reference checks and
four original captures (RTC only), but matched scene7.5568 ->7.5691s (+0.163%).
First pair slower, second flat. Reverted; packed arithmetic success in MADD does
not establish profitable reuse in every vector helper. Saved implementation and
captures under vu-full-vector-rejected and vufullvector-*.

### HG-LEARN-030 - Masked GPU stores can avoid CPU readback before partial writes

Profiled PSMT8 host writes were waiting for GPU-owned pages only to preserve other
bytes. Merge masks/values per unique word and execute exact integer RMW on GPU;
keep normal materialization at CPU consumers and draw hazards. CPU-owned pages
retain scalar stores.1042 cases per mode and four original captures pass; scene
7.53305 ->7.33985s (2.565% lower), both pairs faster. Retained with opt-in residency.
This removes a demonstrated synchronization boundary rather than merely reducing
readback bytes. Full console fidelity and30FPS remain unproved.

### HG-LEARN-031 - Snapshot only operands that a paired instruction can change

Static VU lane/write metadata proves111 of131 SQ sites cannot observe same-pair
upper VF changes. SQ reads no ACC/flags and upper writes no VI, so these sites
need no whole-state copy;20 overlaps retain original snapshots.104 Python tests
and four original captures pass. Scene7.2706 ->7.16805s (1.410% lower), both pairs
faster. Generated VU block replaced only after exact old-emitter matching; no
heavy arithmetic inlining or timing change. Evidence vu-sq-snapshot-* artifacts.

### HG-LEARN-032 - Combine host arithmetic calls without fusing guest rounding

One guarded AVX2 call inlines the existing integer product followed by exact add,
retaining product saturation and combined UF/OF masks.4M chained comparisons and
four original captures pass; scene7.1987 ->7.14815s (0.702% lower), both pairs faster.
Inlining stays in the small ISA source, outside the generated CFG. No host FMA,
clock change or general claim that broader inlining helps. Evidence PERFORMANCE.

### HG-LEARN-033 - Prove repeated readiness checks redundant at build time

Track per-lane readiness within static blocks, reset at every alternate entry,
and retain original masks after clock wrap. Skip only proven no-op VF checks;
keep all advances/ready writes/Q/P completions and VI checks.49152 compiled state
comparisons cover wraps, loops, delays and faults; four originals pass RTC-only.
Scene7.1256 ->6.9381s (2.631% lower), both pairs faster. Unlike rejected runtime
combined-clock helpers, this removes host checks without changing the clock path.
Evidence vu-readiness-proof-summary.json; model fidelity limits still apply.


### HG-FAIL-021 - Four-stage prepared matrix path regressed the scene

This guarded MULA/MADDA/MADDA/MADD preparation with four runtime commit calls
passes69632 compiled CFG and1048576 stage comparisons plus four original captures
(RTC only), but warmed alternating scene7.80045 ->8.53190s is9.377% slower, both
pairs. Reverted emitter/generated/CMake activation and archived new tests/source
under matrix183227-rejected. Preparation, flag storage and commit dispatch costs
are hypotheses, not isolated causes. Revisit only with a different measured
bottleneck or lower-overhead design; fewer arithmetic dispatches alone is no gain.
Existing compatibility fixes and readiness optimization remain. Scope this scene
and implementation; no general proof against matrix batching. See PERFORMANCE.

### HG-LEARN-034 - Measure actual fallback classes before adding renderer coverage

In gameplay31M..33M, an observation-only triangle census counted264257 triangle draws after clipping:244 untextured,264013 textured STQ-ready,0 textured FST,0 STQ-unready and0 fogged. The existing GPU path attempted all264257, accepted245616 and rejected18641 to CPU fallback. Therefore adding FST triangle support would not help this measured gameplay at all despite looking like an obvious source-code gap. Profile the actual rejection class before implementing new coverage; target the18641 proven backend/predicate rejections instead. Scope is this fixed gameplay segment, not a claim that FST is unused elsewhere. Evidence project-link-runtime/20260921T075705Z-9767c8ed0c76495590730899ff6f8e85.

### HG-LEARN-035 - Split backend rejection causes before broadening GPU semantics

A second31M..33M census resolved the18641 triangle CPU fallbacks:263 fail the front-end texture-format restriction and18378 are rejected only by the OpenGL backend's texture/output page-alias guard; frame/depth alias and every other front-end/backend category are zero. Do not jump from 'alias rejection' to unsafe parallel feedback rendering. First tighten conservative ownership footprints using already-proved wrap/swizzle/page rules; admit only cases whose tighter read/write sets are disjoint, leaving true feedback on CPU. Evidence project-link-runtime/20260921T080133Z-cf47199dc55a474ea17fc0669cdfa971.

### HG-LEARN-036 - Verify semantic output before treating overlap as removable

The18378 measured triangle backend alias rejects were split by actual output semantics in gameplay31M..33M. Every reject overlapped framebuffer pages, zero overlapped depth pages, and zero used FRAME.FBMSK=0xffffffff or otherwise had no-effect writes. So this workload's feedback class is not a false dependency created by fully masked color output. Stop no-op-output admission work here and investigate genuine ordered framebuffer feedback instead. Evidence project-link-runtime/20260921T081347Z-cf5afc140bad4634a1c56ca8265a5f8c.

### HG-LEARN-037 - Reduce genuine feedback to the exact texture interpretation before designing synchronization

The PCSX2-guided HG feedback-map capture found an unusually narrow class: all18378 rejected gameplay triangles use nearest-filtered PSMT8H, all use TEX0.TBP0 equal to FRAME.FBP and texture stride equal to framebuffer stride; CT32, PSMT8, linear and other formats are zero. PSMT8H reads the high byte of the same CT32 word-address layout and then uses that byte as a CLUT index. This turns the next correctness question from general framebuffer feedback into exact source-word versus destination-word dependence per covered pixel. Prove that relation before choosing framebuffer-fetch versus snapshot/copy semantics. Evidence project-link-runtime/20260921T083235Z-0a4b130471b5400693210ae98b0b9330.

### HG-FAIL-023 - Tighter whole-texture page bounds did not admit triangle work

The backend's broad texture interval looked capable of false feedback aliasing, so a tested whole wrapped texture-page footprint (including REGION_CLAMP minima, REGION_REPEAT bounds, base straddles and VRAM wrap) replaced it only for GPU triangle hazard admission. Coverage tests passed and the qualified candidate preserved frame/EE/GS/VU/normalized-IOP state, but GPU triangles remained exactly305856 and steady gameplay measured9.060FPS; no rejected triangle was newly admitted. The18378 measured alias fallbacks therefore still overlap the output pages at this page granularity. Reverted helper/backend/tests. Do not repeat page-bound tightening for this workload; next inspect whether the output side marks semantic no-op writes as hazards before considering true feedback support.

### HG-FAIL-022 - Moving CLUT draw barriers did not remove rendering work

Gameplay31M..33M showed23399 CLUT loads with pending work and zero conservative
FRAME/ZBUF overlap with the incoming palette source, but preserving old palettes
with per-draw snapshots and deferring those barriers regressed qualified steady
FPS from9.3183 to4.2280 in the exact layout-matched control. GPU work stayed
essentially identical:48195 sprites,305856 triangles,1014 triangle batches and842
masked-transfer batches with similar flush/readback time. The candidate also kept
more completed draw history and paid overlap/snapshot bookkeeping. Matching frame,
EE RAM,GS VRAM,VU and normalized IOP prove this was throughput,not visible state.
A costly call site can contain required work merely charged there; moving the
synchronization point is not an optimization unless it actually reduces work or a
measured host synchronization. Reverted all snapshot/probe/test source; keep the
observation evidence only. Evidence verifier d0149b1954... vs6a47185539c0....

### HG-FAIL-024 - Proven same-pixel PSMT8H feedback was correct but slower on the current compute backend

The exact feedback proof succeeded: all18378 measured gameplay31M..33M rejects had Q=1.0 at every vertex and exact 12.4 source-minus-screen deltas dx=dy=8, while794236/794236 covered samples independently mapped to their own destination CT32 word. A narrow candidate therefore admitted only nearest PSMT8H same-base/same-stride draws satisfying the constant-Q positive-subtexel proof and wrap-identity guard; its per-pixel ordered shader read the current destination word high byte as the palette index. The full33M replay moved GPU triangles305856 ->328503 (+22647 across the full replay) without faults. The fixed verifier preserved frame SHA02723437..., EE RAM8747a80e..., GS VRAMa30ba399..., GS stateeaf454f2..., VU state5adb6544... and normalized IOP01dc90c6....

Despite semantic equivalence, a layout-matched marker-off A/B was slower with feedback enabled: candidate final heavy windows3.15966/3.16915s =9.480455FPS versus control3.13418/3.14582s =9.554140FPS. Candidate total heavy host time6.32881s versus6.28000s (+0.777%), and both individual heavy windows regress. Reverted the specialization/backend/shader/tests. Keep the proof as reusable knowledge, but do not repeat this in-place per-pixel PSMT8H compute specialization unchanged; moving these CPU fallbacks to the existing GPU tile backend adds enough batching/synchronization/work to erase the saved CPU raster cost. Evidence candidate verifier b894c58157df40d79cfafcf2ec461843; layout-matched control e093bf9d0518463e8661e387e22aadad; semantic proof project-link-runtime/20260921T084132Z-1247e861480f4e5a9833ce0ee51da2b4.

### HG-FAIL-025 - Coalescing fragmented readbacks by over-reading did not improve gameplay

A full33M classification found resident multi-page CPU reads responsible for1006640500ns of1017485600ns measured readback time (98.93%) and2676965376 of2681552896 downloaded bytes (99.83%).1731 dirty multi-read events expanded to3537 synchronous glGetBufferSubData ranges. A second observation showed one first-to-last bounding read per event would add only23704 pages over327338 dirty pages (+7.241% transfer volume), so a production candidate fetched fragmented events into a reusable staging buffer and copied back only the original dirty runs; clean/CPU-owned gap pages, ordering, barriers, faults and gpu_dirty semantics were preserved.

The candidate preserved the established frame/EE/GS/VU/normalized-IOP hashes and qualified864 original0x001bef84 writes, but throughput did not improve. Full replay readback rose to1056517700ns and download volume to2880323584bytes. Qualified steady gameplay was9.378693FPS (windows9.470325/9.288817), below the stronger recent layout-matched9.554140FPS control. Source was restored byte-exactly. Reducing glGetBufferSubData range count by bounding-span over-read/staging is therefore not useful on this workload; do not repeat this mechanism unchanged. Next isolate readback/synchronization inside the actual31M..33M heavy window before investing in a genuinely different mapping/fence/staging mechanism. Evidence project-link-runtime/20260921T103855Z-dbe0e619b99a418ab34b14ee61f1eada, project-link-runtime/20260921T104328Z-311b52eb2a58411eb94a3697ae3b1591, candidate project-link-runtime/20260921T104813Z-6724e7c1730a4ccda7d3d5603d87f622 and verifier d043105abbb14e42a897f37a3ec532a3.
