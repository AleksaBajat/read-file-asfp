### Performance

Compiled the baseline using:

```bash
g++ -std=c++20 -O2 -g -fno-omit-frame-pointer main.cpp -o gas_stations
```

Running using:
```bash
taskset -c 0 ./gas_stations # Intel(R) Core(TM) Ultra 7 270K Plus has P and E cores - locking to a P core.
```

Measured using the high resolution clock:

```text
elapsed: 254.08 ms
min: -99.9
max: 99.9
avg: -0.00319572
```

Perf tool report:

```
taskset -c 2 perf record -g --call-graph fp ./gas_stations
```
