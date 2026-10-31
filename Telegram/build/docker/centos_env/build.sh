#!/bin/bash
set -e

ScriptPath="$(cd "$(dirname "$0")" && pwd)"
cd "$ScriptPath/../../../.."

CONFIG="${CONFIG:-Release}"
BUILD_DIR="cmake-build-$(echo "$CONFIG" | tr '[:upper:]' '[:lower:]')"

# KEEP_GOING=1 lets ninja report every failing translation unit in one
# run instead of stopping at the first; the exit status still fails.
cmake --build "$BUILD_DIR" --config "$CONFIG" ${KEEP_GOING:+-- -k 0} "$@"
