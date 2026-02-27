#!/bin/bash
set -e

ScriptPath="$(cd "$(dirname "$0")" && pwd)"
cd "$ScriptPath/../../.."
./configure.sh "$@"
cmake --build ../out --config "${CONFIG:-Release}"
