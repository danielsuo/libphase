#!/usr/bin/env bash

# Optional: Valgrind, Perf
sudo apt-get install -y valgrind linux-tools-common linux-tools-generic linux-tools-`uname -r` parallel

# Optional: Allow perf without sudo
sudo sh -c 'echo 1 >/proc/sys/kernel/perf_event_paranoid'
sudo sysctl -w kernel.perf_event_paranoid=1
sudo sh -c 'echo kernel.perf_event_paranoid=1 > /etc/sysctl.d/local.conf'

# Optional: for spec
brew install gcc@9

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
rm -rf __MACOSX cpu2017.zip

pushd cpu2017
./install.sh
ln -s $(pwd)/../ref/spec/gcc-mixed_asm_C.cfg config/default.cfg
./bin/runcpu -a build all
./bin/runcpu -a run all --fake
popd # cpu2017

# Generate inputs
./scripts/spec/setup.sh

