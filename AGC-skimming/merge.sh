#!/bin/sh

hadd="$1"

for p in single_top_s_chan.nominal single_top_t_chan.nominal single_top_tW.nominal ttbar.ME_var ttbar.nominal ttbar.PS_var ttbar.scaledown ttbar.scaleup wjets.nominal; do
  "$hadd" -fk $p.root $p.*.root &
done
wait
