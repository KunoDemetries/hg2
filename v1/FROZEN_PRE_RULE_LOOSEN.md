# Frozen pre-rule-loosen snapshot

This `v1/` directory is the preserved Haunting Ground recompilation state from **before the rendering rules were loosened on 2026-09-23**.

## Do not modify this copy

- Treat everything under `v1/` as **read-only archival/reference state**.
- Do not continue optimization, rendering changes, cleanup, regeneration, or gameplay work inside this directory.
- New native-renderer work belongs in the active project outside `v1/`.
- This snapshot preserves the stricter renderer/fidelity rules that applied before the user authorized a secondary native-rendering direction.

## Snapshot scope

Included: source, runtime, tests, tools, configuration, documentation, and the coherent generated `out/` state.

Intentionally not duplicated: `build/`, `emu/`, the local `Haunting Ground (USA)/` game dump, caches, and other protected/binary payloads. Those remain external/local dependencies rather than part of this frozen source snapshot.
