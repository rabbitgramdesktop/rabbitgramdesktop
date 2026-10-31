#!/bin/bash
set -e

FullExecPath=$PWD
pushd `dirname $0` > /dev/null
FullScriptPath=`pwd`
popd > /dev/null

"$FullScriptPath/run.sh" bash -lc "cmake --build cmake-build-debug --config Debug --target Telegram ${KEEP_GOING:+-- -k 0} \"\$@\""
cd "$FullExecPath"
