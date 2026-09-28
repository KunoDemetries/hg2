#!/usr/bin/env bash
# Runs once, after the container is created.
set -euo pipefail

cd "$(dirname "$0")/.."

# The tree is a bind mount from the host, so git sees an owner that is not the
# user running it.
git config --global --add safe.directory "$PWD" || true

# The tools default to `Haunting Ground (USA)/` at the top of the tree. The dump
# is mounted at its own path (docker-compose.yml), so link it there -- the link
# target is the same path on the host, so it is valid outside the container too.
# An existing directory or link is never replaced.
dump_link="Haunting Ground (USA)"
if [ -n "${HG_GAME_DIR:-}" ]; then
    if [ -f "$HG_GAME_DIR/SLUS_210.75" ]; then
        if [ ! -e "$dump_link" ] && [ ! -L "$dump_link" ]; then
            ln -s "$HG_GAME_DIR" "$dump_link"
        fi
        echo "Haunting Ground dump mounted at $HG_GAME_DIR."
    else
        echo "HG_GAME_DIR=$HG_GAME_DIR has no SLUS_210.75 -- check the path in .env." >&2
    fi
elif [ ! -e "$dump_link" ]; then
    cat <<'MSG'
  ------------------------------------------------------------------
  No game dump is mounted.

  Copy .env.example to .env -- at the top of the repository, not in
  .devcontainer/ -- point HG_GAME_DIR at your own Haunting Ground (USA)
  directory, and rebuild the container. Without it the runtime and
  synthetic tests build, but nothing can be generated or verified.
  ------------------------------------------------------------------
MSG
fi

# The identity git commits with. Nothing is copied from the host: ~/.gitconfig
# and ~/.ssh are volumes this container owns (see docker-compose.yml), so the
# first container asks once and every rebuild after it stays quiet.
if ! git config --global --get user.email >/dev/null 2>&1; then
    cat <<'MSG'

  ------------------------------------------------------------------
  git has no identity in this container yet. It is not taken from your
  host config on purpose -- set it once and the volume keeps it:

      git config --global user.name  "you"
      git config --global user.email "you@example.com"

  For signed commits, put a key in /root/.ssh (also a volume, also
  yours) and point git at it:

      ssh-keygen -t ed25519 -f /root/.ssh/id_ed25519
      git config --global gpg.format ssh
      git config --global user.signingkey /root/.ssh/id_ed25519.pub
      git config --global commit.gpgsign true
  ------------------------------------------------------------------

MSG
fi

echo
echo "Ready. Linux build (separate from any Windows build under build/):"
echo "  cmake -S . -B build/linux -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
echo "  cmake --build build/linux && ctest --test-dir build/linux"
echo "  build/linux/hg_opengl_host --frames 3     OpenGL smoke test in a window"
