#!/usr/bin/env bash

INS=$1000000000

DIR=$(realpath "$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && echo $(pwd)/../../)")

DISKS=(/data/sda /data/sdb /data/sde /data/sdf /data/sdg /data/sdh /data/sdi /data/sdj)

mkdir -p $DIR/tmp/trace
cmds=$DIR/tmp/trace/cmds.sh
rm -f $(dirname $cmds)/*

COUNTER=0

for i in `find $DIR/cpu2017/benchspec -maxdepth 3 -type d | grep run`; do

  COUNTER=$((($COUNTER + 1) % ${#DISKS[@]}))
  DISK=${DISKS[$COUNTER]}

  benchmark=$(basename $(dirname $i))
  speccmds=$(find $i | grep -E "speccmds\.cmd" | head -n 1)
  echo $benchmark

  # Copy executable
  rundir=$(dirname $speccmds)
  cp $rundir/../../exe/* $rundir

  # Convert commands
  raw=$(dirname $cmds)/$benchmark.raw.sh
  invoke=$(dirname $cmds)/$benchmark.sh
  $DIR/cpu2017/bin/specinvoke -n $speccmds > $raw

  # Delete last line (speccmds exit: rc=1)
  sed -i '$d' $raw

  # Remove all comments
  sed -i '/^#/d' $raw
  
  counter=0
  while read; do
    tracedir=$DISK/trace/$benchmark.$(( ++counter ))
    mkdir -p $tracedir

    echo "export OMP_NUM_THREADS=1 && cd $rundir && $DIR/build/deps/pin/pin-git-prefix/src/pin-git/pin -t $DIR/ref/pintools/tracer/obj-intel64/tracer.so -d $tracedir -i $INS -- $REPLY" >> $invoke
  done < $raw

  # Make script executable
  chmod +x $invoke

  # Append script to parallel input
  echo $invoke >> $cmds
done

