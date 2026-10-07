# 07 ACE_POLISH: best backend per leaf for the top front genes

Decision rows 52-56. Data: `scratch/polish/gps_p{0,5}`, `runs/search/<split>/{ctl,pol5}_g60_s<seed>`.

## 1. Question
The user asked for GA changes that judge shuffle (transform) + backend combinations better. If every good gene gets its best backend on each leaf, the GA would rank transform chains, not backend luck. Does that find smaller graphs on points?

## 2. Change
Commit ce3681c on `ar/ace`, binary `bins/zli_ace_v7`.

- `ACE_POLISH=N`: each generation, take the N smallest front-0 genes that were not polished before. For each one, do one coordinate-descent pass over its leaf graphs: try each leaf with entropy / huffman / fse / field_lz / zstd / bitpack / constant / store / flatpack, and keep the smallest. The polished gene is added to the children of that generation.

Run labels (`runs/search/<split>/<label>_g<gens>_s<seed>`, from `search_ab.py`):
- `ctl`: no change (control). No brute selection.
- `pol5`: `ACE_POLISH=5`.
- No `_n` suffix: size-only mode (`ACE_SIZE_ONLY=1`). `g60` = 60 generations. `s<N>` = `ACE_SEED`.
- Smoke runs `scratch/polish/gps_p0` (`ACE_POLISH` unset) and `gps_p5` (`ACE_POLISH=5`).

## 3. Method
- Smoke: gps_time, split A, size-only, seed 1, 30 gens. Control check: `zli_ace_v7` with `ACE_POLISH` unset must equal `zli_ace_v6` size-only seed 1 bit for bit (`scratch/det/so1_r1`).
- Main test: points, size-only, same seed with and without polish, 60 gens, seeds 1-3 x splits A/B/C (18 runs).
- Training bytes = sum over the 5 ACE graphs of the last `ACE_CENSUS` `best=`.
- Decode MB/s from the benchmark CSVs (one iteration, not pinned).

## 4. Results
Smoke (gps_time A, seed 1, 30 gens): `gps_p0` held-out 10.0259 (identical to `scratch/det/so1_r1`, so the control is exact). `gps_p5` 10.0375 (+0.12%). Train bytes 10919829 vs 10930872 (-0.10%). Train time 323 s vs 287 s.

Main test, points held-out:

| split | seed | pol5_g60 | ctl_g60 | gain | worst file (gain) | train s (var / ctl) | bench dec MB/s (var / ctl) |
|---|---|---|---|---|---|---|---|
| A | 1 | 3.8129 | 3.8067 | +0.16% | 3828740_103001 (+0.1%) | 1304 / 1197 | 238 / 256 |
| A | 2 | 3.8907 | 3.8956 | -0.13% | 264500_3290500 (-0.6%) | 1150 / 1149 | 243 / 203 |
| A | 3 | 3.7242 | 3.7248 | -0.02% | 264500_3290500 (-0.0%) | 882 / 849 | 362 / 321 |
| B | 1 | 3.8983 | 3.8940 | +0.11% | 533084_6793800 (-0.2%) | 1016 / 1203 | 206 / 289 |
| B | 2 | 3.8606 | 3.8430 | +0.46% | 2519000_735700 (-2.6%) | 926 / 1168 | 327 / 229 |
| B | 3 | 3.8592 | 3.8521 | +0.18% | Z3_6190800_206 (-0.9%) | 931 / 997 | 313 / 412 |
| C | 1 | 3.7167 | 3.6689 | +1.30% | 2519000_735700 (-1.2%) | 864 / 795 | 200 / 186 |
| C | 2 | 3.8190 | 3.8265 | -0.20% | 220721_191602 (-0.3%) | 823 / 867 | 310 / 263 |
| C | 3 | 3.6924 | 3.6926 | -0.00% | 533084_6793800 (-1.1%) | 763 / 855 | 315 / 321 |

Mean gain +0.21% (min -0.20%, max +1.30%), 5 of 9 cells > 0.

Training bytes:

| split | seed | pol5 train bytes (sum of 5 graphs) | ctl train bytes | diff |
|---|---|---|---|---|
| A | 1 | 17914490 | 17934892 | -0.11% |
| A | 2 | 17675568 | 17627237 | +0.27% |
| A | 3 | 18040847 | 18037099 | +0.02% |
| B | 1 | 18128406 | 18149456 | -0.12% |
| B | 2 | 18208684 | 18283195 | -0.41% |
| B | 3 | 18219829 | 18252842 | -0.18% |
| C | 1 | 19153321 | 19203272 | -0.26% |
| C | 2 | 18709621 | 18674889 | +0.19% |
| C | 3 | 19010416 | 19026420 | -0.08% |

Mean training bytes -0.08%; 3 of 9 cells are worse on training.

## 5. Verdict
Rejected. +0.21% mean held-out and -0.08% training bytes are within noise; only one cell (C seed 1, +1.30%) moves more than 0.5%. The leaves of the best genes are already close to their best backend, so backend choice is not what limits the search on points.

## 6. Caveats
- Size-only mode, 60 gens: a test bed, not the production setting.
- Decode MB/s are from one benchmark iteration on a shared machine; they vary up to 1.4x between same-code runs and are not usable for a speed verdict.
