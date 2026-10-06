# The devcontainer image: the toolchain README.md's Requirements asks for, the
# desktop libraries that let hg_game / hg_opengl_host open a window in here, and
# podman for container builds, and Claude Code.
#
# Nothing PS2-related is in this image -- no BIOS, emulator, dump or generated
# source. Those are the user's local inputs (AGENTS.md) and are mounted or
# generated at run time.
FROM debian:trixie-slim

# README.md Requirements: Python 3.11+ (stdlib only), CMake 3.24+, a C++17
# compiler, GLFW 3.3+ and a desktop OpenGL 3.3 driver. Trixie ships Python 3.13,
# CMake 3.31, GCC 14 and GLFW 3.4. clang/clangd are a second compiler and the
# editor's language server.
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
	&& apt-get install -y --no-install-recommends \
		build-essential \
		cmake \
		ninja-build \
		pkg-config \
		clang \
		clangd \
		lld \
		ccache \
		python3 \
		git \
		ca-certificates \
		curl \
		file \
		xz-utils \
		unzip \
		libglfw3-dev \
		libgl-dev \
		libegl-dev \
	&& apt-get clean && rm -rf /var/lib/apt/lists/*

# Fail the image build, not the first configure inside it.
RUN python3 -c 'import sys, tomllib; assert sys.version_info >= (3, 11)' \
	&& pkg-config --exists glfw3 \
	|| { echo "base image no longer satisfies README.md Requirements" >&2; exit 1; }

# Running the OpenGL host in here, not only building it: Mesa drivers (the
# container renders on the host GPU through /dev/dri) and audio clients for the
# bound PipeWire/PulseAudio sockets. mesa-utils is a diagnostic: `glxinfo -B`
# says whether the GPU arrived when a window comes up black.
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
	&& apt-get install -y --no-install-recommends \
		libgl1-mesa-dri libglx-mesa0 libegl-mesa0 libgbm1 mesa-utils \
		libwayland-client0 libwayland-egl1 libxkbcommon0 libdecor-0-0 \
		libx11-6 libxrandr2 libxinerama1 libxcursor1 libxi6 \
		libpulse0 libasound2t64 \
		libudev1 \
	&& apt-get clean && rm -rf /var/lib/apt/lists/*

# The editor side. linux-perf samples native replays (`perf record -g`); user-space
# sampling of our own processes needs no extra capabilities in docker-compose.yml.
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
	&& apt-get install -y --no-install-recommends \
		gdb less procps sudo bash-completion openssh-client ripgrep linux-perf \
	&& apt-get clean && rm -rf /var/lib/apt/lists/*

# **Container builds work in here, with podman** -- the project builds none
# today, but anything that calls `docker build`/`docker run` later works
# unchanged (podman-docker is the `docker` command). There is no docker socket:
# handing one in would give the container the host's daemon. Podman runs as this
# container's root, which the rootless host daemon maps to you, so nested builds
# get no more than you have. docker-compose.yml supplies what it needs
# (SYS_ADMIN inside our user namespace, seccomp unconfined for pivot_root/keyctl,
# a storage volume); the config below covers what a container cannot do:
#  - netns=host: netavark cannot create bridges (netlink EPERM); builds and
#    runs share this container's network instead.
#  - cgroups=disabled: /sys/fs/cgroup is read-only in here.
#  - keyring=false: session keyrings are denied.
#  - short names resolve on Docker Hub, as `FROM debian:trixie` does in Docker.
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
	&& apt-get install -y --no-install-recommends \
		podman podman-docker crun uidmap \
	&& apt-get clean && rm -rf /var/lib/apt/lists/* \
	&& printf '%s\n' '[containers]' 'netns = "host"' 'cgroups = "disabled"' 'keyring = false' \
		'[engine]' 'cgroup_manager = "cgroupfs"' > /etc/containers/containers.conf \
	&& printf '%s\n' 'unqualified-search-registries = ["docker.io"]' 'short-name-mode = "permissive"' \
		> /etc/containers/registries.conf.d/50-docker-hub.conf \
	&& touch /etc/containers/nodocker

# XDG_RUNTIME_DIR for root; docker-compose.yml binds the host sockets into it.
# Created here so it has the 0700 the spec requires.
RUN mkdir -p /run/user/0/pulse && chmod 700 /run/user/0

# Claude Code, from Anthropic's release channel (the npm install is deprecated
# and the binary is self-contained). `stable`/`latest` are moving channels;
# anything else is a pinned version. The checksum comes from the release
# manifest, so a truncated or substituted download fails the build.
ARG CLAUDE_CODE_VERSION=latest
RUN set -eu; \
	base=https://downloads.claude.ai/claude-code-releases; \
	case "${CLAUDE_CODE_VERSION}" in \
		stable|latest) version=$(curl -fsSL "$base/${CLAUDE_CODE_VERSION}") ;; \
		*) version="${CLAUDE_CODE_VERSION}" ;; \
	esac; \
	case "$(uname -m)" in \
		x86_64) platform=linux-x64 ;; \
		aarch64) platform=linux-arm64 ;; \
		*) echo "no Claude Code build for $(uname -m)" >&2; exit 1 ;; \
	esac; \
	curl -fsSL "$base/$version/manifest.json" \
		| python3 -c "import json,sys; print(json.load(sys.stdin)['platforms']['$platform']['checksum'])" \
		> /tmp/claude.sha256; \
	curl -fsSL -o /usr/local/bin/claude "$base/$version/$platform/claude"; \
	echo "$(cat /tmp/claude.sha256)  /usr/local/bin/claude" | sha256sum -c -; \
	chmod +x /usr/local/bin/claude; \
	rm -f /tmp/claude.sha256; \
	claude --version

# The image owns the version; rebuild the container to move it.
ENV DISABLE_AUTOUPDATER=1
ENV DISABLE_INSTALLATION_CHECKS=1

# Mount points for the volumes docker-compose.yml puts here: the SSH directory
# you put your own key in, git's global config, and Claude Code's state. Created
# in the image so the volumes inherit these modes -- .ssh in particular is 0700
# or ssh refuses to use what is in it. **Nothing is copied from the host**.
RUN mkdir -p /root/.ssh /root/.config/git /root/.claude \
	&& chmod 700 /root/.ssh

# HTTP(S) remotes need a password or token on every push unless a credential
# helper keeps it. git's default `store` file, /root/.git-credentials, is on no
# volume and is gone after a rebuild; `cache` is memory only. Store it in the
# gitconfig volume instead, so you log in once. System config (the image), so it
# needs no setup. Plain text, mode 0600, never leaves the volume; no keyring (no
# session D-Bus).
RUN git config --system credential.helper 'store --file /root/.config/git/credentials'

RUN mkdir -p /commandhistory && touch /commandhistory/.bash_history
ENV HISTFILE=/commandhistory/.bash_history

# The to-do list and command reference (welcome.sh) in every interactive
# terminal -- post-create output only reaches a creation log nobody reads. The
# whole thing the first time in a container, only the to-do items after that.
# Non-interactive shells (`devcontainer exec claude`, lifecycle hooks) skip it.
RUN cat >> /root/.bashrc <<'SH'

if [[ $- == *i* ]] && [ -f /workspaces/hg2/.devcontainer/welcome.sh ]; then
	if [ -e /tmp/.welcome-shown ]; then
		bash /workspaces/hg2/.devcontainer/welcome.sh --brief
	else
		bash /workspaces/hg2/.devcontainer/welcome.sh; touch /tmp/.welcome-shown
	fi
fi
SH

ENV DEVCONTAINER=true
