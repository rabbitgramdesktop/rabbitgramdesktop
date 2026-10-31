#!/bin/bash
set -e
FullExecPath=$PWD
pushd `dirname $0` > /dev/null
FullScriptPath=`pwd`
popd > /dev/null

RepoRoot="$(cd "$FullScriptPath/../../../.." && pwd)"

# Find DesktopPrivate: check sibling of repo root first, then fallback
DesktopPrivateDir=""
if [ -d "$RepoRoot/../DesktopPrivate" ]; then
  DesktopPrivateDir="$(cd "$RepoRoot/../DesktopPrivate" && pwd)"
elif [ -d "$HOME/Telegram/DesktopPrivate" ]; then
  DesktopPrivateDir="$HOME/Telegram/DesktopPrivate"
fi

if [ "$#" == "0" ]; then
  set -- bash
fi

CpuCount=$(nproc)
DockerCpus=$((CpuCount > 4 ? CpuCount - 2 : CpuCount))
MemTotalGb=$(($(grep MemTotal /proc/meminfo | awk '{print $2}') / 1048576))
DockerMemoryGb=$((MemTotalGb > 12 ? MemTotalGb - 4 : MemTotalGb))

MountArgs=(
  -v "$RepoRoot:/tmp/rtgdesktopdev"
  -v "$RepoRoot:/usr/src/tdesktop"
)

if [ -n "$DesktopPrivateDir" ]; then
  MountArgs+=(
    -v "$DesktopPrivateDir:/tmp/DesktopPrivate"
    -v "$DesktopPrivateDir:/usr/src/DesktopPrivate"
  )
fi

if [ -d "$HOME/.cache/ccache" ]; then
  MountArgs+=(
    -e CCACHE_DIR=/var/cache/ccache
    -v "$HOME/.cache/ccache:/var/cache/ccache"
  )
fi

TTY_ARG="-i"
if [ -t 0 ]; then
  TTY_ARG="-it"
fi

docker run $TTY_ARG --rm \
  --cpus=$DockerCpus \
  --memory=${DockerMemoryGb}g \
  -u $(id -u) \
  -w /tmp/rtgdesktopdev \
  "${MountArgs[@]}" \
  tdesktop:centos_env "$@"
