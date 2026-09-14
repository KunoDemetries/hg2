# Performance measurement

## Active measurement,2026-09-13 continuation

Latest resumed checks supersede paused notes below. Replay25 verifies the
RGB24 reduction: matching26M..31M takes9.1022s versus10.7102s in23 (1.177x
throughput); runner raster cost falls3.1498242s to0.9096532s for298draws.
150 matching movie CSC starts span4.969730modeled seconds and9.0565host
seconds (median59.5ms), still below natural30fps. Full32M ends in39.126s with
native-iop-budget. Guest CSC times and OPENING read range agree exactly.
472of473 sampled hashes agree in order; unique samples are in the pre-opening
corridor/menu transitions, not conflicting opening images. Complete-VRAM
synthetic tests were already passing before this run.

A zero-tail IDCT experiment passed1280 sparse/dense/cancellation reference
cases but replay26 took9.9283s over the same interval, with raster also slower.
No speed benefit established; the experiment was reverted. Keep the expanded
mathematical regression. Device sampling now separates IPU/scratchpad, VIF/GIF
and raster phases; replay27 measures the restored transform. External evidence:
%TEMP%/haunting-toc-probe/speed-replays23-26.json. These are host optimization
checks under the experimental issue-slot clock, not physical-console parity.

Replay28 verifies aligned CSC input-lane consumption against restored27:
26M..31M9.9756s ->9.1673s (1.088x throughput);150 CSC starts span the same
4.969730modeled seconds,9.9176s ->9.1200s host. All474 sampled image hashes
match in order. Final EE RAM, GS VRAM, GS JSON and native machine JSON are
byte-identical; IOP RAM differs only at RTC second/minute bytes0xbfcd1/2.
Synthetic byte-stream comparison covers every initial bit offset0..127,
input starvation, full output FIFO and two consecutive macroblocks.
External speed-replays27-28.json records timing/image counts. This change
consumes the same bytes at the same device-service boundaries; it adds no
guest throughput multiplier and preserves the unaligned extraction path.

Replay29 direct static intra-page branches:26M..31M8.8138s versus9.1673in28.
Final EE RAM, GS VRAM, GS metadata and native machine JSON match exactly;
473common image hashes retain order (one preview sample omitted).150 matching
CSC starts span8.7698host seconds, median56.9ms. This modest single-run gain
does not establish statistical significance alone. Native synthetic loops
compare every budget0..63 against single-dispatch execution, including likely
annulment, J/JAL, delay slots, trace PCs and register state. All21CTest passed.
External speed-replays28-29.json retains the comparison. Indirect/cross-page
targets still use the checked static dispatcher; no runtime decoding is added.

Replay30 restricts MMIO selection and kernel-overlay searches to addresses
that require them.26M..31M8.4318s vs29's8.8138s; EE RAM, GS VRAM/metadata and
machine JSON remain identical. Refreshed21CTest pass including relinked EE
diagnostic. Aliases, protected mapping spans and scratchpad are covered.

Replay31 sustains1066 OPENING CSC events over35.611639modeled seconds in
58.8315host seconds (median53.8ms), versus22's77.9186s. Full60M83.9024s,
native-iop-budget. All opening guest timestamps agree exactly;1313common
sampled images retain order. Final GS VRAM/metadata and machine JSON agree.
EE RAM differs from22 at one retained byte0x47e373 (12 ->11), already present
in25 and stable through31; input pulse timings differ. Original references
place it in the0x47e360 input-related object, but exact field semantics are
not yet established. Do not call22->31 a fully byte-identical state comparison.
IOP RTC bytes differ with launch time. Evidence speed-opening-replay31.json.

IPU31 changes the separable transform loop order across independent X outputs
while preserving each output's U/V summation order.1280 mathematical cases
pass. An external alternating microbenchmark shows matching checksums and
roughly13-15% gain on AC workloads (%TEMP%/haunting-ipu-loop-bench/results.json).
The DCT lookup indexes the existing tables in source order with two64KiB byte
arrays; exhaustive tests compare every16-bit prefix and low-bit extremes to
the linear lookup. Invalid codes and sign/stream consumption remain unchanged.
The combined connected interval improves8.4318s ->8.1394s. Neither optimization
changes modeled timing. Natural host playback remains below target (~61%).

Replay32 AOT fixed-width scalar accesses reduce sustained OPENING58.8315s
to53.6108s (same35.611639modeled seconds/1066events; median49.3ms). All1313
sampled hashes match in order. Final EE RAM, GS VRAM/metadata and machine
JSON match byte-for-byte. Full60M76.8996s, native-iop-budget. All21 refreshed
tests pass, including scalar width/sign/alias/protection/span/watch comparisons.
Evidence speed-opening-replay32.json. The methods inline only aligned valid
user RAM; protected/kernel, device and watched accesses use existing checks.
Current natural-rate fraction is about66.4%, not a displayed-FPS measurement.

Replay33 replaces per-thread modulo in EE/IOP selection with one normalized
cursor and increment/wrap, and combines IOP delayed wakeups with selection.
Priority, equal-priority cyclic order and all wakeups remain unchanged before
execution.160 readiness/cursor combinations per scheduler plus existing locked
worker/blocking fixtures pass. Sustained opening53.1226s vs32's53.6108s;
this small single-run difference is not statistical proof. Final EE RAM,
GS VRAM/metadata and machine JSON match;1311common captures retain order.
All21CTest pass. Evidence speed-opening-replay33.json.

Replay34 word-based IPU prefix reader is rejected:53.42s sustained opening
vs33's53.1226s, despite identical final EE/GS/machine state and all1311sampled
hashes. Existing bit-offset/width and full stream tests pass, but no connected
speed gain is established. Restore the original inline byte reader; retain
the previously verified33 optimizations. Evidence speed-opening-replay34.json.

User paused work during replay24. RGB24 build is complete and all refreshed
regression targets/21CTest pass. Replay24 was interrupted before a complete
opening timing comparison, so no connected RGB24 speedup is established.
Resume with a fresh numbered replay and compare against23; do not treat the
interrupted partial run as a completed benchmark.

Replay22 clears the original audio stall:1066 OPENING CSC starts over35.611639
modeled seconds/77.9186host seconds, median34.463/71.4ms respectively. Reads
11202sectors2113282..2124483; visible cellar scenes advance. Full60M budget
takes111.424seconds. See external opening-replay22-evidence.json. There are
additional conversions between CAPCOM and OPENING; do NOT label every CSC
after sequence185 as OPENING. First OPENING CSC in22 is219 at24,367,072us.
Replay23 directly times nonempty runner GS batches.26M..31M:3.15seconds in
rasterization of10.7host seconds. Inline FINISH/CLUT rasterization is excluded.
Checked RGB24 sprite pixel-stage reduction is building; generic pipeline/full
VRAM equivalence fixture including masking and texture feedback passes. No
connected speed improvement yet; verify fresh replay24 after the build.

Replay18 enters OPENING and reads1322sectors2113282..2114603. Four CSC
conversions then stall through60M modeled us; host runtime57.7958s is therefore
not a playback-speed measure. Original audio clock clamps to zero because
produced8192 minus queued16384 is negative. Native MODLOAD temporary storage
overlapped the original SYSMEM audio synchronization allocation; original
DS2O_S1.IRX bytes exactly identify the overwrite. Connected native allocations
now use the original AOT SYSMEM allocator, with dedicated call stacks reserved.
Reboot and native/Python suites pass. Replay20 reaches a previously uncompiled
CRI transfer-command switch at12.383634M us, before movie playback. The bounded
14-entry original/live table is added; connected replay after rebuild is pending.
No natural playback rate or sustained OPENING advancement is yet verified.

Replay16 after fixed-UV hoisting:185 CAPCOM CSC starts, median host interval
55.35ms versus53.05ms in replay15 (fresh extraction from its log). Modeled median
33.3655ms. No connected gain demonstrated. Replay16 times out from the active
New Game menu to missing demo case12e534 at54,059,827us/77.2441s; its Cross was
too early during fade. Replay17 waits for
the highlighted New Game row and advances to missing menu-switch case12e578.
Seven original/live table entries at44e8a0 are independently verified and added,
along with six downstream movie/demo descriptors. No OPENING consumption yet.
Replay17 stops24,263,393us/30.1982s; median CAPCOM host55.85ms. Replays16/17
both preserve replay15's first149 captured hashes exactly. External comparison:
clock-bursts-16-17-timing.json. Replay16 is NOT a budget exit.

Latest handoff: replay15 visibly reaches title and New Game menu, then missing
130a40 at41,780,650modeled us /52.6577host seconds. No OPENING consumption yet.
Pending fixed-UV sprite optimization hoists identical integer U calculations
per column and V per row while retaining row-major shading. gif_tests passes;
diagnostic regeneration/build and connected measurement remain pending.
Do not claim a speedup: latest measured CAPCOM median is33.365ms modeled and
55-56ms host (~18 conversion events/sec, not measured displayed FPS).

Replay10 after ordinary-return/DC changes stopped at missing1cda10 after
19,274,631us in22.1177 host seconds. All149 sampled hashes match replay9
in identical order; capture spans18.0188s vs18.2978s. No measured overall
speedup or natural timing claim. Original CAPCOM raw picture-start-code count
is187 (nominal6.2333s at30fps); packet demultiplexing/presentation timing still
needs verification before treating that count as exact movie duration.

Original1cd4e8/f4 registers1cda10 through1c5388; replay10 object+78 agrees.
That callback is now rooted; downstream3cff70 table matches original and its
slot18 target1e32b8 was already compiled. No shared implementation copied;
narrow non-rendering reference search only corroborated containing function.
External stream-completion-callback.json retains evidence.

IPU CBP/DC/DCT lookups now extract one32-bit prefix and compare shifted views,
retaining exact codebook, sign, consumption and faults. Existing complete
codebook/mask native tests and all21 CTests pass. No isolated speedup claimed.
Optional profile now logs successful contiguous disc-read ranges and observed
CSC begin/end times (decode/conversion events, not screen presentations).
From original ISO DATA.CVM extent1460594 plus CVM origin3sectors, CAPCOM starts
at LBA2111088 (1704sectors), OPENING at2113282 (54366sectors).

Replay11 ends at default callback1c4ce8 after19,274,818us/21.6912host seconds.
Disc telemetry verifies all1704CAPCOM sectors consumed; no OPENING read.
182 CSC starts span6.732996modeled/10.6956host seconds, median interval
33.365ms modeled/55.4ms host.158/181host intervals exceed50ms; only2modeled
intervals do. This separates host slowness from the roughly30Hz median modeled
conversion cadence; it does not establish frame presentation or complete decoding.
Evidence: clock-bursts-11-csc-timing.json and startup-clock-bursts-11.log.

Byte extraction optimization collects up to8bits per source byte while
preserving MSB-first stream order, little-endian qword storage and read-only
position semantics. Independent bit-by-bit tests cover8patterns, all128start
positions and widths0..32, including qword boundaries. Runtime tests pass;
replay12 passes both former stream callbacks and reaches100Mquanta without
fault. Its185 CSC conversions have median host interval56.05ms: no measured
speed gain over replay11. The first149 sampled image hashes remain identical.

Replay12 also exposed4830submitted but unexecuted draws after CAPCOM. Servicing
GS drawing in the device phase (GS manual pp38-43) independently of previews
reveals the red corridor, replay13 capture149. The original scene waits for
Start; replay14 presses it through the controller and progresses until an
oversized IMAGE upload. This is a functional transport stop, not decoder
performance. Synthetic memory-oracle evidence and the pending correction are
documented in ORACLE.md. No OPENING consumption or natural host rate yet.

Target: reproduce the original program's presentation cadence on desktop hardware.
Current diagnostic slices are cooperative work boundaries, not PS2 cycles or video
frames. One slice advances the diagnostic clock by one microsecond. Faster host
execution must not silently change this clock, device order or instruction budget.
Real-time gameplay is not verified.

## Profiling

`hg_system_diagnostic --profile` samples roughly one in4096 slices using a mixed
slice index to avoid locking to periodic loops. It prints host nanoseconds by
phase, elapsed throughput and the most frequent sampled EE boundary PCs. With
`--checkpoint-every`, reports appear during a long run as well as at exit/fault.
Boundary-PC frequency is not a per-function CPU-time profile. Small phases include
timer overhead; rare I/O or rendering spikes can be missed. Elapsed time includes
final diagnostic output. Compare identical workloads and retain state captures.

## Windows startup dispatch experiment, 2026-09-13

Release,30M slices, same28-module IOP bundle, synthetic TOC probe, no host input or
preview. All runs used sampling. External logs/captures live in
`%TEMP%/haunting-toc-probe/` with the following prefixes:

| Prefix | EE/IOP dispatch | Host seconds | Slices/second |
|---|---|---:|---:|
| profile-baseline | monolithic/monolithic |35.9562|834348|
| profile-pages | partitioned/monolithic |14.6356|2049800|
| profile-both-pages | partitioned/partitioned |4.56141|6576920|

The observed overall gain is7.88x on this workload, not a gameplay-FPS claim or
a controlled multi-run statistical estimate. Initial sampled costs: EE47%, IOP31%,
diagnostic history2.5%. Both generators now emit separate, non-inlined C++ functions
for4KiB address regions and a static outer dispatch. All instruction effects,
delay slots, trace boundaries, faults, native waits and execution budgets remain.
This is build-time organization of native code, with no instruction decoding/JIT.

EE RAM and GS VRAM match byte for byte. Every captured metadata field matches.
IOP RAM differs only at0xbfcd1/2, the seconds/minutes of the original RTC result;
the runner initializes RTC from host UTC at launch. Comparison retained in
`profile-state-comparison.json`. Native tests cover cross-region delay slots,
multi-step budgets, unknown addresses and whole-budget return on blocked IOP waits.

Next: profile connected movie playback, quantify actual guest presentation events
separately from preview sampling, and measure frame-time distribution. Do not
increase per-slice quanta or skip waits merely to report faster playback. Changes
to the provisional timing model require independent hardware/program evidence.


## Experimental work/time correction

The legacy diagnostic schedules one EE boundary per microsecond, while its
bootstrap uses a larger special budget. This is not an EE cycle model. Local
EE manual p36 gives147.456MHz bus; Core manual p78 gives processor/bus ratio2.
Core pp14/18 describe dual issue and stalls, so an AOT boundary cannot be called
one accurate hardware cycle. IOP clock profile is36.864MHz.

Opt-in `--clock-profile issue-slots` now allocates294/295 EE and36/37 IOP native
boundaries between1us device updates, retaining fractional budgets exactly.
Cooperative bursts yield at native services, syscalls and JR RA returns. A
build-time-proved NOP*/RAM-load/zero-branch/NOP loop may yield while its real
aligned RAM flag remains zero; MMIO, watched loops and overlays are excluded.
This avoids millions of redundant host polling instructions. It does not signal
completion or change the flag. Unused budgets at yields, branch bundles,
interrupt service budgets and cache/pipeline effects remain timing limitations.
Legacy timing remains default; this experimental path is not yet validated for
natural movie playback. The first burst probe exposed an overrun past the IOP
bootstrap return sentinel; return-boundary regression coverage was added.


Connected burst replay4 completed30M microsecond quanta in52.1107 host seconds,
with129 unique sampled images including advancing CAPCOM characters. This is
about0.576 modeled seconds per host second, NOT natural movie FPS. Earlier replay3
at the card prompt reached1.009 modeled seconds per host second. Workload matters.
A real nested-bootstrap stack collision was fixed using an independently allocated
0x2000-byte IOP stack for synchronous nested initializers; suspended bootstrap CPU
and RAM stack now survive. Full21 CTests pass after this change. Preview publication
for experimental pacing follows field events with a16ms host cap; RGB output uses
one bulk write. It does not flush guest draws. Short host input pulses replace the
old1.5-second holds, which repeated menu selection at faster execution speeds.

User explicitly authorized PCSX2 inspection for timing, without copying (2026-09-13).
Local installed `Documents/PCSX2/inis/PCSX2.ini` inspection found EECycleRate=0,
EECycleSkip=0, FramerateNTSC=59.94, NominalScalar=1. This corroborates our existing
scan cadence, not instruction timing or full-speed movie playback. No PCSX2 source,
rendering algorithm or game patch was inspected/copied in this check.


The IPU diagnostic pump previously moved one128-bit qword per microsecond,
imposing a16MB/s cap unrelated to hardware throughput. EE manual pp41-43 defines
peripheral DMA arbitration slices of eight qwords. The opt-in profile now services
up to eight real FIFO transfers per direction per quantum, stopping on backpressure.
This does not implement cycle-exact bus arbitration. Replay5 -> replay6 reduced
CAPCOM-to-scene host time58.8558s ->27.7091s (different subsequent scene stop).
All148 captured unique frame hashes are identical in order; this is sampled-image
agreement, not a claim to capture every original video frame. Replay6 reached
20,447,297us and missing3913b0, passing prior120e80. Focused4 regressions passed.
Original/live table47a790 has14 methods; all independently verified/rooted as a
batch,157508 words976 boundaries. Long opening remains unverified.

Profiler now additionally weights EE quantum-entry PCs by sampled EE execution
cost, so frequently visited cheap waits can be distinguished from expensive
quanta. A quantum can enter another function: this is still not per-function
CPU attribution. Native straight-line dispatch experiment is under validation;
retain profile-before-fallthrough as its exact legacy30M baseline.


Original movie target independently verified: first sequence headers of CAPCOM
and OPENING both carry frame_rate_code5, sequence-extension n=0,d=0 and
progressive_sequence=1. H.262 Table6-4 (PDF p52) therefore specifies30 progressive
frames/s for these sequences. This is distinct from NTSC59.94 field scanout.
External `movie-rate-evidence.json` records raw header bytes, original CVM offsets
and parsed fields. No source-code or PCSX2-derived movie implementation was used.


Straight-line EE goto dispatch passed86 Python/native translation tests and exact
legacy benchmark state comparison (`profile-fallthrough-comparison.json`): EE RAM,
GS VRAM and metadata match; only IOP RTC second/minute bytes differ. Timing4.56995s
before vs4.66269s after shows no legacy single-boundary gain. Replay7 burst reached
scene in25.8812s vs replay6 27.7091s, but different scene roots/input make that an
uncontrolled comparison. Replay8 removed four obsolete movie instruction watches
that enabled per-instruction trace recording;20,447,349us took22.4743s. Capture poll
now10ms so it can observe faster content. These are not yet30fps playback results.

Current unpromoted optimization recognizes only a LW/SRL8/ANDI1/NOP*/BNE/NOP
loop, whose loaded register already equals1. It may park at the header only for
IPU CHCR3/4 when the actual modeled STR bit is set. CHCR reads have no completion
side effect; DMA continues through real FIFO transfers outside the CPU burst.
All other addresses execute normally; watches and overlays disable parking.
87 Python tests and native tests check release on actual STR clearing, unrelated
RAM fallback, watched-loop execution and mutation/mask rejection. Connected replay
validation is pending the generated build.


Replay9 with bounded DMA poll parking reached20,580,407us in21.5898s and stopped
at130460, a follow-on scene descriptor already identified for the next build.
No natural30fps or long opening verification yet. Final pending build43368 changes
ordinary returns to preserve the remaining CPU quantum; only actual host return
sentinels stop on JR RA. Native tests cover both ordinary continuation and sentinel
stopping. An IPU DC-only fast path uses the same two basis products and existing
rounding when reconstructed AC terms are all zero. Runtime tests pass for constant
inverse-transform values across six block destinations, four precision settings,
negative/positive values and clipping edges. Connected validation is pending;
these optimizations have no claimed measured speedup yet. Fresh replay10 helper is
prepared outside the repo. Rebuild/relink after43368 before starting it.


## Major speed audit and denser sampling (2026-09-13)

Replay35 uses1/256 host-only sampling and separate IPU input/output phases.
At matching26M..31M checkpoints sampled EE cost13.2965ms exceeds IPU input
5.1271ms, VIF/GIF4.6609ms, IOP3.0359ms and IPU output2.3574ms. These are
sampled totals, not full phase wall times; large sparse device events vary.
Full32M diagnostic completes in34.4431s with native-iop-budget. No speedup
claim is based on sampling alone. Earlier1/4096 reports remain historical.

Fresh full original-input audit and unresolved-site classification are recorded
in DECODING_SCAN.md. No evidence currently attributes measured opening host
slowdown to an omitted executed EE routine. Guest polling and incomplete cycle
modeling still require care; explicit translation gaps remain for later paths.

Local PCSX2 timing settings remain unmodified at normal cycle rate/skip and
NominalScalar1. Official https://pcsx2.net/docs/troubleshooting/performance/
distinguishes internal FPS, video output VPS and speed percentage; conversion
throughput here must not be reported as presented FPS. The page also identifies
host power configuration as a performance consideration. This host is a Ryzen7
9800X3D using Balanced; no power-plan changes or new live oracle run were made.
No PCSX2 implementation, algorithm or rendering material was copied.


Byte-copy batching replay36 was rejected:1066 events retain exact guest times,
final EE RAM/GS/machine state and all1311sampled images match33; host span
54.2326s versus53.1226s shows no gain. Recognizer/helper and candidate-only
tests were removed. Original115bb8..115bf0 inspection shows wider copying
precedes this remainder loop. Next candidate reduces redundant ordinary-RAM
checks for128-bit access, with full-span atomic store validation and fallbacks.


Replay37 retains guarded128-bit ordinary-RAM access:51.8379host seconds for
1066 conversions across35.611639modeled seconds, median48ms (~68.7% modeled
rate). Guest timestamps, final EE RAM/GS/machine state and1311sampled images
match33 exactly. All21 refreshed tests pass; six startup tests rerun after
successful relink. Earlier link collided with test execution; serial relink
resolved the file lock. No timing run overlapped compilation or tests.

Separate original-formula IDCT benchmark built with MSVC /arch:AVX2 is about10%
faster across1/16/64 coefficient inputs with matching sampled checksums. Results
are external haunting-ipu-arch-bench/results.json, not whole-game evidence.
No project architecture flag was changed. Native function sampling38 is next;
that replay is diagnostic-only because thread sampling perturbs host timing.


Native profile39 captured3254 main-thread instruction pointers with0context
errors over five seconds during opening. Map-based attribution: EE trace_pc161,
EE r174/w77, rasterize_sprite196, psmct32_word160, IPU rgba126 and IDCT53.
This diagnostic run is not a speed benchmark. It identifies tiny accessor and
disabled trace call overhead as more valuable than AVX2 IDCT work (~1.6% of
samples in IDCT). Candidate40 adds explicit empty-watch guard at emitted EE
trace call sites and forces checked r/w inline. No guest behavior changes intended.
All rendering observations are from this independent native runtime only.


Combined EE trace/inlining40 was rejected and reverted. Guest timestamps,
EE RAM/GS/machine state and1311shared samples match37, but53.2729host seconds
versus51.8379s shows no gain. All21checks passed. Sampling attribution alone
does not establish that a proposed inline change helps. Next41 reuses additive
X/Y terms of our existing framebuffer address formula, preserving live pixel
order and validating the whole GS coordinate domain at three strides/wrap.


Framebuffer-address41 was rejected and reverted: final state and all1066guest
times match37,1311shared images retain order, but opening57.5964s. Total raster
12.4892s vs12.6544s37 did not translate to a whole-run gain. Isolated candidate42
adds build-time EE history removal (default ON; local experiment OFF), separately
from rejected forced inlining. Its synthetic disabled-watch fault and branch
checks are being validated; no connected speed claim yet.


Initial source-publication validation: a fresh headless Release build, with no
game-derived output configured, passes all16CTest entries using Python3.11.
This includes87Python cases and the new trace-disabled synthetic execution test.
The first auto-selected Python3.14 run hit sandbox temporary-file permission
errors; the unchanged tests pass with the project's verified3.11 interpreter.
No new Linux or physical-console verification is claimed.
