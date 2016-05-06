#!/bin/bash

SCRIPT=$(readlink -f "${BASH_SOURCE[0]}")
SCRIPT_DIR=$(dirname "$SCRIPT")

cd "$SCRIPT_DIR"
cd ../../../

if [ ! -d "glslang" ]; then
  git checkout https://github.com/KhronosGroup/glslang.git glslang
else
  cd glslang
  git pull
  cd ..
fi

cd glslang
mkdir -p build
cd build
cmake ..
make -j$(nproc)

cd "$SCRIPT_DIR"
cmake .
make -j$(nproc)

banner() { #http://unix.stackexchange.com/a/250094
    msg="# $* #"
    edge=$(echo "$msg" | sed 's/./#/g')
    echo "$edge"
    echo "$msg"
    echo "$edge"
}

echo
banner "You can now run ./Hologram in \"$SCRIPT_DIR\""
