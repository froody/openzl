# 04 ACE_ISLANDS: seed the main search with independent island searches

Decision rows 33-40. Data: `runs/isl/{A,B,C}`, `runs/gen30/A/up_base_s{1,2}`.

## 1. Question
Different seeds give points results 3.7% apart on split A (report 03), and training size ranks held-out results well (Spearman 0.88 over 8 runs, row 33). Can we turn this seed variance into a gain by running several independent searches and merging them into the main search, at equal compute?

## 2. Change
Commit 3ebc6ac on `ar/ace`, binary `bins/zli_ace_v4`.

- `ACE_ISLANDS=K`: before the main search, run K-1 independent searches ("islands") with seeds `seed + 7919*i`, each to its full generation budget. Then load all their final populations into the main search, which then runs its normal generations.

Run labels (`runs/isl/<split>/<label>_s<seed>`, from `isl_ab.py`). The number after the underscore is `ACE_MAX_GENERATIONS`:
- `isl3_30`: `ACE_ISLANDS=3`, 30 gens per search (2 islands + main = about 3x the compute of `base_30`).
- `isl3_83`: `ACE_ISLANDS=3`, 83 gens per search (about the compute of 250 gens).
- `base_30`, `base_90`, `base_250`: one plain search with 30, 90 or 250 gens. `base_90` is the equal-compute control for `isl3_30`; `base_250` for `isl3_83`.
- `s<N>`: `ACE_SEED`.

## 3. Method
- Normal mode (3 objectives). Points only.
- Round 1: split A, seeds 1-3, `isl3_30` vs `base_90` vs `base_30`. `base_30` seeds 1 and 2 are the `runs/gen30/A/up_base_s1/s2` runs from report 03 (same upstream code, `bins/zli_ace_v3`).
- Round 2 (production compute): splits A, B, C, seeds 1 and 2, `isl3_83` vs `base_250`.
- The "production base" column is the `runs/base` result (seed 0, 900 s per graph, report 01).
- Decode MB/s from the benchmark CSVs (one iteration, not pinned).

## 4. Results
Round 1, split A, points held-out:

| seed | isl3_30 | base_90 (equal compute) | base_30 | isl3_30 vs base_90 | train s isl3_30 / base_90 |
|---|---|---|---|---|---|
| 1 | 3.8726 | 3.7949 | 3.7613 | +2.05% | 729 / 653 |
| 2 | 3.8299 | 3.8735 | 3.9007 | -1.13% | 815 / 646 |
| 3 | 3.8395 | 3.7800 | 3.6936 | +1.58% | 795 / 653 |
| mean | 3.8473 | 3.8161 | 3.7852 | +0.82% |  |

Round 2, production compute, points held-out:

| split | seed | isl3_83 | base_250 | gain | train s (isl / base) | bench dec MB/s (isl / base) | production base (seed 0, 900 s) |
|---|---|---|---|---|---|---|---|
| A | 1 | 3.8412 | 3.8386 | +0.07% | 1702 / 1366 | 241 / 353 | 3.8535 |
| A | 2 | 3.9108 | 3.9088 | +0.05% | 1804 / 1745 | 379 / 232 | 3.8535 |
| B | 1 | 3.8926 | 3.8609 | +0.82% | 1932 / 1548 | 461 / 208 | 3.8798 |
| B | 2 | 3.8869 | 3.8803 | +0.17% | 1914 / 1705 | 351 / 64 | 3.8798 |
| C | 1 | 3.7994 | 3.8001 | -0.02% | 1662 / 1482 | 223 / 322 | 3.7605 |
| C | 2 | 3.7998 | 3.8008 | -0.03% | 1738 / 1487 | 221 / 415 | 3.7605 |

Mean of round 2: +0.18%. Without split B seed 1: +0.05%.

Where the seed spread at 250 gens comes from (row 40, split A, `base_250` seed 1 vs seed 2):

| test LAZ (split A) | base_250_s1 | base_250_s2 | s2 vs s1 |
|---|---|---|---|
| 140 | 4.0944 | 4.077 | -0.42% |
| 220721_191602 | 3.4575 | 3.4257 | -0.92% |
| 2568500_7558000-CuG4mpzj0rop | 3.4085 | 3.3821 | -0.77% |
| 264500_3290500-Cvzq7Q5dpOIE | 3.1682 | 3.1451 | -0.73% |
| 3828740_10300196-dcPTQ5dVqIkfK | 3.9787 | 3.98 | +0.03% |
| 474000_6632000-dcpb7qGtnTTLP-t | 3.4455 | 3.8751 | +12.47% |
| 533084_6793800-dcmGP6Y9h97vy | 3.6465 | 4.1238 | +13.09% |
| 752132_1153215-dcVOpD5fiTczR | 3.2669 | 3.243 | -0.73% |
| z5GMxol0oJkH | 4.688 | 4.6666 | -0.46% |
| zMuOt68XGoj6 | 4.3315 | 4.3084 | -0.53% |
| znHYtqPtsNgf | 4.4622 | 4.4467 | -0.35% |
| zw7rJF499NRr_345856_6365696 | 5.2744 | 5.2541 | -0.38% |

Only 2 test files move a lot (474000 trees.json +12.5%, 533084 +13.1%). All other files move less than 1%, the other way. Taking the per-chunk minimum of the two models gives 3.9298 on A (vs 3.8386 / 3.9088), and 3.8039 on C (vs 3.8001 / 3.8008).

## 5. Verdict
Rejected. The +0.8% mean at 30 generations (and halved worst case: 3.830 vs 3.780) disappears at production compute: `isl3_83` equals `base_250` on A (+0.07%, +0.05%) and C (-0.02%, -0.03%). Islands also do not reduce the 1.8% seed spread on A at 250 gens.

Useful finding for later reports: the seed spread is a generalisation effect on 2 out-of-distribution files, not a search failure. Training sizes of the two seeds differ by less than 0.5% (row 40). This led to selecting between candidates per chunk (report 05).

## 6. Caveats
- Row 39 says islands = base "within 0.1%" over A/B/C. The result files show split B seed 1 at +0.82% (3.8926 vs 3.8609) and seed 2 at +0.17%. The verdict stays the same (small and within seed noise), but the B gain is larger than the log says.
- 2-3 seeds per cell. Normal mode runs are not reproducible even with the same seed (report 06), so each pair also carries about 3% single-run noise.
- Wall-clock train times and decode MB/s were measured with other users' jobs on the machine (load 150-550). The decode numbers vary 4x between runs of the same code and are not usable.
