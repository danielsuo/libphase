#!/usr/bin/env bash

DIR=$(realpath "$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && echo $(pwd)/../../)")

for i in `find $DIR/cpu2017/benchspec -maxdepth 3 -type d | grep run`; do
  benchmark=$(basename $(dirname $i))
  speccmds=$(find $i | grep -E "speccmds\.cmd" | head -n 1)
  inputgen=$(find $i | grep -E "inputgen\.cmd" | head -n 1)

  # Copy executable
  rundir=$(dirname $speccmds)
  cp $rundir/../../exe/* $rundir

  echo $inputgen
  if [[ ! -z $inputgen ]]; then
    $DIR/cpu2017/bin/specinvoke -nn $inputgen > $i/inputgen.sh


    # Delete last line (speccmds exit: rc=1)
    sed -i '$d' $i/inputgen.sh

    chmod +x $i/inputgen.sh
    $i/inputgen.sh
  fi
done
