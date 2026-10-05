#!/usr/bin/env bash
# **Runs on the host, before the container exists** (devcontainer.json's
# initializeCommand -- VS Code, the devcontainer CLI and claude.sh alike).
#
# 1. Compose looks for .env beside the compose file; the project keeps it at the
#    top of the tree. A link rather than a copy so there is one .env, made only
#    when there is one to link to. Both paths are gitignored by `*.env`.
#
# 2. The image is built here, with a plain `docker build`, and docker-compose.yml
#    only names it. Letting the dev container tooling build it does not work on
#    current Docker: the devcontainer CLI (and the VS Code extension, which uses
#    it) writes a generated Dockerfile under /tmp, outside the build context,
#    and Compose's buildx bake refuses that without an fs.read entitlement
#    ("additional privileges requested: pass --allow=fs.read=..."). A normal
#    build of this directory needs no extra privilege. Layer caching keeps it
#    quick when nothing changed; "Rebuild Container" runs it again.
#    CLAUDE_CODE_VERSION in the host environment (stable, latest, or a version)
#    is passed through; add --no-cache by hand to re-fetch a moving channel.
set -euo pipefail

cd "$(dirname "$0")/.."

if [ -f .env ]; then
    ln -sfn ../.env .devcontainer/.env
fi

build_args=()
if [ -n "${CLAUDE_CODE_VERSION:-}" ]; then
    build_args+=(--build-arg "CLAUDE_CODE_VERSION=$CLAUDE_CODE_VERSION")
fi

docker build "${build_args[@]}" -t hg2-devcontainer:latest .devcontainer
