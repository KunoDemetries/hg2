#!/usr/bin/env bash
# Runs once, after the container is created. Idempotent and never destructive:
# nothing that already exists is replaced.
set -euo pipefail

cd "$(dirname "$0")/.."

# The tree is a bind mount from the host, so git sees an owner that is not the
# user running it.
git config --global --add safe.directory "$PWD" || true

bash .devcontainer/enable-plugins.sh

# The tools default to `Haunting Ground (USA)/` at the top of the tree. The dump
# is mounted at its own path (docker-compose.yml), so link it there -- the link
# target is the same path on the host, so it is valid outside the container too.
# An existing directory or link is never replaced.
dump_link="Haunting Ground (USA)"
if [ -n "${HG_GAME_DIR:-}" ] && [ -f "$HG_GAME_DIR/SLUS_210.75" ] \
    && [ ! -e "$dump_link" ] && [ ! -L "$dump_link" ]; then
    ln -s "$HG_GAME_DIR" "$dump_link"
    echo "Dump linked: $dump_link -> $HG_GAME_DIR"
fi

# Everything to tell you -- a missing dump, git identity, signing key, the
# commands -- is in welcome.sh, which checks the state as it is *now* and runs
# again in every terminal and before claude.sh starts Claude. Output from this
# script only reaches the creation log, which nobody sees twice.
bash .devcontainer/welcome.sh || true
