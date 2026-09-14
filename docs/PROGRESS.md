# Handoff: Haunting Ground independent static recompilation

## Goal
Build a playable static recompilation of the user's USA Haunting Ground dump,
targeting Windows, Linux and other feasible desktop systems, using OpenGL.
Independently decode/translate; never use ps2recomp. PCSX2 is memory-oracle-only.

## Current state
Last checkpoint: **2026-09-13 America/Chicago**. User explicitly says keep
working until told to stop. Continue through checkpoints; do not end at a milestone.
Repository root: `C:/Users/johnn/Documents/ChatGPT/Haunting`. Not playable yet.

### Latest verified checkpoint

**2026-09-13 natural-speed optimization continuation.** User resumed work and
requested faithful1:1 speeds, quickly fixing inefficiencies. The major milestone
remains sustained natural host playback/presentation cadence. It is NOT reached.
Continue through checkpoints when session resources permit; no new permission is
needed for authorized local work. AOT only, no changed guest waits or time budgets.

- Retained performance reference: replay37,1066 OPENING CSC conversions spanning
  35.611639modeled seconds in51.8379host seconds (median48.0ms). Earlier22 takes
  77.9186host seconds: about50.3% more throughput, currently68.7% modeled rate.
  These are conversion events, not displayed FPS or physical-console parity.
- Retained changes: checked RGB24 sprite reduction; aligned CSC byte-lane reads;
  X-inner separable IDCT with original per-output sum order; indexed existing DCT
  codebooks; static intra-page EE branch labels; fixed-width inline user-RAM
  access with full checked fallback; bounded scheduler cursor/wakeup scans.
  Split host device profiles distinguish IPU/scratchpad, VIF/GIF and raster.
- Sparse-IDCT zero-tail experiment26 showed no speed benefit and was reverted.
  Word-based/out-of-line bit reader34 also showed none (53.42s versus53.1226s);
  it is reverted in source. Do not reintroduce either without new evidence.
- Replays31->32 and32->33 have identical final EE RAM, GS VRAM, GS metadata and
  machine JSON; all shared sampled images retain order.32->33 shares1311images.
  All1066 opening timestamps agree with22. Compared to22, EE byte47e373 differs
  (12 ->11), already stable from25 onward in an original input-related object;
  exact field semantics remain unproven. Host input pulse lengths differ.
  IOP RTC bytes vary by launch time. Do not claim full22->33 byte equivalence.
- All21CTest passed after refreshing affected targets and relinking hg_diagnostic,
  including87Python tests. Added full codebook prefix equivalence,1280 IDCT cases,
  all128 CSC offsets with starvation/backpressure, scalar alias/sign/fault/watch
  cases, branch budget cuts, and scheduler priority/cyclic/wakeup combinations.
- Reader restore build, hg_diagnostic relink and21CTest completed. Dense host
  sampling now1/256 with11 phases (separate IPU input/output); replay35 completed
  32M slices normally. EE remains largest sampled opening phase. Baseline33
  remains the last full60M retained measurement.
- Byte-copy batching36 is REJECTED and removed: exact1066 guest timestamps,
  EE/GS/machine state and1311images, but54.2326s vs53.1226s showed no gain.
- Retained new wider-RAM access37: full-span store validation once and checked
  fixed-width load/store halves preserve device/watch fallbacks. All21 refreshed
  checks pass, including startup checks rerun after successful diagnostic relink.
  Replay37:1066 events,35.611639guest seconds in51.8379host seconds (~68.7%),
  median48ms. Exact guest timestamps, EE/GS/machine state and1311images match33.
  Evidence speed-opening-replay37.json. No1:1 claim.
- Native host profile39 completed:3254samples,0context errors over five seconds
  during opening. EE trace_pc161samples (~5%), r174 and w77; raster196,
  psmct32_word160, IPU rgba126, IDCT53. Profile-only, no wall-time comparison.
  Earlier38 completed before manual sampling;39 automates sampling at350images.
  External native-profile39.json/log and %TEMP%/haunting-native-sample.py.
- Combined EE trace-guard/forced-accessor candidate40 is REJECTED and reverted:
  all1066guest times, EE/GS/machine state and1311shared images agree with37,
  but53.2729host seconds vs51.8379s. All21checks had passed. Do not claim
  profile attribution proves this change saves time. Existing traces accounted
  for in DIAGNOSTICS; no debug probe silently removed.
- Framebuffer-address41 is REJECTED and reverted: exact1066guest timestamps,
  EE/GS/machine state and1311shared images, but57.5964host seconds. Raster total
  slightly improved (12.4892s vs12.6544s37) while whole playback worsened. Keep
  whole-sequence comparison authoritative over local counters.
- Current candidate42 makes HG-DIAG-001 a build-time EE history option. Default
  HG_EE_INSTRUCTION_TRACE=ON retains existing tracing. Local throughput build
  OFF removes only history calls and explicitly rejects PC watches. Separate
  synthetic no-trace target verifies branch state and preserved unknown-PC fault.
  Diagnostic OFF build7403 completed. Fresh source-only Release build passes
  all16CTest entries (including87Python cases and explicit trace-disabled checks),
  using Python3.11 under %TEMP%/haunting-source-release-20260913. Python3.14
  encountered sandbox temporary-directory permissions;3.11 passes unchanged tests.
  Next run prepared42 against37; no build concurrent with timing.
- User requested a SOURCE repository push, confirmed explicitly, to
  https://github.com/KunoDemetries/hg2.git. Remote empty; SSH publickey failed,
  HTTPS read works and GCM account KunoDemetries exists. Initial source snapshot prepared on codex/console-speed-checks, origin HTTPS.
  Fresh16-test source-only validation passes; explicit source/doc/config file
  list reviewed and secret-pattern scan clear. .gitattributes normalizes text
  line endings. Publish normally and verify remote commit. Original data/emu/
  generated outputs remain ignored; no executable release requested.
- Separate native IDCT AVX2 benchmark has matching sampled checksums and about10%
  speed gain, external %TEMP%/haunting-ipu-arch-bench/results.json. No project
  architecture flags changed and no whole-game gain established from this.
- User additionally requested debug-code accounting and further EE checks.
  AGENTS now requires stable HG-DIAG identifiers; docs/DIAGNOSTICS.md inventories
  ten diagnostic/experimental families, activation, guest effects, cost and cleanup.
  EE disabled-trace candidate40 was rejected; IOP per-instruction history remains active
  (50/3254samples), with downstream host-history consumers audited in runner.
  Synthetic TOC input and issue-slot clock are explicitly guest-affecting probes,
  not silent production defaults. Current replay has no EE watches. Do not remove
  correctness checks or add unproven EE roots to make boundary counts look better.
- User explicitly requested major missing-code/timing scan and PCSX2/web checks.
  Fresh %TEMP%/haunting-major-speed-audit/major-scan.json: EE162833words,
  1071boundaries; all29IOP94718words,125residuals,244BREAKtraps. ROMDRV accounts
  for178-word difference from built28modules; no evidence it causes opening
  slowdown. Four unsupported EE boundaries are vector/cache bodies, still
  explicit faults and not reached by completed60M opening. No guessed roots.
- Local PCSX2 retains EECycleRate=0/EECycleSkip=0/NominalScalar=1/NTSC59.94;
  no PCSX2 process running. Official performance docs distinguish FPS/VPS/speed.
  Host Ryzen7 9800X3D, Balanced power plan; no power setting changed. Continue
  measuring host execution costs while preserving independently derived timing.
- EE coverage unchanged:162833words/1071boundaries;28IOP modules/94540words.
  Windows verified; no new Linux or other-platform verification this session.

Evidence under %TEMP%/haunting-toc-probe:
- speed-replays23-26.json, speed-replays27-28.json, speed-replays28-29.json.
- speed-opening-replay31.json through speed-opening-replay34.json.
- startup-clock-bursts-22..34 logs/dumps and clock-bursts-N-frames/manifest.json
  (24 interrupted; no complete dump). Retained33 is60M, as are31/32/34.
- Local helpers %TEMP%/haunting-clock-bursts-N-replay.py; input and preview use
  %TEMP%/haunting-input.txt and %TEMP%/haunting-live.ppm. Helpers may exit0 when
  diagnostic exits2: inspect native JSON kind and log. Recent runs are ordinary
  native-iop-budget completions, not unsupported-hardware faults.
- %TEMP%/haunting-ipu-loop-bench/results.json is a synthetic loop-order benchmark,
  not connected playback. No game-derived content was added to version control.

### Earlier checkpoints (historical; superseded by the state above)

- **Handoff requested with optimization checks:** retain paused state. On
  explicit resume, use fresh replay25 from24, with no rebuild needed for the
  current RGB24 optimization. Compare full32M run to replay23, separating
  OPENING using its disc-read start (not a fixed CSC sequence). Compare median
  and span host/guest CSC times, runner raster totals over matching26M..31M
  checkpoints, and corresponding image hashes/order. Capture polling can omit
  frames, so distinguish sampling gaps from real image mismatches. Confirm
  native diagnostic result from log/JSON rather than helper exit. Full-VRAM
  synthetic comparison already passes masks0/0055aa33/ffffffff and live texture
  feedback; all refreshed regressions pass. Only broaden tests after new changes.
  No established speedup yet; remaining goal is natural host playback cadence.

- **PAUSED at user's request,2026-09-13:** no further autonomous continuation
  until the user resumes. RGB24 sprite diagnostic build67560 completed.
  All14 native regression targets were refreshed against current headers;
  all21CTest pass (including87Python tests via tools). Replay24 was stopped
  on request by terminating confirmed hg_system_diagnostic PID52316; helper
  session88537 finished with child exit4294967295. Partial replay24 is NOT a
  completed timing comparison; no connected RGB24 speedup claim yet.
  No diagnostic/build processes remain. A process inventory also found no
  hg_opengl_host; viewer38888 was not stopped by this pause operation.
  On resume: run a fresh replay25 from32M no-watch24 helper, compare against
  replay23 timing/images, and continue natural-speed work. Latest sustained
  opening evidence remains replay22 below. Do not restart work while paused.

- **Major milestone replay22: sustained OPENING visibly advances.** Shared
  original-AOT SYSMEM allocation plus CRI switch/rate callbacks clears the stall.
  Opening reads11202sectors2113282..2124483;1066CSC conversions from24,367,072
  to59,978,711us,35.611639modeled seconds spanning77.9186host seconds. Distinct
  brick wall/cellar/dog scenes captured;1314unique images across full startup.
  Replay ends native-iop-budget60M /111.424host seconds, no hardware fault.
  Evidence external opening-replay22-evidence.json, clock-bursts-22-frames.
  Median opening interval34.463modeled ms/71.4host ms: NOT natural console speed.
  Next major milestone: natural host cadence without changing guest waits.
  Replay23 measured runner rasterization: during26M..31M it consumes3.15host
  seconds out of10.7elapsed. Instrumentation times only nonempty runner batches,
  excluding any inline FINISH/CLUT-triggered drawing. Replay23 ends32M budget.
  A checked RGB24 sprite reduction now bypasses redundant pixel stages only
  with no blend/alpha/DATE, Z ALWAYS+masked, valid formats/storage and no scanmask.
  Live texture reads, row order, FBMSK and padding preserved; synthetic complete
  VRAM comparison against generic pipeline passes, including texture feedback.
  Active diagnostic build67560, haunting-build-rgb24-sprite.log. Next replay24
  from no-watch23 after build; compare corresponding captured frames and raster/
  CSC timing. All other natural-speed/performance work remains active.

- **Replay21 CRI rate callback:** previous14-entry switch clears; now stops
  at19,333,856us/23.8347host seconds on18ab4 ->18e38. Original181e0 installs
  seven rate-dependent callbacks in object+40; original relocated code matches
  RAM, and live24408+40=18e38 for48000/48000. Added all seven verified roots;
  evidence external cri-audio-callbacks-181e0.json. Generated94540IOP words.
  Active diagnostic build58194, haunting-build-cri-rate-callbacks.log.
  Replay22 helper prepared from18; next run after build. No movie advancement
  claim yet. All21CTest passed after previous switch, before these seven roots.

- **Shared-SYSMEM connected replay20:** build completed;21CTest including
  original reboot and87Python tests pass. Replay20 stops at12,383,634us /
  11.7071host seconds on CRI indirect transfer10f0c ->10f30, before movie input.
  Original10ef4 bounds selector below14; original relocated1aaa0 table agrees
  with live RAM in all14 slots. Added the complete bounded switch; evidence
  external cri-command-switch-10f0c.json. Generated28IOP modules93928words.
  Building diagnostic session68535, haunting-build-cri-transfer-switch.log.
  Next replay21 helper is ready from no-watch18; verify continued startup and
  OPENING/audio synchronization after this switch. Viewer38888 retained.

- **Active shared-SYSMEM fix build27390:** haunting-build-shared-sysmem.log.
  Replay19 confirms audio clock0 because8192produced minus16384queued clamps0.
  Root cause: native MODLOAD scratch overlaps original SYSMEM SIF allocations.
  Audio buffer121b00..1223d0 contains2254/2256bytes of original DS2O_S1.IRX
  atfile1d60; its0x880payload at121b40 matchesEE3cf1c0 exactly. Evidence:
  external opening-audio-buffer-overlap.json. Native ordinary scratch, worker
  and HEAPLIB allocations now delegate to original AOT SYSMEM exports4/5;
  static image/call stacks reserved in original pool. No fabricated audio time.
  New synthetic shared-allocation/plan-isolation regression passes iop_tests.
  Next: complete build, game-reboot/full checks, replay20 from no-watch18;
  verify preserved synchronization buffers and continuing OPENING conversions.
  Replay19 finished; no diagnostic/oracle active; viewer38888 retained.

- **Replay18 OPENING start verified:** clears menu selection and reads1322
  sectors2113282..2114603 from OPENING,24380208..24481837us. Four CSC frames
 186..189 start24433535..24604741us; first dark brick image capture233 appears,
  then no more conversions through60M budget/57.7958host seconds. Not sustained
  cinematic advancement or natural playback. All87Python/21CTest pass after
  menu roots. Generated162833EE words1071boundaries. No build active.
  Investigating actual original audio-clock path2429c8->242a88->1d3210->1d30c8;
  MPEG1081440, movie3e893c, audio3b72fc. MPEG decoded queue4, audio-position
  record1084960+24 remains0, rate+28=48000. Global display count3b7230 advances.
  Replay19 is a30M targeted trace of1d310c/3118/3134/31c0 and242a2c, external
  helper haunting-clock-bursts-19-replay.py. Next: inspect traces and identify
  actual audio producer/consumer blockage; never fabricate playback time.

- **Replay17 current:** automated title then active highlighted New Game now
  succeeds; missing EE12e578 is a bounded menu switch at12e4e8, selector<7.
  Original/live44e8a0..bc agree; added table and six verified movie/demo start,
  wait and end member descriptors. Evidence: external menu-selection-switch.json.
  Generated162833EE words1071boundaries; active build menu-selection log.
  Replay16 cleared130a40, showed active New Game, then timed out to demo case
  12e534 at54,059,827us/77.2441s (NOT a60M budget exit); early Cross during fade
  was ignored. Replay17 uses bright highlighted row recognition and stops at
  12e578 at24,263,393us/30.1982s. Both cases belong to the added switch.
  Both retain explicit faults; no OPENING consumption yet. All87Python/21CTest
  passed before the new switch/descriptor roots. Viewer38888 remains alive.
  Sprite change has no measured gain: CAPCOM median host55.35ms in16 versus
  53.05ms in15, modeled33.3655ms. Next: complete build, checks, replay18 from17.

- **Active continuation:** next major milestone is visibly advancing OPENING
  frames in connected OpenGL, corroborated by ISO reads in2113282..2167647,
  followed by host cadence measurement against the30fps movie target.
  Regenerated pending menu roots:162042EE words,1063unresolved boundaries.
  Building menu/sprite changes with haunting-build-menu-sprite.log; viewer38888
  remains alive. Next: full checks and automated replay16 title/New Game input.

- **Handoff 2026-09-13, latest state supersedes older active-build notes:**
  Replay15 reached the title and New Game/Load Game/Options menu in connected
  OpenGL. Captures: external clock-bursts-15-frames/198.png,200.png,258.png.
  It stopped at missing EE130a40 after41,780,650 modeled us /52.6577 host seconds.
  Original12c31c installs table46a110; live9c90e4 points there; its two methods
  are130a40 and1309d0. Both roots are now in config but NOT regenerated/built.
  Evidence menu-table-46a120.json is misnamed: actual table is46a110.
  GS fixed-UV sprite coordinates now calculate U once per visible column and
  V once per row, preserving exact integer formulas and row-major shading.
  Latest gif_tests build/run passes; connected performance is NOT measured.
  No build/diagnostic/oracle active at handoff; keep viewer38888 alive.
  Next executable action: python tools/hg.py emit, then clean diagnostic build
  using haunting-clean-build.ps1 -Project hg_system_diagnostic with a fresh log.
  Run full CTest, prepare replay16 from15 with automatic second Start at title
  and Cross on New Game, then capture next fault/OPENING consumption.
  Replay15 second Start was MANUAL at39.620M and released39.680M; helper15
  only automates the earlier red-corridor Start. Do not repeat that idle wait.

  Natural console speed is NOT matched: CAPCOM median CSC intervals remain
  33.365ms modeled versus55-56ms host (~18 conversions/sec, not presented FPS).
  No OPENING asset consumption or advancing long cinematic verified yet.
  Latest complete suite before pending sprite/root edits:87 Python,21 CTest pass.
  Last generated coverage162005EE words/1063boundaries;28IOP modules/93389words.
  Completed build5291 and replay15 supersede their historical pending status.

- **Latest active build5291:** haunting-build-gs-surplus.log;162005EEwords,
 1063boundaries. Replay14 Start press at19.480M transitions the scene then stops
 at20,859,685us/23.6322s: GS IMAGE surplus at EE1b7960. Original packet4f23b0
 sends17920qwords for PSMT4 640x448 (8960needed). Independent synthetic GS
 memory-oracle probe verifies exact/double upload equality for CT32/CT16/T8/T4;
 evidence in external gs-surplus-oracle and docs/ORACLE.md. Probe44816 closed.
 Native completed-transfer state accepts surplus only after valid completion
 in measured formats, preserves strict idle/cancel faults and resets on TRXDIR.
 New GIF regressions pass. Added already-verified downstream stream callback
 1d4ff0 from original1d5590..1d55a0 and replay10 live+48, avoiding another build.
 No diagnostic/oracle active; viewer38888 retained. Next: finish5291, full21
 checks, fresh replay15 derived from Start-capable14. Verify next screen and
 OPENING consumption, continue profiling; natural30fps still not verified.

- **Visible post-CAPCOM milestone:** replay13 now shows the red corridor in
  connected OpenGL (clock-bursts-13-frames/149.png).35Mquanta/34.6264s, no fault.
  Previous replay12 was100Mbudget/41.6691s, but4830draws remained unexecuted;
  the runner now services ordered GS draws after VIF/GIF independently of preview.
  GS manual pp38-43 provides kick/order provenance; all21 CTests pass. No shared
  rendering material used. Natural movie host timing still slow (median~55ms);
  byte extraction has no measured gain.185CAPCOM conversions, no OPENING reads.

  Live scene descriptor888444 targets130460. Its original bit3 input test gates
  transition to130170 while44e958 points at a LOOP_DEMO resource. Added host
  Start command (active-low bit3, no pressure channel). Replay14 helper presses
  Start on verified red-corridor pixels after CAPCOM; session/next result pending.
  Viewer38888 retained. Next: inspect Start transition, trace next fault or
  verify OPENING sectors2113282..2167647 and advancing cinematic captures.

- **Timing continuation current:** replay11 clears1cda10, stops at related
  default callback1c4ce8 (RA1c5ddc),19,274,818us/21.6912host seconds. Original
  registration1c4dc8..1c4e08 proves defaults1c4cb8/1c4ce8; both now rooted.
  Generated161788words1051boundaries. Build46027 active, external log
  haunting-build-byte-prefix.log. No diagnostic/oracle running; viewer38888.
  IPU peek now consumes byte portions instead of individual bits; exhaustive
  independent bit-reference tests cover8patterns,128offsets, widths0..32 and
  existing MPEG tests pass. Full native suite after build remains pending.
  Prefix lookup already passed full21 tests before the byte extraction change.
  Prepared replay12 helper outside repo; launch after build and final relink.

  Replay11 profile explicitly consumed all1704 CAPCOM ISO sectors2111088..2112791;
  no OPENING sector consumption yet.182 CSC conversions span6.733modeled seconds
  and10.696host seconds; these are conversions, not presentation events. Thus
  host execution is the larger observed shortfall. Movie header raw count187
  gives6.233nominal seconds but packet/presentation completeness is unverified.
  Prior replay10 all149 image hashes exactly matched replay9. Long opening and
  natural30fps remain unverified. Continue through next original stop, no bypass.

- **Active timing continuation:** prior build completed; final diagnostic relink
  succeeded and all87 Python tests pass. Next measurable milestone: advancing
  OPENING frames in connected OpenGL plus original asset-consumption evidence;
  then measure presentation cadence against the verified30fps movie target.
  Full native checks and experimental reboot are running before replay10.
  Viewer38888 retained. Issue-slot scheduling remains experimental, not PS2
  cycle parity; natural playback must be measured independently of throughput.

- **USER-REQUESTED HANDOFF,2026-09-13 15:58 America/Chicago:** build session43368
  still RUNNING, cl PID42784 observed (verify identity anew); log
  `%TEMP%/haunting-build-budget-scene-closure.log` appears only on completion.
  No diagnostic/oracle active. Viewer38888 alive, watches `%TEMP%/haunting-live.ppm`.
  Generated161695 EE words1050 boundaries,28 IOP modules93389 words. Current
  build includes ordinary-return budget preservation (only reserved EE/IOP return
  sentinels yield; native services/syscalls still yield), DMA busy-loop parking,
  seven more verified scene-state descriptor roots (external
  scene-descriptor-closure.json). Runtime IPU constant-DC transform optimization
  added during build; runtime_tests built/passed against new IPU library. After
  43368 completes, rerun clean diagnostic build to guarantee final relink with new
  IPU source/library, then full CTest and experimental --verify-reboot.
  Native EE/IOP return-budget tests and DC constant transform edge tests pass;
 87 Python passed before final ordinary-return edits. Run Python again in batch.
  The latest changes are NOT connected-replay verified. Fresh helper prepared:
  `%TEMP%/haunting-clock-bursts-10-replay.py` (100M quanta,no obsolete watches,
  short60ms inputs,10ms capture polling). Launch only after build/checks complete.

  Last connected replay9 ended20,580,407us in21.5898s at130460; scene transitions
 130460/12f290/12f500 plus descriptor closure are now generated. Replay8 ended
 20,447,349us in22.4743s at1306e0. Replay9 has149 unique images; still only CAPCOM
  verified. DMA busy-loop optimization improved modestly; no30fps claim. Ultimate
  goal remains advancing long OPENING cinematic in connected OpenGL, with original
  asset consumption evidence. User priority: profile/fix natural rate now. Original
  CAPCOM/OPENING header target30 progressive fps verified independently; NTSC
 59.94 fields is separate. PCSX2 inspection now allowed as timing lead, never copy
  implementation/algorithms or use emulator backend; installed normal timing
  settings inspected, no PCSX2 source or shared rendering material used.


- **Active build1960:** `haunting-build-dma-wait-members.log`,158568 words1004
  boundaries. New narrow generated IPU CHCR busy-loop parking tested (87 Python,
  native translation pass), restricted to read-only CHCR3/4, fixed register result,
  real STR bit and no in-loop trace watch; no completion is fabricated. Adds four
  scene member transitions1306e0/130520/12fb50/12f720, independently verified
  original/live descriptors3b0070..a0 and actual stores in1306e0. Replay8 ended
 20,447,349us in22.4743s at1306e0, after passing all prior scene virtual stops.
  No replay active; viewer38888. Replay8 removed obsolete EE instruction watches
  and samples captures every10ms;149 unique images. Replay7 (with watches) ended
 20,413,938us in25.8812s at2d4020; its7-method table and next virtual slots verified
  and included in current binary. Direct straight-line dispatch preserved complete
  benchmark state (EE/GS exact, IOP only RTC seconds/minutes); legacy30M4.56995s
  before vs4.66269s after, no legacy gain; burst runs improve modestly, not isolated.
  Next: finish1960, launch fresh clock-bursts-9 from no-watch replay8 helper,
  inspect next scene/movie and measure busy-loop gain. Long opening and30fps remain
  unverified. Original CAPCOM/OPENING sequence+extension headers independently
  verified progressive30fps (H.262 Table6-4), distinct from59.94 display fields.


- **Current build52429:** native EE straight-line goto optimization, tests86Python
  and translation native pass; full diagnostic build in progress
  `haunting-build-direct-step.log`. Exact30M legacy baseline retained as
  `profile-before-fallthrough.*`. After build run identical profile-after-fallthrough
  and compare EE RAM/GS/IOP/metadata (RTC time exceptions only), then replay7.
  No diagnostic active; viewer38888. Replay6 ended20,447,297us in27.7091s at
  missing3913b0. Eight-qword IPU DMA service per experimental quantum (EE manual
  pp41-43) removes artificial16MB/s cap. All148 unique captured frame hashes
  match replay5 in order; prior run58.8558s. Four focused tests pass. New scene
  table47a79014 methods verified against original/live and rooted;157508 words976
  issues. Replay7 must use fresh prefix, same short-input helper,100M budget.
  Long opening/natural FPS still unverified. Latest PCSX2 permission allows timing
  inspection without copying, recorded in AGENTS; no implementation copied.


- **Live replay5:** session8046/PID33300 `startup-clock-bursts-5`,100M quanta,
  field-sampled OpenGL preview; at31M quanta CAPCOM still advancing, no fault.
  Viewer38888. Replay4 ended30M budget,52.1107s host,129 images. Natural FPS
  and long opening still unverified. Burst JR RA return boundaries fixed/tests
  pass. Nested bootstrap calls corrupted suspended stack; allocated separate
  checked0x2000 stack fixes observed IOP return-to-c failure. Full21 tests pass.
  EE scene table46a040 methods12c220/130810/12e1b0 now rooted;156359 words956
  boundaries, build9331 and field-preview build passed. Input pulse shortened;
  preview now field-driven (16ms host cap), one bulk RGB write. No diagnostics
  other than33300, no oracle active. User permits PCSX2 timing inspection, no
  copying; installed normal timing settings match existing59.94 field target.
  Next: inspect replay5 natural CAPCOM end and next fault/movie; optimize measured
  CPU cost while keeping experimental clock opt-in. See PERFORMANCE.md.


- **Current performance continuation:** full21 CTests passed after burst/clock
  changes;86 Python tests pass. First burst replay `startup-clock-bursts`
  exposed bootstrap return overrun at IOP1f0000 after28752us; no game fault
  bypassed. Generators now yield cooperative bursts after original JR RA (delay
  slot still executes); native sentinel regression tests added. Build session25265
  (`haunting-build-burst-returns.log`) active; test build78914 active. No replay
  active, viewer38888 retained. First instruction-granular clock probe ended3B
  slices at budget with display-flag polling dominant; not natural playback.
  Experimental clock now uses1us device quanta with294/295 EE and36/37 IOP
  boundary budgets; this is approximate functional scheduling, NOT cycle parity.
  Narrow read-only RAM polling loops may yield until real flag change, excluding
  watched loops and overlays. Legacy timing stays default. Next: finish builds,
  sentinel regressions, fresh clock-bursts-2 replay and measure progress/host cost.
  Long opening still unverified; verified missing scene table46a040 methods
  12c220/130810/12e1b0 remain to root as a batch after current build.


- **Active priority changed by user:** fix natural playback rate now. Long-intro-3
  passed both cleanup roots and stopped130810 via11fb04 at1326167984us,
 285.26s host (4.649M slices/s). No long cinematic. Captures preserved.
  Investigating the legacy1us-per-AOT-step mismatch; manuals EE bus147.456MHz
  (EE p36), processor/bus ratio2 and single/double issue (Core pp14,18,78).
  Added opt-in experimental --clock-profile issue-slots (legacy remains default),
  exact294.912MHz slots/8:1 IOP ratio, fractional microseconds; NOT pipeline/cache
  cycle accuracy, branch bundles/bootstrap special budgets still approximation.
  Clock arithmetic/native timer test passes. Clock-probe replay17523/PID36460
  active, currently400M slices, no fault, no GS draws yet. Next inspect whether
  proper work/time ratio reduces movie wait overhead; do not promote without
  evidence or present experimental pacing as hardware parity. Viewer38888 alive.

- **Profiling/optimization requested by user:** added optional host sampled phase
  profiler, then split EE/IOP native dispatch into4KiB functions, preserving
  instruction budgets/delay slots/native wait returns. Same30M workload measured
  35.9562s baseline ->14.6356s EE-only ->4.56141s both (7.88x). EE RAM/GS VRAM
  and all metadata match; IOP only RTC seconds/minutes differ. Full evidence and
  limitations in docs/PERFORMANCE.md and external profile-*. Build92817/88052
  passed; native EE/IOP partition regressions pass, Python85 pass. Full21 suite
  passed. Profiled long-intro-3 replay37703/PID37660 active, at200M slices
  in46.13s, viewer38888 alive. Member380480 root included, long movie unverified.

- **Long-intro-2 completed with a new explicit stop:** destructor fix clears old
  2c8c10 stop; at1325849980us member helper100b6c reaches380480 via11fafc,
  RA11fb04. Object888444 descriptor matches original44aee0 and static RAM
  exactly (0,-1,380480);380740/4c loads and38076c..78 installs it. Added
  root380480; generation155541 words941 boundaries. Build30192 active,
  haunting-build-long-intro-member.log. No diagnostic/oracle running; viewer38888
  retained. Replay3 helper prepared, not launched. Latest capture145 still CAPCOM
  fade; long cinematic unverified. Next: finish build, reboot regression and
  long-intro-3 replay, then capture next stage.
- Scanner now additionally detects loaded direct-member descriptors through integer
  and floating-point copies. Pre-fix scan finds421 records/393 missing targets,
  including44aee0->380480 which table-only scan missed. Candidates retain load
  PCs and require live-use verification; no graph auto-expansion. All85 Python
  tests pass, including stale-base/profile rejection and no target auto-rooting.

- **Scanner refinement requested during live replay:** audit found924/936 boundaries
  are indirect transfers but old fixed-address pointer scan returned0 candidates.
  Added read-only literal object-table installation scan, including call delay-slot
  stores, source PCs, slots, compiled status, deduplicated missing targets and256-byte
  scan limit. Current scan finds56 tables/547 unique uncompiled candidate targets;
  independently catches46ecc0 installed at2c8c3c and37f8cc. These are unverified
  pointer-run leads, never automatic AOT roots or runtime substitutions. Synthetic
  tests cover delay slots/no auto-discovery, stale constants, stack stores, data
  exclusions and cap. All83 Python tests pass; full major-scan succeeds (EE155463/936, IOP29 modules93567 words, linked126 residual). Removing only the new destructor root in an in-memory pre-fix config proves scanner flags2c8c10 as the sole missing46ecc0 method via37f8cc, before any replay. Live replay83496/PID37628 remains
  uninterrupted beyond600M slices, no fault. Next: inspect full scan and continue
  CAPCOM-to-opening transition; use table families to avoid one-target replays.

- Destructor AOT root added after original/live verification;155463 reachable words936 unresolved. Release diagnostic build19009 passed (haunting-build-long-intro-destructor.log); unchanged DMA/runtime/translation3/3 pass. Fresh long-intro-2 replay83496/PID37628 active,3B slices; external helper/captures/log use long-intro-2 prefix. Viewer38888 retained. Downstream destructor callbacks239160 and1bb9c0 already translated. game_reboot check46898 passed; focused checks4/4. Next: monitor natural CAPCOM end, trace next stop or capture advancing OPENING.

- **ACTIVE continuation:** long-intro-1 finished CAPCOM fade-out and faulted after1.3B slices at missing static target2c8c10, call2d1b78, RA2d1b80. Object94fb40 points to46ecc0; original ELF/live table match, slot8 is2c8c10.146 captures, latest145 inspected (CAPCOM nearly faded). Diagnostic36672 exited; viewer38888 remains. Next milestone remains successive long-opening frames with asset provenance; root verified destructor and rebuild/replay. No long cinematic verified.

- **HANDOFF requested by user (2026-09-13):** long-intro-1 replay remains RUNNING, tool session16811, diagnostic PID36672; OpenGL viewer38888 alive. Latest checkpoint1.2B slices, EE1e5398,15424 GS draws/rasterized, no fault.133 unique post-input captures through132.png. Inspected025/035/060/076/093/113/126: complete CAPCOM character motion, both leave screen, latest inspected126 holds logo. Long OPENING cinematic NOT yet verified. Do not restart this expensive replay: inspect current log/process/frame first and let it finish/trace next fault. Bound3B slices, final external startup-long-intro-1.ram sidecars appear on completion. Helper can return0 despite diagnostic2. This session changed only diagnostic CLI cap500M->3B and docs; diagnostic build passed, no new runtime implementation/test changes. Exact next action: poll log and frame manifest, inspect latest PNG, continue through natural CAPCOM end to long intro.

- Long-intro-1 live update: replay16811/PID36672 passed1B slices without fault; over100 unique captured frames show Hewie then Fiona moving across CAPCOM. Still awaiting the long OPENING cinematic. OpenGL38888 remains available. No new runtime changes or tests during this uninterrupted run.

- **ACTIVE new goal:** user requested continuation until the next, long intro cinematic plays. Next measurable milestone is successive visible frames from that cinematic beyond CAPCOM, with original asset/runtime provenance. Long-intro-1 replay16811/PID36672 is active with3B-slice bound and external frame capture. Runner had a hard500M argument cap; raised only that diagnostic cap to3B and rebuilt successfully (haunting-build-long-intro-budget.log). Prior500M run stopped only on its budget. Keep OpenGL visible and trace any new original stop without bypassing checks. Original CVM index confirms OPENING.SFD extent652685,size111341568,512x448 sequence header; selected-movies.json under external long-intro-assets records it and CAPCOM metadata.

- **Requested visible-movie milestone complete.** startup-movie-services reached its500,000,000-slice bound with no runtime fault (diagnostic exit2 is the existing budget-stop result; JSON kind native-iop-budget). Final EE1e40f0, virtualtime500000000us. Thirty unique post-input captures include the advancing CAPCOM movie fade; frames006/007/008/013/028 inspected. External final RAM/EE/IPU/GS sidecars and frame manifest preserved under %TEMP%/haunting-toc-probe. Connected OpenGL38888 remains alive holding the latest movie image. No diagnostic, build or oracle process remains active. Release build33723 and game-reboot pass; latest relevant DMA/runtime/translation/game-reboot4/4 passed after motion changes. Current generation155372 words934 unresolved. Visible output achieved; full gameplay/audio/real-time performance and exact physical rounding are not verified. Non-intra mathematical IDCT still differs by one in24/12288 synthetic oracle samples; this limitation remains documented. Next broader work, if requested: measure movie presentation pace and continue beyond the CAPCOM intro with a fresh evidence prefix; there is no new captured hard fault to bypass.

- **Visible movie milestone achieved (2026-09-13):** movie-services build33723 passed and startup-movie-services clears all prior stops. Original CAPCOM.SFD intro is visibly rendering through connected OpenGL PID38888. Sequential external movie-services-frames/006.png,007.png,008.png and013.png show CAPCOM logo/Fiona/Hewie fading in; manifest contains frame hashes and host timestamps. At240M slices15134 draws/rasterized, no fault. Replay42474/PID34944 still running toward its500M bound to retain final RAM evidence. Game-reboot check passes; latest focused4/4 passed on motion-spr runtime. This verifies advancing movie imagery only, not full playability, real-time speed, audio or physical1:1 rounding. No runtime interpreter/JIT/emulator backend.

- Movie-worker build4650 passes; startup-movie-worker advances to registry dispatch25646c ->243428,time171500228us. Verified seven14-slot service tables against live RAM/ELF; added98 static callback destinations as one batch (external movie-service-tables.json). Generation155372 words934 unresolved. Movie-services build active; next fresh replay. Captured worker frames remain warning/startup transitions; no movie playback visible.

- Motion-spr build67897 and focused4/4 pass. startup-motion-spr clears prediction path, reaches movie worker callback254bd8 via254b58,time171404733us. Verified original254134..419c registration and live slots; added four default workers plus previously verified2429c8. Emission151672 words911 unresolved; movie-worker build4650 active. Fresh haunting-movie-worker-replay.py records unique post-input frames in external movie-worker-frames directory. OpenGL still shows warning; no visible playback yet.

- Non-intra build13680 passed. startup-nonintra clears residual decode and stops2a2b90 starting toSPR source-chain105. Original tags1990500..530 are three REF and finalREFE,48qwords each. Implemented bounded toSPR chain with real packet copies/SADR wrap/completion/IRQ and explicit unsupported tags; DMA tests pass. Verified16 original motion callbacks from3ed6d0..710, registered atcontext+5c8/+5d8 and consumed2a0f20/34. Added all associated packed operations from EE manual; synthetic translation tests pass. Generation151561 words911 unresolved. Combined motion-spr diagnostic build active; next fresh replay. No visible movie; viewer38888 retained.

- CSC build86554 and focused4/4 pass. startup-csc completes896macroblock conversion and advances to non-intra BDEC20010000 at2a1fe4,time152880599us; warning frame still displayed. Non-intra implementation now handles CBP/first coefficient/signed quantization/residuals; all63CBP native tests pass. Probe32cases shows24/12288 one-value IDCT discrepancies explicitly documented in ORACLE.md. Added manual-derived PADDH from adjacent prediction-copy routine. Native translation tests pass; emission/rebuild next. No oracle active; viewer38888 retained. User requested faster progress: batch residual and pixel-add changes in one connected build.

- Movie-callbacks build80716 passed; startup-movie-callbacks reaches CSC70000380 at2a7110 with real RAW8 input. Synthetic CSC memory probe completed1024pixels/five agreeing views; measured rounding discrepancy documented in ORACLE.md. Added bounded RGB32 conversion and pipeline tests; native runtime tests pass. CSC diagnostic build active. Next fresh CSC replay; no movie frames verified yet. Oracle40060 closed.

- Movie-pack build79437 passed; focused4/4 and32 Python tests pass. startup-movie-pack clears pixel copy and reaches original callback24fc70 through2a5c90. Verified original24f9a0..fa00 registration of24fac0/24fbc8/24fc70/24fc98 plus all four live12-byte records atbd6bcc; bytes match ELF. Added static targets;150701 words914 unresolved. Movie-callbacks diagnostic build80716 active, next fresh replay. No movie frame verified.

- startup-bdec clears intra decoding and48-qword scratchpad output, then stops at original2a299c ADDI. Added checked ADDI, PMAXH/PMINH and PPACB; synthetic native translation tests pass. Verified executed mask-word00ff00ff via independent memory probe (docs/ORACLE.md), added exact DSRA32 decoding with neighboring reserved cases rejected. Generation150613 words915 unresolved. Connected movie-pack build active; next fresh replay. No movie frame visible yet. Oracle9176 closed.

- BDEC diagnostic build passed (haunting-build-bdec-diagnostic.log). ALL_BUILD96409 failed only relinking the live/locked hg_opengl_host; retained viewer remains available. Runtime/translation/reboot checks pass. DMA test stack-overflowC00000FD fixed by heap-allocating its existing large State instances; rebuilt DMA test passes unchanged assertions. Fresh haunting-bdec-replay.py running, startup-bdec evidence prefix. No movie frame verified yet.

- Resumed after accidental interruption. Synthetic BDEC v2/v3 completed;32 cases/12288 pixels agree with independent mathematical IDCT using observed intra no-mismatch/0..255 clipping profile. Provenance/limitations in docs/ORACLE.md. Oracle35604 closed; no oracle active. Native block tests pass; full BDEC build/replay next. No visible movie yet.

- User reaffirmed: solely a recompilation, not an emulator (2026-09-13). Recorded in AGENTS.md. Own ELF scan/decoder verifies dispatches; current static generation150562 words916 unresolved. No runtime interpreter/JIT or emulator execution backend. PCSX2 stays optional memory oracle only.

- Output DMA build74980 and focused4/4 passed; replay96534 reaches BDEC2c010000 at2a1fe4 with DMA3 armed80001800/QWC48. Added independently derived MPEG2 intra/frame block decoder in runtime/ipu.cpp, separate hg_ipu library; synthetic runtime tests pass both111-entry coefficient tables, DC precision0..2, escapes, start codes and output. Native connected build/replay NOT run for BDEC yet. IDCT mathematical rounding not oracle-verified. Synthetic BDEC oracle CPU-FIFO v1 stalled at stage10; PID16968 closed. External generator haunting-ipu-bdec-probe.py now emits DMA-based v2 (hash857e6ad676a57ab019a545535ad3b745778fa1716a517c740caa5abe5ff766b7), ready to launch in ipu-bdec-oracle directory. Next: launch hidden v2, read memory, compare synthetic outputs; then build/replay. No movie frame verified.

- VDEC build19296 and focused4/4 checks passed; replay15283 clears address/type VDEC and reaches2a392c arming output DMA3 (MADR80001800/QWC48/CHCR100). Implemented bounded normal output DMA, eight-qword output FIFO, RAM/scratchpad destination validation, real completion and empty-pressure behavior. DMA tests pass. Full ipu-output build74980 active; next focused checks and haunting-ipu-output-replay.py. Image decode remains explicitly unsupported.

- Picture-switch build90438 and focused checks passed. Replay17045 reaches original2a2174 issuing VDEC30000000 (macroblock address increment), beyond movie headers. Implemented all manual-defined VDEC tables (EE manual pp202-204), error result/ECD and pending input/TOP, with focused runtime tests passing all prefixes and128 bit offsets. Full VDEC build active; next focused checks and haunting-vdec-replay.py. Still no movie frame; original warning remains.

- Movie-extensions build34036 and focused translation/reboot checks passed. Replay45656 clears2a42a8 and reaches bounded picture switch2a5288 ->2a52a4. Original index<5 and table45bb60..74 independently verified against live RAM; configured all targets. Emission150562 words916 unresolved; no remaining reachable unresolved JR in2a0000..2b0000. Picture-switch build active, then fresh replay. Still warning frame, no movie playback.

- startup-fdec clears FDEC, consumes more movie data and reaches2a42a8 via2a47f8 (time145422997us). Original11-entry extension table45b9f8..ba24 matches live RAM and copied stack; bounded index>10 clamped to0. Added all verified destinations, emission146347 words911 unresolved. Build movie-extensions active; next fresh automated replay. Warning frame remains; no movie decoding/playback verified.

- FDEC Release build10777 passed (haunting-build-fdec.log); all21 CTests passed. Fresh automated replay67239/PID8172 active, startup-fdec evidence prefix. Shared stream, pending input and table-consumption tests pass; no movie frame verified yet. OpenGL PID38888 alive.

- **ACTIVE continuation (2026-09-13).** User resumed until movie playback is visible. Next major milestone: advancing original movie frames in the connected OpenGL preview, verified by fresh captures and regression checks. Immediate blocker is FDEC at2a26b8; implementing manual-derived shared bit stream and pending input. Previous pause superseded.

- **PAUSED at user request, 2026-09-13.** No build, connected diagnostic or
  PCSX2 process remains running (checked after closing oracle31460). OpenGL
  preview38888 remains available, holding the warning frame. No movie playback
  or playability verified. Wait for a continuation instruction before tests.
  Latest full Release build60127 (haunting-build-ipu-input.log) passed, followed
  by focused DMA/runtime/translation/game_reboot checks4/4. Automated replay89056
  (startup-ipu-input) clears channel4 DMA and stops at original2a26b8 writing
  IPU FDEC(0), command40000000: decode command unsupported. Capture time
  145407939us; IPU has8 FIFO qwords, dma4 CHCR30000105/MADR13456512/QWC120/
  TADR13980688. Evidence startup-ipu-input.log and .ram sidecars under
  %TEMP%/haunting-toc-probe. Current generation145943 words912 unresolved.
  Synthetic FDEC probes v1/v2 completed; exact observations and limitations
  recorded in docs/ORACLE.md. FDEC implementation has NOT been started.
  Next: implement manual-derived FDEC bit reading and internal-buffer/FIFO
  accounting in ipu.hpp, with pending empty/cross-qword input, tests and capture
  fields. Resolve or explicitly reject the observed forward-with-empty-input
  discrepancy rather than copying oracle behavior. SETIQ/SETVQ must consume the
  same stream correctly after bit reads. Then build, focused checks and fresh
  automated replay toward advancing original movie frames in OpenGL.
  Reuse %TEMP%/haunting-ipu-input-replay.py with a fresh evidence prefix.
  The TOC input remains a quarantined synthetic probe.

- Movie-timing build37144 passed; replay24513 (startup-movie-timing) clears
  255208 and stops24ed44 starting IPU input DMA CHCR30000105, time145407653us.
  Original24f230/26c constructs REF/REFE tags through24ed68. Implemented
  independent channel4 normal/source-chain transport with eight-qword input
  FIFO pressure, saved MADR/QWC continuation and real completion status.
  New DMA tests passed; full build haunting-build-ipu-input.log active.
  Diagnostic JSON now captures EE DMA channels and retained IPU input.
  Next focused DMA/runtime/translation/reboot tests, then
  haunting-ipu-input-replay.py (startup-ipu-input). IPU decode/output remain
  unsupported; no movie playback verified.

- Video-header build69679 passed; replay85131 (startup-video-header) clears
  22cff0 and stops255208 via2551ec, RA2551f4, a0=7530. Original
  459cb8..9d00 conversion table matches RAM; seven nonzero destinations
  verified and rooted together. Generation145943 words912 unresolved;
  no new JR/unsupported instruction in inspected22a000..260000 range.
  Build active, haunting-build-movie-timing.log. Next
  haunting-movie-timing-replay.py, startup-movie-timing evidence prefix.
  No movie playback verified.

- Getter build12992 passed; replay53755 (startup-stream-getters) advances
  to144840077us, then stops22cff0 through JR22cfe4, RA22bddc. Original
  22cfc8 bounds selector-1<10; table457490..4b8 matches original/live RAM.
  Added all10 destinations; generation145747 words913 unresolved. No other
  currently reachable unresolved JR or unsupported instruction in22a000..260000.
  Build active, haunting-build-video-header.log; next
  haunting-video-header-replay.py (startup-video-header). Live snapshot
  getters-live.png still shows warning screen, no decoded movie verified.

- Stream-switches build4622 passed; replay72892 clears23a5a0 and stops
  metadata getter258080 via24698c, RA246994. Original246590..6954
  constructs25 literal getter addresses to adapters246980/2469b8; verified
  and added as a family. New getter switch258ec8 is bounded input-1<8;
  original459fc0..fe0 matches live RAM and all8 targets are rooted.
  Generation142820 words914 unresolved. Build12992 active,
  haunting-build-stream-getters.log. Next haunting-stream-getters-replay.py,
  startup-stream-getters prefix. Movie playback remains unverified.

- Stream-property build60394 passed; replay54598 clears23a6e0 and stops
  23a5a0 through bounded switch23a57c. Original index<9 and table457e70..94
  verified against live RAM. Audited currently reachable unresolved JR sites
  in230000..260000: second bounded switch2387f4 (index<6,457840..58) also
  verified; added both. Generation142224 words916 unresolved. Build4622
  active (haunting-build-stream-switches.log). Next automated
  haunting-stream-switches-replay.py, startup-stream-switches evidence.
  No movie frame verified; viewer remains available.

- Payload provenance verified while build60394 runs: captured packet9b2380,
  size800, exactly matches DATA.CVM /CAPCOM.SFD;1 offset800 (extent650491).
  SHA256 abf5d17a1562fed9c492ce148c7bc879bbc64bc4f8daad950a8926994f75716b.
  External stream-packet-origin.json records this comparison. This proves
  original movie data delivery to callback23a6e0, not decoded frame playback.

- Parser-dispatch build16149 passed; replay62259 (startup-parser-dispatch)
  clears248138 and stops23a6e0 via246194, RA24619c, object3e893c and
  data9b2380. Original23a694..b8 registers target as property4b, and
  24616c/253d38 retrieves it; added independently verified root.
  Generation142215 words918 unresolved. Build60394 active, log
  haunting-build-stream-property.log. Next haunting-stream-property-replay.py
  writes startup-stream-property evidence. No movie playback verified.

- DIV1 full build2672 and focused tools/runtime/translation passed3/3.
  Replay23468 (startup-div1) clears1e3774 and stops248138 via247a14,
  RA247a1c. Original4599fc and live RAM agree. Adjacent original pointers
  4599f0/f4/f8->247ad8/247c48/247ea8 also match and have verified executable
  bodies; added four additive roots as a callback family. Generation141995
  words917 unresolved. Build16149 active, haunting-build-parser-dispatch.log.
  Next haunting-parser-dispatch-replay.py, startup-parser-dispatch prefix.
  Preview PID38888 remains; no movie playback verified.

- Stream-copy build91627 passed; replay9754 (startup-stream-copy) clears2419d0
  and stops1e3774 on DIV1 word7066001a, RA1e3694. Manual pp138/140 proves
  second-bank signed/unsigned division. Added DIV1/DIVU1 decoding/emission,
  bank selection in checked divide_word, and synthetic decode/AOT regressions.
  Generation139844 words911 unresolved. Full build2672 active, log
  haunting-build-div1.log. Next focused tools/runtime/translation tests and
  automated haunting-div1-replay.py (startup-div1 evidence). No movie verified.

- Unaligned-store fix verified in connected replay27984 (startup-unaligned-fixed).
  All21 Windows Release CTests passed after full build49559. Stream1 now has
  capacity5b800 and callback3cffa8, with pointer94fd40/chunk800 unchanged.
  Next stop2419d0 via2418d4, RA2418dc, service100b360+3c. Original
  241b1c..28/241ca4..ac install that callback; added verified root.
  Generation139799 words912 unresolved. Build haunting-build-stream-copy.log
  active; next automated startup-stream-copy replay. Movie still unverified.

- **ACTIVE continuation (2026-09-13).** User resumed work. Major milestone: original movie playback visibly advancing in the connected OpenGL preview, supported by fresh frame captures/runtime evidence and focused regression checks. Immediate investigation: trace stream1 capacity from original construction descriptors; do not change max_bps or bypass the error.
  Found reversed memory-lane masks in EE SDL/SDR and SWL/SWR helpers. Manual
  pp99/101/117/119 confirms store lane directions; new byte-wise tests covering
  all offsets fail before the fix. Corrected both store masks; full Release
  build session49559 is active (haunting-build-unaligned-fixed.log). Next run
  CTest, then external haunting-unaligned-replay.py; preview PID38888 verified.
  Replay watches243640/243660/2443a0/24a220, fresh startup-unaligned-fixed prefix.
  Stream1 descriptor at10091cc is four-byte-misaligned; neighbors are aligned.
  This explains zeroed high words, but connected causal verification is pending.

- **PAUSED at user request (2026-09-13).** No build, game diagnostic or PCSX2
  oracle is running. Visible OpenGL viewer PID38888 may remain open, holding
  the last frame. Latest completed build40020; latest completed replay24649
  (startup-parser.log/.ram) cleared231840 and stopped uncompiled error callback
  23b058 via246d4c, RA246d54, error codeff000f1c. No movie playback verified.
  Original error string4580e8 says read buffer is small / increase max_bps.
  Do NOT patch that parameter or suppress the error. Trace its cause first.
  Original24a220..a280 raises this when capacity minus occupied bytes is less
  than object+28 (800). Live main object1007e40 has selected stream index1 at
  +1fc0. Ring records at object+1308, stride0x74: record0 capacity224800,
  record1 at10091bc capacity0 (10091d8), pointer94fd40, chunk800; record2
  capacity5dcc, pointer9abd40. This is the next measurable blocker.
  Original233bc8..bd8 callback231840 registration verified; it is translated.
  Original23b008..b014 registers error callback23b058 (not added yet).
  Original capacity getter244388 reads object+1324+index*0x74; occupied getter
  244ba0 calls243e90/243ea8. Next action: trace ring1 construction/capacity
  and original creation parameters using targeted PC watches or memory oracle;
  inspect2438e4/243cb4 ring setup and original callers. Full original ISO exists
  at Haunting Ground (USA)/Haunting Ground (USA).iso if oracle launch helps.
  No .p2s savestate found under emu. Preserve independence and explicit faults.
  Current generation139709 words908 unresolved; SPR DMA/FPU changes and
  verified callback families are built. Tests: DMA passes after heap allocation
  of extra synthetic State; FPU/translation/reboot passed. Host16MB stack
  reserve is verified. Next replay should keep preview and automatic input.
  Handoff requested and prepared; remain paused until the user resumes work.

- Stack-corrected replay74427 advanced selector2 and stopped231840 via231818,
  RA231820. Original233bc8..bd8 installs this callback at object+d4; live
  1983f44 agrees, adjacent fields are zero. Added root; emission139709 words908
  unresolved. No unresolved JR switch in231840..233c40. Build40020 active,
  haunting-build-parser.log. Next automated startup-parser replay. Evidence
  startup-registry-slot2-stack.log/.ram. Viewer PID38888; no movie verified.

- Registry-slot2 build78554 passed but fresh executable exited C00000FD just
  after checkpoint0. PE reserve was1MB; unwind metadata reports frames928616,
  378152 and319640 bytes among largest, exhausting native stack as AOT grows.
  Added MSVC16MB host-stack reserve for both diagnostics. CMake regeneration
  required a second MSBuild invocation to actually relink new flags; PE header
  now verifies0x1000000. One-slice run no longer stack-overflows (its final
  diagnostic printing hits existing early kernel-map fault; not a runtime pass).
  Visible viewer had exited; reopened with explicit desktop launch PID38888.
  Active fresh replay uses %TEMP%/haunting-registry-slot2-stack.py and
  startup-registry-slot2-stack.log/.ram. Next inspect first actual selector2
  outcome; no movie playback verified. Earlier registry-slot2 log is stack-failed.

- Object-family build66880 passed; replay62899 cleared2b69b0 and reached
  registry selector2 callback246fa8 at2563fc, RA256404. Correct record stride
  is0x44 bytes (68 decimal), nine records at1009d68. Seven populated table+8
  words match original ELF/native RAM:246fa8,247208,249828,2417c8,257518,
  243240,257210. Added all seven roots, emission137960 words906 unresolved.
  Build78554 active, haunting-build-registry-slot2.log. Next automated
  startup-registry-slot2 replay. Evidence startup-object-family.log/.ram;
  movie playback still unverified. Earlier shorthand stride44 means hex0x44.

- Stream-record build39482 passed; replay30362 cleared241580 and stopped
  object callback2b69b0. Original2b6c98..cd4 copies descriptor4126b8 into
  object94fb40+4, matching live dump. Traced subsequent original descriptor
  assignments:4126d8->2b68b0,4126e8->2b6710,4126f8 virtual slot1c. Live
  vtable46ecc0 slots0c..24 all match ELF, including slot1c->2b6510. Added
  verified callback transitions and vtable family before next replay. Initial
  transition build40187 passed; final family build now active with log
  haunting-build-object-family.log. Evidence startup-stream-record.log/.ram.
  Next automated startup-object-family replay. Still no visible movie evidence.

- Preview-retry replay49146 reached241580 via243148, RA243150; callback240a58
  is cleared. Live record100b414 is {3ceb60,241540,241560,241580,2415a0,2415c0}.
  Independently decoded242690..2714 constructs/stores all five methods; shared
  non-rendering lead corroborates addresses only. Added five roots as a group.
  Evidence startup-service-callbacks-retry.log/.ram. Emission124464 words823
  unresolved; build39482 active, haunting-build-stream-record.log. Next automated
  startup-stream-record replay; movie playback remains unverified.

- Service-callback build59545 passed. Replay42272 ended before menu input on
  Windows preview-file sharing violation during remove(haunting-live.ppm);
  callback outcome untested. Added bounded100x1ms publication retries preserving
  completed pending frame; persistent I/O errors still throw. Diagnostic-only
  build passed, haunting-build-preview-retry.log. Active fresh replay uses
  %TEMP%/haunting-service-callbacks-retry.py and startup-service-callbacks-retry
  .log/.ram evidence prefix. User reiterated HG leads allowed but renderer must
  be independent; existing AGENTS constraints remain unchanged.

- SPR callback build73582 passed; replay87357 cleared1cb140 and stopped
  service-list callback240a58 at1e5c10, RA1e5c18. Original23ac74..90 registers
  2409d8/240a58 through240198/240148; live480f18/480f40 agrees. Other four
  populated list callbacks and2409d8 already translated. Added explicit roots
  for this verified pair; emission124296 words823 unresolved. Build59545 active,
  haunting-build-service-callbacks.log. Next automated startup-service-callbacks
  replay. Evidence startup-spr-callback.log/.ram. No movie verified yet.

- SPR build91690 passed. First DMA test hit Windows stack exhaustion from an
  extra State local; moving that synthetic State to heap fixed the test, which
  now passes unchanged assertions. FPU/translation/reboot also passed. Replay
  59189 cleared normal SPR start and stopped callback1cb140, RA1e5b28. Original
  1cbe44..54 registers callback index6/arg0; live480fd0 agrees. Added root;
  emission124270 words823 unresolved. Build73582 active, haunting-build-spr-
  callback.log. Next automated startup-spr-callback replay. Current evidence
  startup-spr.log/.ram. No movie verified; visible viewer preserved.

- Vtable build61085 passed; replay81524 cleared23b618 and stopped at normal
  fromSPR DMA STR write10d8f4, s16=1000d000, RA23c178. Original23c144 selects
  channel8, destination197aa80, QWC400. Evidence startup-movie-vtable.log/.ram.
  Implemented documented normal SPR burst channels8/9 in connected runner with
  full-span validation,14-bit SADR wrap, address/count updates and D_STAT.
  Added tests for both directions, wrap, hold, completion and invalid spans/modes.
  Build91690 active, haunting-build-spr.log. Next focused DMA tests and original
  startup-spr replay; no movie verified yet. Source derivation in SOURCES.md.

- Member build80047 passed; replay68107 cleared2b6bb0 and reached vtable
  method23b618 via2b6bf4, RA2b6bfc. Live object3e893c vptr3e8758; all fourteen
  method slots3e8764..3e8798 match original ELF/native RAM. Added fourteen roots
  as one verified family; emission124204 words823 unresolved. No unresolved
  JR switch in newly covered239000..241000 range. Evidence startup-member.log/
  .ram sidecars. Next build haunting-build-movie-vtable.log then automated
  startup-movie-vtable replay. No visible movie verified yet.

- Service-switch build22962 passed; replay14355 cleared2509e8 and stopped
  2b6bb0 via member thunk100b6c, RA11fb04. Live object94fb40 descriptor+4 is
  {0,-1,2b6bb0}, matching original file-backed descriptors including4126a8..b0.
  Shared non-rendering lead confirms function boundary; body/dispatch verified
  from original ELF/native RAM. Added root, emission123280 words813 unresolved.
  Build80047 active, haunting-build-member.log. Next automated startup-member
  replay. Visually checked startup-service-switches.png: original warning screen,
  not movie. No diagnostic/oracle running during this build; viewer stays open.

- Retry5104 cleared registered worker2429c8, next stop2509e8 via bounded
  switch2509e0. Original index<7 and table459c70..459c8c match live RAM/ELF.
  Audited other reachable unresolved JR sites in240000..258000: only2408dc,
  bounded index<5 and table459660..459674, also fully matching. Added both.
  After emission122722 words810 unresolved, no unresolved JR remains in this
  bounded range. Evidence startup-worker-retry.log/.ram. Build22962 active,
  haunting-build-service-switches.log; next automated startup-service-switches
  replay. Last screen still warning, movie playback remains unverified.

- Worker build98152 passed. Replay20950 stopped before menu confirmation with
  host input-file parse error, so it did not test2429c8. Suspected Windows open
  race during atomic replacement: reader previously conflated open failure with
  malformed contents. Added bounded4x1ms open retries and size check on the opened
  handle; persistent open failure and malformed contents still fail explicitly.
  Diagnostic-only build passed; invalid-command one-slice check still rejects.
  Active retry5104/PID43780 uses startup-worker-retry.log/.ram and
  %TEMP%/haunting-worker-retry.py. Next inspect callback outcome; no movie yet.

- Switch build58961 passed; replay30316 cleared1d5f98 and stopped2429c8,
  RA254b60. Original242988..29a4 constructs2429c8 and registers index2 via
  original255158 (store object+d28+index*4). Live object1007e40 slot1008b70
  matches. Added the independently verified worker root; shared non-rendering
  lead corroborated address but no shared patches/behavior were used. Evidence
  startup-cri-switch.log/.ram. Emission120516 words800 unresolved. Build98152
  active, haunting-build-worker.log. Next automated startup-worker replay.

- CRI buffer build78435 and IOP bundle/reboot tests passed2/2. Replay88210
  cleared158e4 and stopped EE1d5f98 via switch1d5f5c. Original1d5f40 bounds
  index<5; table451d10..451d24 matches live EE RAM and original ELF (targets
  1d5f68,1d5f90,1d5f98). Added bounded indirect_targets entry. Evidence
  startup-cri-buffer.log/.ram sidecars; shared analysis confirms containing
  function1d5f10, but switch derived independently. Emission119939 words800
  unresolved; build58961 active, haunting-build-cri-switch.log. Next reuse
  automated replay with startup-cri-switch prefix. Viewer remains visible.

- Rounded-SQRT build18698 and focused3 tests passed. Replay28714 cleared both
  SQRT sites and stopped at IOP158e4, RA12424, object21500. The original1240c/
  12414/1241c loads object vtable1c460 slot24. Nine non-null methods at slots
  0c..2c match independently relocated original CRI_ADXI .data exactly; added
  all nine as roots in config/iop_modules.toml. Shared non-rendering lists had
  no applicable IOP-address lead. Native evidence startup-sqrt-rounded.log/.ram.
  No movie yet; last scanout warning screen. IOP bundle regenerated28 modules/
  93389 reachable words. Build78435 active, haunting-build-cri-buffer.log.
  Next: IOP regression checks and automated replay with fresh cri-buffer prefix.

- Automated replay83420 completed with input Left99010000/Cross100060000/
  none101060000, clearing SQRT1c6c64 and stopping at SQRT1c6d20. No movie yet.
  startup-sqrt-supported.log/.ram sidecars retain evidence. Focused timer/FPU/
  translation tests passed3/3 before this run. An independent2048-input synthetic
  oracle batch now validates positive-normal nearest rounding against the current
  PCSX2 reference profile; physical-console parity remains unverified (ORACLE.md).
  Integer rounding implementation and boundary tests added. Build session18698
  is active, %TEMP%/haunting-build-sqrt-batch.log at completion. Next: focused
  tests then reuse %TEMP%/haunting-sqrt-replay.py with a fresh evidence prefix.

- 2026-09-13: live input successfully selected YES and advanced to the original
  warning screen, then stopped at EE1c6c64 SQRT(2) at343583286us. Evidence:
  %TEMP%/haunting-toc-probe/startup-live-input.log and .ram sidecars.
  Independent synthetic SQRT ELF oracle (fpu-sqrt-oracle/verified.json; provenance
  in ORACLE.md) measured 2->3fb504f3 and 0.5->3f3504f3. Implemented only the
  supported normal power-of-two case and measured flag clearing; other inexact
  significands still stop explicitly. Release build44639 and fpu_tests passed.
  Next measurable milestone remains connected movie playback. Active replay
  uses %TEMP%/haunting-sqrt-replay.py, startup-sqrt-supported.log/.ram, automatic
  preview-driven YES/Cross, and the existing visible haunting-live.ppm viewer.
  Capture the next explicit blocker if movie playback is not reached.

- Clock-enabled replay26866 reached500M without an explicit hardware/AOT fault,
  but final scanout still shows the no-card prompt with NO selected. T0/T1 mode82
  are counting and T2 mode382 runs; T1 count43177 in final metadata. Do not infer
  completed movie startup from the larger slice count. Input-repeat/missed-input
  remains a hypothesis. DECI2 output has only known heap/cache messages.
- Added --input-file (none/left/cross, poll10000 slices, reject malformed input
  and combination with scripted pulses) so input can be sent to an active run.
  Build passed. Active session43666 is fresh500M startup-live-input.log/.ram with
  watches25c04c and2c8c70, input %TEMP%/haunting-input.txt currently none, and the
  existing visible preview. Next: wait for prompt, atomically pulse Left briefly,
  inspect YES selection, then Cross; retain observed input slice numbers.

- Timer-handler build27301 passed. Focused rerun47234 passed all6 entries in
  54.91s, including the three previously failing fileio/cdvd/reboot tests plus
  timer/translation/prefix. The earlier other18 tests passed with clock attached.
  Fresh visible replay26866 is active: startup-ee-timer-handler.log/.ram sidecars,
  500M budget,30M checkpoints, Left180M/Cross181M, haunting-live.ppm viewer4016.
  30M checkpoint EE1ee728 proves original timer IRQ no longer stops early startup.
  Next measurable gate: pass original T1_COUNT read25c04c and capture later
  connected startup/movie state. Rendering/viewer stays observational only.

- Complete global registry build42207 and focused tests passed. Replay90578
  cleared230fd8 and reached EE25c04c at197454482us, reading T1_COUNT10000800
  after original mode82 setup. Timer model was not connected to runner clock.
  Final five initializer callbacks were independently verified from2309a0..b44
  and live globals3e82b0..c0; library68586 passed,119722 words798 unresolved.
- Added EeTimerClock using manualp36 BUSCLK147.456MHz, fractional microsecond
  conversion and INTC9..12 flag-edge delivery. Timer unit tests pass. Clock
  attachment exposes original registered INTC11 handler277110 at227303us;
  alarm list3ec068 is null in this capture. Full suite58913:18 pass,3 fail only
  at this untranslated handler (fileio/cdvd/reboot). Do not disable interrupts
  to restore tests. Added verified277110 root; emission119891 words799 residuals.
  Build27301 is active. Next run full regression gate and visible startup replay.
  Latest captures: startup-global-callbacks.ram and startup-ee-clock.ram/log.
  Visible viewer PID4016 remains active at haunting-live.ppm.

- Preview-enabled replay66544 is terminal, next untranslated230fd8 via22b2fc,
  RA22b304, live callback global3e828c. Full CTest84858 passed21/21 in65.46s.
  Added13 missing targets from the same live registered global block3e8274..ac,
  each independently decoded with matching original indirect-call wrappers.
  Emission117464 words,788 unresolved items. Build42207 in progress; next
  run must retain --preview-file %TEMP%/haunting-live.ppm for visible viewer4016.
  Viewer holds the last captured warning screen between runs; no movie yet.

- User confirmed the live OpenGL window is visible, with their screenshot of the
  no-memory-card prompt. Tool-isolated launches were invisible even with Normal;
  explicit approved desktop launch succeeded, viewer PID4016. Prior viewers40340
  and13668 were stopped. Preserve4016 across diagnostic replays using the same
  external haunting-live.ppm path. Original game execution is still diagnostic66544.

- User requested visibility during tests. Added diagnostic --preview-file (external
  P6, every1M slices and final dump, committed VRAM only) and OpenGL host
  --watch-display (250ms polling, bounded complete-frame reads, retained last
  frame, aspect-preserving viewport). Viewer checks a presented pixel against
  the source framebuffer. Synthetic and original warning-frame3-frame checks
  passed on NVIDIA OpenGL3.3. Build passed after NOMINMAX/clean-environment fixes.
  Live viewer session33382 watches %TEMP%/haunting-live.ppm. Keep it open for
  the user while tracing; Escape closes only the viewer.
- Registry replay32540 is terminal, next callback230e38 via22b280/RA22b288;
  live global3e8284 contains230e38 (ELF initial value0, dynamically registered).
  Own decoder verifies callback body; shared non-rendering analysis range agrees.
  Added that observed root:116947 reachable words,779 unresolved items.
  Runtime replay66544 uses startup-visible-230e38.log/.ram sidecars and live
  preview. Full CTest84858 is active. Movie playback remains unverified.

- Callback-only build37426 and full21-test suite49018 passed (78.01s).
  Replay45697 confirmed0x246fd0 executes, stopping next at0x248c08 with the
  same caller0x256404. Captures: startup-callback-246fd0.ram sidecars/log.
- Batched the remaining six selected registry callbacks, independently verified
  from the nine-record loop0x256398, selector3, live records0x1009d68+i*0x44,
  and original ELF slots4599c4/459a0c/459754/459eec/4597ac/459eb4.
  Targets248c08/24d708/2425f8/2576c8/243390/257370; two records are null.
  Emission116793 words,779 unresolved items. Library99702 and ALL_BUILD passed.
  Fresh startup-registry-slot3 replay and focused tools/translation/prefix tests
  are active. No movie playback is verified yet; warning-screen milestone stands.

- Fresh replay session50494 is terminal: Left180M/Cross181M clears the five
  vtable methods and reaches untranslated EE0x246fd0, RA0x256404. Captured
  display was visually inspected: the original violence-and-gore warning is
  readable over the castle background. This is a later startup screen, not movie
  playback. GS evidence: 67810 rasterized,65517 retired,640x448 capture.
- External startup-vtable-continue.ram sidecars preserve EE/IOP RAM, GS VRAM,
  registers, draw records and display.ppm/.png; adjacent log records the fault.
  Original0x2563ec/0x2563fc loads/calls slot0x45998c, which contains0x246fd0
  in both the original ELF and live RAM (r3). Own decoder verifies jr ra with
  daddu v0,zero,zero delay slot. Non-rendering manual-list lead agrees on the
  eight-byte range. Added only this observed callback; emission113828 words,
  774 unresolved items. Release ALL_BUILD session37426 is in progress.
  Next: complete build/regression gate and replay toward connected movie playback.

- 2026-09-13 continuation: active major milestone is a later original startup
  screen leading to connected movie decoding/presentation, verified by runtime
  captures and relevant regressions. Fresh 500M Left180M/Cross181M replay uses
  external startup-vtable-continue.log and startup-vtable-continue.ram sidecars.
  The synthetic TOC remains a diagnostic dependency, not physical-drive evidence.
  Prior 90M checkpoint does not prove the later vtable frontier cleared.


- 2026-09-13: Added live `0x46c740`-vtable continuation roots to
  `config/haunting_ground_us.toml`:
  `0x002265d0`, `0x00226620`, `0x00226640`, `0x00226680`, `0x002266d0`
  (names `observed_vtable_*`). Re-ran `python tools/hg.py emit` from this branch,
  which now reports `113826 reachable words, 774 unresolved items`.
- Rebuilt via `Start-Process` + Visual Studio MSBuild clean environment (`/t:Build`,
  Release/x64, `/m:1`,`/nr:false`,`/v:m`) using `build/ALL_BUILD.vcxproj`; build
  log at `%TEMP%/haunting-build.log` and success `0` with refreshed
  `build/Release/hg_system_diagnostic.exe`.
- Restarted the long Left/Cross replay with `--slices 500000000 --checkpoint-every 30000000`
  and synthetic TOC/I/O capture prefix in `%TEMP%/haunting-toc-probe/startup-vtable-2266.ram`,
  including `--press-left-at 180000000 --press-cross-at 181000000 --toc-record synthetic-index-pattern.bin`.
  Checkpoints reached 30M, 60M, 90M slices (`GS_draws` rising to 3241 / `EE=0x1cafb4`),
  then the run was user-interrupted before the next checkpoint/stop.
- Last known frontier before this interruption:
  progress past the prior `0x00226620` vtable-blocker is implied, while earlier
  stop was `EE 0x226620` with setter-like side effects. Next action is to replay or
  resume from this frontier and capture the next explicit stop.

- This continuation scanned all eight project Markdown files (including an
  ignored/hidden-folder inventory) and confirmed the supplied PCSX2 installation
  at emu/PS2 emu/pcsx2-qt.exe. AGENTS.md now preserves the user's authorization
  to access these local files for memory-oracle checks and to scan all project
  Markdown at continuation. No emulator implementation was read or copied.
- Release ALL_BUILD session58140 completed with exit0. Full Release CTest
  session94315 completed with exit0: all21 entries passed in59.23s, including
  the new indexed sampling/extraction/sprite regressions. The nonfatal
  missing-pwsh build warning persists.
- Original Left180m/Cross181m replay is running in session41936 with a240m-slice
  budget and30m checkpoints. External output is haunting-toc-probe/
  startup-sampling-supported.log and startup-sampling-supported.ram with its
  sidecars. Inspect its settled result and continue through the next observed
  blocker toward actual connected movie playback. It still uses the quarantined
  synthetic TOC dependency probe, not a verified physical-drive record.

- 2026-09-12T17:18:40Z: the independent sampling probe now passes all56
  expected colors and all200 undrawn sentinel pixels in three stable stage4
  readbacks. It uses one-pixel constant-UV sprites at the original56 sample
  coordinates. The previous extra-FINISH point probe and a final-two-points
  permutation still missed spatial pixel55; the cause of that point-specific
  observation remains unresolved. Expectations were not weakened.
- External sampling-verified.json records ELF SHA256
  a56cdc58624680f6d6e52df37c9caca5744a4299b7df20c23506cf26868df699,
  owned PID27892 and three agreeing views. The exact owned probe was stopped
  through explicit escalation after observation. No oracle process remains
  active from this segment. Earlier point variants and observations are preserved.
- Removed the provisional odd indexed TEX0 width rejection in gs.hpp, using
  the already independently implemented whole-page address stride. Added
  fixed physical-marker sampling/extraction/sprite regressions for both indexed
  formats at TBW1,2,3,10,11,30,31, including both contexts, aliases, sentinels,
  and continued zero-width rejection. The build and tests are now verified as
  recorded above; the original startup replay is the next runtime gate.
  Movie playback is still the goal.

- Active user goal: continue until the independent recompilation produces a
  screen showing a movie playing from the game. Verify actual connected-runtime
  playback; an extracted movie, emulator game screen, build, or startup prompt
  does not satisfy this goal.
- The former approval-capacity blocker is superseded: the normal explicit
  escalation successfully launched the isolated synthetic sampling ELF at
  2026-09-12T07:06:46Z, owned PID33332. The read-only observer ran at07:06:54Z
  and found three stable stage4 framebuffer views. All agree on55/56 samples;
  pixel55 is still sentinel44556677 instead of8054f10e. All24 even-width
  control samples and all200 undrawn sentinels are correct. Result is FAIL,
  not accepted sampling evidence. The exact owned process was stopped through
  explicit escalation; no process/session remained active at that checkpoint.
- Preserve sampling-first-incomplete-final-pixel.json and
  indexed-sampling-before-draw-fence.elf in the external indexed-stride-oracle
  directory. Investigate draw/readback ordering with a separate drawing FINISH
  fence and ordered DMA submissions; do not weaken the56-pixel acceptance test.
  Runtime odd-TEX0 guard and original EE0x10d744 stop remain unchanged.
- Next measurable verification: complete56-sample observation, sampling
  regressions, then original Left180m/Cross181m startup replay toward movie
  decoding/presentation. Further runtime blockers must be traced as observed.

### Earlier checkpoint history (superseded where noted above)

- Continuation checked at **2026-09-12T06:59:49Z**: no launch was retried,
  no runtime code changed, and no new startup milestone was verified. Read the
  prepared sampling generator and observer without executing their live paths.
  The prior segment reports another approval-capacity rejection and a final
  06:57:23Z zero-owned-process/no-new-log check; that rejection is still the
  unresolved external prerequisite, not a new runtime failure.
- Fresh `Get-Process` inspection at06:59:49Z returned no `pcsx2-qt`,
  `hg_system_diagnostic`, or `MSBuild` processes. The expected external
  `pcsx2-sampling-validated.log` was absent at06:59:31Z. An earlier CIM process
  query returned Access denied; its empty result is not process-count evidence.
  No tool command session remains active from this continuation. Do not retry
  the rejected launch until its approval prerequisite changes, and do not rerun
  unchanged builds/tests solely to report activity. The immediate executable
  action after an approved launch remains the prepared read-only observer;
  require stage4 and verified pixels before changing odd TEX0 sampling.
  The active major milestone remains a later original startup screen reached
  by Left180m/Cross181m after the absent-memory-card prompt.

- Latest continuation: the isolated sampling launch was explicitly reviewed
  again after checking the ELF/executable hashes and temporary data folders.
  Automatic approval review still rejects it because its usage limit is reached;
  the returned message says try again September15,2026 at8:02PM (timezone not
  stated in the error). No alternate launch was attempted after this rejection.
  No emulator or game diagnostic ran in this continuation; TEX0 support and the
  last actual runtime fault are unchanged. No new startup screen is verified.
- Completed unaffected probe validation: external sampling_control_replay.cpp
  replays the exact generated packets through our independent GS implementation,
  executing only six supported even-width controls. Its native build/run pass:
  all24 sampled colors match independent expected indices; all232 undrawn pixels
  retain their sentinel. The32 odd-width draws are deliberately not executed.
  This validates probe preparation against our model, not odd-width behavior or
  an emulator/physical-console observation. Result: sampling-control-replay.json.
- Hardened external observe_indexed_sampling.py: include allocation bases when
  guest RAM is split into regions; check the saved PID's executable path and
  recorded executable/ELF hashes before memory reads; check protection at each
  fixed synthetic read. Thirteen offline self-checks pass, rejecting absent,
  incomplete, short, wrong, unstable and inconsistent completed readbacks.
  The live Windows observation path remains unexecuted. Result:
  indexed-stride-oracle/sampling-observer-selftest.json. The generated ELF and
  sampling-packets.bin are unchanged. The last full21-entry suite pass below
  belongs to the previous continuation; no runtime code changed in this one.

- Current continuation is blocked on the independent sampling probe launch.
  Explicit launch escalation was rejected because Codex automatic approval
  review reported its usage limit. Do not bypass that rejection through another
  launch route. No build, diagnostic or owned oracle process remains active.
  The initial sampling launch22588 stopped at a settings-path error without
  running its ELF and was stopped; prior oracle23948 is also no longer running.
- Full Release ALL_BUILD session82640 is terminal, exit0. Full CTest session77902
  is terminal, exit0: all21 entries passed in59.10s. GIF regressions now cover
  DBW1,3,11,31 for both indexed formats, checking all64 words per256-byte tile
  immediately and after aliasing writes, adjacent sentinels, cross-page reads,
  and invalid-width rejection. The nonfatal missing-pwsh build warning persists.
- The external1616-qword `indexed-sampling.elf` and its generator, manifest and
  read-only observer are prepared under indexed-stride-oracle/. They test odd
  TBW1,3,11,31 against even2,10,30 controls, using physical CT32 block uploads,
  a known palette and56 sampled pixels downloaded into EE RAM. No successful
  observation exists. Observer syntax is checked and incomplete stages,
  mismatches or changed undrawn pixels return failure. Runtime sampling remains
  unchanged; see ORACLE.md and SOURCES.md for evidence versus pending work.
- Next action once the launch can be approved: use the isolated portable copy
  with `-portable -batch -nogui` and the external indexed-sampling.elf, then run
  observe_indexed_sampling.py after saving the returned process ID to
  sampling-pid.txt. Require stage4, repeated stable readback, correct controls
  and expected odd-width pixels. Only then extend TEX0 support, replace the
  provisional rejection tests, rebuild and replay Left180m/Cross181m. Preserve
  the active milestone: a verified later original startup screen after accepting
  the absent-memory-card prompt. Do not count transfer tests as that milestone.

- Previous connected session34382 is terminal, exit2. The original720x540 PSMT8
  upload now passes; the next queued textured draw62385 stops at EE0x10d744
  with `invalid TEX0 indexed texture buffer width`. TEX0=0x2007e006a932e200,
  TBP0x2200,TBW11,PSMT8,TW10,TH10,CBP0x3f00,CLD1. External capture:
  startup-indexed-stride.ram and adjacent state/VRAM/display files. No command
  is active. Its saved640x448 scanout was checked without flushing pending draws:
  all286720 pixels are black. No later original startup screen is verified.
- psmt8_word/psmt4_word and transfer validation now accept64-pixel width units;
  independent synthetic memory observations confirm whole-page row strides for
  DBW11 and DBW1/3/31. GIF tests and diagnostic rebuilt successfully in25351.
  The full native/Python suite has now passed as recorded above.
  Odd TEX0 sampling deliberately retains its guard pending a separate measured
  sampling probe. Immediate action: render synthetic indexed page markers into
  a known framebuffer, observe that data, then add supported sampling/regressions
  and replay Left180m/Cross181m toward a later original startup screen.
- DBW1/3/31 evidence in indexed-stride-oracle/extremes-verified.json is now
  documented in ORACLE.md/SOURCES.md and covered by passing regressions.
  Older active-session and awaiting-first-oracle statements below are superseded.

- Current continuation: session55935 is terminal (exit2), reproducing the same
  PSMT8 transfer at EE0x10d744 after Left180m/Cross181m. No command is active.
  External startup-transfer-provenance.ram.ee-ram.bin contains32MiB EE RAM;
  its JSON includes all EE GPRs. Original submitted packet at0x4f20d0 contains
  BITBLTBUF0x130b220000000000 at0x4f20f0, TRXREG720x540 at0x4f2110,
  and IMAGE REF payload0x8f0080/QWC0x5eec at0x4f2140. This confirms the odd
  DBW11 is present in the original packet, not a live-register decoding error.
- Immediate work: establish odd indexed transfer page stride with an independently
  assembled synthetic upload and a read-only memory-oracle observation. The GS
  manual specifies64-pixel BITBLTBUF width units and128-pixel indexed pages, but
  does not explicitly settle odd-width page-row behavior. Do not weaken guards
  until that missing observation is obtained. The milestone remains a verified
  later original startup screen after accepting the absent-memory-card prompt.
- Everything below is historical where it mentions active sessions96254/1821.
  Those sessions and55935 are terminal. No new visible milestone is claimed.

- Continuation after handoff: the original sound RPC completion is compiled and
  verified by terminal run 18016. Left at 180m and Cross at 181m pass that callback;
  the next stop is EE0x10d744, invalid GS transfer buffer width, at slice183495727.
  Capture: startup-prompt-callback.ram, 640x448,62384 draws/62376 rasterized.
  This does not yet establish a newly visible startup stage.
- Added non-mutating live GS registers and CLUT-load counter to external GS JSON,
  and live BITBLTBUF/TRXPOS/TRXREG/TRXDIR to the diagnostic log. Prior draw
  environments were stale relative to the failing transfer and cannot identify it.
- CMake configuration recovered with clean-environment Start-Process and working
  Python3.11.9. GLFW source was unavailable and its Git fetch failed with Windows
  Schannel SEC_E_NO_CREDENTIALS, so the existing HG_OPENGL_HOST option is OFF
  in the ignored diagnostic build cache. No project CMake source was changed.
  Diagnostic rebuild passed; CTest tools now passes (79 Python tests,0.48s).
  Earlier full native run passed all20 native entries; its Python launcher failure
  is now resolved. Current active diagnostic session96254 replays Left/Cross,
  240m budget,30m checkpoints, external startup-transfer-width.ram and log.
- Immediate next action: poll96254, decode captured live transfer registers,
  derive a bounded correction from original GS documentation, add regression
  coverage, and continue until the original prompt advances to a verified stage.
  Everything below this new continuation block is earlier checkpoint history;
  no old build/test session listed below remains active.

- Active milestone: advance the original missing-memory-card prompt by selecting
  YES with Left and confirming with Cross, then trace the next original startup
  stage. A build or an exhausted slice budget is not completion.
- The earlier startup-csm1-csa run ended at its 180,000,000-slice budget, without
  the old explicit CSA stop. Exit 1 was the diagnostic budget result. Its capture
  contains 61,181 draws/61,018 rasterized, 640x448. The external display image was
  converted to PNG; a monochrome copy was visually inspected and shows the
  no-memory-card prompt with NO selected. Cross alone selects NO.
- AGENTS.md now has operational continuation/milestone rules. Current code work
  replaces the incorrect source-index CSA shortcut with the GS manual's shared
  CLUT temporary buffer and TEX0/TEX2 load control. A Left diagnostic pulse is
  added alongside Cross. Focused synthetic tests and the diagnostic build have
  passed; connected runtime verification is active below.
- Focused `gif_tests.exe` passes after the CLUT implementation and new synthetic
  regressions. Test fixtures now request CLD=1 when loading palettes instead of
  relying on the former incorrect live-VRAM lookup. Direct MSBuild Release/x64,
  clean environment, one worker and node reuse disabled succeeded. The existing
  nonfatal missing-pwsh post-build warning remains.
- Diagnostic rebuild **69359** completed, exit 0. Connected native run **21268**
  is terminal, exit 1: the new CLUT cache passes the earlier palette boundary and
  reaches the Left pulse at 180m slices, then original SIFRPC completion reaches
  untranslated EE0x21f280 (return0x26fad4). It stops before Cross. The 640x448
  capture retains 61,347 draws/61,209 rasterized; a monochrome prompt crop was
  viewed and still shows NO selected. This is not yet a prompt-advance milestone.
  Artifacts: `%TEMP%/haunting-toc-probe/startup-clut-left-cross.ram` and adjacent
  log/state/GS/display files. Full-color fidelity has not been visually verified.
- Original-ELF inspection verifies the three instructions at 0x21f280 and its
  original callback construction at 0x21fd80/0x21fda0, passed at 0x21fda4.
  The supplied non-rendering manual list agrees. Added this exact AOT root and
  regenerated 108,100 reachable EE words/750 unresolved items. Build **1821**
  is active; next poll it and rerun the same Left/Cross sequence into a new
  `startup-prompt-callback` external capture. Synthetic TOC and scripted input
  remain diagnostic profiles, not faithful-disc/playability evidence.
- Focused GIF tests and all 79 Python tests pass in this continuation. Other
  native test binaries have not yet been rebuilt/retested for the CLUT header.
- Previous sessions 43642, 69359 and 21268 are terminal. Older active-session
  references below are historical. Only build 1821 is active.

### Historical checkpoints (superseded where noted above)

- Visible milestone: startup-card-info82677 renders the original memory-
  card Checking/do-not-turn-off screen,640x448,8061draws/7898rasterized;
  external PNG viewed. No playability/save support claim. Runs beyond90m
  slices, then EE383550 via384bdc/100b40, object94f898. Original44af10
  descriptor loaded3842e0/e4, installed38430c..18. Added230-word state
  callback. EE108097words/750issues, IOP92651/28. Build7494 active; next
  180m startup-card-prompt capture. Prior64930/82677 terminal.


- startup-card-alarm26158 passes IOP alarm callback, reaches EE111450
  RPC completion via26fad4. Original1115a8/1115bc passes it in a7 to
  SIFRPC1115cc. Added22-word root; EE107867words/747issues,IOP92651/28.
  Build64930 active; next180m startup-card-info capture. Prior44986/
  26158 terminal. Full20native/Python79 unchanged since last pass.


- Full native20PASS52.95s and Python79PASS after SIO2 extraction/PIO.
  startup-card-pio48108 passes absent-card PIO and reaches MCMAN alarm
  callback3ef84 viaTHREADMANa3b84. OriginalMCMANa13c/a140 constructs
  offset9f84, a144 registers via thbase35. Added only10wordroot;
  IOP92651/28modules, EE107845/747issues. Build44986 active; next180m
  startup-card-alarm capture. Prior67911/5325/48108 terminal.


- startup-card-probes33049 passes DMA probes and reaches PIO81/52,
  SEND3=c0342, three bytes on absent card port2. Added missing-card PIO
  transport. SIO2 extraction completed into runtime/iop_sio2.cpp and new
  hg_iop_peripherals library. Full build67911 active; next full20 CTests
  then180m startup-card-pio capture. Prior40567/33049 terminal.
  Padding fidelity limitation remains in SOURCES; no playable claim.


- startup-card-dma97784 completed first absent-card81/11 probe, then
  hit bounded-profile stop on five-byte81/F3 (SEND3=140572). Generalized
  absent-card selection81 to single36-word blocks on card ports2/3;
  no command interpretation for absent devices. Tail FF stillunverified.
  Build40567 done; iop_tests PASS.180m startup-card-probes session33049
  active. Prepared SIO2 method extraction into runtime/iop_sio2.cpp and
  hg_iop_peripherals library to avoid recompiling92k IOP words for each
  peripheral fix. Do NOT rebuild/relink until33049 finishes.
  Prior80882/97784 terminal. Connected displaystillblack198draws/22rasterized.


- Module-spacing run43043 passes PC8/callback corruption; new stop is
  SIO2 DMA setup7adf8 for absent-card probe81/11, port2, four serialbytes,
  one36-word block each direction. Full native20PASS52.29s/Python79PASS
  before next DMA changes. Verified original BIOS DMACMAN SetSliceDMA
  writes request mode without start; fixed premature-start bug. Added
  bounded missing-card DMA endpoint and buffer/ordering tests. Unused
  140-byte RX padding uses FF profile, NOT hardware/oracle verified.
  Build80882 done; iop_tests+dmac_tests PASS. Connected180m
  startup-card-dma running session97784.
  Prior6517/30114/87735 terminal. No other connected run active.


- startup-sio-callback session27656 finished at IOP PC8. RAM watch proves
  LOADCORE0x96840 wrote module ID8 to0x7afdc (SNDDRV header+12), overwriting
  SIO2MAN BSS. Setter0x7ab18 never ran. Fixed placement SIO2MAN79fd0;
  planner validates full rounded allocations with0x30 headers, runtime
  rejects overlapping static plans. Python79PASS; regenerated IOP92641
  words/28modules. Build6517 active for system diagnostic+iop_tests.
  Next: focused iop_tests, then180m startup-module-spacing capture.
  Previous PollSema run27751 also terminal. EE107845words/747issues.


- startup-service-member reaches PollSema empty-count gap at26c1f4,
  semaphore11, caller1113cc. Original1113d8 tests negative before
  optional WaitSema. PS2tek documents -1 on failure; implemented empty
  return-1 without count/scheduler change. kernel_tests PASS; build done.
  Connected startup-poll-sema180m running session27751. Prior84138/
 10641 terminal. EE107845words/747issues,IOP92641/28modules unchanged.


- startup-byte-switch reaches226220 via22653c/100b40, object4f1850
  descriptor+14. Original1bf350/35c loads3b3028;1bf378..80 installs it.
  Added root226220. EE107845words/747issues. Build10641 active; next
  180m startup-service-member capture. Prior76180/88769 terminal.
  Captured committed display640x448 is BLACK (PNG viewed);198draws,
  22rasterized. This is not a new visible screen/playability milestone.


- startup-object-ae90 reaches interior switch case38228c from original
 3821f0. Verified byte selector<30 at3821f8 and table463bd0, jr382218.
 Added bounded indirect table463bd0..463c48. EE107477words/747issues.
 Build88769 active; next180m startup-byte-switch capture. Prior10667/
 13858 terminal. Runtime and tests unchanged since prior full20pass.


- Native full suite20/20 PASS (50.54sec), Python78PASS after cross/VF0.
  startup-vu-cross reaches1bf340 via2bf414 slot0c, object4f1850.
  Original1bf568..84 installs primary46ae90 + secondary46aeb4 tables;
  added verified8 methods including secondary thunk1bf870. EE107424words/
  748issues. Build13858 active; next180m startup-object-ae90 capture.
  Prior23272/5052/66828 terminal. No active connected run currently.


- Added four startup-selected methods from original vtable469a60:
  slots24/a4/a0/20 ->1225c0,121970,121960,1225d0. Verified lifecycle
  installs table1218b8..c4, original2cf8f0/904/918/97c selections.
  startup-object-9a60 passes methods and reaches VOPMULA10db28.
  Added bounded VOPMULA/VOPMSUB accumulator cross product plus VADD,
  own VU manual116-117/241/305-306. Exceptional OPMSUB accumulator
  product under/overflow or exponent255 remains explicit fault. Unit-axis
  cross/alias/w-preservation test added. Python78pass. Fullbuild66828
  active; next native suite and180m startup-vu-cross capture.
  EE107304words/748issues. Prior17965/67981 terminal.


- startup-default-member reaches virtual method2cf8c0 through1bbed0
  slot0c, object94fa10. Original table46f350 has16cc40/2cf8c0; original
  2d1050..5c installs that table. Added both verified method roots.
  EE107278words/750issues. Build62203 active; next180m
  startup-object-f350 capture. Prior36521/63922 terminal.


- startup-member-states reaches default member callback383440 through
  original384ba0/100b40, object888570+4 descriptor. Verified constructor
  384538/384544 reads44aef0;38456c..78 stores descriptor at object+4.
  Added exact original two-instruction callback as AOT root. No bypass.
  EE106996words/739issues. Build63922 active; next180m
  startup-default-member capture. Prior39732/29878 terminal.


- startup-vu-broadcast passes scalar-vector multiply, reaches member callback
  37fef0 from3806bc helper100b40, object888440. Original380510 copies
  seven descriptors44ae70..44aed8:37fef0,380050,37fe50,37fc60,37fac0,
  37f980,37f7d0. Added verified roots together. EE106994words/739issues.
  Build29878 active; next180m startup-member-states capture.
  Prior52359/70399 terminal. Latest VU/VF0 focused tests pass; last full
  native suite20pass predates broadcast/VF0 changes (not yet rebuilt all).


- startup-vu-normalize completes normalization, reaches scalar-vector
  VMULx at10e64c (not a matrix operation despite earlier commentary).
  Added VMULbc from VU manual299, synthetic alias/scalar test. Corrected
  VF0 in older QMFC2/QMTC2/LQC2/SQC2/VMOVE/VABS/VMR32 helpers:
  constant(0,0,0,1), ignored writes. Tool78pass; runtime target built.
  Build70399 active. Next runtime test + startup-vu-broadcast180m.
  EE102074words/682issues. Prior32137/99092 terminal.


- Full native CTest20/20 PASS (51.05sec) after FBRST/fixture fixes.
  startup-vu-reset reaches VPU-STAT29 read at10ca84. Added checked Ready
  status0; pendingQ status polling explicitly faults until timed completion.
  Added remainder of original normalization sequence: exact VNOP,VDIV,
  VSUB,VMULq. Integrated synthetic vector(4,0,0,1)->(1,0,0,0), zero-divide
  and0/0 flags tests added. Python78pass; runtime target built; build99092
  active. EE102072words/683issues. Next runtime test then180m
  startup-vu-normalize capture. Prior5055/27582/17388 terminal.


- startup-cache-tags passes cache scan then CFC2 control28 at10bfd8,
  original OR0200/CTC2 triggers VU1 reset. Added proper CFC2/CTC2 decode,
  checked FBRST reset/enable state from VU manual21/203. Other control
  registers still explicit unsupported. EE102065words/684issues.
  All78Python tests PASS. Full native suite earlier19/20: translation
  fixture still encoded old wrong QMFC2/QMTC2/SQC2. Fixed generator.
  Fullbuild17388 active; next full native CTest + startup-vu-reset180m.
  Prior93039/77599/11285 terminal. No playable build claim.


- Corrected decoder exposes CACHE DXLTG at original26ca04 in startup-vu-q
  before former graphics boundary. Added exact CACHE0x10 tag lookup
  (coherent RAM => clean invalid TagLO0), CACHE0x14 indexwriteback ordering,
  verified EE instruction manual303/306, original tag/PFN range loop.
  Tool31pass. Build17877 active (runtime_tests built); next runtime test,
  connected startup-cache-tags180m then continue VU boundaries. No active
  run; prior74266 terminal. EE102012words/687issues,IOP92641/28modules.


- VMUL/VADDbc native test passes; startup-vu-multiply reaches VSQRT10db78
  with VF4=(0,0,0,1),VF5=0. Added bounded VSQRT pendingQ/WAITQ/VADDq;
  synthetic zero/negative/nonsquare/synchronization tests pass. Tool31pass.
  Manual audit corrected existing QMFC2/QMTC2 rs1/5 (not2/6), interlockbit,
  SQC2 op62 (not47 CACHE). New negatives tested. EE101944words/687issues.
  Build7451 active. Next180m startup-vu-q; earlier53686 terminal.
  No full native suite after these newest VU changes yet. VF0 handling in
  older transfer helpers remains an audit item (arithmetic reads correct
  constant0,0,0,1). Q profile explicitly requires WAITQ before consuming.


- startup-ds2-driver reaches EE10db6c unsupported VMUL, vector-length
  sequence: VMUL.xyz then VADDy.x,VADDz.x then VSQRT/WAITQ etc.
  Independently read local original VU manual pp26-29,39-42,244,296.
  Added bounded VMUL/VADDbc static decode/emission, lane masks, MAC/status
  flags using shared integer24-bit truncation arithmetic (same documented
  format). VF0 reads constant0,0,0,1 for these operations. Synthetic
  vector-length/mask/alias/overflow/flags tests added. Build18205 active,
  EE102063words/682unresolved. Next runtime/tools tests and native run;
  VSQRT remains explicit next boundary, no Q pipeline implemented yet.


- All20 native CTests PASS (50.25sec) after controller/idle changes.
  startup-dbc-switch reaches DS2O2dbb0 via DBCMAN27818 selector3.
  Verified original2da58..2daa4 constructs six callbacks at2e1a0,
  registers descriptor2dab8. Added all6 roots; IOP92641words/28modules.
  Build89317 active; next180m startup-ds2-driver capture. Prior7328 and
  tests53171/build98820 terminal. Display640x448 enabled with2 queued
  draws,0 rasterized at stop; no new visible-screen/playability claim.


- startup-idle-id passes idle GetThreadId and stops at DBCMAN27cf8.
  This is an interior switch case, NOT a standalone callback. Original
  RPC function27c88 bounds normalized selector<99 at27ca0, then jr27cc0
  through table28560. Added complete independently verified table99,
  generated28modules/92344words. Build50987 active. Next180m capture.
  Prior92485 terminal; kernel test passed. No display enabled at last stop.


- Controller/DBCMAN fixes advance beyond former controller prompt into
  new EE interrupt path: GetThreadId26d078 with current0, INTC active,
  all7 threads waiting/suspended. Diagnostic startup-ee-idle proves state.
  Kernel now supports idle ID0 during initialized interrupt context, per
  SDK declaration; preserves fault for accidental ordinary no-thread call.
  kernel_tests PASS. Connected startup-idle-id180m running session92485.
  All earlier runs/builds terminal. Next inspect final display/new boundary,
  then exercise --press-cross-at if a user-input prompt remains.


- startup-pad-sensors passes pressure negotiation and reaches original DBCMAN
  callback262f4 (IOP static translation gap). Verified callback formed at
  original263c8/263cc and registered by263d0. Added root+2f4; regenerated
  all28 IOP modules,91684 reachable words. Build98860 active.
  Added --press-cross-at slice for a250000us port0 Cross pulse through
  digital bit14 and pressure index6, preserving original controller path.
  Next connected180m run (first without input) after build completes.
  controller_tests passes all12 sensor replies/invalid-index rejection.


- Controller command4F reply now ends5A per independent hardware wire notes.
  Focused controller test passes. startup-pad-reply reaches a NEW explicit
  command40 pressure-sensor initialization gap, input014000000200000000.
  Added bounded twelve-sensor initialization profile and exact reply tests.
  Build83614 currently active (controller target built; IOP recompiling).
  Next run controller_tests then connected startup-pad-sensors180m capture.
  Prior run63531 terminal. Controller recognition still unverified.


- MAJOR VERIFIED checkpoint: startup-gs-addressing180m produces clean
  readable game screen640x448: No DUALSHOCK2 analog controller inserted
  into port1; insert controller and press X. PNG viewed externally at
  startup-gs-addressing.ram.display.ppm.png. All20 native CTests pass
  after full rebuild. Run61020/build58743/tests19453 complete.
  Next controller detection: native sio2_connected defaults{true,false},
  so investigate PADMAN protocol/state; do not merely force connection.
  Rendering supports this screen; gameplay/audio/hostinput remain unproven.

- Corrected all GS *_word strides to2048/64/16 words, CT32/T8 block
  x-bit2 shift tobit4, CT16/T4 blocky-bit2 shift tobit4, indexed column
  row-bit1 XOR rather than addition. Verified original GS manual rendered
  pp164/165/167/169; no external rendering material. gif_tests PASS,
  including all8 formats fullpage uniqueness/coverage/nextpage separation.
  Stale byte-stride expected constants corrected. Build52802 live for
  systemdiagnostic. Next run startup-gs-addressing180m with external GS
  capture; broaden native tests after rebuild as address changes are shared.

- Native temp inspect_texture.exe extracted captured TEX0 source textures:
  upper indexed texture mostlyblack; lower text visibly fragmented;
  intermediate512x512 also corrupted. Temp sources/regs/PNG outside repo.
  FOUND concrete addressing bug: all *_word helpers use page8192/block256/
  column64 as WORD strides. Original GS manual p162 specifies BYTE units:
  correct word strides2048/64/16. Also CT16/T4 block_y&4 times16 exceeds
  block31; needs independently verify figure (likely times4). T8/T4 pixel
  formula adds overlapping bit3 terms; audit against original columnfigure.
  No mapping edits yet. Next correct audited mappings and replace stale
  self-consistent tests with page coverage/boundary tests from specification.

- startup-gs-state run33016 complete; external full GS JSON+VRAM valid.
  Draws11653:6989 textured sprites(PRIM116hex),4664 clear sprites.
  Last cycle draws512x256 indexed textures to intermediate FRAME80110,
  then samples TEX0=664022200 into display FRAME10a0000,640x448.
  Source TEX0s2007e08625423d00 and2007e10625423c00. Next inspect
  source texture/CLUT captures through own native decoder to isolate
  upload/layout versus sprite sampling. No active build/run.

- Added external GS VRAM little-endian words plus full draw environment/
  vertex JSON beside --dump-iop. Builds successfully. startup-gs-state180m
  now running to diagnose corrupt display; same defaultbudget1/syntheticTOC.

- VERIFIED first committed display capture from startup-display180m:
  external startup-display.ram.display.ppm (+PNG viewed),640x448,574colors,
  draws11653/rasterized11652. Output is CORRUPTED: broad white band with
  fragmented/repeated black glyphs and colored noise nearbottom; not a
  usable game frame/playability. Run16163 complete, no live processes.
  Next export GS VRAM/registers and recent draw environments/vertices to
  diagnose texture/layout independently. Draw environment snapshots exist
  and are restored in rasterize_draw; do not assume missing state snapshot.

- Added external .display.ppm beside --dump-iop capture, using existing
  display_image on committed VRAM (no guest state mutation or forcedflush).
  Logs dimensions and draw/rasterized counts, or explicit captureerror.
  Build passed; startup-display180m/checkpoints30m now running to verify
  rendered output. Prior78428 complete; no playability claim.

- VERIFIED startup-texel-sampling completed180m slices WITHOUT unsupported
  fault; exit1 is cooperative budget reached, EE1e53fc/currentthread5,
  IOP1f0000/reboot1. Display configured NTSC/interlace,PMODE66/SMODE2=1.
  Run78428 complete; no active build/run. Next inspect actual GS output
  (add external dump through existing display_image) and continuing startup
  state; no game frame/playability claim yet. Logs/capture external.
  Shared texel conversion regression and existing gif_tests pass.

- Texel refactor gif_tests PASS after fixing new fixture to ensure_vram.
  Build97460 complete. Instrumented startup-texel-sampling run78428 live,
  already100m slices;90m EE1e5f48,100m EE276290,VIFCHCR70000045.
  Former slow run69081 terminal (deliberately stopped after profiling).
  Test invocation39871 terminal; new gif_tests invocation passed in0.064s.

- Identified actual long-run bottleneck: sampled own PID14356/thread39448
  Windows context RIP7ff609130d28 (modulebase7ff609120000,RVA10d28);
  disassembled our executable to texture/CLUT sampling. Source showed
  point_sample_tex0 decoded ENTIRE texture per fragment (quadratic work).
  Stopped PID14356 deliberately after this finding, not on polling timeout.
  Refactored shared texel_from_tex0 conversion, extraction loops over it;
  point sampling now converts one texel and reads live VRAM (no stalecache).
  Added1024x1024 repeated-sample/live-write regression. Building97460;
  next startup-texel-sampling180m with --checkpoint-every10000000 separated.
  Diagnostic checkpoint source already compiled, now included in rebuild.

- Checkpoint option compiles successfully using MSBuild /t:ClCompile only;
  executable deliberately not relinked while run69081 remains active.
  Still no output/fault; PID14356 CPU exceeded831s. Next poll samehandle,
  then link/test checkpoints and inspect bounded diagnostic output.

- Run69081/PID14356 still live, CPU722s with stable61MB working set.
  No fault/output yet. Added optional --checkpoint-every slices to source
  for future runs (EE/IOP PC and VIF/GIF control), defaultoff. Not yet
  rebuilt/tested: preserve active executable until run terminal. Next use
  --checkpoint-every10000000 (separate argument) to locate long-running phase.

- Build79406 completed. startup-object-draw180m/defaultbudget1/syntheticTOC
  is live (session69081, PID14356), repeatedly confirmed running; no output yet.
  CPU time advanced275->427 seconds during continued polling; slice limit
  remains180m. No restart or duplicate run has been launched.
  Do not restart on poll timeout. Current EE102,060/682; IOP91,674.

- startup-eight-state reached EE1bfdb0 via1bb53c slot6c,object7fe400.
  Original1bf8a4 publishes46b050; retained26 entries46b058..46b0bc.
  EE102,060/682; IOP91,674. Build50262/run94148 complete. Rebuilding
  for startup-object-draw180m/defaultbudget1/syntheticTOC.

- startup-resource-table reached EE16c0b8. This is a switch label, not
  a function: original16c03c checks index<8 and16c05c dispatches table
  44f7d0..44f7f0. Added bounded indirect table. EE100,462/665.
  Build38397/run13434 complete; rebuilding for startup-eight-state180m.

- startup-service-table reached EE1f4420 via26bbe4 slot10,object1970690.
  Retained five original table46b1d0 methods46b1d8..46b1e8. Discovery
  hit100k cap; raised configured limit200k, regenerated100,144/660.
  Build77073/run68190 complete. Rebuilding; next startup-resource-table180m.

- startup-member-sequence reached EE26bb00 via3807e8 slot8. Original
  table46d7d0 contains nine contiguous methods46d7d8..46d7f8; retained
  missing roots. EE99,932/660 (near100k discovery limit). Build53826
  and run25589 complete; rebuilding for startup-service-table180m.

- startup-derived-state reached EE380790 through member helper100b40.
  Original380920 loads descriptor44ae40 into object+4; adjacent direct
  descriptors44ae50/60 name380510/380410. Retained three targets.
  EE99,481/652; analysis budget100k near limit (raise if genuinely hit).
  Build18451/run7983 complete; rebuilding for startup-member-sequence180m.

- startup-state-dispatch reached EE380920 via member helper100b40,
  ra11fb04, same object888440. Original37f174 publishes table47a770;
  retained remaining methods37f140/380920/11f990. EE99,025/636.
  Build65865/run8268 complete; rebuilding for startup-derived-state180m.

- startup-object-flags completed at EE11f9c0 via2d1bc0 slot0c,object888440.
  Original shared state dispatcher appears in multiple ELF vtables; retained
  observed target without guessing the live derived table. EE98,807/636.
  Build54218/run99331 complete. Rebuilding; next startup-state-dispatch180m.

- startup-service-five completed at EE1be1f0 via2d440c slot0c,object4f1500.
  Original1be178 publishes46adb0; retained three methods46adb8..46adc0.
  EE98,720/636; IOP91,674. Build6189/run82881 complete. Rebuilding
  for next startup-object-flags180m/defaultbudget1/syntheticTOC.

- VERIFIED VIF1 TTE startup continues: startup-vif-tte reached EE1ca610
  via service dispatcher1e5c10. Original1ca7cc selects callback before
  registrar1ca7d8; CFG join loses path-specific constant. Added explicit
  original root; EE98,557/636, IOP91,674. dmac_tests passed, build48334
  and run36538 complete. Rebuilding; next startup-service-five180m.

- startup-vif-control confirms CHCR145 (TTE source-chain) at10d740.
  Added upper64 tag delivery with physical phase8, from EE manual45/74/86.
  Added integrated TTE DIRECT regression; build underway. Next run
  startup-vif-tte180m/defaultbudget1/syntheticTOC after tests. Mid-packet
  tag crossings explicitly unsupported pending phase tracking. Runs13536
  and88824/build40712 completed; fault diagnostic now includes v0.

- VERIFIED GIF status polling passes in startup-gif-status. Added completed
  synchronous status, VIF mask bit1 and GS BUSDIR bit12 from EE manual
  pp149/164; incomplete packet status explicitly faults. runtime_tests pass.
  Build9586/run19771 complete. Next EE10d740 VIF1 source-chain start rejects
  CHCR mode (original ORs105 into prior CHCR); watch run startup-vif-chain
  now live to establish exact CHCR before implementing applicable transport.

- VERIFIED VIF1 idle polling now passes in connected startup-vif-status.
  Run91673 completed at EE10cabc reading GIF_STAT10003020, masking0c00
  (next: derive GIF status from original/manual, never shared rendering data).
  VIF regression passes; build51797 complete; no live build/run processes.
  EE98,171/635; IOP91,674. No game frame or playability established.

- startup-object-init completed at EE10ca44 reading VIF1_STAT10003c00
  (mask1f000003 checks VPS/FQC). Added synchronous completed/empty status
  and MARK detection/clear from Sony EE manual pp143-144. Incomplete
  packets explicitly reject status reads until FIFO/pipeline is modeled.
  gif_tests pass; build51797 complete. startup-vif-status now running
  180m/defaultbudget1/syntheticTOC. Prior build67317 and run58176 complete.

- startup-allocator completed at EE1beef0 via2d1c48 slot10, object4f14c0.
  Original1be7d4 publishes vptr46adf0; retained five additional methods
  from46adf8..46ae0c (1bed00 already retained). EE98,171/635; IOP91,674.
  Rebuild underway; next startup-object-init180m/defaultbudget1/syntheticTOC.
  Build41010 and run77903 completed. No game frame or playability established.

- startup-main-object completed: EE168fc0 via2cfc3c slot10, object1961440.
  Independently verified original table46a1c0 and vptr publication168c44;
  retained four methods46a1c8..46a1d4. Non-rendering reference lead matched;
  no implementation copied. EE94,165/622; IOP91,674. Rebuilding for next
  default-budget startup allocator diagnostic. Build33549 and run73507 complete.

- startup-member-state completed at EE2cfbd0 via2d1c2c slot24,object487a00.
  Original20dbd8 publishes vptr46f360; eight non-null methods46f368..46f384
  retained together. EE93,808/622; IOP91,674. Rebuild33549 now live; next default
  startup-main-object180m/syntheticTOC. Run53633 complete.

- startup-object-state completed at EE2d1b20 through member-pointer helper
  100b40/100b6c,ra2d1f64,a0=487a00. Original static descriptors414370/414380
  contain{0,-1,2d1b20}/{0,-1,2d1aa0}; retained both helper targets alongside
  existing2d1e40. EE89,472/598; IOP91,674. Build41376 complete; current
  startup-member-state53633 live,180m/defaultbudget1/syntheticTOC. Run37184 complete.

- startup-snd-completion completed at EE1bb9d0, object4f1920 via2a7ae0
  slot28. Original table46ac50 stores it at46ac78; original1aae34 publishes
  the vptr. Retained36 non-null methods46ac58..46ace4 together, exclusively
  from original executable. EE89,219/590; IOP91,674. Build11240 completed;
  startup-object-state37184 live,180m defaultbudget1/syntheticTOC. Run35731 done.

- startup-snd-bank completed at EE21f290 viaSIFRPC26facc,ra26fad4,
  a0=21971640. Original21fac8/21faf4 selects paired callbacks21f2b0/21f290;
  they clear bits0/1 of the caller's flag. Retained both original roots.
  EE74,710 words/514 unresolved; IOP91,674. Build65987 live; next run
  startup-snd-completion180m defaultbudget1 and syntheticTOC. Run87350 done.

- VERIFIED CDVD continuation fixed at defaultbudget1: startup-cdvd-command-completion
  clears timeout and reaches SNDDRV RPC86018 (ra998bc,a0=120000,a1=8fc80,a2=20).
  Metadata drive read_pending0,sectors0,LBA2113120,error0; original streaming
  advanced beyond the failed64-sector read. All20 native CTest targets pass.
  Original85540 registers server77777778 with callback86018; retained SNDDRV
  offsetb018. IOP91,674 words; EE74,696/514. Rebuild9130 now live; next default
  budget1 startup-snd-bank180m. Prior44849 and96704 complete.

- Found concrete intermediate-CDVD IRQ bug. Each partial DMA descriptor
  published I_STAT1; original IRQ2 path b6404..b6458 calls b5b18, which
  signals request completion and disables IRQ35 atb5b54. This stranded the
  nextbuffer regardless ofworkerbudget (budget4 and32 both reachedBREAK).
  complete_cdvd_dma_record now signals DMA only whilecommand stillpending;
  finaldescriptor retains existing drivecompletion3. Added split-command
  regression with two independently armed descriptors; focusedIOP tests pass.
  Rebuild32149 live. Next defaultbudget1 startup-cdvd-command-completion180m.
- EE SignalSema overflow now returns documented-1 without changingcount;
  normal/interrupt regressions pass. No live diagnostic processes; previous
  budget32-sema50694 andbudget4 91303 complete. Defaultworkerbudget remains1.

- Budget32 experiment completed quickly at EE26c1d4 SignalSema overflow
  (semaphore4,ra10eeec), before proving the laterCDVD continuation. Preserve
  defaultbudget1 until timing/profile validated; don't claimCDVD fix.
  No matching non-rendering configuration lead for SignalSema overflow.
  Next establish its original error ABI or compare a smaller budget, keeping
  the independent event-grant fix. No live processes. External startup-iop-budget32
  log/RAM metadata retained. New option is diagnostic, not cycle-accurate timing.

- startup-event-grant completed at same BREAK. Event fix valid but does not
  resolve mode0 worker starvation. Added diagnostic --iop-worker-budget1..128
  (default1 preserved), using existing per-instruction scheduler burst. Metadata
  records budget. Connected runtime previously allows1IOP instruction/us while
  audio consumes48k samples/sec: investigate insufficient CPU throughput.
  Current experiment startup-iop-budget32 at180m slices, budget32, syntheticTOC.
  Build complete, focusedIOP tests pass. Previous37364 terminal.

- Corrected IOP event grants: original iSetEventFlag a11d8 copies result,
  a11f0 clears WEF_CLEAR before making waiter ready. Native set now captures
  output/clear atomically and records granted_event; resumed wait consumes
  the grant without retesting or clearing later signals. Regression verifies
  captured result, preserved second signal, and immediate waiter removal;
  focused IOP tests pass. Build23933 completed. startup-event-grant180m live.
- Diagnostic evidence before fix: CDVDFSVced5c validates sectors203d20..2f
  correctly. Event4 bits0x29; worker8 ready priority81 waitingbit0x20, worker19
  also ready priority39. So current CDVD wait is likely starvation, not bad
  headers; WEF_CLEAR bug is independently proven but this wait usesmode0.
  Worker19 original143f8 waits via1a734 then loops14128 work; inspect its
  semaphore credits/audio pacing if event-grant run reaches same wait.
  External metadata now includes threads/events. Runs77021/74065 complete.

- startup-cdvd-break-state (session86297) completed; metadata JSON parses.
  Important evidence: BREAK occurs with read command8 pending, LBA2112832
  (0x203d40),32 sectors remaining from64, DMA [835208,12,1073742336]
  (MADR0xcbe88,BCR0xc,CHCR0x40000200 inactive). No pendingCDVD IRQ/error;
  status2. Virtualtime97,332,756us. Two intermediate descriptor completions
  published I_STAT1 and originalb630c acknowledged them, but nextDMA was
  not armed. Investigate multi-descriptor read continuation before guessing
  BREAK readback or IRQ. Originalb6314 checksI_STATbit0,b632c checksNstatus
  bit0 then records1/-1; b6350 acknowledgesbit0. No live processes.

- startup-iop-context-return completed at original CDVDMANb81b0 writing1
  to BREAK1f402007; nextb81b4 reads it and discards result via nextload.
  PS2tek documents BREAK stopping current N-command but not readback/IRQ
  timing. Do not fabricate those semantics. Added CDVD command/DMA/status
  and64-entry MMIO trace to external fault metadata to diagnose the timeout.
  Build complete; startup-cdvd-break-state180m now running with synthetic
  TOC. Previous31102 terminal. Next inspect metadata cdvd object, derive
  needed cancellation/completion behavior from original code/specs/oracle.

- Build77829 completed successfully. Current diagnostic31102 confirmed live:
  startup-iop-context-return,180m slices, synthetic TOC, external matching
  log/RAM. Poll this handle; do not restart based on empty output. This run
  tests original timeout callback beyond corrected GetThreadId interrupt ABI.

- startup-cdvd-timeout completed at GetThreadId9fe94 in alarm IRQ.
  Original THREADMAN9fea8 calls QueryIntrContext,9feb0 branches to9fed4
  returning-100. Native adapter now returns that error in interrupt context;
  focused IOP tests pass. Rebuild session77829 live (IOP translation header
  dependency); poll before next180m startup-iop-context-return run.
  Previous58309 completed; no live diagnostic. EE74,696/514, IOP91,645.

- Build76749 completed. Current diagnostic session58309 is live:
  startup-cdvd-timeout,180m slices, synthetic TOC, matching external RAM/log.
  Poll it before any restart. CDVDFSVce920 is the original alarm callback:
  calls cdvdman50(-18) then cdvdman39 and returns whether result is zero.
  No timeout result fabricated. Native tests20/20 passed; latest changes
  after those tests only retain original callback root.

- startup-large-sif clears the larger DMA and reaches original CDVDFSV
  callbackce920,ra a3b84,a0 d3748. Originalcf150/cfba4 register it in a1.
  Retained module offset920; IOP91,645 words. All20 native CTest targets
  passed after larger-SIF change. Next rebuild/run180m startup-cdvd-timeout.
  Run63260 and test69299 are complete. EE74,696/514 unchanged.

- startup-secondary-object advances to a raw SIF descriptor size2800 at
  21f8e0/26c564, exceeding old4096-byte diagnostic cap. Replaced that cap
  with checked native staging60000..80000; aggregate bounds are checked
  before mutation. Kernel regression verifies complete larger transfer,
  snapshot integrity and single/combined overflow rejection; passes.
  Rebuild completed; startup-large-sif now running180m slices with synthetic
  TOC. Previous10376 completed. EE74,696/514, IOP91,624 unchanged.

- Longer120m run60039 completed at new EE target2107d0 via2cfa98,
  object887204. Original constructor20e038 publishes secondary vptr46bf2c;
  its slot4c at46bf78 matches target. Retained91 original non-null methods
  46bf34..46c09c, derived entirely from original table and constructor.
  EE74,696 words/514 unresolved; build52166 completed. Current live run
  session10376: startup-secondary-object,180m slices with synthetic TOC.
  Poll handle and external log/RAM metadata; no other active processes.

- startup-service-registrars completed60m without an unsupported stop;
  transport173,003 (formerly43,064), main priority correctly1, EE at original
  bounded delay1ee71c (loop exits after7552 iterations), IOP worker19 active.
  Original services are issuing repeated transfers. Longer120m run now live
  session60039, startup-service-registrars-120m.log/.ram in external probe dir.
  Poll it before starting another run; compare transport and wait context.
  No frame/playability evidence. Session63363 is complete.

- startup-service-callback reaches1ca680 at the same dispatcher. Added
  original registrar callback ABIs for1e5898(a1),1e59e0(a2),2400e8(a1),
  240198(a0). Evidence recovers1ca660/1ca680 and240990/2409d8 in pairs.
  EE72,690 words/496 unresolved, IOP91,624. Build14238 completed.
  startup-service-registrars is live as session63363,60m slices, synthetic TOC.
  Poll it and inspect external matching log/RAM metadata; keep advancing.

- startup-command-callbacks clears both IOP callbacks and reaches EE240990
  via1e5c10. Original23ac58..23ac70 registers that pointer with2400e8.
  Added the verified callback: EE71,806 words/491 unresolved; IOP91,624.
  Rebuild is in progress; next run startup-service-callback at60m slices.
  20 native CTest targets passed; tools test hit sandbox temporary-file denial,
  then all78 Python tests passed when rerun with approved temporary access.
  No live diagnostic runs; previous build93327 initially hit executable locked
  by CTest, then linked successfully after tests finished.

- Corrected ChangeThreadPriority to return previous priority. Original1cafa8
  saves it and1cb030 restores it; returning universal zero starved EE workers.
  Kernel regression covers0->1->5->1 and implicit current-thread ID. It passes.
  startup-priority-restore clears the former60m/120m identical wait and reaches
  IOP17af0 via10b94. Original17dfc/17e14 register17af0/17b9c through10b0c/10b18;
  retained both callbacks. IOP28 modules/91,624 words; rebuild session93327 live.
  Next run same60m synthetic TOC profile after polling build completion.
  Earlier120m run86361 completed with identical43,064 transport steps; no longer live.
  ORACLE.md interpretation corrected without changing recorded observation.

- startup-next-object reaches60m slices without unsupported stop. EE1693a0
  polls request state via1ca1f0 (byte at object+1), waiting for3/4. IOP worker19
  is runnable at16ecc with interrupts masked; transport steps43,064 show
  continued activity since earlier checkpoints. A120m-slice run is currently
  live as session86361 (startup-next-object-120m). Poll that handle; do not
  restart it based on empty output. This run distinguishes progress from wait.

- Original ctor1e30d0 confirmed publication of table3cff70. Added its nine
  non-null methods together; EE71,536 words/490 unresolved. Current build
  session30503. Next startup-next-object run with60m slices and synthetic TOC.

- VERIFIED CK01 entry: startup-object-table.ram.json watch0xe0030 records
  original MODLOAD call with a0=1,a1=0x17ef80,gp=0xe80b0,sp=0x17ef70,
  ra=0xaa324. The original buffered loader now reaches the compiled module;
  this is execution evidence, not merely preloaded code. No success fabricated.
  Current stop is EE1e3268 via jalr1d1f98,ra1d1fa0,a0=3cffa8,a1=0.
  Original method pointer at3cff94 belongs to table3cff70 slot24. Nine
  non-null methods span3cff7c..3cff9c, followed by zero/object3cffa8.
  NEXT: verify ctor publication, then retain that original table's methods
  together and run startup-next-object at60m slices. No active processes.
  Current EE71,100/483; IOP28 modules/91,509. All21 regressions passed
  after buffered lifecycle implementation; later additions are original roots.
  Not playable. Source-region/module lifecycle work is complete for this
  verified buffer profile; arbitrary dynamic modules/placement remain explicit gaps.

- startup-cri-paired advances to EE method169420 via jalr2cfa3c, object80adc0.
  Original table46a1e0 contains that slot30 at46a210; retained all14 non-null
  methods46a1e8..46a21c together (skip existing roots). Original1692a4 writes
  this vptr during object destruction. EE71,100 words/483 unresolved.
  System rebuild just launched; next run startup-object-table with60m slices
  and --watch-iop-pc0xe0030 to prove CK01 entry execution, not just preloading.
  IOP28 modules/91,509 words. All21 regressions passed before last root additions.

- All21 regression targets pass after buffered lifecycle changes, including the
  new compiled embedded-source fixture. startup-cri-transfer next hits paired
  CRI callback110e8 at jalr10c24. Original registrations11454/1146c prove
  both10e90/110e8. Added the second root; build session33506 running.
  Next startup-cri-paired run,60m slices/synthetic TOC. CDVDMAN:50 selector-9
  independently decodes to b6f80, loading original halfwordbfa44; existing
  29-entry table already covers it, so no invented module result is needed.

- startup-buffered-ck01 builds and starts, but newly reaches untranslated
  CRI_ADXI callback10e90 at jalr10b94 (ra10b9c), before display initialization.
  Own relocated original shows pointer construction at11448. Added+0xe90 root;
  system rebuild session50801. Next run startup-cri-transfer with60m slices.
  Added a third embedded synthetic bundle input to exercise native region loader
  registration and allocation; build/test that fixture after active system build.

- Buffered lifecycle implemented: source-region loader retains verified original
  bytes in a static plan, CK01 configured at0xe0030 (prefix0x30). Original
  MODLOAD entry0xa9440 now has a generated observation hook that compares
  the complete image, rejects unknown/duplicate loads or explicit placement,
  selects the plan, then executes the original entry instruction/body.
  Original allocation and relocation remain active; free resets the plan for
  reload. AOT hook/native lifecycle regression passes (tests/iop_tests).
  IOP bundle28 modules/91,442 words. Current system build session31569;
  next connected run startup-buffered-ck01 with60m slices/synthetic TOC.

- Embedded-region source support is implemented in iop_source.py using
  offset,size,container_sha256 plus the module hash. Source extents and whole
  container identity are checked; iop_build.emit_load uses the existing checked
  load_iop_region_file path. Bundle root remains the common directory of
  non-embedded inputs (MODULES), so embedded ELF paths may resolve through
  ../SLUS_210.75. All78 synthetic Python tests pass, including embedded source,
  changed-container and bounds rejection. No ck01 config added yet: runtime
  buffer lifecycle still needs explicit binding before loading it.
  Original MODLOAD exports:9=0xa9440 LoadModuleBufferAddress;
  10=0xa948c LoadModuleBuffer (confirmed API declarations, not SDK code).
  10 calls9 with a1=a2=0. At9, original stores source a0 at new stack+32
  and builds a load-job record (type2 atstack+16) for0xa9e64.
  Suggested next step: an AOT entry observation hook at9 that verifies the
  buffer against immutable source bytes and selects its static allocation plan,
  while still executing the original body. Do not match solely on memory size.
  Use a free static low-memory range below scratch1m, and let the existing
  high-water reservation protect it. Current native high-water affects SYSMEM
  reservation (system_diagnostic.cpp221), not scratch_low (fixed1m).

- CURRENT boundary: startup-service-table passes service methods and reaches
  embedded IOP ck01 loading. Native LOADCORE0x96cac rejects link range
  a0=0x100030,size0x70,ra0xab530 because the module is not in static plan.
  Captured relocated image header at0x100000, code0x100030, name0x1000a0.
  Original source is embedded in SLUS_210.75 file offset0x34aa20. Its full
  ELF file extent is0x359 (include relocation/symbol/string sections after
  section headers); sha2564ca27d8c457807cd6bcf5a8b4fb79f51b4a1ba00b8c10a0dc94936be9339ec51.
  Captured source buffer0x13dc00 matches all0x359 original bytes exactly.
  One import cdvdman:50 at module+0x54; text size0x70, memory0x90, entry0.
  Code calls that import with a0=-9,a1=stack+16; returns1 on success else5.
  NEXT: add identity-checked embedded-region source support to iop_source.py
  and emit_load (currently ROMDIR or standalone only), then static buffered
  module allocation/link lifecycle. Do not merely bypass LOADCORE check.
  pending_iop_module_path still names a prior file, so buffer loading needs
  explicit identity/lifecycle handling; avoid matching only allocation size.
  Tests: all21 passed before constants change; all77 Python tests pass after.
  All builds complete; no running process. EE69,101/474; IOP91,429 words.

- startup-paired-callbacks reaches method1e4010 via object slot18 at1e4f94,
  ra1e4f9c, a0=3d3fe0. Original ctor1e3c28 publishes table3d3fa8;
  all nine non-null methods3d3fb4..3d3fd4 retained as roots (eight new).
  EE69,101 words/474 unresolved. System build session41029 is running.
  Next run startup-service-table,60m slices with external synthetic TOC.

- startup-service-callback reaches next indirect callback0x1e4ee0 via
  jalr0x1da8b4, ra0x1da8bc. Original setters1da7d0/1da7e0 store callback
  a1 and arg a2 in object slots20/24 and28/2c. Added both callback bindings
  at dispatch sites1da860/1da8b4. Discovery initially lost pointers on an
  unrelated LW; fixed integer loads to invalidate only their destination.
  Synthetic tests verify all12 load forms preserve unrelated pointers and
  kill overwritten ones (31 tool tests pass). Four callbacks now recovered:
  1e09c8,1e0a28,1e4e40,1e4ee0. EE68,663 words/470 unresolved.
  Current system rebuild session78322. Next run startup-paired-callbacks,
 60m slices. All21 native/tool/integration targets passed immediately before
  the discovery fix; focused31 tool tests pass afterward.

- The60m followup progresses beyond resource lookup and faults at callback
  0x1ca660 via original jalr0x1e5c10, ra0x1e5c18. Added that original root;
  it calls0x1d3e58 and expands EE discovery to68,314 words/470 unresolved.
  Build running session20123. Next: startup-service-callback with60m slices.
  The30m resource-lookup snapshot was ongoing work, not a stuck loop.

- Latest startup-pressure-probe reaches30m slices without an unsupported stop.
  EE0x118388 is a string compare called by resource-name table lookup0x16b344;
  DATA.CVM native reads continue (latest LBA0x1649b8). Controller thread24
  completed ExitThread and is dormant at0x1f0020. A60m-slice followup is
  running (startup-pressure-60m, session63418) to distinguish progress from
  a repeat wait. Negative EE syscall-49 now maps interrupt-only to31,
  sharing ReferThreadStatus; full48-byte comparison regression passes.
  Added -49 returning discovery; EE58,372/353 unresolved. ExitThread synthetic
  test now executes the generated AOT adapter and passes. Pressure42 accepts
  short5/9-byte mode-probe prefixes, verified against original5-byte capture.
  All builds complete. Full21 suite passed before ExitThread/status additions;
  focused IOP/kernel/controller tests pass after respective changes.

- Pressure negotiation now passes: startup-controller-pressure reaches original
  THREADMAN panic at0x9f2fc after DS2O0x2c72c calls thbase:8. Original export
  thbase:8 is module+0x1288 (0x9f288), which marks current thread dormant
  then requests context transfer. Added explicit native ExitThread adapter:
  retains ID/stack, stops dispatch at dormant sentinel, permits restart/delete.
  Focused IOP tests pass; system build is running (session37229).
  Next connected run: startup-thread-exit, same30m-slice/synthetic-TOC recipe.
  Current IOP bundle27 modules/91,429 words. Controller pressure full mask
  ff ff03 is supported with retained12 pressure bytes and21-byte polls.
  Controller implementation moved to runtime/controller.cpp for faster builds.
  All21 regression targets passed before ExitThread addition. Game not playable.

- Controller config now advances to command41 (`startup-controller-config`).
  Added the mode-dependent masked-capability response documented by ps2tek;
  source disagreement is recorded in SOURCES for future verification.
  Full preceding run passed20 targets; IOP test hit stack overflow after
  IopState grew. Moved all its local IopState fixtures to heap allocations,
  preserving lifetimes; focused IOP/controller tests both pass. Latest
  controller41 system build running. Next run `startup-controller-query`.

- Controller configuration boundary: DS2O table build passed. Connected
  `startup-controller-states.log/.ram.json` faults on SIO2 command43,
  packet1,67,0,1,0 (digital -> config), descriptor0x140540. Added independent
  stateful Controller class and5/9-byte SIO2 framing. Config/analog/lock,
  query commands45/46/47/4c and rumble map4d are supported, with retained
  axes and motor values. Pressure/watchdog/host controls remain incomplete.
  New controller tests pass. Full build running after fixing a missing<string>
  include in the new standalone header. Next: full regression and connected
  `startup-controller-config` run. No new IOP roots beyond state table yet.

- Added configured interlaced NTSC display timing (27MHz rational phase,
  525 half-lines/field,480 active,60 fields/1001ms) and cooperative EE INTC
  chains preserving full CPU context. next0/front,-1/back,existing-ID/before
  order follows ps2tek API spec. Pending edges during callbacks remain latched.
  Nonzero callback returns still fault. Original INTC2/3 roots0x1beda0,
  0x1bed80 and0x1aac30 added. Display start also wakes native IOP
  WaitVblankStart threads and publishes GS VSINT. Host rendering unchanged.
  Full build passed;20 test targets pass across full run plus focused kernel
  rerun. Old kernel test omitted AddIntcHandler next argument; set it to0
  to satisfy its real ABI, no runtime relaxation. New ee_interrupt tests
  verify rational timing, chain ordering/removal, masked retention, reassertion,
  context restoration, and invalid insertion/return rejection.
  `startup-display-edges.log` now reaches DS2O IOP0x2c2c0 from0x2c2b8.
  Original state byte+66 is bounded<14 and indexes table module+0x1f10.
  Added that full14-entry table. IOP91,464 words; EE58,370/354 unresolved.
  Latest IOP rebuild running, then run `startup-controller-states`.
  Display timing presently supports the observed interlaced NTSC mode only;
  other modes/custom timings remain explicit gaps. No shared rendering used.

- CURRENT BOUNDARY (2026-09-09): original two-flag wait0x1bed00 is now
  compiled. `startup-two-flag-wait.log` and latest `startup-intc-state.log`
  reach30m-slice budget at EE0x1bed1c, ra0x1696d0. Original caller0x169680
  formats `ST_%03X` from0x44f268 before calling object slot+0x1c. Wait
  clears/polls gp-30444 then gp-30440. Original0x1beda0/0x1bed80 set them.
  Original registration0x1bf198..0x1bf1d4 installs those for INTC2/3.
  Latest budget diagnostics establish INTC status0, mask0x80c, CP0 status
  0x70010001, and these handlers: cause11 ->0x277110 arg0;
  cause3 ->0x1aac30 arg1; cause2 ->0x1beda0 arg0x200;
  cause3 ->0x1bed80 arg0x200. Cause3 therefore needs an ordered chain,
  not a single-handler shortcut. Native AddIntc currently retains next/arg
  but dispatcher only handles SIF0 DMA; inspect original handler insertion
  ABI and preserve callback state/return rules. Do not skip either handler.
  DISPLAY IS NOW CONFIGURED: interlace1, mode2, frame0; PMODE/SMODE2 zero.
  Earlier pre-read fault snapshots showing unconfigured display are stale.
  Next: independently implement configured NTSC display edge timing and
  EE INTC delivery, with synthetic timing/chain/context tests; connect IOP
  display-edge wakeups if appropriate. No shared rendering material may be
  consulted. Current handler targets must be verified/rooted from original
  ELF as needed; do not set the wait flags directly or synthesize completion.
  New read-only budget report in system_diagnostic prints INTC/display state.
  Latest system build passed. EE58,238 words/352 unresolved; IOP90,642 words.
  Multi-record runtime is verified by full18 passes plus corrected focused
  IOP pass. Later changes only add EE static coverage/diagnostic reporting.
  All processes finished; no live tool sessions. Capture logs external under
  TEMP/haunting-toc-probe. Synthetic TOC remains required. Header transfer
  and subsequent two-sector read demonstrated; playability not established.

- Multi-record DMA full build passed; all19 CTest checks now pass across
  full run plus focused IOP rerun. The new extra local IopState caused
  Windows stack overflow0xc00000fd in the already-large test main; moved
  its regression into a non-inlined helper with heap state, then reran IOP
  successfully. No runtime fix needed for that test-harness issue.
  `startup-multi-record.log` advances to EE formatter switch0x26e054.
  Verified original89-entry table0x45ae00..0x45af64 at jr0x26e03c;
  configured table, emitted58,208 EE words/352 unresolved and rebuilt.
  IOP remains27 modules/90,642 words. Connected `startup-formatter-switch`
  currently running with30m slices and external synthetic TOC.

- Original read-completion callbacks now execute: IOP+0xb58 and EE0x10f0a8
  compiled successfully. `startup-ee-read-complete.log` next reaches a
  two-record DVD descriptor (MADR0xc3d88,BCR0x0056000c,CHCR0x41000200).
  Own original CDVDMAN decode proves blocksize12 and43 blocks per record.
  Implemented checked multi-record DMA with per-record MADR/BA advancement,
  STR retained until final record, completion only at descriptor end, and
  full descriptor bounds against remaining command/RAM. Added two-payload,
  no-premature-IRQ, final-status and oversized-descriptor regression cases.
  Full build in progress; this newest runtime change is NOT yet verified.
  EE58,086 words/353 unresolved; IOP27 modules/90,642 words.
  Next: inspect build/test results, then run connected startup and save
  `startup-multi-record` captures externally. Earlier retry0 runtime passed
  all19 tests; that pass predates this multi-record change.

- VERIFIED first DATA.CVM sector transfer: `startup-archive-retry0.ram`
  contains the exact2048-byte local header at IOP0xc3d94. Retry0 runtime
  change and full Release build pass all19 CTest targets (42.83s).
  Startup next stops at IOP0xceb58, ra0xb82b0: original CDVDFSV DMA
  completion callback. Original callsites0xcf0f0/0xcf118 construct its
  address; callback sets event bit0x20 through thevent:7. Added root+0xb58.
  IOP now27 modules /90,642 words; system diagnostic build in progress.
  Next: run with same synthetic TOC and inspect completion/next boundary.
  Archive mounting and complete asset delivery to EE are not yet verified.

- Startup now reaches the first real DATA.CVM sector request (LBA0x164972).
  Extended backend build passed. `startup-backend-extended.log` reached
  IOP0xd1c1c; verified CDVDFSV N-command registration ID0x80000595 and
  its19-entry bounded switch; added root+0x3c1c/table+0x53d0.
  IOP bundle now27 modules /90,632 words; EE58,000 /353 unresolved.
  This build passed. `startup-cdvd-n-command.log` next rejects ReadDvd
  retry0 (ISO discovery used retry16): packet114,73,22,0,1,0,0,0,0,2,0.
  Independently verified original byte construction and added the observed
  successful local-image profile; unsupported retry/error handling still faults.
  Added DMA payload/completion regression; full build currently running.
  Next: full CTest and connected startup to verify actual archive-sector DMA.
  Synthetic TOC remains required; mounting/playability are not proven.

- Further original archive startup coverage: worker table0x3cacc0, bounded
  ten-entry stream switch0x453090 (jr0x1dad74), service table0x3d5b18,
  archive dispatch table0x4558a0, and extended backend slot+0x68.
  Own run sequence: `startup-worker-table` ->0x1dadcc;
  `startup-stream-status` ->0x1ed1e0; `startup-service-backend` ->0x1e9260;
  `startup-archive-dispatch` ->0x1e6198. All are now statically covered.
  Latest emit58,000 EE words /353 unresolved; latest extended-slot build
  in progress, not yet run. Previous builds pass. Only static config and
  provenance docs changed; no runtime semantics changed. Existing19-test
  checks passed earlier this session (tools rerun outside sandbox).
  Complete mounting and real archive asset reads remain to be demonstrated.

- Continued startup past stream table and backend table. Own logs
  `startup-stream-table` and `startup-backend-table` stop respectively at
  0x1e6168 and0x1df1c8. Verified original tables0x4555c8 and0x3cacc0;
  batched remaining related callbacks. EE now56,149 words /345 unresolved.
  Backend build passed; worker-table build in progress, not yet run.
  All19 existing CTest targets pass across the main run and focused tools
  rerun with external fixture access (initial tools failure was sandbox
  PermissionError). No runtime semantic changes this session. Captures
  remain external TEMP/haunting-toc-probe. Synthetic TOC still required.

- Handoff checkpoint: independently decoded seek-like callback0x1db268
  observed at0x1d7360 (object slot+0x18). Original pointer appears at0x3c50b0
  in stream table0x3c5098. Added its related non-null callbacks through+0x40
  together, skipping existing roots, using original ELF pointers. User's
  non-rendering list matched the observed function range on the prior run.
  EE generation now55,441 reachable words /337 unresolved items; IOP remains
  27 modules /87,287 words. Latest hg_system_diagnostic Release build PASSED.
  This latest batch has NOT yet been run. Immediate next action: connected
  startup with --slices30000000 and the external synthetic TOC record; save
  fresh external captures. Last executed stop remains0x1db268 in
  `startup-archive-backend.log`, now included in the built translation.
  No active build/diagnostic sessions. All project sources currently untracked
  in git; do not assume commits or stage game/generated files.

- Added original archive callback0x1e8580 reached at object dispatch0x1e6d48.
  Its tail path brings reachable EE coverage to53,553 words. Connected run
  then reached0x1eae68 via0x1e7dc4. Independently read original15-entry
  wrapper table0x455c38 and included its related wrappers together, avoiding
  separate builds per wrapper. Next observed backend0x1e6078 via0x1eae88
  is also original table0x4555c8+0x18; added that entry and direct reachable
  code. Latest EE54,048 words /333 unresolved items; IOP unchanged87,287.
  System diagnostic builds pass after these static coverage additions.
  Latest connected `startup-archive-backend.log/.ram/.ram.json` now stops
  at EE0x1db268, ra0x1d7368, a0=0x3c5bf8, a1=0, t0=-202. Next: verify
  this archive-stream object callback and its original dispatch table.
  Companion captures `startup-archive-method` and `startup-archive-wrappers`
  record intermediate stops. Supplied non-rendering ranges checked where
  applicable; all code generation derives from the original ELF. No runtime
  behavior changed this checkpoint; no full regression rerun. Successful
  archive lookup remains verified, complete mount/playability does not.
  Synthetic TOC still required. Captures external TEMP/haunting-toc-probe;
  no live tool sessions.

- Fixed stale CDVD DMA completion: a disabled channel must not latch an IRQ
  for later enable. Prior TOC completion was delivered at DVD enable before
  the DVD copy, so original0xb98cc interpreted synthetic header0x29 and
  original0xba268 reported error0x20. Native CDVD channel enable now gates
  completion publication; global interrupt masking still retains enabled
  completions. Focused IOP tests pass, including the stale-TOC regression;
  system diagnostic and IOP tests rebuilt successfully. No full suite rerun
  for this change; previous full-suite result predates this correction.
  `startup-dvd-gated-irq.ram` now contains a successful DATA.CVM result at
  0xd4698: LBA0x164972, size0x5e018000. Size matches local file1577156608;
  ISO bytes at the returned LBA match the local archive header. CDVD error0.
  Startup reached original getter0x1ed1d0 via0x1d6a70. Independently verified
  three instructions return0x3d5b80; reference manual range agrees. Added
  root, emitted EE51,760 words/318 unresolved, diagnostic build passes.
  Next connected `startup-archive-found.log/.ram/.ram.json` faults at
  EE0x1e8580, ra0x1e6d50, a0=0x44fe18,a1=0x47e538. Next: verify this
  original object callback and add its static coverage. Archive discovery
  now works, but mount/playability are not proven; synthetic TOC remains.
  All captures external TEMP/haunting-toc-probe; no active tool sessions.

- EE delay is executing normally: `startup-delay-progress.log` reaches its
  return at0x1ee734 with v1=0x1d80, and an offset budget30001000 advances
  the sampled counter from0x1a62 to0x1b08. Scheduler instrumentation confirms
  current thread1, ready1, no interrupt or active DMA dispatcher. Do not keep
  investigating an EE scheduler stall on this evidence.
  `startup-ee-wait.log` traces the caller through0x1e97a4 after failed dot
  lookup, original0x1e8c98 loading zero from current directory0x7f712c, and
  error -100 through0x1ea058 to0x1e73dc. Added v1 and scheduler fields to the
  budget report; system diagnostic rebuild passes. No runtime semantics changed.
  Batched watch runs `startup-directory-init[-30m].log` at0x1e68b4,
  0x1e8c38,0x1e7d88,0x1e82fc have no recorded hits through30m slices.
  These are candidate directory initialization/update paths from our ELF
  scan; next trace upstream archive setup and DATA.CVM search response,
  rather than pursuing the delay loop. Existing synthetic TOC limitation
  remains. User again requested speed: batch related watches, use focused
  verification for small changes, reserve full suites for meaningful fixes.
  All logs external TEMP/haunting-toc-probe; no live sessions.

- CDVD DMA completion now publishes channel3 and dispatches original IRQ35.
  Original registration0xb70b0..0xb70c4 points to0xb8270; the handler sets
  event bit0x20 after its transfer callback. Tests verify payload visibility,
  masked retention and one-time delivery. Full Windows build and all19 tests
  pass (39.92s). Connected `startup-dvd-irq.ram.json` advances to missing
  indirect DVD callback0xb98cc; added verified original root +0x78cc.
  Regenerated bundle27 modules /87,287 words and rebuilt system diagnostic.
  Runs `startup-dvd-callback` (30m slices) and `startup-dvd-callback-60m`
  now end on budget, with no read-timeout or translation fault. Both sample
  EE0x1ee71c, ra0x1e6c08, a0=0x455918, a1=0x6ce and1837 transport steps.
  Own ELF decoding shows a7552-iteration delay at0x1ee710, called twice by
  0x1e6bf0. Next: watch entry0x1e6bf0 to identify its caller/polled condition;
  equal transport counts across longer runs suggest a remaining wait, not
  demonstrated startup completion. Supplied lists agree only on function
  ranges; no applicable fix found. Synthetic TOC remains required; game
  rendering/playability remains unverified. All captures are external under
  TEMP/haunting-toc-probe. No active tool sessions.

- DVD startup now transfers the actual ISO primary volume descriptor. Original
  CDVDMAN builds11 command bytes: LBA/count, retry16, converted spindle2,
  final0. Native successful-read validation now accepts this checked profile
  and rejects incomplete/other profiles. Diagnostic parameter values now use
  decimal consistently. DMA validation accepts equivalent block factorizations
  totaling516 words, including original43x12 (0x002b000c). Full Windows build
  succeeds. Regression run passed18 tests; the new IOP test initially attempted
  an unsupported active-DMA rewrite. Corrected synthetic descriptor setup,
  rebuilt iop_tests, and its focused rerun passes (0.12s). No runtime change
  followed the other18 passes.
  Own external `startup-dvd-blocks.ram` contains ISO LBA16 exactly at0xc3470
  (2048-byte comparison passes, CD001 signature). Startup next stops at an
  untranslated alarm callback0xb59c0, ra0xa3b84, a0=0xbfff8. Original callback
  prints `Read Time Out %d(msec)` then invokes0xb6d18 with a0=-18 and0xb7fbc. Next: verify
  its registration, translate the root, and trace remaining completion wait;
  do not equate successful sector DMA with successful directory initialization.
  Reference-list exact-address search found no match. Synthetic TOC is still
  used and display mode remains unconfigured. Builder provenance in SOURCES;
  external captures `startup-dvd-builder`, `startup-dvd-profile`, and
  `startup-dvd-blocks` under TEMP/haunting-toc-probe. No live tool sessions.

- Native event-status ABI fix verified: original THREADMAN wrappers +0x36dc
  and +0x3774 now dispatch to native status handlers for native event IDs.
  Status layout comes from the identity-checked original copier +0x36a0.
  Tests cover layout, initial/current bits, waiter lifecycle, context checks,
  invalid IDs and destination validation before writes. Full Windows Release
  build and all19 tests pass (40.34 s). Bundle27 modules /86,787 IOP words.
  Own external `startup-native-event-status.ram.json` confirms return0 at
  0xb6414 and current bits0x28 at0xb641c, replacing the former -409 rejection.
  Startup advances beyond the timeout to0xb8890: ReadDvd N-command8 has11
  parameters, while the runtime accepts8. The diagnostic misleadingly prefixes
  decimal values with `0x`: actual bytes are decimal
  [16,0,0,0,1,0,0,0,16,2,0], requesting LBA16/count1 plus three mode bytes.
  Next: independently decode original command construction and establish the
  three mode-byte meanings before extending the accepted DVD profile. Fix
  diagnostic number formatting too. The supplied lists label sceCdRead at
  EE0x110380 but offer no matching IOP parameter fix; continue diagnostics.
  Hardware documentation describes the first8 bytes and2064-byte framing but
  does not establish the trailing bytes. Synthetic TOC remains diagnostic-only;
  display mode is still unconfigured. Captures are under external TEMP folder
  `haunting-toc-probe`, with matching `.log` and `.ram` files.

- User workflow adopted in AGENTS: check supplied non-rendering reference
  lists while continuing diagnostics; independently verify applicable leads.
  Current reference matches label EE0x10f438 sceCdLayerSearchFile and0x10f948
  sceCdSync, but provide no applicable IOP timeout fix. No rendering reference
  material was used. Connected `startup-toc-completion-state.log/.ram/.ram.json`
  reveals original event-status query called by IRQ0xb640c returns -409
  (0xfffffe67) at0xb6414. It receives native event ID4, whereas original
  THREADMAN routine0xa17b0+ validates original encoded handles. Runtime creates
  native event handles via IopState::create_event_flag. Next: audit missing
  native iReferEventFlagStatus mapping and derive status layout from original
  helper0xa16a0, preserving waiter semantics. This is a likely mixed native/
  original event ABI issue, not yet a verified fix. The earlier empty b6440
  watch is inconclusive because it is a branch delay slot (not separately
  recorded by PC tracing).

- Original GetToc timeout callback now included as CDVDMAN root +0x6ebc.
  Identity-checked relocated module setup at +0x7040 independently proves
  a1=module+0x6ebc. Bundle now27 modules /86,870 IOP words; diagnostic build
  passes. Connected `startup-toc-alarm.log/.ram/.ram.json/.ram.console.txt`
  executes original callback and prints `Cmd Time Out 10000(msec)`, then faults
  at0xb8efc on unsupported byte write1 to CDVD BREAK0x1f402007. Display mode
  remains unconfigured. Next: trace command-completion/event wait in0xbb4b8
  reached from0xb9048, including original IRQ acknowledgements and pending
  descriptor/DMA state. Do not treat translating the alarm as fixing the wait.
  Synthetic TOC remains diagnostic-only. No hardware implementation changed.

- Successful CDVD completion now publishes error0 before the completion IRQ
  for validated TOC/DVD DMA, instead of retaining a pre-command byte. This
  is the native success-result contract; exact hardware reset timing and
  write-side0x1f402006 semantics remain unresolved (SOURCES documents scope).
  Focused IOP tests cover stale errors on both successful transfer kinds;
  full Windows build and all19 Release tests pass (45.16 s).
  Connected `startup-toc-success.log/.ram/.ram.json`
  confirms retained error0 at0xccfd1 and advances to untranslated IOP callback
  `0xb8ebc`, ra=`0xa3b84`, a0=`0x179510`. Original code is an alarm callback:
  logs timeout and writes BREAK to0x1f402007, so this is not proof of successful
  directory initialization. Next: verify its original registration at0xb9040
  (a1 formed as0xb8ebc), add the CDVDMAN root, and trace why the alarm fires.
  Synthetic TOC remains in use; no playability established.

- Configurable IOP write trace implemented: `--watch-iop-word` accepts an
  aligned 2MiB RAM word, preserves the default0x9a920, survives reboot, and
  reports the selected address. Tracing guards RAM bounds. Diagnostic build
  passes. Connected `startup-toc-write-trace.log` captures the actual error
  producer: original IRQ `0xb62d4` stores0x84 into0xccfd1 after reading
  MMIO `0x1f402006`. MMIO history shows earlier original write at `0xb84a4`
  put0x84 into that address, then command9 submitted at `0xb8890`, followed
  by successful native DMA completion and IRQ reading the unchanged0x84.
  Runtime currently treats writes to the error register as assigning
  `cdvd_error` and never clears it on command/completion. Next: resolve this
  incorrect retained-error behavior using independent register semantics.
  ps2tek CDVD I/O table marks0x1f402006 read-only; it does not specify write
  side effects or reset timing, so do not guess those without evidence.
  Existing boundary is now a concrete native MMIO issue, not layer mismatch.

- Error0x84 producer investigation: original accessor `0xb8b04` reads retained
  byte `0xccfd1`. `startup-toc-error-source.log` has no hit at conversion
  branch `0xb92e0`; GetToc does hit `0xb8fd0`, which puts0x84 into the
  command descriptor halfword at stack+38 (not itself an error-byte write).
  A follow-up watch `startup-toc-error-write.log` has no hit at explicit
  setter `0xb6fd8`. These observations do not establish whether hardware
  status, timeout or descriptor validation produced the retained error.
  Next: make the existing IOP RAM-write trace address configurable and trace
  aligned word `0xccfd0` to capture the actual producer, rather than guessing
  more write sites. Existing hardcoded trace word is `0x9a920` in IopState
  store; preserve its default and carry selected address across reboot.
  No runtime behavior changed; error remains explicit in original game path.

- Directory/GetToc investigation: corrected the prior layer inference. IOP
  watch histories are before load-delay completion: a2=0x10 at `0xce810`
  becomes0 at `0xd2d88` and `0xbb93c`; request buffer +296 is also0.
  `startup-disc-directory.log` proves search calls initialization `0xbc89c`,
  which fails because DVD-type0x14 path `0xbc94c` -> `0xb64a0` returns0.
  That helper calls original GetToc wrapper `0xb90b0` -> `0xb8f1c` and
  takes failure branch `0xb650c`. `startup-toc-result.log` proves command
  submit returns0 at `0xb901c` (accepted/nonnegative), then error accessor
  returns0x84 at `0xb9070`, causing false return. Next: trace the producer of
  CDVD error0x84 and repeated GetToc completion/event handling. Do not yet
  attribute this to TOC content. New evidence narrows the fault before filename
  matching; no success result or bypass has been introduced.

- Disc-search trace: `startup-search-result.log` proves EE PollSema returns
  expected id4 at `0x10f488`, busy check returns0 at `0x10f4a4`, then the
  normal RPC-result path returns0 at `0x10f700`. Original service id80000597
  registration at `0xd24d4` selects CDVDFSV callback `0xce7ac`; its 300-byte
  request calls original CDVDMAN import84 via `0xd2d88` -> `0xbb93c`.
  Connected `startup-search-iop.log/.ram/.ram.json` proves callback call
  `0xce810` receives request buffer `0xd4698`, filename `\DATA.CVM;1`
  at +36. The pre-load a2=0x10 observation was corrected above: actual layer0.
  It returns0 at `0xce818`.
  Next: inspect original CDVDMAN `0xbb93c` search path and layer handling;
  validate its disc-directory initialization. RPC dispatch
  and filename transport are working at this boundary. No bypass or native
  success was added; the failure remains reproducible and unresolved.

- Cache failure investigation: connected `startup-cache-watch.log` proves
  `0x1e8c90` reads the global state pointer `0x3d5a08` -> `0x7f7100`, whose
  +0x2c current-directory field is zero. It returns zero through `0x1e79b0`
  and `0x1e97a4`. Original ELF string at `0x44f7f0` is `.`; the immediate
  divideFname failure therefore concerns resolving the current directory.
  Original setter `0x1e8ba8` writes +0x2c at `0x1e8c80`; no hit on that
  watch in `startup-cache-population.log`. The earlier DVCI cache-miss message
  is emitted at `0x1db144`; lookup `0x1dc0c8` returns zero at `0x1db11c`,
  then fallback `0x1dac80` also returns zero at `0x1db170`. Next: trace this
  fallback's request/result and underlying original disc search. Do not assume
  the synthetic TOC is causal yet. Original setter pointer is `0x4558c0`.
  No code behavior changed this investigation; external watch captures provide
  the next debugging boundary. The run still reaches its budget, not playability.

- Callback/file-cache continuation: verified original ELF `0x4558dc` ->
  `0x1e9760`, bound callsite `0x1e73c8` (table +0x3c). EE coverage now
  51,757 words / 318 unresolved items; diagnostic build passes. Connected
  `startup-callback3c.log/.ram/.ram.json` reaches the 30M slice budget at
  EE `0x1ee71c` rather than an untranslated target. Retained error context
  a0=`0x455918`, a1=`0x6ce`, ra=`0x1e6c08` corresponds to failure of helper
  `0x1e78a0` called from `0x1e979c`. Added existing retained EE DECI2 text
  to budget diagnostics (previously only its byte count was printed).
  Rebuilt and reran as `startup-callback-error.log`: reports DVCI file cache
  miss for `\DATA.CVM;1`. Next: trace original cache population/search and
  disc metadata; determine whether synthetic TOC causes this miss. Do not
  bypass the error or substitute a success. Synthetic TOC remains diagnostic
  only. This run also logs three successful RNA IOP allocations. No title
  screen, game frame or playability is established.

- Startup object continuation: original ELF `0x46b05c` -> `0x1c2970`
  verified for JALR `0x1bf174`. Connected `startup-object-init` passes it,
  then stops at `0x1c2930`, ra=`0x1b82d8`. Original pointer `0x46b064`
  supports callsite `0x1b82d0`; its same-object tail at `0x1c2954` uses
  `0x46b068` -> `0x1c1e00`. Both built together and connected
  `startup-object-methods` passes them, reaching `0x1c1460`, ra=`0x1b8300`.
  Verified original slot `0x46b074` and caller `0x1b82f8`; also bound later
  same-object calls `0x1b8340`, `0x1b836c`, `0x1b839c` to these original
  +0x18/+0x24 slots. EE generation now 49,583 words / 314 unresolved items.
  Final diagnostic build passes. Connected `startup-object-batch` passes the
  prior stop and reaches untranslated `0x1e9760`, a0=`0x44f7f0`, a1=`0x82a940`,
  ra=`0x1e73d0`. Next: inspect original caller near `0x1e73c8` and verify its
  pointer provenance. No runtime implementation changed; original game input
  only, no reference rendering material. Synthetic TOC remains diagnostic-only.

- EE division continuation: independently implemented integer-significand
  truncation and signed-zero/exponent255/range handling in `Fpu::divide`, with
  cause/sticky flags based on Sony core pp156,158–165 and DIV.S p357. Focused
  FPU tests pass, including inexact thirds, both signs and normalization paths,
  zero division, range limits and flag persistence. Full Windows build passes;
  all 19 Release regression targets pass (40.69 s).
  Hardware-specific last-bit deviations remain unmeasured (see SOURCES); this
  is not a claim of bit-exact divider fidelity. Connected
  `startup-divide.log/.ram/.ram.json` passes the former division boundary and
  reaches untranslated EE `0x1c2970`, a0=`0x7fe400`, ra=`0x1bf17c`.
  Own decoding shows JALR `0x1bf174` loads the object's vtable slot +0x0c.
  Next: verify its original ELF pointer and extend static coverage. Synthetic
  TOC remains diagnostic-only; no game frame/playability established.

- Startup nested-method continuation: independently checked original ELF
  `0x46ab68` -> `0x1a4510` for callsite `0x1b8478`, and the same object's
  slot +0x10 (`0x46ab60` -> `0x1a45a0`) for nested callsite `0x1a4520`.
  Both bindings were built together: 47,793 EE words, 320 unresolved items;
  diagnostic build passes. Connected `startup-rng.log/.ram/.ram.json` passes
  both and stops at `0x1b857c`: `EE FPU divide rounding requires validation`.
  Original DIV.S divides f1 by f0; preceding code loads 180.0 into f0 and
  multiplies f1 by the original single-precision pi constant. Next: establish
  EE division rounding from hardware specifications before extending
  `Fpu::divide_exact` (currently rejects nonzero integer division remainder).
  Existing exact-division tests intentionally reject 1/3 and will need meaningful
  rounding coverage alongside the implementation. Synthetic TOC remains in use.

- Startup slot +0x14 continuation: checked original ELF pointer `0x46ac64`
  -> `0x1bbbb0` and JALR `0x1bbcd8`, then added the static binding. EE
  coverage is 47,622 words with 321 unresolved items. Diagnostic build passes;
  connected `startup-slot14.log/.ram/.ram.json` passes this method and stops
  at untranslated `0x1a4510`, ra=`0x1b8480`, a0=`0x887a00`, a1=-1.
  Next: inspect original caller near `0x1b8478` and its target provenance.
  No runtime behavior changed; validation was the targeted build and connected
  startup run, using the explicitly synthetic diagnostic TOC.

- Startup virtual-method continuation: independently verified original ELF slot
  `0x46ac60` -> `0x1bbc20` and bound callsite `0x1b71fc`. Generated EE coverage
  is now 47,596 words, with 322 unresolved sites. Diagnostic build passes.
  Connected `startup-method.log/.ram/.ram.json` now reaches untranslated EE
  `0x1bbbb0`, a0=`0x4f1920`, a1=13, ra=`0x1bbce0`. Next: inspect this original
  caller and pointer before adding its static binding. Synthetic TOC remains
  diagnostic-only; this is not evidence of playability.
- Speed review: shared inline runtime/MMIO implementation causes large generated
  translation rebuilds after small hardware edits. Proposed improvement (not yet
  implemented or measured): isolate MMIO implementation in compiled source while
  retaining inline RAM access. Batch related startup fixes and focused validation;
  run the full regression suite at meaningful checkpoints. No rendering reference
  material is needed or permitted for this work.

- GS system reset continuation (2026-09-08): consulted original Sony GS v6
  manual pp145-146/154. CSR bit9 reset now cancels native vertex/draw/transfer
  state and restores the runtime initial profile, including all five interrupt
  masks; VRAM contents are retained. Unspecified-by-manual register defaults
  and VRAM retention are explicitly native policy, not hardware-observed reset
  values. Separate FLUSH remains unsupported. Runtime tests pass for these
  effects. Full Windows build passes; all 19 Release regression targets pass (40.38 s).
- Connected `gs-reset.log/.ram/.ram.json` passes reset and reaches EE virtual
  method `0x1bbc20`, a0=`0x4f1920`, a1=13, ra=`0x1b7204`. Own ELF decoding
  proves caller `0x1b71fc` loads object vtable +0x10 before JALR. Next: verify
  its original pointer/target and add the required static binding. No reference
  rendering material was used for reset or caller discovery.

- GIF reset continuation (2026-09-08): independently mapped the observed
  word write1 to GIF_CTRL `0x10003000`. It clears retained incomplete GIF
  packets while preserving separate GS state. Unsupported stop/restart writes
  and widths fault without clearing the buffer. Full Windows build passes;
  focused runtime reset tests pass. Connected `gif-reset.log/.ram/.ram.json`
  passes GIF reset and stops at EE `0x10be94` on the existing explicit GS CSR
  FLUSH/RESET boundary, a0=`0x200`. Next: inspect original CSR write and
  hardware GS reset semantics, preserving the separation between GIF transport,
  GS registers and VRAM. Full Windows Release regression: 19/19 passed
  (44.47 s). The ps2tek register list confirms the CSR address but does not
  supply the reset-state details needed here; consult the GS hardware manual.

- CPU VIF1 FIFO continuation (2026-09-08): SQ to `0x10005000` now submits
  both GPR halves through the existing independent VIF parser and synchronizes
  GS interrupt state afterward. Runtime tests pass for both halves, uncached
  aliases, zero-register NOPs and scalar-width rejection. Diagnostic and runtime
  test builds pass. Connected `vif-cpu-fifo.log/.ram/.ram.json` passes both
  original FIFO writes and stops at `0x10c014`, the word write of1 to
  `0x10003000`. Next: implement that register's reset semantics from hardware
  documentation, with parser/transport reset tests. Latest full 19-test pass
  predates this CPU FIFO change; its focused runtime tests and native startup
  continuation are verified. No rendering material copied from the reference.

- VIF1 reset/MMIO continuation (2026-09-08): independently implemented the
  observed FBRST reset and ERR ME0 writes through checked 32-bit MMIO routing.
  Reset discards pending VIF stream/phase and interface state while retaining
  separate VU data/micro memories. Unsupported controls and widths fault.
  Runtime tests cover alias routing, reset effects and rejection preservation.
  Full Windows build passes. Connected `vif-reset.log/.ram/.ram.json` passes
  these writes and stops at EE `0x10c004`: original SQ to VIF1 FIFO
  `0x10005000`. The next instruction sequence writes another quadword, then
  writes1 to `0x10003000`. Next: route checked CPU quadword FIFO submission
  through the existing VIF parser, then implement the following register from
  hardware specifications. No reference-project rendering material used.
  Final rebuilt Windows Release regression: 19/19 passed (43.58 s).

- MCSERV continuation (2026-09-08): selected the identity-checked module in
  the bundle and shared initial/reboot runtime plan. Independently decoded
  original entry proves worker +0x280; original worker registration proves RPC
  handler +0x324 for service `0x80000400`. Both roots added before rebuilding.
  Bundle now has 27 modules and 86,846 reachable words. Windows diagnostic
  build passes; LOADFILE and reboot regression tests both pass (13.95 s).
  Connected run `mcserv-load.log/.ram/.ram.json/.ram.console.txt` records
  MCSERV id11, ret2, then EE stops at `0x10bfc0` on an unsupported MMIO store.
  Own identity-checked ELF inspection `hg.py inspect --pc 0x10bfb0 --count 18`
  proves the sequence constructs address `0x10003c10` and stores value1;
  the following store targets `0x10003c20` with value2, then accesses VU control
  register28. Next: implement the required register semantics from hardware
  documentation and existing runtime state, respecting the user's prohibition
  on copying rendering-related material from the reference project.
  The full 19-test pass at 37.94 s predates these root/module-plan extensions;
  targeted native load/reboot tests and connected execution are verified.

- MCMAN init continuation (2026-09-08): original driver descriptor `0x4700c`
  points at operations table `0x46fa0`; its init entry `0x44c10` is now rooted
  at module +0xfc10. Original code returns zero. Generated bundle: 26 modules,
  85,865 reachable words. Windows diagnostic build passes; connected run
  `mcman-init.log/.ram/.ram.json/.ram.console.txt` records MCMAN id10, ret2,
  then reaches a real LOADFILE request for `MODULES/MCSERV.IRX`. That module
  is already identity/placement configured but absent from selected bundle and
  shared runtime module plan. Next: add MCSERV to both, regenerate and build.
  Latest full 19-test pass (37.94 s) predates this root-only extension; the new
  build and connected startup continuation are verified.
- User explicitly prohibits copying ANY rendering-related material from the
  shared HG/ps2recomp reference project. This restriction is recorded in AGENTS
  and SOURCES; independently derive and implement rendering.

- MCMAN continuation (2026-09-08): the identity-checked module at configured
  base `0x35000` is now selected in the IOP bundle and shared initial/reboot
  module plan. Bundle: 26 modules, 85,863 reachable words. Full Windows build passes.
  Connected run `mcman-load.log/.ram/.ram.json/.ram.console.txt` passes the
  module-plan boundary and stops at MCMAN `0x44c10`, ra=`0xa79b0`, invoked
  by IOMAN with a0=`0x4700c`. Original relocated pointer `0x46fa0` contains
  this driver callback. Next: inspect its operations table and add proven
  driver callback roots, beginning with module +0xfc10. Not rooted yet.
- Console retention no longer aborts guest printf after 64 KiB. It retains
  the latest 64 KiB, counts discarded bytes and preserves the full printf
  return length. External IOP dumps also write `.console.txt` so later load
  failures can be inspected directly. A regression covers trimming and return
  values; the rebuilt IOP test passes. Full rebuilt Windows Release regression: 19/19 passed (37.94 s).
- User clarified reference use of shared HG configuration is allowed for
  context/ideas, with independent verification and implementation. No copying;
  emulator implementation restrictions remain. AGENTS/SOURCES reflect this.

- Two-port digital poll continuation (2026-09-08): per-port connection state
  and active-low button words now model one connected controller and an empty
  second port. Connected/disconnected/reconnected and port-isolation tests
  pass. Unsupported commands and multitap port indices remain explicit faults.
  Full Windows build passes. All 19 regression targets pass across runs:
  the initial IOP test exceeded stack capacity after adding another large
  state fixture; moving that fixture to the heap fixes it and its rerun passes.
- Connected run `sio2-two-ports.log/.ram/.ram.json` passes port1 and later
  reaches the diagnostic console capacity limit. Own RAM inspection shows
  repeated LOADFILE logging for `cdrom0:\MODULES\MCMAN.IRX`; MCMAN is in
  config but absent from the selected bundle/native module plan. Next: inspect
  that load failure and add the checked module/dependencies to the startup plan.
  A `--stop-on-loadfile-error` probe named `mcman-request` actually stops at an
  earlier SNDDRV result -2, so it is NOT evidence of MCMAN's result. Preserve
  that distinction. The console retention cap also needs decoupling from guest
  printf execution; it must not become a game stopping condition.
- User-shared `../HG/config/` files were inspected for provenance only. They
  include explicitly PS2Recomp-derived configuration/identification. No such
  data was imported or used for implementation; see SOURCES.

- Vblank wait continuation (2026-09-08): diagnostic capture proves no SetGsCrt
  mode exists when DS2O first calls WaitVblankStart. Implemented a native
  per-thread event wait instead of aborting all startup. Only a subsequently
  published display edge grants a return; elapsed time and old edges cannot
  satisfy it. Multiple current waiters wake once. Focused IOP tests pass.
  The display event producer is still unfinished; no artificial edge is sent.
- Connected run `vblank-wait.log/.ram/.ram.json` proves independent startup
  continues with DS2O parked: it reaches DBCMAN RPC callback `0x27cd8`,
  a0=`0x80001301`, ra=`0x998bc`. Original identity-checked relocated pointer
  `0x28560` contains this callback, now added as an AOT root at +0x1cd8.
  New IOP bundle: 25 modules, 71,063 reachable words. Full Windows build passes.
  Connected evidence `dbcman-rpc.log/.ram/.ram.json` passes this command and
  reaches a second SIO2 poll: SEND3[0]=`0x00140541`, FIFO `01 42 00 00 00`.
  This selects controller port1; existing endpoint accepts only port0. Next:
  implement explicit native port connection state and the supported disconnected
  port response before advancing controller configuration. Display mode remains
  unconfigured here; DS2O stays blocked until a genuine display event producer
  exists. Full rebuilt Windows Release regression: 19/19 passed (31.84 s).

- SIO2 digital endpoint continuation (2026-09-08): implemented the observed
  single port-zero CPU-FIFO digital poll, returning active-low native button
  state and connected status. Completion publishes the reply before IRQ17;
  its original SIO2MAN callback +0x584 is independently proven by registration
  and added as an AOT root. Focused tests pass for button payload, IRQ masking,
  delivery, context restoration and FIFO exhaustion. Host input wiring,
  configuration/analog commands, DMA and other peripherals remain unfinished.
- Connected evidence `%TEMP%/haunting-toc-probe/sio2-digital.log/.ram/.ram.json`
  passes the first transfer and reaches DS2O callback `0x2d944` via SIO2D
  `0x7729c`. Identity-checked original relocated data at DS2O `0x2df9c`
  contains this callback, now rooted at +0x1944. IOP bundle: 25 modules,
  70,746 reachable words. Full Windows Release build passes. Connected run
  `sio2-callback.log/.ram/.ram.json` passes that callback and reaches the next
  DS2O callback `0x2d9d8` through SIO2D return `0x77318`. Next: inspect the
  original callback table and root its proven startup callbacks as a batch.
  Final Windows Release regression: 19/19 passed (26.96 s). Original relocated
  DS2O table entries `0x2df9c=0x2d944` and `0x2dfa0=0x2d9d8` independently
  confirm the two successive callbacks. The second is now rooted at +0x19d8 using original table +0x1fa0.
  Regenerated bundle: 70,787 reachable words across 25 modules. Windows
  connected diagnostic build passes; `sio2-attach.log/.ram/.ram.json` now
  reaches `WaitVblankStart` at IOP `0x2de3c`, ra=`0x2c1a0`. This is the
  next actual startup blocker: the DS2O worker needs a connected display
  timing source. Do not replace the wait with unconditional success or an
  unrelated instruction-count delay. The latest full 19-test pass predates
  this root-only extension; the new build and connected run are verified.

- SIO2 request preservation (2026-09-08): unsupported start writes now fault
  before reset bits erase the pending FIFO. External diagnostic JSON records
  the requested control, register bank and input bytes. A synthetic regression
  verifies that rejection preserves the FIFO/registers without completing work.
  The connected 30-million-slice synthetic-TOC run stops at the same original
  IOP `0x7a008`, requested control `0x3bd`, with FIFO `01 42 00 00 00`,
  SEND3[0] `0x00140540`, SEND1[0] `0xffc00505`, SEND2[0] `0x00020014`.
  Evidence: `%TEMP%/haunting-toc-probe/sio2-packet.log/.ram/.ram.json`.
  Next: establish the response and interrupt contract for this exact packet
  from hardware/protocol evidence and the original SIO2MAN consumer before
  implementing a bounded peripheral endpoint. No transfer completion is faked.
  Validation: all 18 non-tools Windows CTest targets passed; the tools target
  initially failed on sandbox temporary-file permissions, then passed when
  rerun with approved external-temp access (0.87 s). All 19 targets therefore
  pass across these runs. The synthetic TOC remains diagnostic-only.

- EE startup volume continuation (2026-09-08): virtual call `0x2105a0`,
  original ELF pointer `0x46c090` -> `0x20e9a0`, now statically translated.
  It exposed C.LE.S (`0x46006036`) at `0x20e9c4`. Implemented its documented
  exact comparison and condition-bit update, preserving other FCR31 bits.
  Decode tests reject nonzero reserved fd; FPU tests cover clamp boundaries,
  negative ordering and both signed zeros. Sources cite Sony v6 p351.
- Connected execution passes C.LE.S and reaches the same object's tail call
  `0x20e9e0`, virtual slot +0x178. Original ELF `0x46c098` -> `0x20e8d0`
  independently verifies the new binding. Emit: 46,266 reachable EE words,
  318 unresolved items. Build and connected run pass this target and reach
  an actual LOADFILE request for `MODULES/DS2O_S1.IRX` at IOP `0x96cac`.
  Full Windows CTest: 19/19 pass (29.61 s); Ubuntu FPU tests pass.
- DS2O_S1 already has verified identity and reserved base `0x2c000` in the
  IOP config (memory size 8,688 bytes). Added it to the connected native
  module-load plan and regenerated a 25-module bundle with 70,119 reachable
  IOP words initially. Initial/reboot module plans are now shared so reboot
  does not silently omit a newly planned module. Original worker +0x164 is
  rooted; vblank ordinal4 is identified and linkable but faults on invocation
  until display timing is connected. Current IOP emit: 70,694 words.
- DS2O linking exposed an overly strict minor-version check: it imports
  SIO2MAN 2.6, while the supplied table is 2.4. Original LOADCORE link path
  `0x96d14 -> 0x97138 -> 0x97038` compares only the high version byte;
  the minor comparison belongs to export replacement. Corrected planner and
  synthetic regression expectations. Build succeeds; DS2O now links and
  starts. Connected execution reaches SIO2 transfer-control store at IOP
  `0x7a008`, a0=`0x3bd`, ra=`0x7a314`, worker11, native time 14,038,821 us.
  It correctly faults because no controller/card endpoint exists. External
  evidence: `%TEMP%/haunting-toc-probe/ds2o-major-link.log/.ram/.ram.json`.
  Next: inspect original SIO2 packet setup and implement the required bounded
  controller protocol using hardware/ABI evidence. The current SIO2 stub
  clears FIFOs for control bits 2/3 before rejecting bit0; preserve pending
  command evidence before that rejection when expanding diagnostics.
  WaitVblankStart remains an explicit separate timing boundary.
  Final Windows Release regression suite: 19/19 pass (24.41 s).
  Its original imports include vblank ordinal4,
  SIO2MAN 51/61/62, SIO2D 6/9/10 and DBCMAN 4/6/7/8; unsupported provider
  behavior must remain explicit. Latest boundary evidence is external
  `%TEMP%/haunting-toc-probe/ee-volume-update.log/.ram/.ram.json`.
  External evidence: `%TEMP%/haunting-toc-probe/ee-volume-method.log/.ram/.ram.json`
  and `ee-less-equal.log/.ram/.ram.json`. Synthetic TOC remains diagnostic-only.

- EE thread-start root cause (2026-09-08): original `0x26cc70` returns
  `(CP0.Status ^ 1) & 1`; StartThread wrapper `0x26d300` rejects nonzero.
  Native launch status `0x70010000` had IE cleared. Launch now uses
  `0x70010001`; DMA callbacks gate on IE/EIE and clear both in callback context.
  A fresh run then reached the explicit ready-thread scheduling boundary at
  `0x26c034`, proving earlier dormant workers were caused by the status bug.
- EE scheduler now switches by priority at cooperative host boundaries and
  preserves per-thread CPU context, guest kernel-return frames and transfer
  provenance. WaitSema blocks; SignalSema reserves a token for its sole waiter;
  SleepThread uses existing WakeupThread state. Multiple semaphore waiters remain
  an explicit ordering fault. Started threads enter with IE/EIE enabled and
  no exception level. Kernel tests pass, including blocked-root/worker context
  isolation and token reservation. Full-suite validation is pending this build.
- Connected run now starts an EE worker at `0x1cb298`, stopping at its absent
  translation. Added original observed CreateThread entries `0x1cb1a8`,
  `0x1cb298`, `0x1cb3a0`, `0x1cb480`, and sound RPC entry `0x21f210` as AOT roots;
  SleepThread `0x32` is a returning syscall. Emit: 46,219 reachable EE words,
  319 unresolved items. Full Windows build and all 19 tests pass (30.65 s).
  Connected continuation executes TIMEMANI timer callback `0xd7878`, then
  SNDDRV callback `0x84b80`, proving its initialization has passed the previously
  stalled bind. Both are now explicit AOT roots (TIMEMANI +0x878, SNDDRV +0x9b80;
  the latter address is installed by original setup at `0x84c54`). Current
  regenerated IOP bundle: 70,017 reachable words across 24 modules. Build
  and connected continuation pass those timer boundaries and reach original
  EE virtual method `0x20e9a0`, a0 `0x887200`, a1=2, ra `0x2105a8`.
  Latest evidence: `%TEMP%/haunting-toc-probe/snddrv-timer.ram/.ram.json/.log`
  and repeat `ee-scheduler-final.log`. Next: inspect call at `0x2105a0` and
  its original ELF pointer before adding the next indirect-target binding.
- Equal-priority RotateThreadReadyQueue now feeds the scheduler's circular
  selection (including root thread), with a synthetic rotation test. Final
  Ubuntu kernel tests pass. Final Windows Release CTest: 19/19 pass (35.19 s). The old IOP SIF0 quantum64 workaround remains for separate
  re-evaluation; do not mix it into the new EE thread-context fix.
- Ubuntu/g++ builds and passes the kernel scheduling/interrupt tests. It exposed
  a pre-existing tokenization problem in `gs.hpp`: `0x4e+context` was parsed as
  an invalid numeric suffix; whitespace at both depth-buffer accesses fixes it.
  No full Linux game build verification claimed.
  External logs: `%TEMP%/haunting-toc-probe/ee-ie-enabled.log`, `ee-scheduled.log`.

- Voice-transfer completion continuation (2026-09-08): the checked one-shot
  SPU2 copy now exposes a per-core completion condition at register +0x344
  bit 7, retired on ATTR transfer-mode changes. This is explicitly an
  inference from original LIBSD's completion consumer, not general verified
  STATX/DREQ hardware semantics (see SOURCES). AutoDMA does not assert it.
  Both-core tests cover committed bytes, status isolation, mode retirement,
  and rejected status writes. Full Windows build and 19/19 tests pass (9.80 s).
- Connected synthetic-TOC execution now exits the LIBSD wait and reaches EE
  `0x20f070` through the original virtual call at `0x2104cc`, return `0x2104d4`.
  External evidence: `%TEMP%/haunting-toc-probe/voice-complete.log/.ram/.ram.json`.
  Original ELF pointer `0x46c050` contains that target; the corresponding
  indirect-site binding has been added. EE emit reports 45,584 reachable words
  and 315 unresolved items. Rebuilding the connected host with this new target
  completed successfully. The connected 30-million-slice run now reaches a
  MODHSYN list traversal at `0x62b20..0x62b50`, called by SNDDRV request
  `0x70000`, with null sentinel at `0x6d520` (should be initialized by the
  original routine `0x64b20`). External `startup-vcall.log/.ram/.ram.json`.
- Focused 16-million-slice `hsyn-init-trace.ram.json` records SNDDRV request
  handler `0x857b4` receiving `a0=0x70000`, but no execution at `0x611b4`
  (call to list initializer) or `0x64b20` itself. Thus initialization has not
  reached these points; do not repair list pointers by hand.
  SNDDRV worker20 is still binding EE server `0x77777779` at `0x84f78`, before
  its original initialization calls at `0x84ff0..0x85010`.
- Added EE thread summaries on budget stops. External `ee-workers.log` shows
  thread7 entry `0x21f210`, priority10, dormant/uninitialized. Own ELF decoding
  confirms this entry sets up the reverse RPC server (constant `0x7777` at
  `0x21f22c`). Other EE workers are dormant too. The next investigation is why
  this server worker has not been started before the `0x70000` sound request.
  Existing EE scheduler also does not preempt active main or block WaitSema
  into scheduler state; address that when original execution proves needed.
  The older SIF0-specific IOP quantum64 workaround is still present and should
  be re-evaluated now that EE callbacks interleave. No speculative scheduling
  or list initialization changes were made at this checkpoint.
  Latest SPU2 tests also pass on Ubuntu/g++; no full Linux verification claimed.

- EE/IOP scheduling continuation (2026-09-08): the 69,918-word IOP bundle
  builds and CRI's RPC worker completes with paced AutoDMA. The synchronous EE
  callback host then stalls in SNDDRV's bind loop for EE server `0x77777779`,
  starving LOADFILE. External evidence: `paced-cri-callback.log/.ram/.ram.json`
  under `%TEMP%/haunting-toc-probe/` (30-million-slice synthetic-TOC probe).
- `EeDmaInterruptDispatcher` now preserves CPU context across host slices;
  the connected diagnostic gives each callback one AOT step while IOP and
  devices continue. Synthetic tests verify suspension, restoration and a
  second pending completion. All 19 Windows CTest targets pass (8.54 seconds).
  The new SPU2 synthetic executable also passed under Ubuntu/g++.
- The interleaved connected probe advances into SNDDRV's watched handler
  `0x857b4` (nonempty watch history), then explicitly faults in LIBSD's IRQ at
  `0x31aac`, native time 14,470,984 us. Original decoded instructions poll
  core-1 register `0xbf900744` bit `0x80`; the retained register reads zero.
  Core 1 just completed a one-shot DMA (MADR `0x34490`, CHCR `0x201`).
  Artifacts: `interleaved-ee.log/.ram/.ram.json` in the same external directory.
  Next: establish SPU2 transfer-status register semantics from primary hardware
  documentation before implementing this status. Do not simply increase the
  IRQ budget or fabricate the polled bit. The earlier supposition that this
  necessarily requires a resumable IOP dispatcher is not yet established.
  Synthetic TOC remains diagnostic-only; no playable-build claim.

- Paced SPU2 input continuation (2026-09-08): `spu2_input.hpp` now implements
  the documented 48 kHz stereo input areas and 256-sample double-buffer halves.
  AutoDMA accepts the observed 16-word blocks with whole 1024-byte stereo pairs,
  retains L/R data separately, and advances a pending DMA only when consumption
  frees a half. Counters/registers/IRQ complete after actual copying; an occupied
  buffer prevents the former immediate rearm loop. Unknown modes, underruns,
  malformed ranges and mutated active descriptors remain faults. Both initial
  halves are primed before playback as an explicit native startup-phase policy;
  channel-index ordering is independently inferred from CRI's alternating
  512-byte queue setup and still needs nonzero hardware observation. Raw input
  history is explicitly truncated and is not mixed/synthesized host audio.
- Full Windows Release build and 19/19 CTest targets pass with the new `spu2`
  suite. The connected synthetic-TOC run now advances through 515 input frames
  to CRI's original stream-completion callback `0x00011d5c`, at native time
  13,100,804 us. External artifacts: `%TEMP%/haunting-toc-probe/paced-input.log`
  and `paced-input.ram` plus `.json`; metadata now includes each input core and
  its pending DMA. This callback is a proven new AOT root at CRI_ADXI `+0x1d5c`.
  The regenerated 24-module bundle contains 69,918 reachable IOP words; its
  connected build/run is the next validation, not yet claimed successful.

- Resume verification (2026-09-08): the current source tree and existing
  local build do not yet reproduce the final September 6 SNDDRV checkpoint.
  A fresh 30-million-slice synthetic-TOC diagnostic reaches an earlier CRI_ADXI
  RPC wait: EE `0x0026c1e4`, semaphore 9, worker 18 at `0x00012ef8`, and worker 19
  at `0x00014128`. SNDDRV server records remain zero. The external log is
  `%TEMP%/haunting-toc-probe/resume-snddrv.log`. The later-discovered external
  `%TEMP%/hg-snddrv-sema420-30m.log` already recorded the same state on September 6;
  the final SNDDRV section below had omitted that later work.
  All 18 existing Windows CTest targets pass across the baseline run and a
  separate tools-test retry with temporary-directory access; the initial Python
  failure was a sandbox PermissionError. No new Linux verification was done.
- The current `iop_dmac.hpp` retains partial-qword lanes, consistent with
  `ORACLE.md`; the September 6 zero-fill entry below does not describe the
  current source. Do not reintroduce zero filling from that historical note.
- `--dump-iop` now also writes a RAM/metadata pair on a budget or diagnostic
  LOADFILE stop, with kind `native-iop-budget` or `native-iop-loadfile-stop`.
  Previously only faults produced a dump, losing evidence for RPC stalls.
  A fresh 30-million-slice run verified a 2 MiB `native-iop-budget` dump and
  parseable JSON at `%TEMP%/haunting-toc-probe/resume-worker.ram` plus `.json`.
- That trace identifies a repeated LIBSD SPU2 DMA interrupt/rearm cycle: the
  callback returns through `0x00031eb0`, CRI worker 19 consumes semaphore 7,
  and lower-priority RPC worker 18 cannot finish its request. The synchronous
  TSA-copy endpoint incorrectly accepted enabled AutoDMA sound input.
  `iop.hpp` now rejects nonzero per-core AutoDMA control before changing DMA
  registers, SPU RAM, counters, or completion flags. Both-core synthetic tests
  cover rejection without mutation and independence from the other core's mode.
  One-shot DMA remains supported; no new scheduling workaround or fabricated
  wake is used. The full Windows Release build and all 18/18 CTest targets pass
  after the changes (13.60 seconds for the final suite).
- The fixed connected diagnostic stops explicitly at IOP `0x00032140`, native
  time 13,090,065 us, worker 18: core 0, AutoDMA control 1, source `0x0001df80`,
  requested size `0x800`. Artifacts: `%TEMP%/haunting-toc-probe/autodma-boundary.log`
  and `autodma-boundary.ram` plus `.json`. This uses the quarantined synthetic
  TOC and is not a correctness or playability claim. The current selected
  generated bundle contains 24 modules / 68,828 reachable IOP words.
- Build-environment note: this session's MSBuild environment contained both
  `PATH` and `Path`, which caused CL task startup to fail. Launching CMake from
  the explicit Python 3.14 executable with `env={k.upper(): v for k,v in
  os.environ.items()}` removes duplicate case variants for that child process.
  `/m:1` succeeded; `/m:4` exited early in this environment. The Windows Store
  Python 3.11 also resolves TEMP differently; use the explicit Python 3.14
  executable for reading the external native captures. No machine settings
  were changed.

- Static decoding now separates synchronous SIFRPC completion from callback
  completion. `0x002700e8` takes the synchronous path when `mode & 1 == 0`,
  creates a semaphore, stores its ID at client `+0x08`, submits command
  `0x8000000a`, and waits on that ID. EE completion handler `0x0026fa60` obtains
  the returned client pointer from completion packet `+0x1c`; when client
  `+0x08` is nonnegative it signals that semaphore before releasing the request.
  Therefore IOP server `+0x30 == 0` is not itself evidence of corruption and
  must not be forced. The `--stop-on-loadfile-error` diagnostic now watches the
  constructor/completion/wait path and prints the completion packet plus its
  returned client record so the next run can identify the first bookkeeping
  divergence without changing guest-visible transport state. No build or test
  was run for this diagnostic-only checkpoint.

- Current LOADFILE/SIFRPC continuation narrowed the remaining completion defect
  further without reintroducing the rejected transport-level semaphore shortcut.
  Independent decoding of ROM SIFCMD confirms `rpc_call_request` at relocated
  `0x00099264` obtains the server record from packet `+0x34`, queues it through
  the server data queue at `server+0x40`, and copies the EE call metadata from
  packet offsets `+0x10`, `+0x14`, and `+0x1c..+0x30` into server offsets
  `+0x34`, `+0x20`, `+0x1c`, `+0x24`, `+0x0c`, and `+0x28..+0x30` before waking
  the worker.  The routine previously labelled `rpc_request_complete` at
  `0x00098bf0` is a distinct IOP-side request completion path, so the next exact
  target is the SIFCMD routine that constructs/emits EE completion command
  `0x80000008`.  Isolated `iop-emit` also exposed and fixed a tooling defect in
  `tools/hgtool/iop_build.py`: single-module `build()` referenced `placement`
  without retaining the plan; it now computes the plan once and passes it to
  `prepare()`.  No build/tests were run for this checkpoint.

- The connected startup no longer stops at the SNDDRV worker-stack boundary.
  Independent decoding of the supplied THREADMAN `CreateThread` export at
  relocated `0x0009ec90` shows its exact stack allocation call: SYSMEM ordinal
  4, mode 1, with the requested size rounded up to 256 bytes. Native workers now
  reserve that same bounded high-side SYSMEM arena instead of an artificial
  60 KiB pool. THREADMAN ordinal 5 is independently identified as
  `DeleteThread`; the native dormant-only adapter releases its stack and reuses
  the stable thread-table slot, with focused allocation/deletion/reuse tests.
  SNDDRV's four live worker entries (`+0x9fcc`, `+0xa400`, `+0xa4f4`, and
  `+0xa5e8`) are now explicit AOT roots. A headless 30-million-slice run using
  the quarantined synthetic TOC reaches the complete budget instead of a fault:
  all four workers execute, three settle into original wait paths, and the
  remaining worker services SIF traffic. This is diagnostic continuation only;
  the synthetic TOC is not correctness evidence. The selected **24-module**
  bundle contains **66,191 reachable IOP words**; EE reachability is **45,303
  words** with **316 unresolved** static boundaries. The full Windows Release
  build and all **18/18** CTest targets pass.

- Startup also gained independently implemented hardware subsets needed before
  that checkpoint: EE FPU add/multiply/MADD/MSUB use integer mantissa arithmetic
  with documented chop-toward-zero, signed-zero, saturation, and FCR31 flag
  behavior; EE DMAC global hold/read/edit/restart is retained at `D_ENABLER` and
  `D_ENABLEW`; and IPU reset plus BCLR/SETIQ/SETVQ/SETTH retain their documented
  FIFO/table/threshold state. No IPU decode/output or rendering behavior is
  claimed. `MODHSYN`, `MODMIDI`, `MODMSIN`, and `SNDDRV` are now statically
  planned and linked. The next connected investigation is the long-running EE
  wait at `0x0026c1e4` / original SIF traffic after SNDDRV startup, using a real
  TOC observation when available rather than promoting the synthetic record.

- The former **250,000,000-slice** checkpoint was not a stable service loop:
  independent ELF inspection identifies EE `0x001e50b0` as the deliberate
  fatal loop following `E0100301: SJX_Init can't allocate IOP Heap`. The FILEIO
  heap RPC reached original ROM SYSMEM ordinal 4 correctly, but ROM SYSMEM's
  allocator global at `0x000950c4` was never initialized. Connected bootstrap
  now invokes the original SYSMEM entry with the 2 MiB IOP RAM size before
  LOADCORE, then uses the original allocator to reserve the entire static AOT
  image through a generated, tested high-water-mark API (`0x000dd7a0`, rounded
  to `0x000dd800`). The returned reservation must begin at the initialized
  heap floor or startup faults. This runs on initial boot and after IOP reboot,
  preventing later allocations from overwriting compiled module data.

- With SYSMEM live, the headless 30-million-slice diagnostic advances beyond
  the SJX failure through CRI_ADXI command registration, a newly observed
  stream worker and its complete non-null method table. The selected 21-module
  bundle now contains **43,179 reachable IOP words**. LIBSD reaches the original
  SPU DMA channel: both core register banks are retained, and the exact observed
  `0x01000201` IOP-to-SPU profile copies a fully bounded BCR descriptor into a
  retained 2 MiB SPU2 local-memory image using the original TSA registers,
  advances MADR, clears START, and records completion. Other modes and
  out-of-range transfers fault explicitly; audio synthesis/output is not yet
  implemented. This exposed and translated the EE formatter's checked 89-entry
  jump table plus live EE object callbacks at `0x001e41b0`, `0x001eae58`,
  `0x001ed1a0`, and `0x001ed1f8`. After those translated successfully, the
  subsequent native runs passed checked service-command and constructor
  callbacks through `0x001e96d8`; the newest stop is the clean object
  constructor entry at `0x002266f0`, now added as the unverified continuation.
  EE reachability is now **42,749 words** with that final root regenerated;
  executing it is the next connected check.

- The same path also covers three compatibility defects: FILEIO's original
  LOADCORE link includes its SYSMEM import; suspending an existing dormant EE
  thread returns `-1` without mutation while invalid/nonexistent targets still
  fault; and EE DECI2 operation `0x10` has a checked, bounded `kputs` diagnostic
  sink. Other DECI2 operations remain explicit faults. The DECI2 return value
  is a native compatibility policy pending original-hardware observation.
  Diagnostic output reports only call/byte counts, SIF0 history marks truncated
  captures, and EE SifSetDma descriptors are retained in a bounded trace.
  Original CDVDFSV then returned a nine-word SIF0 payload; arbitrary nonzero
  partial-qword word counts now use the same bounded retained-lane policy as
  the already-tested one-, two-, and six-word replies instead of rejecting the
  valid transfer shape.
  The full Windows Release build and all 18 CTest targets pass at this
  checkpoint.

- Connected startup now loads and executes `SIO2MAN`, `SIO2D`, `DBCMAN`,
  `LIBSD`, and `CRI_ADXI` in order. DBCMAN's large 2,048-word SIF0 transfer no
  longer aborts at the 16-word diagnostic history limit: transport completes
  while history records its total size and explicit truncation. DMAC2 now
  retains the otherwise-unused `0x1f801560..568` bank and implements DICR2
  channel-7..12 completion flags plus write-one acknowledgement; this lets the
  original SIFMAN DMA-status path complete. LIBSD initializes checked SPU2 bus
  mappings/delays and a retained two-core halfword control-register bank;
  sample DMA and audio synthesis remain explicit future boundaries. CRI_ADXI
  exposed and now tests the R3000 rule that a direct write or newer load in a
  load-delay slot cancels the older delayed write. Its observed worker roots are
  relocated `+0x88` and `+0x7d4`. The selected bundle is 21 modules and 40,420
  reachable words. Full Windows Release build and all 18 CTest targets pass.
  Both CRI_ADXI workers now execute and the connected system has returned to
  EE startup. Independently decoded dispatch targets `0x001df068`, `0x001df220`,
  `0x001dac20`, and `0x001dae68` are now explicit EE AOT roots; the first three
  have executed live in order. The next boundary is the stream-state callback
  at `0x001dae68`. The synthetic TOC remains diagnostic-only and is not
  correctness evidence.

- The second LOADFILE deadlock is fixed. Native `StartThread` now restarts a
  completed/dormant IOP worker with a fresh CPU context and new argument while
  reusing its already reserved stack; it rejects blocked/non-dormant targets.
  A focused regression test covers return-to-sentinel, restart, argument reset,
  and stable stack reservation. Consequently `SIO2D.IRX` links, returns result
  0, and runs its original worker at relocated `+0xae4`. Startup then requests,
  statically allocates, and links `DBCMAN.IRX`; its observed workers at `+0x1d94`,
  `+0x1e2c`, and `+0x1ec4` and SIF callbacks at `+0x1c88`, `+0x1cc8`, and
  `+0x1d68` are explicit AOT roots.
  The selected bundle is now 19 modules and 33,507 reachable words. The full
  Windows Release build and all 18 CTest
  targets pass. The next boundary is execution through DBCMAN's original SIF
  callback; the synthetic TOC remains diagnostic-only and is not correctness
  evidence.

- The connected startup now completes the first real post-reboot module load:
  `SIO2MAN.IRX` returns LOADFILE result **0**, executes its translated module
  entry at static `0x0007a000`, starts its original worker at `0x0007a39c`, and
  the EE submits the next request for `SIO2D.IRX`. This advances the prior
  `-203 -> -400 -> -200` sequence through checked ISO open/read, bounded SYSMEM
  allocation, relocation-range validation, and generated static import linking.
  MODLOAD's observed 0x30-byte allocation prefix is represented explicitly;
  unknown link ranges and modules with unresolved imports still fault. The
  runtime now models the documented SIO2 register window and keeps controller,
  memory-card, FIFO-transfer, and SECRMAN authentication operations as explicit
  boundaries. The 18-module bundle (including SIO2D) contains 32,238 reachable
  words. The next active defect is scheduler/transport lifecycle after the first
  successful LOADFILE reply: MODLOAD waits on event 3 for the SIO2D request, but
  no second wake is delivered despite the consumed SIF1 packet. The synthetic
  TOC remains diagnostic-only and is not correctness evidence.

- A targeted native trace now resolves the post-GetToc LOADFILE failure through
  the original IOMAN dispatch. MODLOAD calls IOMAN ordinal 4 with the correct
  path and flags; IOMAN strips the `cdrom0:` prefix, selects the registered
  `cdrom` descriptor, allocates descriptor slot `0x000a8080`, and calls the
  CDVDMAN open operation at static `0x000b39e0`. That operation returns `-2` at
  `0x000b3ed0`, which MODLOAD maps to `-203`. The deliberately synthetic TOC
  probe contains no valid filesystem catalogue, so this result is expected and
  is not evidence of a path-parser or scheduler defect. The next implementation
  boundary is a checked ISO-backed CDVDMAN open/read compatibility path derived
  from the original request ABI; the synthetic TOC must not be promoted into a
  correctness input.

- The post-GetToc CDVD interrupt path is now statically reachable. The supplied
  CDVDMAN setup at `+0x505c` passes module `+0x4288` while registering hardware
  interrupt cause 2, so that exact callback is now an explicit AOT root. Its
  first live execution exposed the original read of byte register `0x1f402013`;
  the synchronous native endpoint now reports a checked zero idle/decoder state
  and records the access in the MMIO provenance ring. A deliberately synthetic,
  non-promotable TOC probe kept outside the repository then completed the IRQ,
  advanced through the original CDVD request machinery, and reached the real
  `cdrom0:\\MODULES\\SIO2MAN.IRX` LOADFILE request and existing `-203` result.
  This proves the missing IRQ root/register were earlier startup boundaries and
  that the next failure is in MODLOAD's original device-open path; it does not
  validate or replace a real TOC record. The expanded 16-module bundle has
  32,606 reachable words, 41 explicit indirect-transfer boundaries, 95 native
  adapter sites, and 11 BREAK traps. Full Windows Release CTest passes 18/18.

- The configured 16-module IOP startup bundle now has **zero residual
  syscalls**. Independent decoding of the user's external BIOS-derived IOP RAM
  established that selector 32 enters the kernel context-transfer handler at
  `0x5830`, which saves a complete thread frame, calls the registered callback
  at `0x59d0` (live value `0x0000c368`), and restores the frame it returns. The
  supplied game THREADMAN wrapper at static `0x000a4640` selects its ordinary
  (`global+0x5c`) or interrupt/preemption (`global+0x60`) scheduling slot from
  `a3`, stores it at `global+0x54`, and invokes selector 32 at `0x000a465c`.
  That exact site is now a checked native reschedule boundary for the existing
  cooperative thread contexts, including explicit cursor advance, lock release,
  and host-only provenance; it is not a returning no-op. Regeneration yields
  31,448 reachable words, 36 residual indirect transfers, 95 native adapter
  sites, and no residual syscall evidence. The full Windows Release build and
  all 18 CTest targets pass. A fresh connected 20-million-slice run reaches the
  unchanged honest GetToc gate (`MADR=0x000c3464`, `BCR=0x00810004`,
  `CHCR=0x41000200`), with no earlier startup regression.

- The external memory-oracle helper now recognizes the original CDVDMAN
  GetToc call frame from two independently decoded facts: the exact
  `4 x 0x81` channel-3 descriptor and saved return to module `+0x70c4`.
  It verifies the supplied module on every full IOP-RAM sample, follows its
  observed reboot relocation (`0x120370` to `0x22830`), records generic CDVD
  frames separately, and writes candidate bytes only outside the repository.
  Synthetic exact/mismatched-frame tests pass. A fresh same-operation launch
  sampled complete IOP RAM 10,111 times over 25 seconds and observed both
  module placements but no CDVD request frame, so it produced no candidate and
  did not weaken the runtime's explicit GetToc gate. The next oracle attempt
  must use PCSX2's documented `-debugger` entry break and R3000 breakpoint, or
  another timing-controlled guest-register observation; blind host scans remain
  disproven. The full Windows Release build and all 18 CTest targets pass after
  this tooling change. Regenerating the configured 16-module startup bundle
  yields 31,449 reachable IOP words, 37 residual boundaries, 94 native adapter
  sites, and 10 explicit BREAK traps. A fresh 20-million-slice connected run
  reaches the unchanged honest GetToc gate (`MADR=0x000c3464`,
  `BCR=0x00810004`, `CHCR=0x41000200`).

- The OpenGL 3.3 host no longer ends at a disconnected colored-triangle probe.
  Its verified path now creates swizzled CT32 GS local memory, selects it through
  PMODE/DISPFB1/DISPLAY1, extracts the PCRTC source rectangle to portable RGBA,
  uploads that image as an OpenGL texture, and presents a full-window quad. An
  exact center-pixel readback verifies `RGBA=(0x80,0xc0,0x40,0xff)` after the
  complete GS-to-host path. On this Windows/NVIDIA host, a three-frame run reports
  `OpenGL: 3.3.0 NVIDIA 610.88`, presents three frames, and succeeds. The full
  Windows Release build is also **18/18 CTest passing**. The source is still synthetic rather than game-run
  state, so this is a scanout integration checkpoint, not playability evidence.

- A single enabled PCRTC read circuit can now produce a bounded host RGBA image
  directly from game-written GS local memory. The path decodes DISPFB FBP/FBW/
  PSM/DBX/DBY and DISPLAY DX/DY/MAGH/MAGV/DW/DH, validates integral source
  dimensions, reads the correct swizzled PSMCT32/24/16/16S layout, and applies
  the documented display alpha expansion. Tests cover magnified offset CT32,
  CT24 fixed alpha, CT16S RGB5/A1 expansion, and explicit dual-circuit rejection.
  PCRTC circuit blending and PS-GPU24 remain checked presentation boundaries.

- EE `LD`/`SD` now reaches the GS privileged range at `0x12000000` through
  `0x13ffffff` with the documented 64-bit-only width, KSEG aliases, and address
  mirrors. Writes cover PMODE, SMODE2, both DISPFB/DISPLAY circuits, BGCOLOR,
  IMR, BUSDIR, and CSR event clearing; reads cover CSR and SIGLBLID. SIGNAL and
  LABEL apply their 32-bit ID masks, FINISH drains earlier deferred draws, and
  newly unmasked GS events latch EE INTC cause 0 through PATH2/PATH3 or IMR
  writes. IMR resets to all five events masked. Tests cover mirrors, invalid
  widths/BUSDIR, event IDs and clearing, FIFO-empty status, and interrupt latching.
  CSR FLUSH/RESET, second-SIGNAL stall/resume, and local-to-host FIFO data remain
  explicit faults rather than guessed behavior.
  The full Windows Release build is **18/18 CTest passing**. A fresh connected
  20-million-slice run still reaches the exact GetToc record gate at IOP
  `0x1f0000` (`MADR=799844`, `BCR=8454148`, `CHCR=1090519552`), proving the new
  MMIO routing introduces no earlier startup regression.

- Host-to-local IMAGE uploads now reach PSMZ32, PSMZ24, PSMZ16, and PSMZ16S
  through the existing independent Z swizzles. The 32/16-bit HWREG path shares
  the checked raw-pixel writer with local copies, while Z24 uses the documented
  five-RGB-pixels-per-qword packing and preserves its unused storage byte. Tests
  exercise all four depth layouts at valid transfer dimensions.

- Host-to-local and local-to-local GS transfer setup now validates BITBLTBUF
  widths (including the narrower 1..32 encoded range), per-format TRXREG width,
  and indexed-format start-X alignment before any image write or local copy can
  occur. Existing IMAGE tests were converted from convenient but invalid small
  rectangles to documented CT24/CT16/PSMT8/PSMT4/PSMT8H/PSMT4H dimensions.
  A malformed indexed local copy now proves atomic rejection with VRAM intact.

- GS `TRXDIR` now executes checked synchronous same-depth local-to-local transfers
  across every implemented layout: CT32/Z32, CT24/Z24, CT16/CT16S/Z16/Z16S,
  PSMT8/PSMT8H, and PSMT4/PSMT4HL/PSMT4HH. This includes all four
  TRXPOS.DIR traversal orders needed to preserve intentional
  overlapping copies. Source/destination depths and widths plus documented
  width/destination-X alignments are validated; XDIR=3 terminates an active
  transfer without fabricating work. The local-to-host FIFO remains an explicit
  fault. Tests cover cross-layout conversions at 32/24/16/8/4 bits, mismatched
  depth rejection, a one-pixel overlapping right shift, and deactivation.
  The full Windows Release build remains **18/18 CTest passing**. A fresh
  `hg_system_diagnostic --slices 20000000` run reaches the unchanged honest
  external GetToc-record gate at IOP PC `0x1f0000`, with `MADR=799844`,
  `BCR=8454148`, and `CHCR=1090519552`; no record was fabricated.

- Sprite rasterization now follows the GS's fixed primitive attributes: shading
  remains flat and antialiasing remains off even if IIP/AA1 bits are set. It no
  longer reports a false unsupported boundary when the two queued vertex colors
  differ, and a synthetic rectangle proves the drawing-kick vertex supplies the
  flat color under both otherwise-inapplicable bits.

- Deferred GS draws now snapshot the complete 0x80-register drawing environment
  at each drawing kick, rather than consulting later texture, fog, test, blend,
  frame, or depth state during rasterization. The rasterizer installs that
  snapshot only for the draw and restores the live environment on success or
  explicit failure. `TRXDIR` is also an ordering barrier that commits all prior
  draws before host-to-local IMAGE data can alter shared frame, depth, texture,
  or CLUT storage. End-to-end tests change FOGCOL and FRAME after a point kick,
  then overwrite a later queued point through IMAGE and verify both orderings.

- GS drawing kicks now honor `PRMODECONT.AC`: PRIM always supplies primitive
  type, while attributes come from PRIM or PRMODE as selected. The resulting
  context chooses the captured XYOFFSET, and the complete effective primitive
  state travels with deferred draws into framebuffer blending. Tests cover both
  AC modes, context selection, and opposing current-versus-captured ABE values,
  closing a silent state-retiming bug when registers change before rasterization.

- GS `TEX2_1/2` A+D writes now update the documented PSM/CBP/CPSM/CSM/CSA/CLD
  subset of their corresponding `TEX0_1/2` state. TEX0-only base/width,
  dimensions, component, and texture-function fields are preserved. Tests cover
  both contexts and prove that a TEX2-only PSM change reaches actual texture
  extraction, closing a silent stale-state path used by palette/format switches.

- The read-only oracle helper can now repeatedly scan a cached non-executable
  host-region map for the exact GetToc channel-3 descriptor and, on a hit,
  immediately capture the detected guest MADR twice through a separately
  supplied-module-verified IOP mapping. A fresh 60.109-second batch run found
  CDVDMAN at guest `0x22904`, completed seven passes / 9,104,101,668 bytes, and
  observed no raw descriptor; no candidate was written and PCSX2 was stopped.
  This rejects further blind raw-triple scans for the current profile. The next
  capture must obtain MADR from a same-stage guest-register/debugger observation.

- GS line diamond clipping no longer depends on host `double`. Exact rational
  enter/leave intervals now implement the included-start/excluded-end rule from
  the original 12.4 coordinates, while the existing dominant-axis DDA remains
  responsible for attributes. Forward, reversed, and fractional-endpoint 45°
  cases verify identical half-open coverage. This removes a cross-host rounding
  source before antialiased-line coverage is added.

- Perspective STQ now works for varying-Q sprites, Line/LineStrip draws, and
  triangles. The rasterizers compute `(sum(weight*S))/(sum(weight*Q))` and the
  corresponding T ratio directly, cancelling the common interpolation
  denominator and retaining exact signed mantissa/exponent arithmetic. Tests
  distinguish varying-Q perspective division from affine shared-Q sampling and
  exercise every supported multi-vertex primitive without host floating point.

- FST-clear GS Line/LineStrip draws now support exact STQ sampling when both
  vertices share Q. Signed dominant-axis weights preserve the existing DDA at
  diamond-covered samples, including limited endpoint extrapolation, and feed
  the same mantissa/exponent ratio used by sprites and triangles. Direct signed
  extrapolation and two diagonal texels are tested. This shared-Q checkpoint is
  superseded by the varying-Q perspective path above.

- FST-clear GS triangles now support exact STQ sampling when all three vertices
  share Q. Their existing integer coverage edge weights now also form weighted
  S and T mantissa/exponent sums; the common barycentric denominator cancels
  through Q division before the checked texture sampler. Tests cover three
  distinct texels. This shared-Q checkpoint is superseded by the varying-Q
  perspective path above.

- FST-clear GS sprites now support exact STQ sampling when both vertices share
  Q. The two rectangle axes interpolate S and T as exact signed mantissa/exponent
  ratios, cancel the interpolation denominator through perspective division,
  and feed the existing checked TEX0/CLAMP/TFX sampler without host floating
  point. Tests cover direct positive and negative ratios, a four-texel 2x2
  sprite. This shared-Q checkpoint is superseded by the varying-Q perspective
  path above.

- Textured GS points now accept FST-clear STQ coordinates. The point path derives
  normalized `S/Q * width` and `T/Q * height` texels with exact IEEE-single bit
  decomposition and integer ratios, explicitly rejecting non-finite values,
  zero Q, invalid dimensions, and signed-coordinate overflow. Direct ratio edge
  cases and an end-to-end 2x2 texture sample are covered; multi-vertex
  perspective STQ is now covered by the checkpoint above.

- GS draw vertices now retain the raw per-vertex `RGBAQ.Q` value alongside ST
  and UV. PACKED STQ's GIF-Q update followed by RGBAQ and a vertex kick is
  covered with a non-default `Q=2.0` bit pattern. This closes the state-loss gap
  that enabled the perspective interpolation now implemented above.

- GS Line/LineStrip varying attributes now extend from axis-aligned draws to
  diagonal diamond traversal using the dominant-axis DDA parameter. Forward and
  reversed 45-degree gradients verify Gouraud RGBA, depth, included start and
  excluded endpoint; a diagonal FST line verifies two different texels.
  Antialiasing remains the explicit line-coordinate boundary.

- Axis-aligned GS Line/LineStrip draws now interpolate Gouraud RGBA, unsigned
  depth, fog, and FST UV along the exact horizontal or vertical DDA parameter.
  Tests cover horizontal and vertical color/depth gradients, fog gradients,
  two distinct texture samples, endpoint exclusion, and explicit rejection of
  a varying diagonal. Constant-attribute diagonal diamond coverage is unchanged.

- GS triangle Z now varies barycentrically with coverage, UV, Gouraud color,
  and fog instead of requiring identical endpoint depth. The exact unsigned
  weighted sum is proven to fit 64 bits from the GS's 16-bit X/Y domain; tests
  cover ordinary midpoints and a near-maximum-area triangle with `Z=0xffffffff`.

- GS triangles now barycentrically interpolate Gouraud RGBA and fog with the
  same integer edge weights used for FST UV and coverage. Tests verify exact
  vertex and midpoint color/fog results. Flat triangles still use the original
  drawing-kick vertex.

- FST textured triangles now barycentrically interpolate unsigned 10.4 UV at
  every covered pixel instead of accepting only identical endpoint UVs.
  Winding normalization swaps both geometry and vertex attributes while flat
  shading still uses the original drawing-kick vertex. Distinct 2x2 texels and
  both winding orders are covered.

- The GS sprite rasterizer now supports FST textured rectangles: it linearly
  interpolates the two unsigned 10.4 UV endpoints over the original unclipped
  rectangle axes, samples through the existing checked TEX0/CLAMP/TFX path, and
  preserves the second vertex's depth and flat color. A four-texel 2x2 case
  verifies both axes. Varying Gouraud color still faults.

- Rejected the tempting but invalid assumption that native GetToc MADR
  `0x000c3464` can be relocated with CDVDMAN's module base. A fresh 45-second
  read-only oracle run at the derived guest `0x33c94` observed stable executable
  code, not a TOC record. Original translated CDVDMAN shows channel-3 MADR is
  loaded from request-descriptor field `+4`, so it is caller-provided.
  `oracle_capture.py` now requires an explicit same-command-stage
  `--iop-toc-offset` and never derives it from a module anchor.

- Added a read-only `--locate-iop-toc-dma` oracle mode that searches
  non-executable host mappings for the exact active channel-3
  `BCR=0x00810004`/`CHCR=0x41000200` pair and validates the preceding MADR as an
  aligned 2 MiB IOP range. A fresh post-boot snapshot scanned 1.40 GB and found
  no persistent descriptor, proving this must be used at a debugger stop during
  the original active command stage rather than after normal startup has passed it.
  A second scan begun immediately after a fresh process launch covered 803.8 MB
  during startup and also found none; both oracle processes were explicitly stopped.

- The translated EE runtime now supports up to 64 simultaneous read-only PC
  trace watches via repeated `--watch-ee-pc`. Each watch snapshots a bounded
  64-instruction history with the indirect-target registers, arguments, return
  address, stack/GP, and callee-saved state; an empty watch set adds only one
  branch per translated instruction and never changes guest state. Five fresh
  20-million-slice batches covered all 288 current unresolved EE indirect sites.
  None executes before the external GetToc-record gate, establishing that target
  discovery for those sites requires post-gate execution rather than more
  pre-gate startup runs. Runtime and emitter tests cover watch deduplication,
  snapshot contents, and generated instrumentation; all 18 CTest targets pass.

- `SetGsCrt` (`0x02`) now records a validated interlace/display-mode/frame
  request in explicit PCRTC state; it is no longer an analysis boundary or a
  discarded display setup call. Invalid boolean fields and out-of-range mode
  selectors fault. The authoritative EE scan is now 37,940 words / 295
  boundaries (288 indirect, six syscalls, one exception return). Focused kernel
  and startup tests pass.

- The EE GS kernel boundary now implements `GsGetIMR`/`GsPutIMR`
  (`0x70`/`0x71`) against explicit 64-bit privileged GS state rather than an
  ignored write. The public put ABI leaves `v0` untouched; get returns the exact
  stored value. The authoritative EE scan is now 37,938 words / 296 boundaries
  (288 indirect, seven syscalls, one exception return). Focused kernel and
  startup tests pass.

- EE SIF kernel support now includes `SifDmaStat` (`0x76`) and the interrupt
  aliases `isceSifSetDma`/`isceSifDmaStat` (`-0x77`/`-0x76`). The bounded native
  profile tracks its active transfer ID, reports in-progress versus completed,
  reuses the existing snapshotted packet submission, and rejects invalid IDs or
  unsupported queueing. EE interrupt services now also include checked
  `RemoveIntcHandler` (`0x11`), `DisableIntc` (`0x15`), and interrupt-context
  `GetThreadId`/`iWakeupThread` (`-0x2f`/`-0x34`). The authoritative EE scan is
  now 37,936 words / 297 boundaries (288 indirect, eight syscalls, one exception
  return), with the IOP result unchanged at 55 all-module residuals. Focused
  kernel/startup tests pass.

- EE kernel startup now implements checked `StartThread` (`0x22`),
  `ExitThread` (`0x23`), `ReferThreadStatus` (`0x30`), `WakeupThread` (`0x33`),
  `CancelWakeupThread` (`0x35`), `SuspendThread` (`0x37`), `ResumeThread`
  (`0x39`), `RotateThreadReadyQueue` (`0x2b`), and `GetMemorySize` (`0x7f`)
  plus a bounded cooperative worker dispatcher.
  A created worker receives the original entry, argument, GP, aligned stack top,
  and a host-only completion sentinel in an isolated full EE CPU context. Ready
  workers cannot run while the root thread owns the CPU; after an exit, the
  highest-priority ready worker advances one translated instruction per service
  call. Invalid IDs, repeated starts, interrupt-context calls, and exit without
  a running thread fault explicitly. Status queries serialize the documented
  0x30-byte thread record; wakeups release a waiting thread or increment its
  bounded pending count; memory size comes from the runtime's actual RAM vector.
  Wakeup behavior distinguishes sleep from semaphore waits and preserves the
  documented WAIT/WAITSUSPEND/SUSPEND transitions; cancellation returns and
  clears the previous wakeup count. Equal-priority rotation advances the
  dispatch cursor and never schedules a worker while another thread owns the
  CPU. These returning services reduce the EE inventory to 37,890 words / 303
  boundaries (288 indirect, 14 syscall, one exception return). Kernel tests
  cover start, ownership gating, context dispatch, exit, status, wakeup, memory
  size, and root-context restoration. The Windows Release build and all 18 CTest
  targets pass, and a fresh 20-million-slice connected run still reaches the
  exact external GetToc-record boundary.

- Four EE indirect call sites are now rooted solely from exact executable
  pointers already present in the original file-backed image: callback wrappers
  at `0x001009b0`, `0x001009e0`, and `0x00100a10`, plus the method descriptor
  call at `0x001e6c34`. Runtime dispatch remains register-selected and any value
  outside the configured sets still faults. The locally proven `StartThread`
  selector (`0x22`) is also classified as returning for discovery only; its
  unimplemented runtime service still faults explicitly. Together this opened
  3,256 additional EE words (37,880 total) and leaves 308 explicit boundaries:
  288 indirect transfers, 19 syscalls, and one exception return. The authoritative
  IOP result remains 76,531 words / 534 isolated boundaries and 55 all-module
  residuals. The pointer-candidate triage now suppresses already configured
  sites, leaving zero genuinely new file-backed candidates instead of re-listing
  the four resolved calls. Focused EE tooling tests pass (28 discovery/config
  and one triage),
  the Windows Release build succeeds, and all 18 CTest targets pass.

- `oracle_capture.py` now has a bounded continuous GetToc-range monitor that
  waits for a supplied-module anchor, retains only states confirmed by two
  consecutive identical reads, caps unique output, and records final anchor
  stability. Fresh 15-second (2.43 million reads) and 45-second (7.91 million
  reads) PCSX2 memory-oracle runs observed only zeroing, allocator fill, and
  request-metadata initialization at `0x000c3464`, including across an IOP
  lifecycle relocation. Those repeatable hashes are explicitly rejected; no
  captured state was admitted as a TOC record or copied into the repository.

- Native trace watches now use bounded heap-backed storage and accept up to 64
  sites, allowing all 56 then-current dynamic boundaries to be observed in one
  20-million-slice startup run. Four executed before GetToc and yielded guarded
  targets: LOADCORE `+0x076c` to EESYNC `+0x0080`, SIFCMD `+0x18b4` to LOADFILE
  `+0x057c`, SIFCMD `+0x0804` to SIFCMD `+0x1264`, and MODLOAD `+0x2910` to
  MODLOAD `+0x295c`. Unknown values still fault. The audit now has 55 residual
  boundaries (52 dynamic transfers, two SECRMAN imports, and THREADMAN syscall
  32) plus 193 BREAK traps. The external capture and hashes are recorded in
  `docs/ORACLE.md`; no captured bytes entered the repository. The regenerated
  Windows Release build, all 18 CTest targets, focused 19/19 IOP tooling tests,
  and the unchanged GetToc-boundary startup run pass.

- ROM MODLOAD syscall selector 12 is now a narrowly configured checked AOT
  transfer matching the original `CpuInvokeInKmode` ABI: the function address
  comes from `a0`, arguments shift from `a1`/`a2`/`a3`, and the caller's `ra`
  remains the return path.  The planner accepts this only when the named site
  decodes as `syscall` and the immediately preceding original instruction sets
  `v0=12`; unsupported selectors still fault.  The audit now has 59 residual
  boundaries (56 dynamic transfers, two SECRMAN authentication imports, and
  THREADMAN syscall 32), 130 native adapters, and 193 defined BREAK traps.
  Focused IOP tooling tests pass 19/19, the Windows Release build and all 18
  CTest targets pass, and the 20-million-slice startup run still reaches the
  exact external GetToc-record boundary.

- Three further original-byte switches are now statically rooted: LIBSD
  `+0x3470` (20 entries), MODHSYN `+0xb704` (14 entries), and ROM MODLOAD
  `+0x14c8` (7 entries).  Each selector has an independently decoded bounds
  check and every relocated table word resolves into its module's executable
  sections.  The comprehensive audit now covers 76,532 IOP words with 60
  residual boundaries (56 dynamic transfers, two SECRMAN authentication
  imports, and two syscalls) plus 193 defined BREAK traps.  Focused IOP tests
  pass 17/17, the Windows Release build and all 18 CTest targets pass, and a
  fresh 20-million-slice connected run reaches the unchanged explicit GetToc
  record boundary (`MADR=0x000c3464`, `BCR=0x00810004`,
  `CHCR=0x41000200`).

- The two remaining SECRMAN 1.3 ordinal-6 imports were traced through original
  callers.  SIO2D forwards three caller arguments unchanged; MCMAN supplies a
  port-like index, slot/context, and a prior helper result.  This confirms that
  the boundary needs actual card/SIO2 authentication semantics, not merely a
  statically discoverable function target.  It continues to fault explicitly
  rather than fabricating authentication success.

- `hg_system_diagnostic --dump-iop <external-path>` now writes an adjacent JSON
  sidecar with the native fault PC, all 32 IOP GPRs, HI/LO, delayed-load state,
  current thread, and virtual time.  This preserves the missing base-register
  provenance needed to validate future mutable callback targets while keeping
  both RAM and metadata outside the repository.  A fresh GetToc-boundary dump
  was parsed successfully (`pc=0x1f0000`, 32 GPRs, exact 2 MiB RAM).  Watch
  `0x000a9ca0` also retained a full 64-record history with the selected dynamic
  target/base register set and exact `sp=0x001f8dc0`.  All 18 Windows Release
  CTest targets pass.

- A batched original-byte selector pass adds 19 independently bounded relocated
  dispatches across MODHSYN, MODMIDI, LIBSD, CDVDMAN, THREADMAN, FILEIO,
  TIMEMANI, and MCMAN.  All configured table entries are checked executable
  targets.  MODHSYN `+0x9918` compiles only its six valid targets, preserving an
  explicit fault for selector 10's non-code value.  The comprehensive audit now
  covers 76,202 IOP words with 63 residual boundaries (59 dynamic transfers,
  two SECRMAN authentication imports, and two syscalls) and 183 defined BREAK
  traps.  Focused IOP tooling tests, the Windows Release build, and all 18 CTest
  targets pass.  A fresh
  20-million-slice connected run reaches the unchanged explicit GetToc-record
  requirement.

- MODLOAD's `+0x0bbc` indirect tail transfer is now rooted from its independently
  decoded eight-entry, bounds-checked relocated read-only table at `+0x35b0`.
  This expands the all-module audit to 72,415 IOP words.  It intentionally
  exposes two additional runtime callbacks and the selector-12 syscall, leaving
  76 residual boundaries (72 dynamic transfers, two SECRMAN authentication
  imports, and two syscalls) plus 152 defined BREAK traps.  Focused IOP tooling
  tests and all 18 Windows Release CTest targets pass.  A fresh 20-million-slice
  connected diagnostic stops at the same explicit GetToc-record boundary.

- A fresh comprehensive scan covers 34,624 EE words and 71,977 IOP words.
  Module-qualified, executable-range-checked targets compile all 24 live IOMAN
  operation sites for the native `tty`/`cdrom` driver set. The all-module
  residual count falls from 97 to **75**: 71 dynamic transfers, two SECRMAN
  authentication imports, and two syscalls. All 152 BREAK traps remain separate.
  The exact compact inventory is in `docs/DECODING_SCAN.md`.
- `hg_system_diagnostic --dump-iop <external-path>` writes exact native IOP RAM
  on a startup fault only outside the repository. The GetToc-boundary dump
  identified the original `tty` and `cdrom` descriptors and live operation
  vectors; it is external runtime state, not committed input or emulator code.
- After adding those guarded callback roots, the Windows Release suite is
  **18/18 passing**. A fresh 20M-slice connected run reaches the unchanged honest
  GetToc boundary at `MADR=0x000c3464`, `BCR=0x00810004`,
  `CHCR=0x41000200`; the game does not open yet.

- The checked GS IMAGE endpoint now accepts PSMCT16S's packed 16-bit payload
  and the documented PSMT8H/PSMT4HL/PSMT4HH packed byte/nibble payloads, writing
  only their CT32 high-byte/high-nibble lanes. Focused GIF coverage passes.
- The documented startup bundle now contains 16 modules, including required
  `rom_timemani`; it has 28,024 reachable IOP words. The original SIF, FILEIO,
  CDVD, LOADFILE, and reboot diagnostics all pass, as does the full **18/18
  Release CTest** batch. These are startup-path checks only, not playability
  evidence.
- A fresh 20M-slice run of that bundle reaches the original CDVD GetToc request
  and explicitly stops for the required external 2,064-byte record:
  `MADR=0x000c3464`, `BCR=0x00810004`, `CHCR=0x41000200`. No ISO-derived
  replacement was accepted or generated.
- A new bounded PCSX2 batch observation took two coherent, CDVDMAN-anchored IOP
  maps after approximately 20 and 40 seconds. Their full-map hashes differ, but
  both extracted GetToc candidates retain the previously rejected pre-command
  hash. The records remain ineligible and the launched process was stopped.
- `oracle_capture.py` now marks that recorded pre-command candidate hash as
  rejected in its sidecar metadata instead of leaving it generically labeled as
  a candidate. This records negative provenance without retaining captured data
  and prevents accidental use as `--toc-record`; focused tool and CDVD tests
  pass.
- The 2 MiB module-anchored oracle mapping is IOP RAM only, not the CDVD MMIO
  aperture. Prior similarly numbered RAM-offset values are therefore not used as
  CDVD register evidence; future candidates require a separately valid hardware
  register observation. This correction removes a false provenance signal rather
  than synthesizing a transfer or decoder dependency.
- GS LINE and LINESTRIP now rasterize their documented diamond coverage with
  included start and excluded endpoint behavior for constant-attribute,
  non-antialiased draws. Varying DDA attributes remain explicit faults; focused
  horizontal/vertical endpoint and flat-color tests pass.
- VIF1 now writes documented S-32/S-16/S-8 and V4-32/V4-16/V4-8/V4-5 UNPACK
  forms into bounded VU1 data memory, including scalar replication, split
  DMA-qword payloads, signed or unsigned expansion, masked/fill CYCLE behavior,
  and FLG-relative TOPS destinations. V2/V3 forms are also accepted only when
  their documented indeterminate components are explicitly masked, filled, or
  preserved. This is data transport only: unguarded V2/V3 and MSCAL/VU execution
  remain explicit faults.
- COP2/VU0 macro-mode `VABS` now has an exact decoder match and selected-lane
  raw-bit sign clearing, preserving destination lanes and NaN payloads without
  host floating-point behavior. Macro arithmetic, flags, hazards, and every
  non-modeled macro encoding remain explicit faults; focused decoder and AOT
  translation checks pass.
- V4 writes now implement normal, offset, and difference STMOD behavior whenever
  the selected mask lane accepts input, including documented difference-mode Row
  updates. Fill lanes still require Row/Col/preserve selectors because no input
  exists to add.
- VIF1 now preserves documented STROW/STCOL four-word state across split DMA
  payloads, preparing the Row/Col data required by later masked/fill UNPACK.
- VIF1 applies STMASK's documented input/Row/Col/preserve selectors to V4
  UNPACK, including masked `CL < WL` fill cycles with their reduced payload
  length. A fill slot requesting unavailable input faults explicitly.
- VIF1 OFFSET now establishes the documented initial TOPS from BASE, allowing
  checked FLG-relative UNPACK destinations before any unimplemented MSCAL
  buffer toggle.
- VIF1 now accepts documented MPG uploads into bounded VU1 MicroMem, retaining
  split-DMA 64-bit instruction pairs without executing them. MSCAL-family
  activation remains an explicit fault until a separate AOT VU1 path exists.
- `analyze --triage` additionally coalesces each boundary by reason, decoded
  mnemonic, and target register. This leaves the discovery graph untouched but
  exposes repeated dispatch-table shapes as one work item, avoiding repeated
  per-site investigation.
- Its indirect-call queue also groups the two preceding decoded instructions.
  This is read-only setup evidence, not target inference: it distinguishes
  repeated pointer/table load shapes from other dynamic dispatch before a
  representative caller is inspected.
- The triage queue now separately identifies an indirect target loaded from a
  base that was itself loaded from memory. This creates one bounded inspection
  item for repeated table-dispatch chains while preserving every site and
  refusing to infer any targets. Each class now supplies three deterministic
  representative PCs for read-only inspection.
- `iop-bundle` now separates guarded imports linked within the selected static
  bundle from residual discovery boundaries. This avoids treating already
  emitted provider links as missing work while preserving every unlinked import,
  indirect transfer, and break as an explicit report item. The 16-module
  startup bundle reports 332 guarded links and 82 residual boundaries (69
  indirect transfers, 10 breaks, two syscalls, and one unprovided import);
  the Release suite is **18/18 CTest passing**.
- INTRMAN ordinal 8 is now an independently modeled `CpuDisableIntr` adapter.
  It disables CPU interrupt delivery while preserving per-cause masks and
  returns success; its public ABI declaration removes THREADMAN's former
  unprovided-import boundary. The 16-module bundle now has 28,024 reachable
  words and 81 residual boundaries (69 indirect transfers, 10 breaks, two
  syscalls). The Release suite is **18/18 CTest passing**, and a fresh 20M-slice
  diagnostic reaches the same explicit external GetToc-record boundary.
- Bundle reports now group residual IOP boundaries by decoded mnemonic and
  target register, with deterministic module/PC examples. The largest class is
  57 `jalr $v0` sites; its representative IOMAN sequence loads an object from
  `$a0+0x10` and then its `+4` callback field. This is runtime callback evidence,
  not a static-target inference, so it remains an explicit boundary.
- `hg.py vu-inspect --vu-program <external-raw-pairs>` now performs a strict,
  build-time VU microprogram structural pass. It validates 64-bit pair alignment,
  records I/E flags, and computes only documented lower-pipeline direct branch
  edges, including their required one-pair delay slot, explicitly marking targets
  or delay slots outside the supplied capture. No VU execution or interpreter
  fallback exists. Synthetic pair/branch and CLI coverage pass, creating the
  first AOT VU1 CFG foundation.
- Decoder investigation is now batched: one `analyze --triage` pass regenerates
  grouped EE boundary and setup-shape evidence (currently 307 boundaries across
  34,624 reachable words), while `iop-bundle` supplies the equivalent IOP
  grouping. The focused structural decoder checks and all 18 Release CTest
  targets run in parallel; the latest complete batch passed in 1.68 seconds.
- IOP bundle triage now also recovers a syscall selector only when its local
  straight-line setup proves it. This keeps unresolved syscalls as faults while
  separating their ABI work items without per-site manual decoding. The current
  two boundary selectors are 32 in THREADMAN and 12 in MODLOAD.
- `hg.py major-scan` now produces one ignored comprehensive inventory for the
  configured EE image and all 28 configured IOP modules. Its current run covers
  34,624 EE words (307 boundaries) and 69,638 IOP words (544 unresolved
  boundaries plus 152 defined BREAK traps),
  retaining every module/issue/opcode record plus compact priority groupings.
  The leading IOP families are 152 BREAK sites, 93 indirect transfers, and
  imports from THBASE (64), SYSCLIB (61), and LOADCORE (54); this is a worklist,
  not an assumption that any boundary may be bypassed.
- That same one-pass report now also runs all 28 configured modules through the
  independent static-link planner once. It proves 664 guarded provider bindings,
  records 129 native adapter sites, and leaves 97 explicit residual boundaries
  (93 dynamic transfers and the
  remaining unprovided imports/syscalls), while separately retaining all 152
  BREAK traps by code; generated C++ is discarded and no boundary is treated as
  a runtime fallback.
- IOP BREAK is now emitted as a defined terminating guest trap carrying its
  20-bit code instead of being mislabeled as an unsupported translation. The
  all-module inventory groups 92 code-`0x1c00`, 57 code-`0x1800`, two
  code-`0x400`, and one code-`0x7` traps. BREAK in a delay slot terminates before
  either branch successor is discovered. Nothing continues past a trap.
- The regenerated 16-module startup bundle passes all **18/18 Release CTest**
  targets after the version-compatible linking and BREAK reclassification. A
  fresh 20M-slice connected run again reaches the exact external GetToc record
  boundary with `MADR=0x000c3464`, `BCR=0x00810004`, and `CHCR=0x41000200`.
- SECRMAN 1.3 ordinals 4/5 now retain the documented memory-card command and
  device-ID callback pointers, including checked null clearing. Authentication
  ordinal 6 remains an explicit boundary until real SIO2/card behavior exists.
  This removes two provider gaps without returning fabricated authentication.
  The complete Windows Release batch remains **18/18 CTest passing**.
- The all-module report now groups four decoded words around every indirect
  transfer, normalizes architectural zero-shift NOPs, and separates target loads
  through a loaded object base. The largest concrete families are 26
  register-base `lw; nop; jalr`, 18 register-base tail `lw; nop; jr`, and 14
  two-level IOMAN `lw; nop; jalr` callbacks; no target is inferred from a shape.
- Executing the supplied LOADCORE linker against an otherwise identical import
  table with its minor version lowered proved that a newer same-major export is
  accepted. The static planner now mirrors that rule while retaining ambiguity,
  newer-import, and major-version mismatches as faults. This links the bundled
  SYSCLIB 1.4/SYSMEM 1.2 providers to six older-minor module imports and reduces
  the all-module residual count from 257 to 251.
- DMACMAN ordinals 33/34/35 now update only the documented DPCR/DPCR2 four-bit
  priority/enable fields, including SIO2 channels 11/12. No SIO2 transfer is
  fabricated: an unsupported endpoint still faults at transfer start. This removes
  SIO2MAN's three configuration-import boundaries; focused IOP/runtime and tooling
  checks pass.
- The startup bundle recipe includes `rom_timemani`, which the original core
  bootstrap queries before THREADMAN initialization. The prior omission caused
  an early `unknown IOP module` diagnostic failure; the regenerated bundle
  verifies this dependency through the original startup checks.
- Indexed TEX0 palettes now accept CPSM=PSMCT16S in CSM1/CSM2, with the
  existing CT16S swizzle rather than a linear or CT16 alias; synthetic CSM2
  coverage passes.
- TEX0 direct extraction now supports PSMT8H, PSMT4HL, and PSMT4HH by reading
  their documented high-byte/high-nibble CT32 lanes and using the checked CLUT
  route. Synthetic CSM2 palette cases cover all three formats.
- The GS frame/depth pixel path and TEX0 direct-color extraction now support
  PSMCT16S with its documented separate page block order; it retains CT16's
  RGB5A1 formatting, dithering, masks, and destination-alpha behavior without
  aliasing CT16 storage. Synthetic storage and normal depth commits pass.
- The separate IOP MIPS-I AOT decoder/emitter/runtime now models `MTC0` to
  CP0 Status, including LOADCORE's `mtc0 $zero,$12` form. Other CP0 writes
  stay explicit faults. This removes LOADCORE's two unsupported opcode words,
  expands its audited reachability from 1,730 to 1,777 words, and reduces its
  boundaries from 13 to 12; direct state and emitter checks pass.
- The GS frame path now applies PABE: when enabled, source alpha bit 7 gates
  alpha blending per pixel while direct source writing remains active. Synthetic
  enabled/disabled cases pass. The Release suite is **18/18 CTest passing**.
- GS point and equal-fog triangle paths now apply the documented FOGCOL blend
  after texture function and before pixel tests, retaining alpha. Varying
  triangle fog still faults explicitly until DDA interpolation exists;
  synthetic coefficient/point coverage passes.
- PSMZ24 depth reads/writes now share the documented PSMZ32 physical layout
  while retaining only low 24 depth bits and preserving the unused word byte.
  PSMZ16 and PSMZ16S now use their documented distinct 8×4 block tables with
  16-bit depth storage. Synthetic storage coverage passes. The Release suite
  is **18/18 CTest passing**.
- PSMCT16 now has a checked unmasked RGB5A1 frame/depth path: it applies the
  signed DIMX `Y%4,X%4` dither offset before COLCLAMP/RGB5 conversion, applies
  FBA to alpha bit 15, and expands destination alpha to the documented
  `0x80`/zero blend values. Its special FBMSK relation now maps the documented
  pre-conversion RGB/A bits to RGB5A1 precisely. The Release suite is **18/18
  CTest passing**.
- The GS pixel writer and normal depth path now support PSMCT24 alongside
  PSMCT32. RGB24 preserves its unused local-memory upper byte, applies RGB
  FBMSK/COLCLAMP, bypasses DATE as specified, and supplies destination alpha
  `0x80` to blending. Synthetic masked and depth-commit cases pass; RGBA16,
  dithering, and other frame formats still reject explicitly. The Release suite
  is **18/18 CTest passing**.
- GS now rasterizes flat, untextured, equal-depth TRIANGLE,
  TRIANGLESTRIP, and TRIANGLEFAN draw events through the normal pixel path.
  Its 12.4 edge test follows the documented top/left-inclusive and
  bottom/right-exclusive rule after winding normalization; synthetic
  side-sharing and strip cases pass. Identical FST/UV vertices may point-sample
  one TEX0 texel; varying UV, STQ, fog, Gouraud color, and depth interpolation
  still reject explicitly. The Release suite is **18/18 CTest passing**.
- `hg.py analyze --triage` now turns the existing single discovery pass into a
  deterministic, read-only grouped boundary queue (`out/analysis-triage.json`).
  It records every site by reason and decoded mnemonic without a second decode,
  inferred target, or fallback. The local executable run reports 286 unresolved
  indirect transfers, 20 syscalls, and one exception return (307 total across
  34,624 reachable words); its synthetic ordering/provenance check and the
  Release suite are **18/18 CTest passing**.
- Triage now additionally records a deliberately non-authoritative batch of
  strictly adjacent `lw`/`lwu` initialized-file-pointer candidates before an
  indirect transfer. These candidates never become targets or remove the
  explicit boundary because guest code may mutate their storage; the current
  executable has three. Synthetic evidence verifies the distinction.
- GS now rasterizes its first rectangle primitive: untextured sprites with
  documented second-vertex depth and flat draw-kick color use top/left-
  inclusive, bottom/right-exclusive coverage, XYOFFSET, SCISSOR, and the
  normal depth pixel path. Textured sprites are now supported, and IIP/AA1 are
  correctly ignored as fixed flat/off sprite attributes; synthetic coverage
  proves a 2x2 rectangle. The Release suite is **18/18 CTest passing**.
- Primitive rasterization now honors the documented `SCANMSK` row-parity
  control: modes 2 and 3 skip even and odd final framebuffer Y rows,
  respectively, before TEST/frame/depth effects. Direct framebuffer writes and
  transfers remain intentionally outside this primitive-only rule; synthetic
  checks cover both masks.
- The primitive pixel path now implements all documented alpha-test failure
  outcomes after the subsequent destination-alpha and depth tests: KEEP,
  FB_ONLY, ZB_ONLY, and RGBA32 RGB_ONLY (which becomes FB_ONLY in RGB24/16).
  Synthetic tests prove the independent RGBA32 frame/depth effects; the Release
  suite is **18/18 CTest passing**.
- `State::rasterize_gs_draws()` now exposes pending GS draw submission through
  the EE runtime and translates GS errors to guest faults. Empty queue behavior
  is covered in runtime tests; the Release suite is **18/18 CTest passing**.
- GS now has a bounded pending-draw submission queue. It preserves order and
  advances only after successful rasterization, leaving an unsupported draw
  at its queue position for explicit diagnosis. Synthetic queue drain/no-op
  coverage passes; the Release suite is **18/18 CTest passing**.
- A GS rasterizer dispatcher now submits completed point draws and explicitly
  faults all other primitive types, preventing unsupported draws from being
  silently retained or ignored. Synthetic coverage checks both dispatch and
  fault behavior; the Release suite is **18/18 CTest passing**.
- The first actual primitive rasterizer now handles GS points: closest-pixel
  selection from 12.4 coordinates, XYOFFSET, inclusive SCISSOR, and the
  PSMCT32/PSMZ32 normal pixel path. Textured points accept FST/UV and reject
  unimplemented STQ explicitly. Synthetic draw and scissor cases pass; the
  Release suite is **18/18 CTest passing**. Lines and interpolation remain
  unfinished; triangle/sprite implementations currently cover only their
  documented flat, untextured subsets.
- The normal successful GS depth path now composes alpha/DATE pass, PSMZ32
  comparison, PSMCT32 draw-pixel commit, and Z update. Synthetic GEQUAL and
  GREATER cases prove failed depth leaves both buffers intact; the Release
  suite is **18/18 CTest passing**. Selective alpha-failure FB/Z write policy,
  remaining formats, and rasterization remain explicit work.
- PSMZ32 depth reads/writes now use its documented distinct GS block order,
  shared FRAME width, and ZMSK behavior. Synthetic coverage proves that Z32
  does not alias the CT32 address at the same coordinate and preserves depth
  under ZMSK; the Release suite is **18/18 CTest passing**. Integrating this
  into full primitive depth/write policy and other Z formats remains work.
- A composed PSMCT32 GS draw-pixel path now applies alpha-test failure policy,
  DATE, optional PRIM.ABE blending, then the checked frame writer. Synthetic
  coverage includes KEEP, RGB_ONLY, and DATE rejection; the Release suite is
  **18/18 CTest passing**. Z-buffer effects, other frame formats, and actual
  primitive rasterization remain explicit work.
- The portable GS PSMCT32 frame-buffer writer now commits to swizzled local
  memory after COLCLAMP/wrap conversion, FBA alpha correction, and FBMSK.
  Synthetic coverage verifies both clamp/mask and lower-eight-bit paths; the
  Release suite is **18/18 CTest passing**. PSMCT24/16, dithering, Z writes,
  and primitive rasterization remain explicit work.
- The GS TEST stage now has independently tested alpha, destination-alpha,
  and depth predicates with every documented comparison mode represented.
  RGB24's DATE bypass and prohibited ZTE=0 behavior are explicit. The Release
  suite is **18/18 CTest passing**; alpha-failure write masks and final pixel
  pipeline ordering remain unfinished.
- TEX0 point sampling and its TFX/TCC texture function are now composed in a
  single checked GS-side operation, including the active CLAMP context. The
  synthetic path verifies REGION_REPEAT plus DECAL under both RGB and RGBA
  TCC settings; the Release suite is **18/18 CTest passing**.
- The portable GS ALPHA stage now implements documented A/B/C/D selector
  blending with explicit signed `>>7` behavior and preserves RGB values until
  the later frame-buffer write/clamp stage. Synthetic cases cover normal and
  over-range results; the Release suite is **18/18 CTest passing**. Pixel
  tests, dithering, and frame-buffer write semantics remain unfinished.
- The portable GS texture-function stage now implements documented TEX0
  MODULATE, DECAL, HIGHLIGHT, and HIGHLIGHT2 modes, including TCC's RGB/RGBA
  alpha choice and the GS's `>>7` (0x80-unity) multiplication. Synthetic
  coverage exercises every mode and alpha path; the Release suite is **18/18
  CTest passing**. Host shader submission and the remaining pixel pipeline
  are still unfinished.
- Backend-neutral presentation now preserves each vertex's documented unsigned
  10.4 UV coordinates and PRIM.FST selection alongside position/color. This
  connects verified TEX0 point sampling to future host texture submission
  without conflating UV with perspective STQ. Synthetic GIF coverage verifies
  the values; the Release suite is **18/18 CTest passing**.
- The portable GS texture path now point-samples TEX0 images through each
  documented CLAMP mode (REPEAT, CLAMP, REGION_CLAMP, REGION_REPEAT) before
  format, CLUT, and TEXA conversion results are consumed. Invalid resolved
  coordinates fail explicitly. Synthetic GIF coverage exercises all four
  modes; the Release suite is **18/18 CTest passing**. Bilinear/trilinear
  filtering and game draw submission remain unfinished.
- GS TEX0 extraction now applies the documented TEXA alpha conversion to
  RGB24, direct RGBA16, and RGBA16 CLUT colors.  Synthetic coverage verifies
  separate TA0/TA1 selection and AEM's transparent zero-RGB case, rather than
  relying on an implicit opaque host alpha.  The full Release suite is
  **18/18 CTest passing**. CSM1 palette-bank offsets remain an explicit fault
  pending a fully specified temporary-CLUT implementation.
- Startup SIF diagnostics now retain bounded full packet payloads and widened
  IOP call snapshots, so one run distinguishes packet transport, RPC lookup,
  and native service ordering without successive narrow probes.  That evidence
  showed the EE's `SifBindRpc(0x80000592)` targets CDVDFSV, whereas the
  independently registered LOADFILE server correctly has ID `0x80000006`.
  The scheduler now bootstraps original CDVDMAN/CDVDFSV after FILEIO's original
  server registration but before accepting the pending next SIF packet.  The
  native `--verify-cdvd` diagnostic passes: original CD/DVD RPC server binding
  completed through DMA.  This is startup-path evidence only, not an asset-read
  or playability claim.
- The same long-run diagnostic now progresses through the original reboot and
  reaches CDVDMAN's exact GetToc DMA boundary (`MADR=0x000c3464`,
  `BCR=0x00810004`, `CHCR=0x41000200`). Its SIF1 observer retains a 64-word
  prefix and reports truncation without interrupting valid guest DMA, avoiding
  a diagnostic-capacity fault during later startup traffic. No live oracle is
  running; existing external candidates are explicitly pre-command-stage and
  remain ineligible as `--toc-record` input.
- A fresh bounded PCSX2 batch observation at approximately 16 and 27 CPU
  seconds reproduced the same rejected `0x000c3464` candidate hash in two
  independently module-anchored IOP captures; provenance is recorded in
  `docs/ORACLE.md`. The batch process was stopped. Synthetic DMAC coverage now
  proves that a 68-word valid SIF1 packet completes guest DMA while the
  diagnostic retains only its 64-word prefix and records truncation. The
  Release suite is again **18/18 CTest passing**.
- The earlier OpenGL colored-triangle probe is superseded by the connected GS
  local-memory/PCRTC scanout probe described in the latest checkpoint.
- GS draw records now retain the active PRIM context's XYOFFSET register and
  presentation performs the documented signed 12.4 offset subtraction before
  host viewport conversion. The synthetic GIF/presentation test and the
  one-frame OpenGL probe both pass with this coordinate origin in place.
- A bounded, pure GIF packet decoder now accepts complete EOP-terminated
  PACKED and REGLIST streams, preserves descriptor/payload order for the GS
  layer, observes REGLIST odd-entry padding, and rejects truncated/trailing
  data explicitly. State-only PACKED/REGLIST descriptors now commit PRIM,
  RGBAQ, ST, UV, FOG, TEX0, CLAMP and A+D data; vertex-kick descriptors still
  fault until the rasterizer exists. IMAGE payloads now reach checked PSMCT32, PSMCT24, PSMCT16,
  PSMT8, and PSMT4 GS HWREG endpoints, based on the documented 4 MiB
  page/block/column arrangements, byte/nibble lanes, and transfer packing.
  Same-depth local-to-local copies between all implemented layouts honor
  TRXPOS.DIR; local-to-host transfer still faults explicitly. Synthetic `gif_tests`
  cover the documented format edges. Renderer-facing TEX0 extraction now also
  supports PSMT8 and PSMT4 through documented CSM1 (CSA=0) layouts and CSM2
  PSMCT16 placement (CBW/COU/COV and `CSA=0`); CSM1 palette-bank offsets and
  other CLUT formats still fault explicitly.
  The batched Windows suite is **53 Python / 18 CTest passing**; the current
  independent EE analysis reaches **34,624 words** with **307** explicitly
  recorded unresolved items.
- The graphics transport now has a bounded GIF path that accepts split DMA
  qwords, waits for a complete EOP packet, then decodes and commits it in order.
  It preserves an incomplete suffix and explicitly rejects an over-capacity
  packet; the connected channel-2 DMA endpoint feeds it.
- The EE GIF DMAC channel now pumps checked normal and source-chain qwords into
  that path and raises its completion status. Normal GIF packets are verified
  through DMAC, EOP decode, and GS register commit in one synthetic test. MFIFO,
  stall/interleave modes, tag-transfer data, VIF DIRECT, and VU XGKICK remain
  explicit faults or unconnected work.
- EE `State` now owns GS VRAM and its GIF path, and `State::pump_gif()` reads
  checked RAM/scratchpad qwords through the channel-2 model. The normal-DMA
  synthetic path verifies this actual runtime entry point.
- Both native diagnostics now service channel-2 GIF DMA after each EE slice.
  Startup does not yet submit GIF traffic before its current CDVD boundary, so
  this is tested scheduling plumbing rather than visual output evidence.
- The analyzer's conservative straight-line constant pass now proves syscall
  service IDs in `v1`, including signed ABI numbers, rather than requiring an
  immediately adjacent `addiu`. It follows only configured returning services;
  the current executable report is 34,624 reachable words and **307** explicit
  unresolved boundaries (286 indirect transfers, 20 syscalls, one exception
  return), down from 346 before this decoding correction.
- VIF1 now has a bounded normal/source-chain DMA endpoint. Its documented
  DIRECT and DIRECTHL VIFcodes forward complete 128-bit units into the same
  checked GIF PATH2 assembler, and both diagnostics service that endpoint after
  EE slices. Synthetic tests cover split GIF completion through VIF1 and DMA
  completion status. UNPACK, MPG, MSCAL and VU XGKICK remain explicit faults;
  no VU work or draw behavior is fabricated.
- GS PRIM now resets a bounded vertex queue, and packed/REGLIST/A+D XYZ2,
  XYZF2, XYZ3 and XYZF3 writes retain the manual's vertex-kick versus
  drawing-kick distinction. Completed primitive groups are recorded as raw
  draw events for a future OpenGL stage; there is still no rasterizer, texture
  sampling, clipping, depth, blend, or visual-output claim.
- GIF descriptor handling now follows the distinct PACKED and REGLIST rules:
  packed XYZ2/XYZF2 honor ADC when selecting no-draw XYZ3/XYZF3, while REGLIST
  ignores PRE and treats A+D as its documented NOP. Synthetic tests cover both
  boundaries, so no former fault is silently converted into a guessed write.
- PACKED GIF Q is now explicit per-tag transport state: ST supplies Q to later
  RGBAQ payloads in the same tag and every GIFtag resets it to the documented
  1.0 value. A two-tag synthetic stream checks both the within-tag dependency
  and reset edge.
- VIF1 VIFcodes with the interrupt-control bit now fault explicitly until VIF
  interrupt/stall delivery exists. Raw draw records retain all PRIM attribute
  bits alongside per-vertex state, avoiding a later reconstruction shortcut.
- TEX0 context 1/2 fields now drive checked CPU extraction of PSMCT32,
  PSMCT24, and PSMCT16 images from the documented swizzled VRAM mapping into
  portable RGBA buffers. Indexed CLUT formats are still explicit faults, not
  approximated palettes.
- A backend-neutral presentation conversion now turns completed GS primitive
  records into ordered host point/line/triangle vertices while preserving PRIM
  texture, Gouraud, alpha-blend, and context flags. It intentionally leaves
  projection and raster behavior to the future OpenGL stage.
- GIFtag PRE plus PACKED/REGLIST state descriptors commit to a bounded raw GS
  register transport state. Primitive draw descriptors still fault explicitly,
  so this is state transport—not a rendering claim.
- EE `BREAK` trap slots are now recognized as defined terminating guest paths,
  rather than unsupported decoding. AOT code preserves the trap code in an
  explicit fault, and branch-likely CFG handling retains only its annulled
  not-taken continuation. The own analyzer now reports **34624 reachable
  words / 346 unresolved items** (25 fewer explicit unresolved items), and the
  batched Windows suite is **51 Python / 18 CTest passing**. The full local
  game-derived translation was regenerated and compiled, and its independent
  `hg_diagnostic --verify-prefix` check passes.
- The CDVD endpoint now has a strict, real completion route for the originally
  requested GetToc DMA: `hg_system_diagnostic --toc-record <external-file>`
  accepts exactly one 2064-byte hardware record and commits it through the
  original channel-3 completion/IRQ path. It neither derives a TOC from ISO
  user sectors nor invents one; omitting the external record remains a clear
  fault. Synthetic IOP coverage proves the transfer data, descriptor completion,
  and IRQ state, and the complete Windows validation suite is **51 Python / 18
  CTest passing**.
- The read-only oracle capture helper can now emit a separately hashed,
  provenance-linked 2064-byte candidate from the original CDVDMAN DMA target
  (`0x000c3464`) using `--iop-toc-candidate`. It still captures and revalidates
  the full supplied-module-anchored IOP map first. The candidate is intentionally
  not accepted as a TOC record until an independently established command stage
  proves that the DMA has completed.
- A batch-mode PCSX2 launch now reliably runs long enough for coherent, supplied
  module-anchored captures. Two maps 15 seconds apart show an identical
  `0x000c3464` candidate whose physical-format prefix is zero, so it is recorded
  and rejected as pre-TOC state rather than fed to the runtime. This is stronger
  negative provenance, not a synthetic completion.
- The EE analyzer now follows an indirect `jr`/`jalr` only when its source
  register is proven by a straight-line local constant calculation. Joins,
  calls, memory loads, and unknown operations discard the candidate state. The
  game’s reachable total remains **34624**/**346**, proving its listed indirect
  transfers are not missed literal targets under this conservative rule.
- COP2 macro-mode `VMOVE` and `VMR32` now decode only at their exact documented
  encodings and execute as masked raw 32-bit vector-field transfers. Synthetic
  translation coverage checks partial writes, field preservation, and VMR32's
  y/z/w/x rotation. The own analysis remains **34624 reachable words** with
  **346 explicit unresolved items**: these opcodes do not occur in currently
  reachable code, so coverage did not silently change.
- EE unaligned word/doubleword merge instructions (`LWL/LWR/SWL/SWR` and
  `LDL/LDR/SDL/SDR`) now have checked AOT runtime implementations and synthetic
  paired-transfer coverage. The current independent EE analysis reaches **34624
  words** with **346 explicit unresolved items**. The higher unresolved count is
  from newly reachable code after MMI `PADDUB`, not a silent fallback.
- The bundled IOP build now includes original MODLOAD and LOADFILE, with the
  independently decoded LOADFILE worker root at `entry + 0xc8`. Its original
  entry creates the worker; it is not replaced by a host server implementation.
  `--verify-loadfile` confirms its `0x80000006` server, handler, and queue record.
- The original IOP reboot request is received through SIF DMA, validates the
  supported IOPRP request, replaces checked IOP state, reruns original core/service
  entries, and completes the renewed EE/IOP handshake. `--verify-reboot` passes.
- A 20M-slice diagnostic now survives one original IOP reboot and reaches the
  post-reboot EE LOADFILE bind wait at `0x0026c1e4`; the original LOADFILE server
  record is present. The current native scheduler state still leaves that RPC
  transaction unfinished; no server pointer or RPC reply is fabricated.
- Windows MSVC and Ubuntu WSL/GCC both pass **17/17 CTest checks**, including
  game prefix, SIF, FILEIO, CD/DVD, and reboot diagnostics. This is startup-path
  evidence only; it does not establish playability or full cross-platform support.

### Immediate next steps

Current resume priority (2026-09-08): execute the regenerated CRI_ADXI stream
completion callback, finish its original RPC request, then resume SNDDRV's
handler/completion path. Preserve the paced input endpoint and validate its
remaining hardware-policy assumptions; input buffering does not implement audio
mixing, synthesis, or output. The preceding boundary's investigation was:
implement the observed core-0 AutoDMA
sound-input endpoint from independent evidence: 48 kHz consumption, per-channel
512-short-word input areas split into 256-short-word halves, bounded source
transfer and actual completion. Establish channel ordering and initial request
phase before enabling it; an arbitrary delayed completion is not a sound-input
implementation. Keep the explicit `0x00032140` fault until that endpoint exists,
then resume CRI_ADXI RPC completion and SNDDRV startup. Sony SPU2 Overview pp10,
28 and 55 plus supplied LIBSD `0x0003208c..0x00032140` are the starting evidence.
The items below describe the older GetToc investigation;
the real TOC provenance requirement remains open, while subsequent synthetic
diagnostic runs have already advanced beyond its listed module-load boundaries.

1. Acquire a coherent, provenance-verified GetToc DMA record at the original
   pending CDVDMAN command stage via PCSX2's documented `-debugger` entry break
   and an R3000 breakpoint after the post-reboot CDVDMAN base is known. The new
   exact-frame monitor is a safe fallback, but 10,111 complete-map samples saw
   no request frame. Existing candidate files predate that stage and must not be used.
   Keep the 2064-byte record outside the repository, then validate it through
   `hg_system_diagnostic --toc-record <external-file>`.
2. Once the original worker submits its first verified `cdrom0:` operation,
   resolve why MODLOAD helper `+0x28d8` yields `-2` after its two request-vector
   callbacks; only its nonnegative path reaches device-operation transfer
   `+0x22a4`. Then implement the corresponding real CD/DVD-backed open/read
   path; do not pre-load a module or synthesize a successful RPC response. The
   original request path and `-203` reply are now repeatable after a quarantined
   diagnostic TOC completion.
3. Exercise the CDVD N-command path from that original request before extending
   its bounded one-record transport contract. Keep unsupported devices and
   translations as explicit faults.

### EE decoder progress (2026-09-05)

- The own decoder/emitter now supports MMI `MFHI1` and `MFLO1`, which copy the
  second EE multiply pipeline's 64-bit HI1/LO1 registers into a GPR. The behavior
  comes from the EE Instruction Set Manual pp146–147, not an emulator. Synthetic
  translation tests verify full 64-bit values; the original analysis expands by
  17 reachable words and removes one explicit unsupported item.
- MMI `PSUBW` is now a checked four-lane, modulo-32-bit 128-bit subtraction
  (EE Instruction Set Manual p284). A synthetic case checks independent low/high
  lanes and wraparound. It adds 55 reachable words and removes two more explicit
  unsupported items, leaving **32201** words and **358** unresolved items.
- COP1 `SUB.S` now routes through the existing checked EE FPU addition path with
  the second source's sign bit inverted (the operation specified by the EE
  Instruction Set Manual p377); it does not invoke host floating-point arithmetic.
  A synthetic 3.5−1.25 case passes. It expands discovery to **32207** words;
  the unresolved count remains **358** because its newly reachable continuation
  exposes the separately unimplemented `MADD.S` boundary.
- COP1 `MADD.S` and `MUL.S` are now implemented from the EE Instruction Set
  Manual pp359/372 only for exactly representable normalized products. They use
  integer mantissa arithmetic and the existing EE-style add/ACC state; discarded
  product bits, zero/overflow-range products, and an overflow-form ACC still fault
  explicitly rather than using host floats. Synthetic `80 * -0.5` and
  `-16 + 80 * -0.5` checks pass. This extends the graph to **32260** words; the
  newly reached `DIV.S` and additional dynamic calls make the recorded unresolved
  count **362**, not a regression in supported behavior.
- COP0 `MTC0`/`MFC0` register 6 (Wired) is now preserved as native EE state,
  including the original startup `MTC0 $zero,$6` at `0x00275d48`; the synthetic
  round trip passes. This is a register model only—TLB instructions and address
  translation remain independently unsupported rather than becoming a no-op.
  It expands discovery by 106 words to **32366** with the same **362** recorded
  unresolved items.
- COP1 `DIV.S`, `CVT.S.W`, `CVT.W.S`, `MOV.S`, `NEG.S`, `MADDA.S`, and `MSUB.S`
  have been added from the EE Instruction Set Manual pp356–372. The conversion
  paths use integer bit arithmetic (including documented CVT.W.S truncation and
  clamp); exact multiply/divide paths refuse unverified rounding/range cases.
  Tests cover exact quotient/product, ACC output, sign copy/toggle, conversion
  truncation/clamp, and rejected inexact inputs. The static graph now reaches
  **33132** words; newly exposed dynamic/control sites leave **363** explicit
  unresolved items, all still faulting rather than silently executing.
- COP1 `C.EQ.S`/`C.OLT.S` now write FCR31 condition bit23 using the manual's
  no-NaN and signed-zero rules; `BC1F/T` and likely variants branch from that
  state with normal delay-slot handling. Synthetic tests cover zero equality,
  signed ordering, and a taken BC1T delay-slot transfer. This removes three
  explicit items and reaches **33252** words with **360** unresolved items.
- COP1 `SQRT.S` is now decoded only when its required `fs` field is zero (the
  remaining operand is `ft`, and the result is `fd`, per the EE Instruction Set
  Manual p376). Its portable implementation uses integer mantissas for exact
  positive roots and preserves exponent-zero signed zero; inexact roots,
  negative inputs, and exponent-255 cases remain explicit faults pending their
  complete flag/result coverage. Decoder, FPU, and synthetic-translation tests
  cover valid and rejected field encodings plus exact even/odd-exponent roots.
  The analysis now reaches **33256** words with **359** explicit unresolved
  items. Windows Release CTest passes **17/17**.
- MMI `PADDUB` now implements all sixteen unsigned byte lanes with independent
  saturation at `0xff` (EE Instruction Set Manual p168). This was the former
  unsupported word at `0x00102390`; decoding it expands the static graph by
  1,283 words and exposes 15 previously hidden explicit boundaries. Synthetic
  tests cover all-low/all-high lanes and saturation; Windows Release CTest
  remains **17/17**.
- Trapping integer `SUB` now has its specified 32-bit signed behavior (EE
  Instruction Set Manual p114): canonical word operands yield a sign-extended
  result, while undefined noncanonical operands and integer overflow preserve
  the destination and fault explicitly until EE exception dispatch exists. This
  covers the former return-delay-slot opcode at `0x00102380`; normal and
  overflow-preservation synthetic cases pass. The overall graph/issue totals
  remain **34539**/**374**, because its newly executable continuation is an
  already-accounted dynamic return boundary. Windows Release CTest is **17/17**.
- COP2/VU0 now has a dedicated 32-register 128-bit vector-register transfer
  foundation. `QMFC2`, `QMTC2`, and the EE-specific `SQC2` encoding move or
  store full quadwords with checked EE RAM bounds; no VU arithmetic or
  microprogram is implied. This independently resolves the former `QMFC2` at
  `0x0010bfd8`, its paired `QMTC2`, and both reachable `SQC2` sites, extending
  the graph to **34558** words and reducing explicit issues to **373**. Tests
  cover both transfer directions and aligned-down two-half store behavior;
  Windows Release CTest is **17/17**.
- CP0 `TagLo` is now retained and transferred through checked `MFC0`/`MTC0`
  register paths, covering the two former `MFC0 $v0,$28` boundaries in the
  newly reached VU-adjacent code. This does not claim cache-tag operation
  support: TagLo is preserved state only. The graph expands to **34624** words
  and the explicit issue count falls to **371**; synthetic readback and all
  Windows Release CTest checks (**17/17**) pass.
- The reciprocal VU0 data-transfer instruction `LQC2` now loads a checked,
  aligned 128-bit EE quadword into a vector register. Together with `SQC2`,
  this establishes bidirectional VU0/EE memory movement without claiming VU
  arithmetic or microprogram execution. Decoder and synthetic tests cover the
  opcode plus an aligned-down round trip; the static totals remain
  **34624**/**371** because no currently reachable path used LQC2. Windows
  Release CTest remains **17/17**.
- MMI `PSUBH` now supplies the remaining ordinary lane-subtract width: eight
  independent modulo-16-bit subtractions over the full 128-bit GPR pair (EE
  Instruction Set Manual p271). Synthetic coverage includes both halves and
  wraparound; this is proactive family coverage, so the reachable totals remain
  **34624**/**371**. Windows Release CTest remains **17/17**.
- MMI `PSUBSB`, `PSUBSH`, and `PSUBSW` now complete the signed-saturating
  subtract group (EE Instruction Set Manual pp272,274,276). Each lane clamps
  independently to its signed byte, halfword, or word range; all halves and
  both saturation directions are covered synthetically. This proactive family
  work leaves the current reachable totals **34624**/**371** and Windows
  Release CTest at **17/17**.
- MMI `PADDUH` completes the unsigned-saturating add widths: eight halfword
  lanes clamp independently at `0xffff` (EE Instruction Set Manual p170).
  Synthetic all-lane saturation coverage passes; this proactive family work
  does not alter the current **34624** reachable-word/**371** issue totals.
  Windows Release CTest remains **17/17**.
- MMI `PADDSB`, `PADDSH`, and `PADDSW` now complete signed-saturating integer
  addition (EE Instruction Set Manual pp162,164,166). Byte, halfword, and word
  lanes clamp independently in both directions; synthetic packed vectors prove
  all three widths. The reachable totals remain **34624**/**371** and Windows
  Release CTest is **17/17**.
- Regenerating the local EE AOT source and running the connected 100M-slice
  diagnostic reaches the unchanged explicit GetToc DMA boundary with the same
  descriptor (`MADR=799844`, `BCR=8454148`, `CHCR=1090519552`). This confirms
  the decoder extension did not alter the validated startup path.

### Batched LOADFILE provenance (2026-09-05)

- The AOT IOP entry loop now records a bounded, host-only PC/argument provenance
  ring.  It reads the committed register file directly, so it cannot introduce a
  MIPS-I load-delay hazard.  `hg_system_diagnostic` reports the ring alongside
  the existing SIF and event rings, allowing a single long startup slice to
  establish an entire original call chain without trace files or repeated probes.
- One 100M-slice run shows the native `0x8000000a` LOADFILE request reaches its
  original command-0 handler at `0x000d5150`, validates the request, and calls
  MODLOAD export ordinal 7 at `0x000a94b0` with `a0=0x000d6d88`, the checked
  guest-RAM string `cdrom0:\\MODULES\\SIO2MAN.IRX`.  The observed `-203` reply
  is therefore downstream of RPC dispatch, request validation, and EE SIF0
  completion.  The remaining boundary is the original MODLOAD/CDVD file path;
  no semaphores, callbacks, or module-success results are fabricated.
- The regenerated 16-module bundle remains at **27977 reachable words** and
  the Windows Release build succeeds after the provenance change.  Re-run the
  full CTest set after the next device-path implementation change.
- `hg_system_diagnostic --slices 100000000 --stop-on-loadfile-error` now stops
  at the first original `-203` reply (213 transport steps in the current
  checkpoint) instead of overwriting the provenance with the remainder of the
  diagnostic budget.  This is a diagnostic stop only; it does not alter guest
  DMA, callback, or reply behavior.
- `--watch-iop-pc 0xADDRESS` may be repeated for up to eight translated IOP
  instructions. Each boundary retains its own provenance snapshot, including
  across the checked reboot. Combined with the stop option, it replaces
  edit/rebuild/re-run trace experiments with one bounded execution for related
  hypotheses; without an explicit watch, the proven MODLOAD `0x000ab294`
  mapping remains the default.
- The same diagnostic now retains a 64-entry CDVD MMIO ledger (PC, direction,
  register, width, value, repeat count). Consecutive identical polls coalesce,
  preserving setup commands beside long waits. It is read-only provenance,
  intended to establish a complete original command/status contract before
  extending the mounted-media state machine.
- The oracle helper now has an opt-in `--suspend` mode. It takes one coherent,
  fingerprint-verified VM-read snapshot while PCSX2 is OS-suspended and resumes
  it in `finally`; it never writes process or guest memory. This removes the
  previous debugger-UI pause dependency and avoids repeated moving-target
  captures when the next CDVD state observation is required.
- Its first fresh-process exercise produced coherent EE snapshots before and
  after a 30-second launch interval, but neither contained the full SIO2MAN path;
  they are negative stage observations only. The initial RAM locator found several
  mirrored verified EE mappings, so future captures must keep a selected base and
  revalidate its three ELF fingerprints as the helper does.
- `--locate-iop rom_cdvdman` now finds a unique, locally supplied unrelocated
  CDVDMAN text anchor in PCSX2 data memory; `--iop-base … --iop-module rom_cdvdman`
  captures and rechecks the complete 2 MiB map. A fresh observed map established
  CDVDMAN at guest `0x22830`, but its `0x000c3464` buffer still held pre-TOC
  request/allocator data. This rejects it as a TOC source without using captured
  code or emulator implementation, and makes a later one-pass TOC capture routine.
- The TOC DMA request is now guarded by the independently observed single-record
  channel-3 descriptor (`CHCR=0x41000200`, `BCR=0x00810004`, aligned in-RAM
  MADR), just as ReadDvd is. A malformed active descriptor explicitly faults;
  it cannot turn into an arbitrary implicit transfer while the TOC payload remains
  unresolved. The synthetic IOP test covers both the exact TOC request and this
  rejection. Windows Release rebuild and CTest pass **17/17**, including the
  connected CDVD, LOADFILE, reboot and tool checks.
- A longer fresh PCSX2 launch made measurable progress but its coherent IOP maps
  still contain no full `cdrom0:\\MODULES\\SIO2MAN.IRX` request and retain
  pre-TOC data at `0x000c3464`. It is not a payload source; leave the tool ready
  for a run that reaches the established native request boundary.
- First ledger result: the failing request reaches CDVDMAN's `0x000b87ac`
  polling loop, which reads drive-status register `0x1f40200a` as `0x00`.
  Independent PS2 hardware documentation identifies `0x0a` as a mounted,
  spun-up, paused drive (bits 1 and 3), so the native mounted-DVD profile now
  initializes both current and sticky status to that state. The code will now
  determine the next required transition; it still must not manufacture a
  successful LOADFILE result or bypass the native poll.
- That transition exposes original CDVDMAN N-command `0x09` with one zero byte
  (GetToc). The native endpoint accepts only that exact observed shape and marks
  it as a pending TOC DMA operation. Its native descriptor is now established:
  `MADR=0x000c3464`, `BCR=0x00810004`, `CHCR=0x41000200`, the same 2064-byte
  channel-3 shape as the guarded ReadDvd path. It explicitly faults at delivery
  until the 1024-byte DVD TOC payload is independently captured or specified.
  ReadDvd remains separately guarded as command `0x08` with eight parameters.
- The stop-point shows registered original IOMAN drivers `cdrom` and `tty`; its
  descriptor slots have already been released, so absence of a retained slot is
  not evidence that the CD-ROM driver was unavailable.  A static-PC snapshot at
  MODLOAD's actual `-203` construction instead proves the active chain:
  CDVDMAN returns `-2` through IOMAN at `0x000a62fc`, then MODLOAD maps that
  failure at `0x000ab294`. Later independent static inspection identifies
  CDVDMAN `0x000b3de0` as the intentional `-2` return following its original
  THREADMAN event operation, not a still-unknown device-state boundary. The
  earlier LOADFILE-local error-path hypothesis was tested and rejected; no
  local error value is fabricated. Current execution has advanced beyond this
  historical checkpoint to the GetToc DMA gate described above.

### CDVDFSV/LOADFILE follow-up (2026-09-05)

- Tracing the original priority-80 CDVDFSV worker established that it calls
  THREADMAN `ClearEventFlag` with its real event identifier and an AND mask
  (`~0x4`) before calling original `WaitEventFlag`.  The prior translated
  THREADMAN routine expected an unimplemented internal RAM event-object layout,
  returned an error, and caused the worker to retry.
- `clear_event_flag` is now a checked native THREADMAN boundary at offset
  `0x324c`.  It rejects interrupt-context calls and applies the original
  AND-mask operation to the checked event flag.  `tests/iop_tests.cpp` covers
  masking and the interrupt-context rejection.
- Before reboot, the worker now genuinely waits and the original LOADFILE
  worker registers `0x80000006` at `0x000d6d38`.  After reboot, the diagnostic
  still reaches the LOADFILE bind wait with all workers ready, but native IOP
  CPU interrupts are false (`locked_thread=0`), so cooperative dispatch does
  not begin.  Event diagnostics show flags `3=0x29` and no fabricated RPC
  state.  A restart-time enable experiment did not survive subsequent original
  startup code and was reverted.
- Next: trace the exact post-reboot original path that leaves
  `IopState::interrupts_enabled` false, including its suspend/resume token, and
  establish whether the scheduler guard or the CPU-interrupt lifecycle is wrong
  before changing either.

### Post-reboot LOADFILE progression (2026-09-05)

- Cooperative worker contexts now retain their own CPU-interrupt state.  A
  masked worker may execute and block, but its status no longer leaks into the
  dispatcher and suppresses completed IOP DMA callbacks.  A blocked masked
  worker also releases scheduler ownership.
- SIFMAN's observed control command sequence `0x40` then `0x20` is modeled as
  a last-command register (not an OR latch), and an identical active SIF1 CHCR
  write is accepted as an idempotent re-arm.  Changes to an active register
  still fault.
- Post-reboot channel-9/10 completions now dispatch original callbacks; system
  event bit `0x800` reaches the original LOADFILE worker.  It registers and
  receives its first real RPC request.  The handler's bounded 11-entry command
  table at LOADFILE offset `0x1ca8` is emitted statically.
- The request progresses through original MODLOAD worker roots `0xca0` and
  `0x295c`, then reaches original CDVDMAN `0x000b39e0` (offset `0x19e0`), now
  added as the next root.  No LOADFILE RPC response has been injected.
- Windows MSVC CTest remains **17/17** after this progression.  The current
  next boundary is a byte read of CD/DVD register `0x1f40200f` at original
  CDVDMAN PC `0x000bbc00`, reached after root `0x19e0` expands.  It is not in
  the implemented CD/DVD register subset; preserve the explicit fault until
  the original command/status contract is established.  Do not substitute a
  guessed ready/status value.

### CD/DVD disk-type boundary (2026-09-05)

- The faulting byte register `0x1f40200f` is the read-only CD/DVD disk-type
  register, established from the approved register protocol reference and the
  original CDVDMAN classifier at `0x000b9328`. That classifier recognizes
  `0x14` as the PS2-DVD route. The checked local-dump profile now returns
  `0x14`, with a synthetic alias-read test. It is explicitly a mounted-media
  identity only: N commands, CDVD DMA and sector-read completion remain absent.
- Original CDVDMAN subsequently writes its request error byte to `0x1f402006`;
  native state retains that single byte for the original write/read flow. Error
  meanings and device effects remain unsupported. The next expected boundary is
  CDVD DMA-channel setup/transfer, not a fabricated request completion.
- With those two register contracts implemented, the normal post-reboot
  diagnostic reaches original CDVDMAN `0x000b8500`: it programs channel 3 from
  its request descriptor and writes CHCR `0x41000200`. The native IOP DMAC
  rejects it because only bounded SIF endpoints exist. This is now the correct
  explicit blocker: a real CDVD DMA provider must read the local image into the
  descriptor's RAM destination and complete the original interrupt path. The
  supplied ISO is standard ECMA-119 at origin zero (the default `0x1800` is
  specific to `DATA.CVM`). Channel 3's observed `0x41000200` CHCR arm is now
  retained and guarded, without transfer or completion. Continue through the
  original code to establish the subsequent CDVD command/data contract.
- The arm advances past the former device fault, but the 20M-slice diagnostic
  then remains in THREADMAN's original priority-8 dispatch worker (`0x000a9ba4`)
  at queue traversal `0x000a3f90`, with queue root `0x000a54e0` and current
  callback `0x000b8224`. That callback is a reachable runtime continuation not
  yet emitted as a callable target. Its original bytes form a normal CDVDMAN
  function at module offset `0x6224`, so it is added as the bounded static root
  `cdvd_finish_request`. The regenerated bundle now emits `0x000b8224` and
  reaches 27096 words; Windows CTest remains 17/17. The dispatch worker still
  loops in the preceding queue-match traversal before invoking that callback,
  so inspect the original queue record transition next; do not force a wakeup
  or inject the callback result.
- The queue root is BSS at `0x000a54e0`, still zero because the diagnostic had
  linked THREADMAN exports without running its original module entry. A checked
  entry attempt reaches unresolved `heaplib:4` at `0x000a4c10` before its list
  setup, so that premature invocation is reverted to preserve the validated
  baseline. Implement a real HEAPLIB provider before running the original
  THREADMAN entry; do not synthesize its empty-list sentinel.

### THREADMAN initialization dependencies (2026-09-05)

- Public HEAPLIB declarations identify the first missing import as
  `CreateHeap(heapblocksize, flag)` (ordinal4). The decoded original call is
  `CreateHeap(0x800, 1)`. A bounded guest-RAM heap provider now implements
  HEAPLIB ordinals4–8 (create/delete/allocate/free/free-size), with synthetic
  allocation and invalid-handle tests. It uses no SDK implementation code.
- The original THREADMAN entry now passes HEAPLIB and reaches INTRMAN's original
  context-switch callback setters (ordinals28 and30). Native state retains the
  validated guest callback addresses; their execution is still governed by the
  cooperative scheduler rather than injected calls.
- Linking the supplied original TIMEMANI provider advances THREADMAN setup to
  its first timer device register access at original PC `0x000d74dc`, through
  physical `0x1f8014a4` (Timer5 mode) through the IOP cached alias. The native
  timer register banks now implement Timer0–5 count/mode/target storage,
  mode-write counter reset and mode-read flag acknowledgement.
- Original `DisableIntr(16, nullptr)` is now a checked mask transition. The
  following original THREADMAN helpers at offsets `0x3e8` and `0x418` are static
  roots. Its bootstrap call `ChangeThreadPriority(0, 126)` records explicit
  bootstrap-thread state before native workers exist.
- THREADMAN's original entry now runs during core initialization, with the
  original TIMEMANI provider linked. The integrated 16-module bundle emits
  **27844 reachable words**. `hg_system_diagnostic --verify-sif` and Windows
  CTest both pass (**17/17**) through the original EE/IOP SIF handshake.
- The original post-reboot dispatcher next called `CpuInvokeInKmode` (INTRMAN
  ordinal14) with a translated THREADMAN callback. The native import adapter
  validates that callback, shifts the public varargs argument registers and
  transfers directly into its AOT code; it does not invoke host code or an
  interpreter. A normal 20M-slice run now reaches its cooperative budget without
  a new fault, with the original queue's `cdvd_finish_request` node populated
  and ten original workers established. Timer progression/IRQ delivery and a
  real CDVD transfer are still outstanding.
- The CDVD boundary is now decomposed into independently tested states instead
  of a long diagnostic probe: ReadDvd (`Ncmd0x08`) accepts exactly its decoded
  8-byte little-endian LBA/count request; the active original descriptor
  `CHCR=0x41000200`, `BCR=0x00810004` transfers exactly one 2064-byte DVD
  record; the host reads only that checked sector from the configured local ISO;
  and an original IRQ2 handler receives the completion. The record’s 12-byte
  DVD prefix/trailer and local-image bounds are checked. The normal request has
  not yet issued Ncmd0x08, so this endpoint is not claimed exercised by the
  full startup run.
- Timer progression now uses each timer's own fixed-point 36.864 MHz phase, so
  prescaled timers do not lose fractions on one-microsecond cooperative slices.
  A compare/overflow event sets the documented mode status bit and is delivered
  only to a registered, unmasked original AOT handler. THREADMAN Timer5 IRQ16
  now enters its static root at offset `0x5a2c`; its original callback programs
  the next target rather than faulting. The 16-module bundle is **27975 reachable
  words**. A 20M-slice startup pass reaches its budget after one reboot with
  Timer5 count `0x103ed9df` and target `0x147689a9`; it does not establish exact
  hardware timing beyond this observed native path. CTest is **17/17** on Windows.
- Static/batched startup tracing now records the last 32 original IOP event
  producer/consumer edges and each SIF0 DMA arm site. This avoided repeated
  slice probes: a 100M-slice pass reached original MODLOAD return offset `0x2964`,
  added as the verified root (`27977` reachable IOP words), then reached an exact
  two-word SIF0 source tag at `0x000d6f80`. Its `count=2` and EE tag
  `0x10000001` are now independently tested as a bounded payload shape.
- The real path subsequently signals MODLOAD event 3 and transfers the two-word
  LOADFILE reply, but the EE tag is non-final: EE SIF0 remains armed (`CHCR=0x184`)
  with both waiting RPC semaphores unsignaled. The original SIFMAN producer that
  armed channel 9 is PC `0x0009c810` (return `0x0009c6b0`). Do not convert this
  CNT tag into a synthetic completion; trace its required final-tag/interrupt
  transition next. Full Windows CTest remains **17/17**.
- Follow-up packet history proves that each short LOADFILE payload is followed
  by original SIFMAN's terminal 16-word tag (`IOP address 0x8009b000`, EE tag
  `0x90000004`). Native EE SIF0 completion has occurred 16 times and its original
  channel-5 callback at `0x0026f568` runs. The live request nevertheless returns
  `0xffffff35` (`-203`) and leaves the original EE caller at `0x002702a4` waiting
  on semaphore 8. Therefore the current blocker is the original LOADFILE
  service-`0x8000000a` request/reply contract, not a DMA terminal-tag or callback
  omission. Decode that service's request fields and only then add its missing
  behavior; do not signal semaphore 8 from the transport layer.

- Independent ELF/config/decoder/CFG/C++ emitter and native C++17 runtime work.
  Current analysis: **32129 reachable words, 361 unresolved items**. Normal `jr ra` returns
  are now separate `return_sites`; counts are not whole-game coverage percentages.
- Full hash-checked native ELF loader, explicit BSS zeroing and SHA-256 implemented.
  Synthetic tests validate malformed images, transactional rejection and digest vectors.
- FPU addition handles exact alignment cases, signed zero, overflow/underflow and
  FCR31 flags; discarded alignment bits deliberately fault pending oracle validation.
  Added SYNC, LQ/SQ address masking, 64-bit/variable shifts, likely branches, MOVZ/MOVN.
- `hg_diagnostic --verify-prefix` loads the ELF, verifies the original 66-instruction
  register reset, poisons `[0x0047b200,0x01992000)`, runs the game's BSS clear and
  verifies its first SetupThread arguments at 0x001001c8. Expected success exit 0.
- `hg_diagnostic --boot` additionally enables independently written boot services:
  SetupThread, SetupHeap, EndOfHeap, CreateSema and SetSyscall; custom handlers run
  translated guest code with a native return gateway. No guest interpreter/JIT.
- Native boot executes through kernel/library setup and constructors into SIF setup.
  Standalone boot now reads real SIF registers and waits for an IOP partner.
  **Original SIF RPC initialization completes on Windows and Linux**: two EE
  commands, IOP reply, both DMA callbacks, event wakeups. `--verify-sif` checks
  completion invariants. Default runner continues into RPC server discovery;
  EE semaphore create/signal/wait/delete and reusable handles now work. Original FILEIO
  heap server registration and EE RPC bind are verified on Windows (`--verify-services`).
- New conservative callback analysis in `tools/hgtool/constants.py` follows declared
  helper ABIs (array constructor a1/destructor a2) using proven local constants,
  including delay slots. Unknown effects, joins and intervening calls kill constants.
  Evidence is reported. Manual pointer-table hints resolve virtual slots; member
  helper 0x00100b40 uses a three-word descriptor with negative word1 for direct calls.
- New `runtime/include/hg/dmac.hpp`: D_STAT W1C status/mask toggles, channel register
  masks, explicit SIF0 destination-chain input. `State::receive_sif0` transfers supplied
  payload to checked RAM/SPR, supports cnt/end and tag interrupts. Unknown priority,
  cnts/stall control, unrelated endpoints and active-register writes fault. Both SIF
  directions now have bounded FIFO transport; CPU interrupt delivery remains partial. Undefined reset fields use zero
  as native initialization policy. SifSetDChain (void ABI) arms receive, never completes it.
- Explicit overlays: source 0x003eb8a8 -> 0x80076000 (0x740), 0x003ebfe8 ->
  0x00082000 (0x28), and 0x003eaa70 -> 0x80075000 (0x330). Game performs copies;
  compiled cases verify bytes first. Last patch handles syscall registrations 0x55-59.
- Manual indirect pointer-table support reads bounded arrays from the checked ELF.
  Startup constructor table [0x00469460,0x0046969c) contains 143 unique entries,
  called at 0x00100b0c. Virtual targets are added only after native callsite/object
  inspection; no automatic guessing. Native dump remains external in TEMP.
- Added COP0 Status/EPC/ErrorEPC moves, EI/DI, no-delay ERET, integer DIV/DIVU,
  MULT/MULTU and pipeline1 variants, MADD/MADDU variants, PMFHL.LW, LWC1/SWC1,
  PSUBB and packed logical operations. Undefined division zero/noncanonical operands
  fault. A signed/unsigned multiply selection bug was caught by synthetic tests
  and fixed (never infer unsigned by checking whether "u" occurs in "mult").
- Fixed CFG discovery of branch-likely with a trap slot: not-taken annulled path
  remains reachable. Emitter faults invalid slots only on paths that execute them.
- Boot services now also implement first EnableIntc, CreateThread (dormant metadata),
  GetThreadId, ChangeThreadPriority without ready competitors, console settings
  query/set. No running-worker scheduler yet. First enable returns 1; priority
  success returns 0, both observed at original syscall return breakpoints.
- Four timer banks implement registers and explicit bus-cycle advance. Reads of
  active counters fault until a scheduler attaches a clock. INTC models request
  latches, W1C acknowledgement and mask toggles; event delivery remains incomplete.
- Native compatibility kernel data is explicitly synthesized in low 512 KiB RAM
  when boot services initialize. Syscall table is at physical 0x10000, native gateway
  addresses at 0xff000000 + id*4, guest callback return at 0xff001000. This is not
  BIOS memory. Other low kernel accesses and unimplemented MMIO fault.
- Last full Windows test run: **11 CTest targets passed at 20:04**, including new
  asset volume/IRX inventory/relocation/inspector synthetic tests in the tools target.
- Ubuntu 24.04 WSL2, GCC 13.3, Python 3.12.3, CMake 3.28: build and 11 tests passed (including distinct IOP AOT/loader tests).
  Native boot stops at SAME SIF boundary and RAM+register JSON is byte/content identical
  to Windows. RAM SHA256: 8382946378709a42bbef29c833751c3dc3dca3cb2bc82b4202b1aa82d9008d86.
  Linux build: /home/johnn/.cache/haunting-build-linux; Linux snapshot prefix:
  /home/johnn/.cache/hg-native-linux. This WSL clears /tmp between sessions: do not
  build there. Installed Ubuntu g++, CMake, make, libglfw3-dev and libgl-dev plus dependencies.
- OpenGL clear/readback/presentation smoke test passes Windows NVIDIA and Linux WSLg
  Mesa 4.5 core (GLFW 3.3.10 distro package). No game graphics. macOS not tested.
- `tools/hg.py assets`: independently indexed local CVM at origin 0x1800, 770093
  blocks of 2048 bytes, 2425 files +207 directories. Metadata out/assets.json ignored.
  Bounded reads/hashes only, no extraction. `/ADX00/AD_01.ADX;1` size3221504 SHA256
  18a793e4598fb640e21b1c02b2c89904b3a93c93c6e6e0b2a8f7b355e7b24153.
- `tools/hg.py modules`: all 12 local IRXs inspected, metadata out/modules.json ignored.
  `--relocation-base 0x10000` independently dry-runs each module at that base (not a
  linked layout). 13627 relocations: 1036 type2,4989 type4,3801 type5,3801 type6;
  symbol indexes all0, HI16/LO16 always consecutive. No IOP execution/relocation at runtime.
- IOP module placement: `config/iop_modules.toml` reserves [0,0x10000) and
  [0x1f0000,0x200000), places12 standalone IRXs through0x92e90 plus16 embedded
  reboot-image modules at0x94000..0xdd7a0. Manual roots are `{name,offset}` per module.
  Current plan resolves643 exact-version local bindings,64 native ABI bindings and
  leaves73 imports explicit. Native INTRMAN4/5/6/17/18 adapters are configured globally.
  SYSCLIB and STDIO both export stdio1.3; ambiguous providers remain blocked, never chosen
  by file order. No unresolved stub may become empty return.
- Distinct IOP MIPS-I decoder/emitter/runtime implemented: 32-bit registers,2MiBRAM,
  one-instruction pending loads, branch/slot timing. Hazard dependencies deliberately
  fault pending PS2 validation; no devices/COP0/full arithmetic yet. `iop-emit` defaults
  to MODMSIN at0x76000:191 reachablewords,3 imported-service boundaries.
- `runtime/iop_image.cpp` validates module SHA256, segment bounds and original words
  for generated relocation patches BEFORE changing RAM; fills BSS. No runtime decode.
  `iop_source.py` and native region loader now verify whole-container AND member hashes,
  with compiled slice bounds. No extraction. Synthetic tampering/bounds tests pass.
  Integer MULT/MULTU/DIV/DIVU now implemented; undefined divide-zero/overflow results
  explicitly fault. Signed arithmetic uses widened values and avoids host UB.
- `hg_iop_diagnostic --verify-modmsin` passes FIVE original validation-function cases
  on Windows AND Linux: null descriptor, valid descriptor, invalid tag, short entry,
  null entry. Entry without flag stops at0x76364 intrman:17. Valid test descriptor tag
  is2 (not0); pointed entry word must be>=8. These were decoded from the supplied module.
- `tools/hg.py reboot` parses game-supplied IOPRP300.IMG without extraction:19entries,
 16embedded IRXs including SYSMEM,LOADCORE,SIFCMD,SIFMAN,THREADMAN,IOMAN,MODLOAD,
 FILEIO,CDVDMAN,CDVDFSV,LOADFILE,TIMEMANI,ROMDRV,EESYNC,SYSCLIB,STDIO. This is the
 source for missing IOP providers. ROMDIR at0 size320; EXTINFO at0x140 size608;
 first ELF0x3a0. Final alignment padding is absent (file278305 vsaligned278320).
- SYSCLIB now1463 reachablewords,2boundaries (break and loadcore import).
  `--verify-sysclib --module "Haunting Ground (USA)/MODULES/IOPRP300.IMG"` executes
  96 memcpy/memset/strlen cases successfully on Windows AND Linux (length0..64 and
  misalignment with guard bytes), plus5 sprintf cases passed Windows; Linux formatting
  cases not yet rerun individually. Explicit library/ordinal export lookup added.
  Manual indirect hints support target offsets or relocated table offset/count;
  thirteen formatting callbacks ->0x16b0 and121-entry table at0x18c0 decoded locally.
  SIFCMD also compiled/loaded on both:1297 reachablewords,24boundaries; entry stops
  at0x99b2c loadcore:12. Single-module builds still block all imports intentionally.
  LOADCORE1725words: isolated original initialization0xcc..0xd8 plus boot-mode exports
  12/13 pass5 cases Windows/Linux. No full module boot. ADD/ADDI/SUB overflow checks added.
  `iop-audit` reports all28modules without overwriting generated source.
- Current generated IOP source is a **5-module bundle: modmsin,rom_sysclib,rom_loadcore,
  rom_sifcmd,rom_sifman**,5583reachablewords. `--verify-bundle --module
  "Haunting Ground (USA)/MODULES"` passes Windows/Linux. It runs original LOADCORE linker
  helper0x11d8 to patch SIFCMD imports; guarded imported calls then reach translated
  LOADCORE. Static dispatch checks planned J word and unchanged ordinal delay word.
  Bundle loader stages all writes before commit; synthetic late-file failure proves
  destination RAM/registers unchanged. **All12 CTest targets pass Windows and Linux**.
  Input root from common module directory; generated paths relative for portability.
- `hg_system_diagnostic` combines EE and IOP AOT code with shared SIF state and
  cooperative execution. Six-module bundle: MODMSIN, SYSCLIB, LOADCORE, SIFCMD,
  SIFMAN, THREADMAN; **21500 reachable IOP words (before latest device-control table expansion)** after explicit native adapters.
  Original LOADCORE initializes empty boot modes and links SIF imports. Original
  SIFMAN/SIFCMD entries return; SIFCMD export4 publishes CMDINIT then waits for its
  system event. Native EE transport initialization publishes only SIFINIT after
  a real EE receive chain is armed. No synthetic command-ready flag or buffer address.
  SIFCMD export14 calls export4, creates an RPC event, and sends the real reply.
  `--verify-sif` verifies system event bits0x900, EE RPCINIT1, two submissions,
  eight transport steps, empty FIFOs and rearmed EE receive.
- Both transfer paths move actual qwords: IOP channel9 source tags -> bounded FIFO
  -> EE channel5 destination chain; EE channel6 source tags -> reverse FIFO ->
  IOP channel10 destination chain. Synthetic tests check payload/guard bytes,
  backpressure, disarmed receivers and completion. Supported source tags are
  cnt/next/ref/refe/end on EE and original SIFMAN tags on IOP (word-aligned).
  IOP six-word replies retain preceding high lanes in the final qword, matching
  the external oracle capture. One-/three-word fragments and short partial-only
  packets still fault, as do unsupported modes and active channel mutations.
- Native system event ID1 stores actual bits; THREADMAN adapters provide its query,
  create, set and cooperative AND wait. Full THREADMAN startup/thread scheduling is not implemented.
  SIFCMD command/IRQ pointers from original initialization are manual function roots.
  Both IOP and EE DMA dispatch save/restore CPU state and run original AOT callbacks.
  IOP IRQ43 initialization options0x200 are retained; only the observed subset is used.
  EE negative iSifSetDChain is permitted only inside its native interrupt context.
- `--start-sif` is still an isolated IOP diagnostic with flags initially clear; use
  `hg_system_diagnostic` for connected startup. Both generated translation libraries
  are compiled once and reused across diagnostic executables to reduce build time.
- Native cache adapters at LOADCOREoffset1bf0/1cf4 provide coherent-memory fences
  and independent epochs. MFC0 supports processor ID only with one-instruction delay.
  Native hardware probe profile: PRId0x1f,1f801450read1,1d000060read0; observations
  and external register capture provenance in ORACLE.md. Only identical config writes
  accepted; unknown device semantics fault.
- All14 CTest targets passed Windows and Linux at22:05, including original heap
  server binding and the native worker context/interrupt-ownership checks. HG_SIF_BOOTSTRAP_TESTS
  needs the six-module handshake bundle; new HG_FILEIO_BOOTSTRAP_TESTS requires the
  ten-module startup bundle listed in README. `--verify-services` passed Windows:
  actual original FILEIO heap server registration and EE bind via DMA, no injected reply.
- Added native cooperative IOP workers (`iop_thread.hpp`): full register/load-delay
  context switching, priority selection, round robin among equals, interrupt-disabled
  ownership, sleep/wake, timed waits. Native diagnostic clock advances1us per slice;
  this is compatibility timing, not an inferred PS2 cycle rate. Worker stacks reserve
  [0x1f1000,0x200000), separate from existing bootstrap/interrupt stacks. Create/start,
  thread ID/status/priority and single-waiter semaphore services are explicit adapters.
  FILEIO creates its two initial workers and a third file-request worker itself.
- Since22:05 baseline, Windows startup additionally completes original CDVDMAN entry
  and starts CDVDFSV's bootstrap plus3 RPC workers. Linux has not yet tested these
  latest additions. DMA channel3 configuration exists; starting its data transfer still
  faults. Native dmacman4/5/6/8/9/14/15 bind register get/set only; no completion shortcut.
- Added LWL/LWR/SWL/SWR, including documented MIPS-I merge forwarding from a
  preceding pending load. Synthetic consecutive pairs pass all4alignments with
  guarded stores. Other load hazards still fault. BCD RTC command8 returns native
  UTC startup snapshot; command5 copies sticky status; command03:00 reports explicit
  native1.0.0 compatibility profile. Other drive commands fault. No real disc reads yet.
- Event waits now suspend workers and let lower-priority producers run. ModesAND/OR
  plusCLEAR supported; original THREADMAN0xa14b8 clears all bits after result capture.
  Native test covers priority dependency and OR+CLEAR. Multi-waiter clearing order
  has not been verified; avoid claiming complete IOP scheduler compatibility.
- Original IOMAN initializes its tables and built-in device; unsupported CD/DVD device
  operations still return the original missing-driver result. FILEIO retries that worker.
  Native bounded console implements %s/%d/%u/%x/%c/%%; other conversions fault.
  Explicit library_providers selects rom_stdio for stdio1.3 (duplicate supplied exports).
- EE SifGetReg reads shared hardware/software state. SifSetReg supports software
  IDs0..2. SifSetDma accepts a single aligned command descriptor (attr0x44,size16..112),
  snapshots padded payload into native kernel data at0x60000, constructs real tags,
  and arms channel6. Busy native submission capacity returns0; IDs are independent.
  Cooperative instruction stepping after SIF initialization ensures peripherals run
  between consecutive submissions. No completion is manufactured.
- CACHE DHWBIN orders coherent stores and tracks epochs. Native RAM aliases
  0x20000000/0x30000000 cover only32MiB. Game-used0x20000000 cache writeback is an
  explicit native compatibility policy; hardware manual leaves that case undefined.
- PCSX2 v2.6.3 is paused at **0x0026c5a8**, RPC-ready SifSetReg return (id0x80000002,
  value1, v0=1). External `sif-first-dma-20260904.p2s` and `sif-rpc-ready-20260904.p2s`
  capture only memory observations; paths/hashes in ORACLE.md. No captured code used.
  Earlier AddDmacHandler return3 is an opaque ID (native first DMA ID1). SIF setup
  observed v0/a0=0x184; public void ABI confirms constructed CHAIN/TIE/STR setup.
  MMIO memory view shows ?? and is not evidence. OSD breakpoint 0x0026c258 disabled;
  0x001001cc,0x0026bee8,0x0026c038,0x0026c588 still enabled. No image saved.
- Read-only `tools/oracle_capture.py` finds guest RAM via three local ELF fingerprints.
  It excludes executable pages and writes captures ONLY outside repository.
  Verified main RAM mapping in this process: PID 4080, host base 0x7ff670000000.
  Re-locate after restart/process change! RAM has page-sized/protection-split regions.
- Oracle capture:
  `C:/Users/johnn/AppData/Local/Temp/haunting-oracle/ee-ram-20260904T231224539237Z.bin`
  with adjacent JSON metadata, SHA-256
  `b79ae856c5d8fa068f2cb8233c53097696685ca68a4ca6504c060ccf709a1b3f`.
  Observed v0=0x01ffed60, t1=0x01ff7000 after SetupThread. Args at 0x019709c0:
  argc=1, first argv pointer=args+68, null next pointer, launch path matching SYSTEM.CNF.
  Runtime CONSTRUCTS the ABI block; it does not embed the memory capture.
- No images saved. Game/BIOS/emulator source untouched; PCSX2 session and breakpoint
  changed for observations. No commits/publication.

### LOADFILE/SIFRPC completion bookkeeping checkpoint (2026-09-06)
- Independently decoded ROM SIFCMD `rpc_call_request` at relocated `0x00099264`
  preserves the incoming call packet into the selected server record: packet `+0x14`
  -> server `+0x20`, packet `+0x1c` -> server `+0x1c`, packet `+0x20` -> server `+0x24`,
  packet `+0x24` -> server `+0x0c`, packet `+0x28/+0x2c/+0x30` -> server
  `+0x28/+0x2c/+0x30`, and packet `+0x10` -> server `+0x34`.
- The strongest original call-completion path is relocated `0x00099914..0x000999d8`.
  It allocates a completion packet, copies server `+0x1c` into completion `+0x1c`,
  writes command marker `0x8000000a` at completion `+0x20`, and tests server `+0x30`.
  When that field is nonzero it retries SIFCMD send `0x00098580` with command
  `0x80000008`, a 64-byte completion packet, optional result source/size, and the
  EE result destination from server `+0x28` until the send succeeds.
- The same routine then builds actual DMA descriptors: optional RPC result data goes
  to server `+0x28`; the 64-byte completion packet goes to server `+0x20`. Therefore
  these fields are transaction metadata, not native semaphore shortcuts.
- EE-side call construction around `0x002701d4..0x00270218` explicitly writes the
  outbound packet `+0x30` gate before sending command `0x8000000a`. The next task is
  to identify the exact high-level call argument represented by that saved register,
  compare it with the observed LOADFILE request, and then inspect only the native
  state transition that could lose/corrupt it. Do not patch the transport or force
  the EE semaphore.
- No runtime behavior change was made from this checkpoint because the missing or
  corrupt transition has not yet been proven. No tests/builds were run in this step.

## Key decisions
- Conservative traversal instead of linear scanning: the ELF is stripped and has
  one nonempty RWX load segment mixing code/data. File-backed bytes: 3,650,048;
  virtual base: 0x00100000; memory size: 25,763,840; entry: 0x00100008.
- Compile generated C++ ahead of time; switch dispatch selects compiled addresses,
  never runtime-decoded guest opcodes. Optimize into larger basic blocks later.
- Fail at unsupported behavior; no interpreter, fake syscalls or success no-ops.
- GLFW is pinned to official 3.4 commit
  `7b6aead9fb88b3623e3b3725ebb42670cbe4c579`; optional fetch stays in ignored build/.

## Constraints & preferences
- Read AGENTS.md and root SKILL.md before work; apply its fenced briefing format
  for a requested handoff. Keep this file concrete and future-agent-readable.
- OpenGL only for graphics backend. Windows and Linux are targets; other OS support
  depends on toolchain/GLFW/OpenGL. Linux WSL2/WSLg is tested; macOS is not.
- No ps2recomp, including its discovery output. PCSX2 may supply memory observations
  only; independently derive behavior and document provenance in docs/SOURCES.md.
- Temporary files outside repo; any necessary temporary images inside repo go only
  in `UNEEDED images/`. Preserve the user's spelling. Do not add game/BIOS data to Git.

## Superseded historical notes

The following retained notes document earlier milestones only. They are not the
current plan; use **Immediate next steps** above, whose first gate is the
provenance-verified external GetToc record.

1. Continue the original LOADFILE RPC transaction and semaphore release. The game
   now reaches post-reboot LOADFILE bind/wait processing (EE WaitSema at0x26c1e4).
   Server0x80000006 must come from the original LOADFILE worker; inspect real
   packets/server records and add only the required original service paths. Do not
   force the semaphore, bind, or reply.
2. IOP reboot is verified on Windows and Linux: the original SIFCMD dispatcher
   receives0x80000003, the checked IOP image replaces fresh IOP state, original
   services restart, EESYNC runs, and the EE/IOP RPC handshake is renewed.
   `--verify-reboot` covers that milestone. The current 15-module bundle adds
   MODLOAD and LOADFILE; LOADFILE's original worker at entry+0xc8 is explicitly rooted.
3. EE LDL/LDR/SDL/SDR and LWL/LWR/SWL/SWR now have paired-transfer tests. DMA
   control/priority/interleave/ring/stall registers follow the EE manual. All17
   checks pass on Windows and Linux; default runner accepts up to50000000 slices.

1. Continue the actual IOP reboot lifecycle. Default connected runner now receives
   command0x80000003 through the original SIFCMD dispatcher and native registered
   callback at reserved gateway0x1f0030. Exact request: mode0,
   `rom0:UDNL cdrom0:\MODULES\IOPRP300.IMG;1`. It explicitly stops after receipt;
   no reset completion or BOOTEND flag is fabricated. Implement checked image
   loading, fresh IOP service state and real bootstrap before signaling readiness.
   The original dispatcher clears the size byte BEFORE copying the callback packet
   (0x98740); native request validation expects this consumed header.
   Both platforms passed all15 tests through original CD/DVD RPC server binding.
   Windows additionally returns from the first CD/DVD init RPC, removes DMA handlers
   and reaches reboot. Latest native reboot-callback tests/build need validation.
   Current bundle: modmsin rom_sysclib rom_loadcore rom_sifcmd rom_sifman rom_threadman
   rom_fileio rom_stdio rom_ioman rom_sysmem rom_cdvdman rom_cdvdfsv (22057 words).
   EE SifSetDma now handles raw payload+command pairs transactionally; original first
   CD/DVD call uses4-byte payload and64-byte command. Semaphores/DMA callback handles
   are reusable. DisableDmac0x17, RemoveDmacHandler0x13 and idle SifStopDma0x6b work.
   Mid-packet SifStopDma faults. Hardware SifSetReg1..4 updates actual shared address/
   flag state; native return policy is submitted value (original reboot ignores it).
   GCC generated libraries now use -O1 in Release by default via
   HG_FAST_TRANSLATION_BUILDS; disable for later optimization. Latest Linux15-test
   session36600 completed successfully; no build currently running.
   Keep `--verify-sif`, `--verify-services`, `--verify-cdvd` passing. Default200000
   slices is insufficient; use --slices20000000 (with a space), max50000000.
2. Validate thread scheduling and timer clock when required. Created worker entry
   0x0026cea0 is configured; worker remains dormant. ConsoleSettings defaults to
   English, 4:3, RGB, SPDIF on, UTC; original timezone field was 270 (not embedded).
3. Expand syscall/interrupt/scheduling services only with real state transitions;
   semaphore blocking, negative syscalls, full COP0, DMA/VU and IOP remain unfinished.
4. Extend compiled synthetic tests/game-prefix checks. Windows and Linux are now
   verified through the current boundary; keep their states comparable as runtime changes.
5. Continue graphics/platform roadmap in docs/ARCHITECTURE.md. No game renderer yet.

## Watch out for
- Startup uses **PADDUW** (MMI op 28, function 40, subop 16), not PXOR. Initial
  zero-input-only reasoning hid a misidentification; corrected against manual p172
  and guarded by nonzero/saturating tests. Actual PXOR is function 9/subop 19 (p285).
- Do not infer every word in a load segment/function range is executable. Manual
  function end is a traversal stop boundary, not a sandbox preventing outgoing calls.
- No complete architectural exceptions/MMIO/TLB, full kernel/IOP or native asset/CVM service,
  VU/DMA/VIF/GIF/GS, audio, input or saves. Boot-service subset is not a complete OS.
- PowerShell inline Python with nested escaped quotes failed; use single-quoted
  here-strings for multiline Python. `python` is 3.11.9, while CMake found Python
  3.14.4; both passed tools tests. CMake is installed but not on this shell's PATH.
- Reference PDF is in `%TEMP%/hg-ee-instruction-manual.pdf`, and temporary pypdf
  dependencies in `%TEMP%/hg-python-deps`. Pip needed `--no-user --target` because
  local pip defaults conflict with --target. PDF text may need ASCII replacement
  when printed through this Windows console. These are not build dependencies.

## Relevant files and commands
- `README.md`: ordinary portable setup/build instructions and config examples.
- `config/haunting_ground_us.toml`: dump identity and manual discovery hints.
- `out/analysis.json`, `out/listing.txt`, `out/translated.cpp`: ignored, regenerable.
- `runtime/diagnostic.cpp`: game-prefix/boot verification; `runtime/kernel.cpp`: boot ABI; `runtime/opengl_host.cpp`:
  GL host smoke test; `tests/`: redistributable synthetic tests.
- `docs/SOURCES.md`: references/provenance; `docs/ARCHITECTURE.md`: staged roadmap.

PowerShell from the root (existing configured build):

```powershell
python tools/hg.py emit
$hgCmake = 'C:/Program Files/Microsoft Visual Studio/18/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe'
$hgCtest = 'C:/Program Files/Microsoft Visual Studio/18/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/ctest.exe'
& $hgCmake --build build --config Release -j 4
& $hgCtest --test-dir build -C Release --output-on-failure
& ./build/Release/hg_opengl_host.exe --frames 3
& ./build/Release/hg_diagnostic.exe --verify-prefix
```

Initial configure used `-G "Visual Studio 18 2026" -A x64 -DHG_FETCH_GLFW=ON`
and `-DHG_TRANSLATED_SOURCE=C:/Users/johnn/Documents/ChatGPT/Haunting/out/translated.cpp`.

## Oracle UI access (current session)
Computer-use skill was read along with guidance/API/confirmations. Its tools are
available through discovered `tools.mcp__node_repl__js` in functions.exec.
Initialize `globalThis.sky` via `await import('@oai/sky')`. Persistent node bindings:
`oracle` main window (returned id 4850048), `debuggerWindow` (id 2625870), latest
`os` and `ds` states. Reobserve before ANY input; coordinates/indexes are ephemeral.
`sky.list_windows()` returns currently targetable objects; use them, never fake handles.
Accessibility index clicks on Qt modal fields failed as unavailable. Screenshot-backed
clicks work; click editable fields explicitly and inspect focus before typing. Never
save screenshots (automatically displayed). Main menu System/Reset restarts paused;
press debugger Run to reach breakpoint again. User allowed memory oracle use.

Read-only capture example (only if current PID/base still fingerprint-valid):
```powershell
python tools/oracle_capture.py --pid 4080 --locate
python tools/oracle_capture.py --pid 4080 --base 0x7ff670000000 --out "$env:TEMP/haunting-oracle" --note 'describe exact paused state'
```

## Earlier oracle ABI observation
At 18:42, original paused at independently selected PC 0x0026bee8 immediately
following first EnableIntc syscall. GPR v0=1, a0=11, v1=0xffffffffb000f010.
Native first enable returns 1 and updates mask; repeated enable deliberately faults
until its return ABI is validated. No screenshot file saved, no new RAM capture.
Popup input requires get_window_state and screenshotId click in the SAME node call;
use popup screenshot[1] coordinates relative to that popup. IDs from previous calls
can be rejected as unknown. Avoid copying incidental debugger disassembly.

## Current diagnostic artifacts
`runtime/diagnostic.cpp --dump-state` writes external RAM+JSON on boot boundary.
Latest files: `%TEMP%/haunting-oracle/native-current.ram` and `.json` (overwritten
for each diagnostic run, native execution only). `tools/hgtool/inspect.py` checks
image identity, shows own-decoded PC/caller windows and possible a0 object table.
New original capture: `%TEMP%/haunting-oracle/ee-ram-20260904T234911830789Z.bin`,
SHA-256 cbd783d57bc43e67a58bfe1c810838e0ed3c5a58ea50de8fa47c7f049a0bac05.
See docs/ORACLE.md for detailed settings and ABI observations.

## IOP build commands
```powershell
python tools/hg.py iop-emit --iop-module modmsin
& $hgCmake -S . -B build "-DHG_IOP_SOURCE=C:/Users/johnn/Documents/ChatGPT/Haunting/out/iop-translated.cpp"
& $hgCmake --build build --config Release -j 4
& ./build/Release/hg_iop_diagnostic.exe --verify-modmsin
```
Linux cache was configured with HG_IOP_SOURCE pointing to the same source through
/mnt/c/Users/johnn/Documents/ChatGPT/Haunting/out/iop-translated.cpp.

### SNDDRV completion transport checkpoint (2026-09-06)
- The active `cdrom0:\\MODULES\\SNDDRV.IRX` LOADFILE request is now proven to
  return from the MODLOAD helper, re-enter the LOADFILE worker, reach the SIFRPC
  completion builder at relocated `0x00099914`, and take its real completion-send
  branch because LOADFILE server `0x000d6d20 + 0x30 == 1`.
- At relocated `0x0009995c` the same request prepares command `0x80000008` with
  completion packet `0x0009b000`; the SIFCMD sender returns nonzero DMA ID
  `0x00440002` at `0x00099964`. The worker/completion-builder path is therefore
  no longer the leading stall hypothesis.
- EE still waits at `0x0026c1e4` on semaphore 10 for client `0x0198cbc0`; retained
  EE completion snapshots only show earlier completions on semaphore 9. The
  remaining gap is between accepted IOP SIF0 completion DMA and EE completion
  dispatch/semaphore signaling.
- `runtime/system_diagnostic.cpp` now captures the first event-time SIF0 record
  whose payload is command `0x80000008` for client `0x0198cbc0`, including its
  tag, payload, EE destination memory and EE SIF0 channel/status state. This is
  diagnostic-only and does not alter guest-visible transport behavior.
- `runtime/include/hg/iop_dmac.hpp` now zero-fills partial SIF0 source fragments before
  enqueue so retained lanes from prior chunks cannot bleed into the completion payload.
  This replaces the prior `source_lanes` reuse on `words < 4`, which could preserve a
  stale status word as the final payload value at EE side. This fix is scoped to
  transport copy semantics and keeps the same SIF0 completion path and channel
  behavior. The next step is to rerun the focused `--verify-loadfile`/sif capture
  to confirm destination payloads remain `0x8000000a` and wake semaphore 10.
- Next: run the 30-million-slice synthetic-TOC diagnostic and use this snapshot to
  determine whether the active completion reached EE RAM intact. If it did, trace
  EE command dispatch into `0x0026fa60`; if it did not, narrow the channel-9/FIFO/
  EE destination-chain transition. The synthetic TOC remains diagnostic-only.

### Post-LOADFILE SNDDRV RPC checkpoint (2026-09-06)
- The former semaphore-10 LOADFILE stall is resolved with real transport. A focused
  15-million-slice connected run records SIFMAN completion ID `0x10b80002` while
  EE semaphore 10 is active at virtual time 14943752; LOADFILE's worker returns to
  its original RpcLoop rather than remaining at MODLOAD `0xa9ef8`.
- IOP scheduling now normally advances one translated worker instruction per host
  slice. Only while SIF0 source DMA is armed does the connected diagnostic give the
  sender a bounded 64-instruction quantum to reach its original wait, followed by
  one lower-priority dispatch before asynchronous SIF service. This preserves the
  observed blocking window without globally multiplying scheduler cost. Synthetic
  worker tests cover the block/handoff behavior.
- EE `SifSetDma` now accepts the observed single raw descriptor profile (`attr=0`,
  nonzero word-multiple size through 4096) in addition to terminal RPC command
  descriptors. The original startup transfer `0x0219715c0 -> 0x000884c0`, size
  `0x80`, completes through the real SIF1 receiver; raw transfers leave the IOP
  destination channel armed because they request neither remote END nor output IRQ.
  The kernel test covers the 0x80-byte raw transfer and completion state. All 18
  Windows Release CTest targets passed after these runtime changes.
- SNDDRV's original first RPC handler is now an explicit AOT root at module
  `+0xa7b4` (runtime `0x857b4`), proven by the original registration sequence at
  `0x85448..0x8544c`. The full generated IOP bundle contains 24 modules and 68,417
  reachable words.
- A 30-million-slice connected trace proves SNDDRV workers registered both service
  records: queue/server `0x92da8/0x92dc0` has ID `0x77777777`, handler `0x857b4`,
  buffer `0x91480`; queue/server `0x92e04/0x92e1c` has ID `0x77777778`, handler
  `0x86018`, buffer `0x8fc80`. EE executes both post-bind checks at `0x2202c0` and
  `0x220318`, so missing registration is no longer the leading hypothesis.
- At the 30-million-slice boundary EE is later back in the common SIFRPC wait at
  `0x26c1e4` on semaphore 10 while server `0x77777777` contains active client
  `0x019756c0`. The next boundary is therefore the first real SNDDRV RPC
  request/handler/completion path, not `SifBindRpc`. Trace the request function,
  execution through `0x857b4`, and completion command before changing behavior.
