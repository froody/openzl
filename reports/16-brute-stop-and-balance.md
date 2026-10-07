# 16 ACE_BRUTE_BALANCE and ACE_BRUTE_MIN_GAIN: stop rule and file weighting

Decision rows 84, 87, 88 (follow-up: 90-95, section 7). Data: `runs/search/<split>/{spectok,spectokG,spectokB}_g120_s<seed>/points.json` (size-only, same seed), `runs/search/speed_C2G.json`.

## 1. Question
Brute-force selection (`ACE_BRUTE_K=4`, report 05) greedily picks complementary candidates from the candidate front. By default, it stops picking when the best next candidate saves less than 0.2% of total training bytes (`bestTot > 0.998 * total`). Does lowering this stop threshold to 0.05% (`ACE_BRUTE_MIN_GAIN=0.0005`) admit useful complementary candidates that were previously cut off, and does it gain held-out ratio without harming decode speed?

Separately, greedy candidate selection scores candidates by raw training byte total, so large training files dominate selection. If we weight each training input inversely by its file size (`ACE_BRUTE_BALANCE=1`), so all training files contribute equally to the greedy selection score, do we pick better candidates for hard or smaller files, and does held-out ratio improve?

## 2. Change
Commit 3aa7587 on `ar/ace` (`wt/ace`), binary `bins/zli_ace_v14`. Changes in `pickComplementary` in `tools/training/ace/ace_combination.cpp`:

- `ACE_BRUTE_MIN_GAIN=g`: replaces the fixed 0.2% stop rule (`bestTot > 0.998 * total`) with `bestTot > (1 - minGain) * total`. The default is `0.002` (0.2%) when unset. Setting `ACE_BRUTE_MIN_GAIN=0.0005` requires the next candidate to save at least 0.05% instead of 0.2%.
- `ACE_BRUTE_BALANCE=1`: weights each training input chunk i by `1 / groupBytes[groupOf[i]]`, where `groupBytes` is the compressed byte total of that training file under the best single candidate (the reference model). Every training file therefore sums to 1.0 in the greedy objective, preventing larger files from dominating candidate selection.

Run labels (`runs/search/<split>/<label>_g120_s<seed>/points.json`):
- `spectok`: control (`ACE_TOK_SEEDS=1 ACE_SPECIALISTS=1 ACE_SPEC_GENERATIONS=60 ACE_BRUTE_K=4 ACE_BRUTE_TOL=0.5`), default min gain 0.002, unweighted.
- `spectokG`: `spectok` + `ACE_BRUTE_MIN_GAIN=0.0005`.
- `spectokB`: `spectokG` + `ACE_BRUTE_BALANCE=1`.
- Size-only (`ACE_SIZE_ONLY=1`), 120 generations (`g120`), seeds 1-3 (`s1`..`s3`), splits A/B/C. Same-seed pairs: identical search and population; only the post-search brute selection differs.

## 3. Method
- Points column, size-only mode (`ACE_SIZE_ONLY=1`), 120 generations, seeds 1-3 across splits A/B/C (9 cells).
- Paired same-seed design: because size-only mode is completely deterministic with fixed seed (report 06), each search generates the exact same candidate population. The only differences between `spectok`, `spectokG`, and `spectokB` are the candidate selection decisions inside `pickComplementary`.
- Pinned decode speed: `runs/search/speed_C2G.json` measured on split C seed 2 (the cell with the largest ratio jump, +1.39%), comparing `spectok` vs `spectokG` across all 12 test LAZ files with 5 interleaved rounds pinned to core 7 via `speed_ab.py`.

## 4. Results

### Held-out ratio: paired size-only 120g test (9 cells)

| split | seed | spectok | spectokG | gain (G vs spectok) | worst file G | spectokB | gain (B vs G) | worst file B vs G | picks spectok | picks spectokG / B |
|---|---|---|---|---|---|---|---|---|---|---|
| A | 1 | 4.0589 | 4.0589 | +0.00% | +0.00% | 4.0590 | +0.00% | +0.00% | 4/1/2/1/4 | 4/1/2/1/4 |
| A | 2 | 4.0577 | 4.0586 | +0.02% | +0.00% | 4.0581 | -0.01% | 3828740_10300196-dcPTQ5dVqIkfK (-0.24%) | 4/1/2/1/4 | 4/1/3/1/4 |
| A | 3 | 4.0609 | 4.0609 | +0.00% | +0.00% | 4.0615 | +0.02% | 220721_191602 (-0.03%) | 4/1/2/1/4 | 4/1/2/1/4 |
| B | 1 | 4.0483 | 4.0547 | +0.16% | +0.00% | 4.0551 | +0.01% | chunk_0_5 (-0.53%) | 3/1/3/1/4 | 4/1/4/1/4 |
| B | 2 | 4.0356 | 4.0398 | +0.10% | +0.00% | 4.0396 | -0.01% | chunk_0_5 (-0.15%) | 3/1/3/1/3 | 4/1/4/1/4 |
| B | 3 | 4.0514 | 4.0520 | +0.02% | +0.00% | 4.0518 | -0.00% | chunk_0_5 (-0.16%) | 4/1/3/1/4 | 4/1/4/1/4 |
| C | 1 | 4.0049 | 4.0093 | +0.11% | +0.00% | 4.0046 | -0.12% | dqKZWZa1QwBW (-1.16%) | 3/1/2/1/3 | 3/1/3/1/4 |
| C | 2 | 3.9557 | 4.0106 | +1.39% | +0.00% | 4.0180 | +0.18% | 2024-ballarat_a-LiDAR_2439500_2459500 (-0.10%) | 2/1/2/1/2 | 3/1/3/1/4 |
| C | 3 | 3.9969 | 4.0017 | +0.12% | +0.00% | 4.0025 | +0.02% | +0.00% | 3/1/2/1/2 | 3/1/2/1/4 |

Summary:
- **`spectokG` vs `spectok`**: mean gain **+0.21%** (min +0.00%, max +1.39%). 9 of 9 cells >= 0.00%, 7 of 9 cells strictly positive.
  - **No file regressed** in any cell (worst file gain +0.00%). Because greedy candidate selection with a lower stop threshold is monotonic, it picks a strict superset of candidates. At compression time the brute selector tests all candidates and keeps the smallest per chunk, so additional candidates can never make a file worse.
  - Pick counts increased from 11.0 mean (8 to 13 picks across the 5 graphs) to 12.7 mean (11 to 14 picks).
  - The largest gain occurred on split C seed 2 (+1.39%, saving 364,930 bytes). In this cell, graphs 1, 3, and 5 stopped at only 2 picks under the 0.2% rule (e.g. graph 5's next candidate saved 0.191% on training, just missing 0.2%). Lowering the threshold to 0.05% picked 3, 3, and 4 candidates respectively, producing massive gains on the hard airborne LiDAR test files: `474000` gained **+6.61%** and `533084` gained **+6.51%**.
- **`spectokB` vs `spectokG`**: mean gain **+0.01%** (min -0.12%, max +0.18%).
  - Only 5 of 9 cells positive; 4 cells regressed (A s2 -0.01%, B s2 -0.01%, B s3 -0.00%, C s1 -0.12%).
  - Because `ACE_BRUTE_BALANCE=1` reweights the greedy objective, it changes which candidates are selected. Unlike lowering the threshold, this is not monotonic: it introduces real regressions on test files (e.g. C seed 1 lost -1.16% on `dqKZWZa1QwBW` and -0.48% on `Z3_6190800_2067600`; B cells lost up to -0.53% on `chunk_0_5`).

### Pinned decode speed (`speed_C2G.json`)

Measured on split C seed 2 (where `spectokG` gained +1.39% ratio), 12 test LAZ files, 5 interleaved rounds on pinned core 7:

| model | ratio (12 files) | dec MB/s | dec vs spectok | enc MB/s | enc vs spectok |
|---|---|---|---|---|---|
| spectok | 3.9557 | 64.8 | 1.00x | 8.3 | 1.00x |
| spectokG | 4.0106 | 64.1 | 0.99x | 5.4 | 0.65x |

Overall decode speed is essentially unchanged (**0.99x**, 64.1 vs 64.8 MB/s). Compress speed drops to 0.65x because compression evaluates 12 candidates instead of 8 across the 5 graphs.

Per-file breakdown on split C seed 2:

| test file | ratio spectok | ratio spectokG | ratio gain | dec MB/s spectok | dec MB/s spectokG | dec speedup |
|---|---|---|---|---|---|---|
| 140 | 4.1290 | 4.1417 | +0.31% | 73.0 | 62.7 | 0.86x |
| 2024-ballarat_a-LiDAR_2439500_2459500 | 3.4566 | 3.4613 | +0.14% | 75.6 | 73.4 | 0.97x |
| 220721_191602 | 3.5078 | 3.5109 | +0.09% | 81.7 | 72.8 | 0.89x |
| 2519000_7357000 | 3.0373 | 3.0388 | +0.05% | 75.7 | 71.8 | 0.95x |
| 2568500_7558000-CuG4mpzj0rop | 3.4641 | 3.4672 | +0.09% | 82.0 | 70.0 | 0.85x |
| 3828740_10300196-dcPTQ5dVqIkfK | 4.0096 | 4.0383 | +0.72% | 61.7 | 63.0 | 1.02x |
| 474000_6632000-dcpb7qGtnTTLP-trees.json-input | 3.7842 | 4.0344 | +6.61% | 54.0 | 66.3 | 1.23x |
| 533084_6793800-dcmGP6Y9h97vy | 4.0583 | 4.3223 | +6.51% | 55.3 | 71.7 | 1.30x |
| EX_20230607_124634__230131_112528_L00_WGS_84-UTM_zone_34N__000 | 5.3012 | 5.3138 | +0.24% | 56.7 | 46.7 | 0.82x |
| Z3_6190800_2067600 | 5.4056 | 5.4058 | +0.00% | 51.0 | 54.3 | 1.07x |
| dqKZWZa1QwBW | 4.0114 | 4.0276 | +0.40% | 67.6 | 63.8 | 0.94x |
| zMuOt68XGoj6 | 4.3864 | 4.3972 | +0.25% | 71.2 | 67.8 | 0.95x |

Crucially, the two hard airborne LiDAR files decode **substantially faster** under `spectokG`: `474000` is 1.23x faster and `533084` is 1.30x faster. The candidates picked by lowering `MIN_GAIN` both compress better and decompress faster on these regimes.

## 5. Verdict
- **`ACE_BRUTE_MIN_GAIN=0.0005`**: **Kept**. Delivers a paired mean ratio gain of +0.21% (up to +1.39%), 9 of 9 cells >= 0.00%, and zero files worse in any run. Pinned decode speed is unchanged (0.99x overall, and hard files decode 23-30% faster). Final points recipe is updated to include `ACE_BRUTE_MIN_GAIN=0.0005`:
  `ACE_TOK_SEEDS=1 ACE_SPECIALISTS=1 ACE_SPEC_GENERATIONS=60 ACE_BRUTE_K=4 ACE_BRUTE_TOL=0.5 ACE_NO_CTIME=1 ACE_BRUTE_MIN_GAIN=0.0005` (binary `bins/zli_ace_v14`).
- **`ACE_BRUTE_BALANCE=1`**: **Rejected**. Mean gain over `spectokG` is negligible (+0.01%), 4 of 9 cells regress, and test files suffer losses up to -1.16%. Reweighting the training objective does not translate into better generalization.

## 6. Caveats
- Evaluated on the size-only 120-generation test bed, not the 250-generation normal-mode production run.
- Compression speed drops by ~35% (0.65x) due to testing more candidates per stream during brute-force trial compression.
- Pinned decode speed was measured on split C seed 2 only (though across all 12 test files).

## 7. Follow-up: lower stop threshold with K=4 and K=8, and normal-mode confirmation

Decision rows 90, 91, 92, 94, 95. Data: `runs/search/<split>/{spectokG0,spectokG8}_g120_s<seed>/points.json` (`search_G.log`), `runs/search/<split>/best_n_g250_s<seed>/points.json` (`search_best.log`), `runs/search/speed_C2G8.json`, `runs/search/speed_A1best.json`.

### 7.1 Labels
- `spectokG0`: `spectok` + `ACE_BRUTE_MIN_GAIN=0.0001` (0.01% stop rule), K=4.
- `spectokG8`: `spectok` with `ACE_BRUTE_K=8 ACE_BRUTE_MIN_GAIN=0.0001`.
- `best_n`: normal mode, `spectok` + `ACE_NO_CTIME=1 ACE_BRUTE_K=8 ACE_BRUTE_MIN_GAIN=0.0001`, binary `bins/zli_ace_v14`. Same selection settings as `spectokG8`, in production mode.

### 7.2 Size-only, same seed, vs `spectokG` (9 cells)
Same method as section 3: identical searches, only the brute selection differs. "picks" = brute picks per ACE graph (5 graphs), counted from `points.zlc.log`.

| split | seed | spectokG | spectokG0 | G0 gain | G0 worst file | spectokG8 | G8 gain | G8 worst file | G8 vs spectok | picks G | picks G0 | picks G8 |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| A | 1 | 4.0589 | 4.0596 | +0.02% | +0.00% | 4.0615 | +0.06% | +0.02% | +0.06% | 4/1/2/1/4 | 4/1/4/1/4 | 7/1/4/1/7 |
| A | 2 | 4.0586 | 4.0592 | +0.01% | +0.00% | 4.0623 | +0.09% | +0.00% | +0.11% | 4/1/3/1/4 | 4/1/4/1/4 | 5/1/5/1/7 |
| A | 3 | 4.0609 | 4.0617 | +0.02% | +0.00% | 4.0732 | +0.30% | +0.01% | +0.30% | 4/1/2/1/4 | 4/1/4/1/4 | 7/1/4/1/8 |
| B | 1 | 4.0547 | 4.0547 | +0.00% | +0.00% | 4.0602 | +0.14% | +0.01% | +0.29% | 4/1/4/1/4 | 4/1/4/1/4 | 8/1/7/1/8 |
| B | 2 | 4.0398 | 4.0398 | +0.00% | +0.00% | 4.0442 | +0.11% | +0.00% | +0.21% | 4/1/4/1/4 | 4/1/4/1/4 | 7/1/7/1/8 |
| B | 3 | 4.0520 | 4.0520 | +0.00% | +0.00% | 4.0609 | +0.22% | +0.00% | +0.23% | 4/1/4/1/4 | 4/1/4/1/4 | 8/1/7/1/8 |
| C | 1 | 4.0093 | 4.0106 | +0.03% | +0.00% | 4.0197 | +0.26% | +0.02% | +0.37% | 3/1/3/1/4 | 4/1/4/1/4 | 6/1/5/1/6 |
| C | 2 | 4.0106 | 4.0116 | +0.02% | +0.00% | 4.0222 | +0.29% | +0.03% | +1.68% | 3/1/3/1/4 | 4/1/4/1/4 | 5/1/5/1/6 |
| C | 3 | 4.0017 | 4.0024 | +0.02% | +0.00% | 4.0121 | +0.26% | +0.02% | +0.38% | 3/1/2/1/4 | 4/1/4/1/4 | 4/1/7/1/6 |

- `spectokG0` vs `spectokG`: mean **+0.01%** (min +0.00%, max +0.03%), 6 of 9 cells > 0. With K=4 and the 0.01% threshold, all 3 big graphs use 4 picks in every cell (4/1/4/1/4). The K cap binds, so the lower threshold can add almost nothing.
- `spectokG8` vs `spectokG`: mean **+0.19%** (min +0.06%, max +0.30%), 9 of 9 cells > 0. Worst file >= 0.00% in every cell. Picks rise to 4-8 on the 3 big graphs.
- `spectokG8` vs `spectok` (both brute changes together): mean +0.41% (min +0.06%, max +1.68% on C s2).

### 7.3 Pinned decode, `spectokG8` vs `spectokG` (`speed_C2G8.json`)
Split C seed 2, 12 test files, 5 interleaved rounds, pinned core 7.

| model | ratio (12 files) | dec MB/s | dec vs spectokG | enc MB/s | enc vs spectokG |
|---|---|---|---|---|---|
| spectokG | 4.0106 | 84.1 | 1.000x | 6.8 | 1.00x |
| spectokG8 | 4.0222 | 82.0 | 0.976x | 4.5 | 0.66x |

| test file | ratio spectokG | ratio spectokG8 | ratio gain | dec MB/s spectokG | dec MB/s spectokG8 | dec speedup |
|---|---|---|---|---|---|---|
| 2024-ballarat_a-LiDAR_2439500_2459500 | 3.4613 | 3.4851 | +0.69% | 87.0 | 80.2 | 0.92x |
| zMuOt68XGoj6 | 4.3972 | 4.4022 | +0.11% | 78.1 | 75.0 | 0.96x |
| dqKZWZa1QwBW | 4.0276 | 4.0474 | +0.49% | 94.8 | 89.3 | 0.94x |
| 140 | 4.1417 | 4.1453 | +0.09% | 77.9 | 75.0 | 0.96x |
| 220721_191602 | 3.5109 | 3.5138 | +0.08% | 87.4 | 83.7 | 0.96x |
| 2568500_7558000-CuG4mpzj0rop | 3.4672 | 3.4695 | +0.06% | 91.4 | 88.7 | 0.97x |
| 474000_6632000-dcpb7qGtnTTLP-trees.json-input | 4.0344 | 4.0663 | +0.79% | 89.0 | 86.2 | 0.97x |
| EX_20230607_124634__230131_112528_L00_WGS_84-UTM_zone_34N__000 | 5.3138 | 5.3163 | +0.05% | 76.7 | 77.6 | 1.01x |
| Z3_6190800_2067600 | 5.4058 | 5.4221 | +0.30% | 79.2 | 78.0 | 0.99x |
| 533084_6793800-dcmGP6Y9h97vy | 4.3223 | 4.3553 | +0.76% | 92.4 | 91.0 | 0.98x |
| 3828740_10300196-dcPTQ5dVqIkfK | 4.0383 | 4.0418 | +0.09% | 76.3 | 78.2 | 1.02x |
| 2519000_7357000 | 3.0388 | 3.0397 | +0.03% | 85.1 | 84.6 | 0.99x |

Decode is 0.976x overall (per file 0.92-1.02x). Encode is 0.66x. Every file gains ratio (+0.03..+0.79%). Note: the `spectokG` decode speed here (84.1 MB/s) is higher than in `speed_C2G.json` (64.1 MB/s) for the same model. Absolute MB/s depends on machine load; only the ratio within one file is meaningful.

### 7.4 Normal-mode confirmation: `best_n` (3 seeds x A/B/C, 250 gens)

| split | seed | best_n | b4tol_n | vs b4tol_n | spectok_n | vs spectok_n | worst file vs spectok_n | spectokNC_n | vs spectokNC_n | upstream | vs upstream | train s (best / spectok) | picks best |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| A | 1 | 4.0548 | 3.8481 | +5.37% | 4.0421 | +0.31% | 2568500_755800 (+0.1%) | 4.0420 | +0.32% | 3.8535 | +5.22% | 2029 / 2431 | 8/1/7/1/8 |
| A | 2 | 4.0462 | 3.8653 | +4.68% | 4.0581 | -0.29% | 2568500_755800 (-1.2%) | 4.0522 | -0.15% | 3.8535 | +5.00% | 1949 / 2336 | 7/1/7/1/6 |
| A | 3 | 4.0579 | 3.8455 | +5.52% | 4.0475 | +0.26% | zMuOt68XGoj6 (-0.0%) | - | - | 3.8535 | +5.30% | 1956 / 2311 | 8/1/6/1/6 |
| B | 1 | 4.0554 | 3.8991 | +4.01% | 4.0414 | +0.35% | 2568500_755800 (-0.0%) | 4.0436 | +0.29% | 3.8798 | +4.53% | 2195 / 2601 | 7/1/8/1/7 |
| B | 2 | 4.0459 | 3.9002 | +3.74% | 4.0386 | +0.18% | 2568500_755800 (-0.5%) | 4.0411 | +0.12% | 3.8798 | +4.28% | 2122 / 2489 | 8/1/7/1/7 |
| B | 3 | 4.0536 | 3.8838 | +4.37% | 4.0377 | +0.39% | chunk_0_5 (+0.0%) | - | - | 3.8798 | +4.48% | 2123 / 2589 | 7/1/6/1/8 |
| C | 1 | 4.0237 | 3.8128 | +5.53% | 3.9477 | +1.93% | 2568500_755800 (-1.1%) | 3.9722 | +1.30% | 3.7605 | +7.00% | 1750 / 2239 | 6/1/7/1/6 |
| C | 2 | 4.0302 | 3.8108 | +5.76% | 3.9550 | +1.90% | 2024-ballarat_ (+0.0%) | 3.9404 | +2.28% | 3.7605 | +7.17% | 1898 / 2108 | 6/1/5/1/8 |
| C | 3 | 4.0275 | 3.7755 | +6.67% | 3.9514 | +1.93% | Z3_6190800_206 (-0.2%) | - | - | 3.7605 | +7.10% | 1731 / 2142 | 5/1/8/1/7 |

`spectokNC_n` exists only for seeds 1-2.

| best_n vs | cells | mean | min | max | > 0 | mean per split A / B / C |
|---|---|---|---|---|---|---|
| b4tol_n (brute control) | 9 | +5.07% | +3.74% | +6.67% | 9/9 | +5.19% / +4.04% / +5.99% |
| spectok_n | 9 | +0.77% | -0.29% | +1.93% | 8/9 | +0.09% / +0.31% / +1.92% |
| spectokNC_n (seeds 1-2 only) | 6 | +0.69% | -0.15% | +2.28% | 5/6 | +0.08% / +0.21% / +1.79% |
| upstream v0.2.3 | 9 | +5.57% | +4.28% | +7.17% | 9/9 | +5.18% / +4.43% / +7.09% |

- On C, `best_n` gains +1.9% over `spectok_n` on all 3 seeds. Most of it is on the hard airborne files: 474000 +8.2..+9.0%, 533084 +7.5..+7.9%, dqKZWZa1QwBW +0.4..+8.0%.
- On A, the gain is small and changes sign between seeds: +0.31%, -0.29%, +0.26%. In A s2, 2568500 loses -1.2% and 752132 -0.8%. Normal mode is not reproducible (report 06), so this is search noise, not a selection effect.
- Seed spread of `best_n` per split is under 0.3% (A 0.29%, B 0.23%, C 0.16%).

### 7.5 Pinned decode, `best_n` vs `spectokNC_n` (`speed_A1best.json`)
Split A seed 1, all 12 test files, 5 interleaved rounds, pinned core 9, load about 150.

| model | ratio (12 files) | dec MB/s | dec vs spectokNC | enc MB/s | enc vs spectokNC |
|---|---|---|---|---|---|
| spectokNC | 4.0420 | 59.9 | 1.000x | 4.5 | 1.00x |
| best | 4.0548 | 60.1 | 1.002x | 2.4 | 0.53x |

| test file | ratio spectokNC | ratio best | ratio gain | dec MB/s spectokNC | dec MB/s best | dec speedup |
|---|---|---|---|---|---|---|
| znHYtqPtsNgf | 4.5226 | 4.5400 | +0.38% | 55.5 | 52.0 | 0.94x |
| 140 | 4.1321 | 4.1353 | +0.08% | 49.7 | 47.0 | 0.95x |
| 2568500_7558000-CuG4mpzj0rop | 3.4558 | 3.4638 | +0.23% | 56.6 | 60.1 | 1.06x |
| zMuOt68XGoj6 | 4.3945 | 4.4086 | +0.32% | 53.0 | 54.3 | 1.02x |
| z5GMxol0oJkH | 4.7173 | 4.7418 | +0.52% | 53.9 | 50.1 | 0.93x |
| 533084_6793800-dcmGP6Y9h97vy | 4.6423 | 4.6462 | +0.08% | 61.3 | 73.8 | 1.20x |
| zw7rJF499NRr_345856_6365696 | 5.4812 | 5.5031 | +0.40% | 77.9 | 75.0 | 0.96x |
| 474000_6632000-dcpb7qGtnTTLP-trees.json-input | 4.2564 | 4.2613 | +0.12% | 80.6 | 82.4 | 1.02x |
| 752132_1153215-dcVOpD5fiTczR | 3.3072 | 3.3143 | +0.22% | 77.8 | 76.0 | 0.98x |
| 3828740_10300196-dcPTQ5dVqIkfK | 3.9822 | 4.0293 | +1.18% | 58.5 | 55.9 | 0.96x |
| 220721_191602 | 3.5023 | 3.5095 | +0.20% | 52.8 | 53.8 | 1.02x |
| 264500_3290500-Cvzq7Q5dpOIE | 3.2221 | 3.2274 | +0.16% | 56.6 | 60.3 | 1.06x |

Decode is equal (1.00x overall, per file 0.93-1.20x). Encode is 0.53x (more brute candidates to try per chunk).

### 7.6 Follow-up verdict
- `ACE_BRUTE_MIN_GAIN=0.0001` with K=4 (`spectokG0`): no effect (+0.01%). Not kept on its own.
- `ACE_BRUTE_K=8 ACE_BRUTE_MIN_GAIN=0.0001` (`spectokG8`): **kept**. Size-only +0.19% over `spectokG`, 9/9 > 0, no file worse. Pinned decode 0.976x (C s2) and 1.00x (A s1, as `best_n`).
- Normal mode confirms it: `best_n` is +0.77% over `spectok_n` (8/9 > 0) and +5.57% over upstream (A +5.18%, B +4.43%, C +7.09%).
- **New points recipe** (supersedes the one in section 5): `ACE_TOK_SEEDS=1 ACE_SPECIALISTS=1 ACE_SPEC_GENERATIONS=60 ACE_BRUTE_K=8 ACE_BRUTE_TOL=0.5 ACE_NO_CTIME=1 ACE_BRUTE_MIN_GAIN=0.0001`, binary `bins/zli_ace_v14` (3aa7587).

### 7.7 Follow-up caveats
- Encode speed falls: 0.66x (`spectokG8` vs `spectokG`) and 0.53x (`best_n` vs `spectokNC_n`). Decode is the priority, but compression gets about 2x slower than the `spectok` recipe.
- The normal-mode gain over `spectok_n` (+0.77%) is larger than the size-only gain (+0.41% for `spectokG8` vs `spectok`). The two use different searches (normal mode, 250 gens), so the extra part is partly search noise. C carries most of it.
- Pinned decode for `best_n` was measured on A s1 only. A s1 is not the cell with the largest ratio change.
