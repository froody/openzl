# 01 Baseline: upstream v0.2.3 vs froody/dev, harness and splits

Decision rows 1-4, 7, 15, 16, 26. Data: `runs/base`, `runs/froody`.

## 1. Question
What is the held-out compression ratio of the stock trained OpenZL models on our LiDAR columns? Does the bigger froody/dev node set give a better held-out ratio than upstream v0.2.3 at the same training budget?

## 2. Change
No ACE change. Two stock binaries:

- `bins/zli_upstream`: OpenZL release v0.2.3 (commit 1730823, worktree `wt/upstream`).
- `bins/zli_froody`: froody/dev (worktree `wt/froody`, branch `ar/base`, commit 5f94c43). froody/dev does not build as is: `zl_quars.h` was never committed (`wt/froody/build.log:230`). Commit 5f94c43 removes the quars registration lines. Everything else is froody/dev.

Run labels:
- `base`: the upstream matrix (`runs/base/<split>/<column>.json`).
- `froody`: the froody/dev matrix (`runs/froody/<split>/<column>.json`).

Harness (`bench.py`, `run_all.py`):
- 20 LAZ files were dumped with `pointcloud_sampler` (row 1). They come from different sources.
- Each column of each LAZ is cut into 256 KiB chunks (the writer default `openzl_chunk_size`). At most 8 MiB per LAZ per column is kept (row 3).
- Training: `zli train --trainer full-split --use-all-samples` on the chunks of the 8 train LAZ files. `points` and `color` use the SDDL profiles. The other columns use builtin profiles (`le-i64`, `le-u16`, `le-i16`, `serial`).
- Test: `zli benchmark -n 1` on the chunks of the 12 test LAZ files. The held-out ratio is the sum of source bytes over the sum of compressed bytes.
- Row 7 fix: the `zli benchmark` CSV has no path column, so `bench.py` now benchmarks each test LAZ directory on its own and writes `<column>.<laz>.csv`.

## 3. Method
- Splits (row 4): A = the original corpus split (`split_A.json`). B and C = random 8/12 re-splits with seed 7. Each split has 8 train and 12 test LAZ files.
- Budget: `--max-time-secs 900` per ACE graph (the `points` SDDL has 5 ACE graphs). Normal mode (3 objectives: size, compression time, decompression time). Default seed. One run per cell.
- Columns: points, gps_time, intensity, color, classification, point_source_id.
- Overall ratio = sum of source bytes over sum of compressed bytes across all 6 columns (`total.py`).
- Speed: from the benchmark CSVs (`dtimeMs`, one iteration, not pinned, machine shared).

## 4. Results
Held-out ratio, recomputed from `runs/<tag>/<split>/<column>.json` (diff = froody vs base):

| column | A base | A froody | A diff | B base | B froody | B diff | C base | C froody | C diff |
|---|---|---|---|---|---|---|---|---|---|
| points | 3.8535 | 3.7976 | -1.45% | 3.8798 | 3.8747 | -0.13% | 3.7605 | 3.7209 | -1.05% |
| gps_time | 10.0590 | 10.0657 | +0.07% | 7.6039 | 7.7665 | +2.14% | 7.0254 | 7.0202 | -0.07% |
| intensity | 1.7922 | 1.7849 | -0.40% | 2.0527 | 2.0527 | +0.00% | 2.0001 | 2.0036 | +0.17% |
| color | 15.0943 | 14.9839 | -0.73% | 12.6612 | 12.6400 | -0.17% | 73.1986 | 73.3901 | +0.26% |
| classification | 53.0332 | 53.0332 | +0.00% | 51.6305 | 51.6305 | +0.00% | 39.7162 | 39.7162 | +0.00% |
| point_source_id | 27.5476 | 27.5476 | +0.00% | 21.0341 | 21.0341 | +0.00% | 14.9956 | 14.9956 | +0.00% |
| **all columns** | 5.6795 (107.52 MB) | 5.6434 (108.21 MB) | -0.64% | 5.6885 (104.18 MB) | 5.7023 (103.92 MB) | +0.24% | 5.5885 (99.19 MB) | 5.5759 (99.42 MB) | -0.23% |

Decode MB/s from the benchmark CSVs (sum of srcSize / sum of dtimeMs), base / froody:

| column | A base / froody | B base / froody | C base / froody |
|---|---|---|---|
| points | 250 / 235 | 473 / 211 | 528 / 222 |
| gps_time | 853 / 261 | 1038 / 86 | 1158 / 91 |
| intensity | 262 / 162 | 784 / 46 | 309 / 161 |
| color | 391 / 39 | 389 / 93 | 415 / 39 |
| classification | 1844 / 42 | 1838 / 79 | 1672 / 46 |
| point_source_id | 1267 / 30 | 1735 / 73 | 1663 / 59 |

Generic coders on the same split A test chunks (`scratch/generic_A.txt`, row 13): points zstd-19 2.64, xz-9 3.25; gps_time 7.52 / 9.63; intensity 1.55 / 1.68. Every later report compares against `base` (upstream) or against a brute control built on top of upstream.

## 5. Verdict
- `base` is kept as the reference for all later work.
- froody/dev is rejected as a base: no reliable gain. The overall ratio is worse on 2 of 3 splits (A -0.64%, C -0.23%, B +0.24%). points is worse on all 3 splits (-1.45%, -0.13%, -1.05%). The only clear gain is gps_time on B (+2.14%). classification and point_source_id are identical. This matches the user's earlier experience with froody/dev.

## 6. Caveats
- Wall-clock budget: 900 s per graph. froody trains the 5 points graphs in parallel inside one time budget, upstream trains them one after the other (row 20). So the two did not get the same number of generations. Later reports use fixed generations.
- One run per cell, default seed. Later reports show seed noise of 2-4% on points (report 03, 06), which is bigger than most differences here.
- The machine was shared (load 150-550 from other users). The decode MB/s above are not pinned and the froody numbers were measured at a different time, so they say nothing reliable about froody decode speed.
- `train_secs` is missing in the `base` JSON files (they were re-benchmarked with `rebench_tag.py` after the row 7 fix).
- Row 26 gives points A -1.5% and C -1.0%; the result files give -1.45% and -1.05%. Same within rounding.
