#!/usr/bin/env bash

DIR=$(realpath "$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && echo $(pwd)/../../)")

mkdir -p $DIR/tmp/perf
cmds=$DIR/tmp/perf/cmds.sh
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
    perf=$(dirname $cmds)/$benchmark.$(( ++counter ))
    touch $perf

    echo "export OMP_NUM_THREADS=1 && cd $rundir && perf stat -a -x, -r 10  -e power/energy-pkg/,power/energy-ram/,instructions:u -I 1 -o $perf.energy.csv $REPLY" >> $invoke
    echo "export OMP_NUM_THREADS=1 && cd $rundir && perf stat -x, -r 10  -e major-faults,minor-faults,page-faults,instructions:k,instructions:u -I 1 -o $perf.faults.csv $REPLY" >> $invoke
    echo "export OMP_NUM_THREADS=1 && cd $rundir && perf stat -x, -r 10  -e branches,branch-misses,instructions:k,instructions:u -I 1 -o $perf.branches.csv $REPLY" >> $invoke
    echo "export OMP_NUM_THREADS=1 && cd $rundir && perf stat -x, -r 10  -e L1-dcache-load-misses,L1-dcache-loads,,instructions:k,instructions:u -I 1 -o $perf.l1.csv $REPLY" >> $invoke
    echo "export OMP_NUM_THREADS=1 && cd $rundir && perf stat -x, -r 10  -e l2_rqsts.references,l2_rqsts.miss,instructions:k,instructions:u -I 1 -o $perf.l2.csv $REPLY" >> $invoke
    echo "export OMP_NUM_THREADS=1 && cd $rundir && perf stat -x, -r 10  -e LLC-load-misses,LLC-loads,instructions:k,instructions:u -I 1 -o $perf.llc.csv $REPLY" >> $invoke
  done < $raw

  # Make script executable
  chmod +x $invoke

  # Append script to parallel input
  echo $invoke >> $cmds
done

