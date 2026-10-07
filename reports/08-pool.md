# 08 ACE_POOL + ACE_BRUTE_TOL: pool independent searches into the brute selection

Decision rows 57, 58, 61, 63, 66. Data: `runs/search/<split>/{b4tol_g120,pool3b4tol_g40}_s*` (size-only), `runs/search/<split>/{b4tol_n_g250,pool3b4tol_n_g83}_s*` (normal mode), `runs/search/speed_{A1,B1,C2,A1max}.json`.

## 1. Question
Independent searches find complementary graphs: in `runs/paired`, the per-chunk minimum over the 3 seeds' brute models beats the mean single brute model by +1.26% (A), +0.60% (B), +1.44% (C) (row 57). If we split the same compute into several independent searches and let the brute selection pick from all of them, do we gain held-out ratio?

## 2. Change
Commit a93556e on `ar/ace`, binary `bins/zli_ace_v8`; commit e9b2c7a, binary `bins/zli_ace_v9` (fix).

- `ACE_POOL=K`: after the main search of each ACE graph, run K-1 more independent searches (seed `seed + 104729*p`, same generation budget each). Their final populations are added to the saved state, so the brute selection can choose from all K searches.
- `ACE_BRUTE_TOL=t`: the brute candidates are all genes whose size is within (1+t) of the smallest, not only front 0. This is needed with merged pools (the best genes of one search are not always on the merged front) and in size-only mode (front 0 has 1 gene).
- e9b2c7a raises the CBOR decode depth limit for ACE genes from 100 to 4096. Merged-pool genes went over 100 and split B seed 3 failed (row 61).

Run labels (`runs/search/<split>/<label>_g<gens>_s<seed>`):
- `b4tol`: `ACE_BRUTE_K=4 ACE_BRUTE_TOL=0.02`, one search. This is the brute control used in all later reports.
- `pool3b4tol`: `ACE_POOL=3 ACE_BRUTE_K=4 ACE_BRUTE_TOL=0.02`.
- `_n` suffix = normal mode (3 objectives). No suffix = size-only (`ACE_SIZE_ONLY=1`).
- `g<N>` = generations per search. Equal compute: `b4tol_g120` vs `pool3b4tol_g40` (3x40), and `b4tol_n_g250` vs `pool3b4tol_n_g83` (3x83).
- `s<N>` = `ACE_SEED`.

## 3. Method
- Points only. Equal number of generations in total.
- Size-only (deterministic, row 58): seeds 1-3 x splits A/B/C, 120 gens vs 3x40 gens.
- Normal mode (production check, rows 63, 66): seeds 1-4 x splits A/B/C, 250 gens vs 3x83 gens.
- Pinned decode speed: `speed_ab.py` runs `zli benchmark -n 5` on one pinned core (`taskset`), on 5 test LAZ files, 5 interleaved rounds (order flips each round), median time per file. Decode MB/s = sum of srcSize / sum of median dtimeMs. Files: A s1 and B s1 and C s2 on 5 ordinary test files; `speed_A1max` on A s1 with the hard files 533084 and 474000 added.
- "bench dec MB/s" in the tables is from the one-iteration benchmark CSVs (not pinned).

## 4. Results
Size-only, equal compute (3x40 vs 1x120), points held-out:

| split | seed | pool3b4tol_g40 | b4tol_g120 | gain | worst file (gain) | train s (var / ctl) | bench dec MB/s (var / ctl) |
|---|---|---|---|---|---|---|---|
| A | 1 | 3.9594 | 3.8070 | +4.00% | 752132_1153215 (+1.3%) | 2605 / 1948 | 68 / 224 |
| A | 2 | 3.9122 | 3.9115 | +0.02% | 2568500_755800 (-0.5%) | 2544 / 1723 | 255 / 255 |
| A | 3 | 3.9154 | 3.7304 | +4.96% | 264500_3290500 (-0.5%) | 2468 / 1667 | 222 / 342 |
| B | 1 | 3.8985 | 3.8946 | +0.10% | chunk_0_5 (-0.2%) | 2586 / 1742 | 316 / 328 |
| B | 2 | 3.8974 | 3.8482 | +1.28% | 2519000_735700 (-1.3%) | 2660 / 1690 | 346 / 309 |
| B | 3 | 3.8740 | 3.8682 | +0.15% | chunk_0_5 (-0.2%) | 2606 / 1775 | 313 / 324 |
| C | 1 | 3.9028 | 3.6688 | +6.38% | 2519000_735700 (-0.6%) | 2041 / 1407 | 215 / 345 |
| C | 2 | 3.8199 | 3.8255 | -0.15% | dqKZWZa1QwBW (-2.6%) | 2187 / 1408 | 145 / 270 |
| C | 3 | 3.8871 | 3.7349 | +4.08% | 2568500_755800 (-2.1%) | 2379 / 1520 | 322 / 236 |

Mean gain +2.31% (min -0.15%, max +6.38%), 8 of 9 cells > 0.

Row 61 reported +2.58% mean over 8 cells, because split B seed 3 failed then. With the rerun from `bins/zli_ace_v9` (B s3 +0.15%) the mean over all 9 cells is +2.31%. Gains are on the hard airborne files: 3828740 up to +13.4%, 474000 up to +16.7%, 533084 up to +18.5%. Other files move between -2.6% and +10.3%.

Normal mode, production compute (3x83 vs 1x250), points held-out:

| split | seed | pool3b4tol_n_g83 | b4tol_n_g250 | gain | worst file (gain) | train s (var / ctl) | bench dec MB/s (var / ctl) |
|---|---|---|---|---|---|---|---|
| A | 1 | 3.9270 | 3.8481 | +2.05% | 220721_191602 (-1.1%) | 1619 / 1391 | 324 / 372 |
| A | 2 | 3.9470 | 3.8653 | +2.11% | zMuOt68XGoj6 (+0.1%) | 1685 / 1389 | 281 / 357 |
| A | 3 | 3.8896 | 3.8455 | +1.15% | 220721_191602 (-0.1%) | 1588 / 1294 | 482 / 520 |
| A | 4 | 3.9860 | 3.8918 | +2.42% | 2568500_755800 (-0.0%) | 1727 / 1322 | 477 / 366 |
| B | 1 | 3.8989 | 3.8991 | -0.01% | Z3_6190800_206 (-0.1%) | 2150 / 1832 | 339 / 320 |
| B | 2 | 3.9037 | 3.9002 | +0.09% | 140 (-0.1%) | 2017 / 1735 | 503 / 266 |
| B | 3 | 3.8989 | 3.8838 | +0.39% | 533084_6793800 (-0.0%) | 1744 / 1475 | 430 / 408 |
| B | 4 | 3.8987 | 3.8860 | +0.32% | 533084_6793800 (-0.0%) | 1984 / 1447 | 413 / 370 |
| C | 1 | 3.8140 | 3.8128 | +0.03% | dqKZWZa1QwBW (-0.2%) | 1653 / 1432 | 342 / 312 |
| C | 2 | 3.8182 | 3.8108 | +0.19% | dqKZWZa1QwBW (-3.5%) | 1679 / 1435 | 430 / 252 |
| C | 3 | 3.7902 | 3.7755 | +0.39% | 140 (-1.4%) | 1492 / 1225 | 414 / 472 |
| C | 4 | 3.8115 | 3.8172 | -0.15% | 474000_6632000 (-0.4%) | 1591 / 1356 | 427 / 357 |

Mean gain +0.75% (min -0.15%, max +2.42%), 10 of 12 cells > 0.

Split A carries the gain (+1.15..+2.42%); B and C are near 0. On A the gain is on the hard files (474000 and 533084 up to +12.7% / +12.8%).

Pinned decode speed (`speed_ab.py`), `ctl` = `b4tol_n_g250`, `pool` = `pool3b4tol_n_g83`:

| file | cell | ratio ctl | ratio pool | dec MB/s ctl | dec MB/s pool | dec pool/ctl | enc MB/s ctl | enc MB/s pool |
|---|---|---|---|---|---|---|---|---|
| speed_A1.json | A s1 | 4.1677 | 4.1499 | 86.5 | 94.2 | 1.09x | 12.4 | 9.3 |
| speed_B1.json | B s1 | 3.6715 | 3.6722 | 87.8 | 88.7 | 1.01x | 6.8 | 6.8 |
| speed_C2.json | C s2 | 3.8377 | 3.7918 | 81.1 | 91.3 | 1.13x | 12.0 | 13.4 |
| speed_A1max.json | A s1 (incl. hard files) | 3.7833 | 3.9722 | 62.5 | 64.2 | 1.03x | 9.1 | 6.3 |

## 5. Verdict
Kept (small), then superseded. In normal mode at equal compute, ACE_POOL=3 gives +0.75% mean over 12 cells, 10 of 12 positive, worst -0.15%. It never hurts in a meaningful way, and decode is not slower (1.01-1.13x pinned). Train time is about 1.2x (the merged front is benchmarked during brute selection). The size-only gain (+2.31%) mostly disappears in normal mode, because normal-mode searches already keep diverse fronts (the time objectives do that).

Later, ACE_SPECIALISTS gave a much bigger gain and pool added nothing on top of it (report 14, `final`), so ACE_POOL is not in the final recipe.

## 6. Caveats
- Normal-mode runs are not reproducible (report 06), so each normal-mode pair carries about 3% single-run noise on hard files. 4 seeds were used to compensate.
- Pinned speed runs were done while other trainings ran (load about 110-200). Compare models inside one speed file only. `speed_A1max` absolute MB/s is about 25% lower than the others.
- Speed was measured only on 5 test files per cell, for points only.
