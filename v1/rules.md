# Change workflow

Read this file and `learned skills.md` before implementation, adjustments, or mass
edits. These rules complement `AGENTS.md`; the user's current instructions govern.
Keep the workflow practical: batch related work, reuse evidence, and avoid repeating
expensive checks without a relevant change or unresolved question.

1. **Plan the change.** Identify the observed problem, evidence, suspected cause,
   next measurable milestone, and what would disprove the hypothesis. Prioritize
   runtime speed and correct map entry before substantial new gameplay work.
2. **Write out the changes before editing.** Record the proposed files and behavior,
   correctness source, expected performance effect, and validation in the current
   section of `docs/PROGRESS.md`. Distinguish facts from hypotheses.
3. **Copy the current state.** Before modifying existing files, save their current
   contents and a file/hash manifest outside the repository under the task's temp
   directory. Include the dirty working contents, not just HEAD. Record the backup
   path. Record new files as new. Preserve baseline binaries/artifacts when needed
   for comparisons. Restore only the failed experiment's changes, never unrelated
   user or inherited work.
4. **Implement and test.** Make a bounded change, run relevant regressions, then
   replay the original program through the affected stage. Compare output, guest
   state, input schedule, clocks and faults with the saved baseline. Unsupported
   behavior must remain explicit; never bypass original calls to reach a milestone.
5. **Investigate correctness and speed together.** Record progress and before/after
   performance under comparable conditions. Investigate an unexplained regression;
   do not accept a large negative effect without finding its cause. Establish whether
   the change is faithful to console behavior using original inputs/specifications
   and permitted independent observations. If faithful behavior costs more, retain
   correctness and reprofile/optimize toward console speed. Never hide the cost by
   changing guest clocks, dropping required work, or disabling correct rendering.
   Actual console frame rate, modeled time, generated video updates and window
   swaps are different metrics; label them. State-equivalence with an old build or
   an emulator observation alone does not prove full 1:1 console fidelity.
6. **Keep reusable lessons.** Add a successful technique to `learned skills.md`
   when evidence shows meaningful progress consistent with verified console
   behavior. Record the scope and remaining fidelity limits. If an approach is
   disproved, document why and the conditions under which it must not be repeated
   in its failed-skills section. Distinguish a generally invalid approach from an
   inconclusive experiment or an optimization that merely did not help this workload.

Each result should record: hypothesis/change, backup, validation/evidence, measured
speed, fidelity confidence, decision, and next action. Maintain detailed run data in
the existing performance/provenance documents; keep reusable lessons concise and
searchable rather than duplicating the entire work log.
