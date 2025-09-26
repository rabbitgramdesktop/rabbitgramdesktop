#!/bin/bash

set -e

export DOCKER_BUILDKIT=1

TOTAL_MEM=$(free -b | awk '/^Mem:/{print $2}')
SAFE_MEM=$((TOTAL_MEM - 4*1024*1024*1024))
MAX_MEM=$((16*1024*1024*1024))

DOCKER_MEM=$((SAFE_MEM < MAX_MEM ? SAFE_MEM : MAX_MEM))

FullExecPath=$PWD
pushd `dirname $0` > /dev/null
FullScriptPath=`pwd`
popd > /dev/null

cd "$FullScriptPath/../docker/centos_env"
poetry install
poetry run gen_dockerfile | docker build \
    --memory=$DOCKER_MEM \
    --memory-swap=$DOCKER_MEM \
    -t tdesktop:centos_env -

cd "$FullExecPath"