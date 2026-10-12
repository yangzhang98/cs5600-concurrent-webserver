#!/usr/bin/env bash
# Runs the same load against one server so all three systems get an identical test.
# Usage: ./run_load.sh <port> <name>      e.g.  ./run_load.sh 8001 flask
# Needs wrk:  sudo apt install wrk
set -e
PORT=${1:?port}; NAME=${2:?name}
OUT=results/$NAME; mkdir -p "$OUT"
DUR=20s
for CONC in 8 64; do
  for EP in "hello" "io?ms=20" "cpu?ms=20"; do
    TAG=$(echo "$EP" | cut -d'?' -f1)
    echo "== $NAME  /$EP  connections=$CONC"
    wrk -t4 -c$CONC -d$DUR --latency "http://127.0.0.1:$PORT/$EP" | tee "$OUT/${TAG}_c${CONC}.txt"
  done
done
