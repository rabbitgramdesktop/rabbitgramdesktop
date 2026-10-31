#!/bin/bash
set -e

FullExecPath=$PWD
pushd `dirname $0` > /dev/null
FullScriptPath=`pwd`
popd > /dev/null

HomePath="$FullScriptPath/../.."
cd "$HomePath"

CONFIG="${CONFIG:-Release}"
BuildDirName="cmake-build-$(echo "$CONFIG" | tr '[:upper:]' '[:lower:]')"
ProjectPath="$HomePath/../$BuildDirName"
ReleasePath="$ProjectPath/bin"
BinaryName="rabbitGram"

if [ ! -d "$ProjectPath" ]; then
  mkdir -p "$ProjectPath"
fi

if ! command -v cmake >/dev/null 2>&1; then
  ln -s cmake3 /usr/bin/cmake
fi

cmake --build "$ProjectPath" --config "$CONFIG" --target Telegram "$@"

Error () {
  cd "$FullExecPath"
  echo "$1"
  exit 1
}

if [ ! -f "$ReleasePath/$BinaryName" ]; then
  Error "$BinaryName not found in $ReleasePath!"
fi

echo "$BinaryName build complete!"
cd "$FullExecPath"
