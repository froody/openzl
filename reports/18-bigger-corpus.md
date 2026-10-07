# 18 Bigger training corpus (corpus2)

Decision rows 85, 89, 96, 98, 107, 109, 111. Data: `corpus2.json`, `split_{A,C}{24,48}.json`, `runs/search/{A,C}{,24,48}/{b4tol_n,spectokNC_n}_g250_s1/points.json` (`search_big.log`), `runs/search/{A48,C48}/best_n_g250_s1` (`search_big2.log`), `runs/search/{A48c,C48c}/*` (`search_clean.log`, `search_sl48.log`), `runs/search/speed_C48c.json`.

## 1. Question
The specialist ceiling (report 10) and ACE_SPECIALISTS (report 11) say that variety in the training corpus is the lever. Does training on more and more varied LAZ files raise the held-out points ratio on the same test files? Does it help the generalist control and the specialist recipe the same way?

## 2. Change
No new code. Data only.
- `corpus2.json`: a pool of 79 new LAZ files from the `~/plp-data` source dirs (3 per source dir, 20-400 MB each, none in `corpus.json`). All 79 were dumped; 76 have points (row 85).
- New splits. The test list is unchanged (the same 12 test files as A or C):
  - `A24` = A's 8 train files + 16 new files; `A48` = A's 8 + 40 new files.
  - `C24` = C's 8 train files + the same 16 new files; `C48` = C's 8 + the same 40 new files.
  - The 16 new files are a subset of the 40. They come from 11 source dirs (16 files) and 23 source dirs (40 files).
- Training bytes, points: A 68.0 MB / 333 inputs, A24 208.9 MB / 1,097, A48 903.8 MB / 4,084. C 72.5 MB / 350, C24 213.3 MB / 1,114, C48 908.3 MB / 4,101 (from `points.zlc.log`).

Run labels (`runs/search/<split>/<label>_g250_s1/points.json`):
- `b4tol_n`: generalist control, `ACE_BRUTE_K=4 ACE_BRUTE_TOL=0.02`.
- `spectokNC_n`: `ACE_TOK_SEEDS=1 ACE_SPECIALISTS=1 ACE_SPEC_GENERATIONS=60 ACE_BRUTE_K=4 ACE_BRUTE_TOL=0.5 ACE_NO_CTIME=1` (the points recipe of report 14).
- `best_n`: `spectokNC_n` + `ACE_BRUTE_K=8 ACE_BRUTE_MIN_GAIN=0.0001` (report 16).

## 3. Method
- Points only, normal mode, 250 gens, seed 1. One run per split and label.
- The 8-file baselines are `runs/search/{A,C}/{b4tol_n,spectokNC_n}_g250_s1`. Upstream: `runs/base/{A,C}/points.json` (A 3.8535, C 3.7605).
- With `ACE_SPECIALISTS`, there is one extra specialist search per training file, so 48 files mean 48 specialist searches.
- "bench dec MB/s" = unpinned one-iteration benchmark CSVs. No pinned speed run.

## 4. Results

### 4.1 Held-out points ratio

| test set | train files | b4tol_n | vs 8 files | spectokNC_n | vs 8 files | spectokNC vs b4tol | spectokNC vs upstream | train s (b4tol / spectokNC) | bench dec MB/s (b4tol / spectokNC) | picks spectokNC |
|---|---|---|---|---|---|---|---|---|---|---|
| A | 8 | 3.8481 | +0.00% | 4.0420 | +0.00% | +5.04% | +4.89% | 1391 / 1869 | 372 / 282 | 3/1/2/1/4 |
| A | 24 | 3.7699 | -2.03% | 4.0580 | +0.40% | +7.64% | +5.31% | 3849 / 5976 | 421 / 373 | 3/1/2/1/4 |
| A | 48 | 3.7764 | -1.86% | 4.0624 | +0.50% | +7.57% | +5.42% | 7792 / 14613 | 454 / 430 | 3/1/2/1/3 |
| C | 8 | 3.8128 | +0.00% | 3.9722 | +0.00% | +4.18% | +5.63% | 1432 / 1647 | 312 / 441 | 2/1/3/1/3 |
| C | 24 | 3.7118 | -2.65% | 4.0665 | +2.37% | +9.56% | +8.14% | 3781 / 5777 | 400 / 376 | 3/1/2/1/3 |
| C | 48 | 3.7122 | -2.64% | 4.0897 | +2.96% | +10.17% | +8.75% | 7444 / 15020 | 441 / 452 | 4/1/3/1/3 |

### 4.2 Per-file changes, A test set
Byte share = the file's share of the 8-file `spectokNC_n` compressed bytes. b4tol and spectokNC columns are gains vs the same label with 8 train files.

| test file | byte share | b4tol_n 8 files | b4tol 24 | b4tol 48 | spectokNC_n 8 files | spectokNC 24 | spectokNC 48 |
|---|---|---|---|---|---|---|---|
| 140 | 8.6% | 4.1120 | +0.09% | +0.48% | 4.1321 | +0.62% | +0.67% |
| 220721_191602 | 9.0% | 3.4884 | -0.08% | +0.48% | 3.5023 | +0.88% | +0.62% |
| 2568500_7558000-CuG4mpzj0rop | 9.0% | 3.4396 | -0.22% | +0.49% | 3.4558 | +0.85% | +0.78% |
| 264500_3290500-Cvzq7Q5dpOIE | 9.7% | 3.1941 | -0.01% | +0.41% | 3.2221 | +0.41% | +0.35% |
| 3828740_10300196-dcPTQ5dVqIkfK | 8.9% | 3.9153 | -3.25% | -4.83% | 3.9822 | -0.33% | +1.03% |
| 474000_6632000-dcpb7qGtnTTLP-trees.json- | 9.3% | 3.4420 | -8.60% | -8.55% | 4.2564 | -0.51% | -0.42% |
| 533084_6793800-dcmGP6Y9h97vy | 7.3% | 3.6592 | -8.09% | -7.61% | 4.6423 | -0.09% | +0.03% |
| 752132_1153215-dcVOpD5fiTczR | 10.7% | 3.2899 | -0.21% | +0.44% | 3.3072 | +0.62% | +0.59% |
| z5GMxol0oJkH | 6.7% | 4.6891 | +0.24% | +0.28% | 4.7173 | +0.75% | +0.55% |
| zMuOt68XGoj6 | 7.1% | 4.3392 | +0.43% | +0.60% | 4.3945 | +0.59% | +0.41% |
| znHYtqPtsNgf | 7.7% | 4.4701 | +0.25% | +0.36% | 4.5226 | +0.65% | +0.93% |
| zw7rJF499NRr_345856_6365696 | 6.0% | 5.2722 | +0.08% | +0.04% | 5.4812 | +0.41% | +0.53% |

### 4.3 Per-file changes, C test set

| test file | byte share | b4tol_n 8 files | b4tol 24 | b4tol 48 | spectokNC_n 8 files | spectokNC 24 | spectokNC 48 |
|---|---|---|---|---|---|---|---|
| 140 | 8.8% | 4.1075 | +0.17% | +0.19% | 4.1434 | +0.34% | +0.36% |
| 2024-ballarat_a-LiDAR_2439500_2459500 | 6.3% | 3.4354 | +0.01% | +0.02% | 3.4743 | +0.33% | +0.33% |
| 220721_191602 | 9.2% | 3.4859 | -0.03% | -0.02% | 3.5197 | +0.41% | +0.43% |
| 2519000_7357000 | 10.7% | 3.0203 | -0.15% | -0.14% | 3.0373 | +0.19% | +0.20% |
| 2568500_7558000-CuG4mpzj0rop | 9.2% | 3.4341 | -0.07% | -0.06% | 3.4841 | +0.10% | +0.10% |
| 3828740_10300196-dcPTQ5dVqIkfK | 9.1% | 3.9917 | -1.01% | -1.00% | 4.0055 | +0.35% | +0.38% |
| 474000_6632000-dcpb7qGtnTTLP-trees.json- | 10.5% | 3.4259 | -8.19% | -8.18% | 3.8402 | +9.07% | +11.29% |
| 533084_6793800-dcmGP6Y9h97vy | 8.5% | 3.6452 | -7.75% | -7.74% | 4.0933 | +11.78% | +13.78% |
| EX_20230607_124634__230131_112528_L00_WG | 6.1% | 5.2542 | -0.02% | -0.00% | 5.2768 | +0.26% | +0.47% |
| Z3_6190800_2067600 | 6.1% | 5.2027 | -3.16% | -3.14% | 5.4081 | +3.01% | +4.18% |
| dqKZWZa1QwBW | 8.2% | 3.5787 | -6.75% | -6.74% | 4.0470 | +1.97% | +3.62% |
| zMuOt68XGoj6 | 7.3% | 4.3540 | +0.06% | +0.07% | 4.3886 | +0.57% | +0.72% |

- The generalist gets worse with more files: A -2.0% / -1.9%, C -2.6% / -2.6% (24 / 48 files). The loss is on the hard airborne files: 474000 -8.2..-8.6%, 533084 -7.6..-7.7%, and on C also dqKZWZa1QwBW -6.7% and Z3 -3.1%. One search over many files is pulled towards the common regimes.
- The specialist recipe gets better with more files: A +0.40% / +0.50%, C +2.37% / +2.96%.
  - On C, most of the gain is on files whose regime C's 8 train files miss: 474000 +9.1% / +11.3%, 533084 +11.8% / +13.8%, Z3 +3.0% / +4.2%, dqKZWZa1QwBW +2.0% / +3.6%. The other C files gain +0.1..+0.7%.
  - On A, 9 of 12 files gain +0.3..+0.9% with 24 files. The hard airborne files are flat or slightly worse (474000 -0.5% / -0.4%, 533084 -0.1% / +0.0%, 3828740 -0.3% / +1.0%). A's 8 train files already contain this regime.
- The gap between the recipe and the generalist grows: A +5.0% -> +7.6%, C +4.2% -> +10.2%.
- With 48 files, `spectokNC_n` on C (4.0897) is above 8-file `best_n` on C (4.0237-4.0302, report 16).

### 4.4 `best_n` on A48 / C48
`runs/search/{A48,C48}/best_n_g250_s1` (`search_big2.log`, `bins/zli_ace_v15`). Gains are vs `best_n` with 8 train files, seed 1 (A 4.0548, C 4.0237, report 16).

| test set | best_n 8 files | best_n 48 files | vs 8 files | vs upstream | train s (48 files) |
|---|---|---|---|---|---|
| A | 4.0548 | 4.0747 | +0.49% | +5.74% | 14086 |
| C | 4.0237 | 4.0954 | +1.78% | +8.91% | 14506 |

- No test file is worse by more than 0.05% (C 3828740 -0.05%). A: every file +0.29..+0.71%.
- On C the hard files gain most: 533084 +6.6%, Z3_6190800 +4.2%, 474000 +3.8%. The other C files gain +0.5..+1.4%.
- C48 `best_n` (4.0954) is about equal to `best_n` with x/y swap on C (4.0835, report 20).

### 4.5 Clean check: A48c / C48c
`A48c` / `C48c` = the 8 original train files + 40 corpus2 files from source dirs with **no** test file (row 98). Same test files. `runs/search/{A48c,C48c}/{spectokNC_n,best_n}_g250_s1` (`search_clean.log`), seed 1, `bins/zli_ace_v15`. "48" = the leaky A48 / C48 runs above.

| test set | label | 8 files | 48 | 48c | 48 vs 8 | 48c vs 8 | 48c vs 48 | 48c vs upstream | train s 48c |
|---|---|---|---|---|---|---|---|---|---|
| A | spectokNC_n | 4.0420 | 4.0624 | 4.0569 | +0.50% | +0.37% | -0.14% | +5.28% | 13355 |
| A | best_n | 4.0548 | 4.0747 | 4.0744 | +0.49% | +0.48% | -0.01% | +5.73% | 14225 |
| C | spectokNC_n | 3.9722 | 4.0897 | 4.0761 | +2.96% | +2.61% | -0.33% | +8.39% | 13977 |
| C | best_n | 4.0237 | 4.0954 | 4.0946 | +1.78% | +1.76% | -0.02% | +8.88% | 13585 |

- For `best_n`, clean and leaky are within 0.02%. Leakage is not the cause of the gain.
- For `spectokNC_n`, clean is a bit lower (A -0.14%, C -0.33%), but most of the gain stays.
- `best_n` 48c per file vs 8 files: every C file gains (+0.19..+6.57%; 533084 +6.6%, Z3_6190800 +4.2%, 474000 +3.7%). A worst file 533084 -0.17%, best znHYtqPtsNgf +1.04%.

### 4.6 Pinned decode, C (`runs/search/speed_C48c.json`, core 11, `bins/zli_ace_v16`)
The machine was busy, so absolute MB/s are low. Only ratios within this file are compared.

| model | ratio | dec MB/s | dec vs 8 files | per-file dec vs 8 files | enc vs 8 files |
|---|---|---|---|---|---|
| best_n, 8 files | 4.0237 | 63.7 | 1.00x | - | 1.00x |
| best_n, C48c (clean) | 4.0946 | 53.4 | 0.838x | 0.69-0.95x | 0.75x |
| best_n, C48 (leaky) | 4.0954 | 56.5 | 0.887x | 0.69-1.13x | 0.87x |

- Both 48-file models fail the 0.97x decode bar.
- The 48-file models keep 8/8/8 picks per big slot (K=8 is the cap). More training files give more and slower picks. This is the same effect as the x/y swap (report 20).

### 4.7 Next test: ACE_BRUTE_SLACK on 48c
`bests100_n` / `bests300_n` = `best_n` + `ACE_BRUTE_SLACK=100` / `300` (1% / 3%). Picks are listed fastest-decoding first. The selector takes the first pick within the slack of the smallest. This is encoder-only. Earlier 0.2% / 0.5% slack had no effect on 8-file models (report 13). Runs on C48c / A48c (`search_sl48.log`) are **still running**. When this report was written, none had a result file.

## 5. Verdict
- More varied training files help **only** the specialist recipe. A +0.50%, C +2.96% at 48 files. They hurt the single generalist search by about 2%.
- The gain depends on what the original 8 train files lack: large on C (whose train files miss the airborne regime), small on A.
- Production should train the specialist recipe on as many varied files as are available (row 96).
- `best_n` gains too: A +0.49%, C +1.78% with 48 files (row 107). With clean files (A48c / C48c) the gain is the same: A +0.48%, C +1.76% (row 109). Leakage is not the cause.
- But as is, the bigger corpus fails the decode bar: C48c `best_n` decodes at 0.838x (row 111). Not kept yet. Next: `ACE_BRUTE_SLACK=100/300` (still running).
- Cost: training time grows about linearly with files for the generalist (1391 -> 7792 s on A) and faster for the recipe (1869 -> 14613 s on A, 1647 -> 15020 s on C), because there is one specialist search per file.

## 6. Caveats
- One seed per cell, normal mode. Normal mode is not reproducible (report 06), so the A gains (+0.4..+0.5%) are close to seed noise (`best_n` seed spread on A is 0.29%, report 16). The C gains (+2.4..+3.0%, up to +14% on single files) are far above noise.
- The 16 and 40 new files are a sample (3 per source dir, then a subset). Other samples may give different numbers.
- The source dirs of the test files were not excluded when corpus2 was sampled (the clean check in 4.5 fixes this for 48 files). Some new train files share a source dir with a test file: A24 3 test files (2568500, zMuOt68XGoj6, znHYtqPtsNgf), A48 6, C24 3 (2568500, dqKZWZa1QwBW, zMuOt68XGoj6), C48 5. In C48 and A48, `treeseg-cell-failure/539000_6806000-CYn2P8SVRH7f-trees.json-merged` (another tile from 533084's source dir, about 6 km x 13 km away by name) is a train file. Content overlap was not checked. C24 has no train file from 533084's or 474000's dir, and still gives 533084 +11.8% and 474000 +9.1%, so most of the C airborne gain is not from same-dir files.
- Decode speed in 4.1 is from unpinned benchmark CSVs. The pinned check (4.6) is C only, one run, on a busy machine.
- Brute picks do not grow with more files (3/1/2/1/3 .. 4/1/3/1/3); K=4 and the 0.2% stop rule limit them. `best_n` (K=8, 0.01%) may use more picks with 48 files; its picks were not read here.
