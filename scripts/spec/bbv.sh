#!/usr/bin/env bash

DIM=$1000000

DIR=$(realpath "$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && echo $(pwd)/../../)")

mkdir -p $DIR/tmp/spec_$1m
cmds=$DIR/tmp/spec_$1m/cmds.sh
rm -f $(dirname $cmds)/*

for i in `find $DIR/cpu2017/benchspec -maxdepth 3 -type d | grep run`; do
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
    bbv=$(dirname $cmds)/$benchmark.$(( ++counter )).bbv
    touch $bbv

    echo "export OMP_NUM_THREADS=1 && cd $rundir && valgrind --tool=exp-bbv --bb-out-file=$bbv --interval-size=$DIM $REPLY" >> $invoke
  done < $raw

  # Make script executable
  chmod +x $invoke

  # Append script to parallel input
  echo $invoke >> $cmds
done

