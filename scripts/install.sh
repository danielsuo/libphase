#!/usr/bin/env bash

# Assuming you're in a python virtual environment
pushd python
pip install -e .
popd

mkdir build
pushd build
cmake ..
make -j
popd

ln -s $(pwd)/build/compile_commands.json .

# Install spec
confirm=$(wget --quiet --save-cookies /tmp/cookies.txt --keep-session-cookies --no-check-certificate "https://docs.google.com/uc?export=download&id=$FILEID" -O- | sed -rn "s/.*confirm=([0-9A-Za-z_]+).*/\1\n/p")
wget --load-cookies /tmp/cookies.txt "https://docs.google.com/uc?export=download&confirm=$confirm&id=$FILEID" -O cpu2017.zip && rm -rf /tmp/cookies.txt

unzip cpu2017.zip

pushd cpu2017
./install.sh
ln -s $(pwd)/../ref/spec/gcc-mixed_asm_C.cfg config/default.cfg
./bin/runcpu -a build all
./bin/runcpu -a run all --fake

rm -f cpu2017.zip

# Generate inputs
./scripts/spec/setup.sh

popd # cpu2017
