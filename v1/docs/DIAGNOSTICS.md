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
