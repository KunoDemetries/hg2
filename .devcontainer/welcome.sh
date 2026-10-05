#!/usr/bin/env bash
# What still needs doing in this container, and how to work in it. Read-only:
# it checks the current state and prints, nothing else, so it is safe to run
# any number of times.
#
# post-create.sh's own output only ever reaches the creation log (VS Code hides
# it, `devcontainer up` buries it in the build log, and a reused container never
# runs it again), so this is what actually puts it in front of you:
#   - claude.sh runs it before starting Claude,
#   - the image's .bashrc runs it in every interactive terminal (full the first
#     time in a container, only the to-do items after that),
#   - post-create.sh runs it last, for the creation log.
#
#   welcome.sh           to-do items + the command reference
#   welcome.sh --brief   to-do items only
#
# Exits 10 when there is something to do, so callers can wait for a keypress.
set -uo pipefail

cd "$(dirname "$0")/.."

brief=0
[ "${1:-}" = "--brief" ] && brief=1
todo=0

# The game dump. post-create.sh links it; this says when it could not.
dump_link="Haunting Ground (USA)"
if [ -n "${HG_GAME_DIR:-}" ]; then
    if [ ! -f "$HG_GAME_DIR/SLUS_210.75" ]; then
        todo=1
        echo "  ! HG_GAME_DIR=$HG_GAME_DIR has no SLUS_210.75 -- check the path in .env."
    elif [ ! -e "$dump_link/SLUS_210.75" ]; then
        todo=1
        echo "  ! \`$dump_link\` in the workspace is not the mounted dump -- remove it and"
        echo "    run: bash .devcontainer/post-create.sh"
    fi
elif [ ! -e "$dump_link/SLUS_210.75" ]; then
    todo=1
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
    todo=1
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

  Then print the public key and add it to your git server (GitHub:
  Settings > SSH and GPG keys > New SSH key, key type "Signing Key";
  Gitea/Forgejo/GitLab have the same under SSH keys). Add it again as
  an "Authentication Key" if you also push over SSH with it:

      cat /root/.ssh/id_ed25519.pub

  Pushing over HTTP(S) instead: the first push asks for your password
  or token and git keeps it in /root/.config/git/credentials (the same
  volume), so it is asked once, not on every push or rebuild.
  ------------------------------------------------------------------

MSG
elif [ -f /root/.ssh/id_ed25519.pub ] \
    && [ -z "$(git config --global --get user.signingkey 2>/dev/null)" ]; then
    # A key is in the volume but git is not signing with it yet.
    cat <<'MSG'

  A key exists in /root/.ssh but git does not sign with it. To use it:
      git config --global gpg.format ssh
      git config --global user.signingkey /root/.ssh/id_ed25519.pub
      git config --global commit.gpgsign true
  and add the output of this to your git server as a Signing Key:
      cat /root/.ssh/id_ed25519.pub

MSG
fi

if [ "$brief" = 0 ]; then
    cat <<'MSG'

Ready. Linux build (separate from any Windows build under build/):
  cmake -S . -B build/linux -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  cmake --build build/linux && ctest --test-dir build/linux
  build/linux/hg_opengl_host --frames 3     OpenGL smoke test in a window
  python3 tools/hg.py --help                analysis / generation tools
  docker build ...                          container builds run here via podman
This list again: bash .devcontainer/welcome.sh
MSG
fi

[ "$todo" = 1 ] && exit 10
exit 0
