# 15 Secondary columns and the point_source_id diagnosis

Decision rows 80, 81, 82, 83, 86. Data: `runs/search/<split>/{b4tol_n,spectok_n,tok_n,spec_n}_g250_s<seed>/<column>.json`, upstream `runs/base/<split>/<column>.json`.

## 1. Question
The final points recipe (`spectok`, report 14) was built and judged on points only. Does it also help the secondary columns (color, gps_time, intensity, scan_angle, point_source_id)? Where it hurts, which part causes it: ACE_TOK_SEEDS or ACE_SPECIALISTS? Which recipe should each column use?

## 2. Change
No new code. Binary `bins/zli_ace_v13`.

Run labels (`runs/search/<split>/<label>_g250_s<seed>/<column>.json`):
- `b4tol_n`: control, `ACE_BRUTE_K=4 ACE_BRUTE_TOL=0.02`.
- `spectok_n`: `ACE_TOK_SEEDS=1 ACE_SPECIALISTS=1 ACE_SPEC_GENERATIONS=60 ACE_BRUTE_K=4 ACE_BRUTE_TOL=0.5`.
- `tok_n`: `ACE_TOK_SEEDS=1 ACE_BRUTE_K=4 ACE_BRUTE_TOL=0.02`.
- `spec_n`: `ACE_SPECIALISTS=1 ACE_SPEC_GENERATIONS=60 ACE_BRUTE_K=4 ACE_BRUTE_TOL=0.5`.
- `_n` = normal mode, `g250` = 250 gens, `s<N>` = seed.

## 3. Method
- Normal mode, 250 gens, splits A/B/C. Profiles: color SDDL, gps_time `le-i64`, intensity and point_source_id `le-u16`, scan_angle `le-i16`.
- Seed 1: `b4tol_n`, `spec_n`, `spectok_n` on all 5 columns x A/B/C (rows 80, 82; `search_other*.log`, `search_spec_cols.log`).
- Diagnosis (row 81, `search_psid.log`): `tok_n` and `spec_n` on point_source_id for B and C.
- Seed 2 (row 86, `search_cols_s2.log`): `b4tol_n`, `spec_n`, `spectok_n` on gps_time, intensity, scan_angle x A/B/C (27 runs). No seed 2 for color and point_source_id.
- "worst file" = file with the lowest per-file ratio gain. Each worst file is listed with its share of the control's compressed bytes. A second column gives the worst file among files with at least 1% of the bytes (see 4.4).
- Speed: only the unpinned one-iteration benchmark CSVs ("bench dec MB/s", sum of source bytes / sum of decode time over the test chunks). No pinned speed run on these columns.

## 4. Results

### 4.1 Seed 1, all columns: `spec_n` and `spectok_n` vs `b4tol_n`

| column | split | b4tol_n_s1 | spec_n_s1 | spec gain | spectok_n_s1 | spectok gain | worst file spec (gain, byte share) | worst file spec, share >= 1% | bench dec MB/s (ctl / spec / spectok) |
|---|---|---|---|---|---|---|---|---|---|
| color | A | 15.1665 | 15.2252 | +0.39% | 15.2752 | +0.72% | zw7rJF499NRr_3 (-0.5%, 44.43% of bytes) | zw7rJF499NRr_3 (-0.5%) | 268 / 424 / 344 |
| color | B | 12.6151 | 12.7254 | +0.87% | 12.7209 | +0.84% | 752132_1153215 (+0.4%, 54.21% of bytes) | 752132_1153215 (+0.4%) | 291 / 386 / 394 |
| color | C | 73.5410 | 73.9550 | +0.56% | 74.0369 | +0.67% | 2024-ballarat_ (+0.4%, 93.80% of bytes) | 2024-ballarat_ (+0.4%) | 381 / 394 / 406 |
| gps_time | A | 10.0629 | 10.8336 | +7.66% | 10.7879 | +7.21% | 3828740_103001 (+0.7%, 15.86% of bytes) | 3828740_103001 (+0.7%) | 1481 / 1236 / 1385 |
| gps_time | B | 7.7270 | 7.9593 | +3.01% | 7.9454 | +2.83% | dqKZWZa1QwBW (+0.0%, 0.02% of bytes) | Z3_6190800_206 (+1.2%) | 753 / 768 / 1192 |
| gps_time | C | 7.0242 | 7.2711 | +3.52% | 7.2827 | +3.68% | 2519000_735700 (+0.0%, 15.45% of bytes) | 2519000_735700 (+0.0%) | 1117 / 840 / 1257 |
| intensity | A | 1.7928 | 1.8369 | +2.46% | 1.8431 | +2.80% | zw7rJF499NRr_3 (+0.6%, 8.41% of bytes) | zw7rJF499NRr_3 (+0.6%) | 444 / 484 / 599 |
| intensity | B | 2.0838 | 2.5174 | +20.81% | 2.5260 | +21.22% | 140 (-0.3%, 6.58% of bytes) | 140 (-0.3%) | 876 / 664 / 659 |
| intensity | C | 2.0447 | 2.0590 | +0.70% | 2.0629 | +0.89% | 533084_6793800 (+0.0%, 0.01% of bytes) | 2519000_735700 (+0.0%) | 381 / 331 / 581 |
| scan_angle | A | 4.3921 | 5.0802 | +15.67% | 5.1609 | +17.50% | 2568500_755800 (+9.6%, 24.33% of bytes) | 2568500_755800 (+9.6%) | 508 / 731 / 585 |
| scan_angle | B | 5.4888 | 6.4565 | +17.63% | 6.0154 | +9.59% | z5GMxol0oJkH (-18.1%, 0.04% of bytes) | 2519000_735700 (+3.3%) | 670 / 431 / 801 |
| scan_angle | C | 3.3466 | 3.5674 | +6.60% | 3.5892 | +7.25% | 2568500_755800 (-0.3%, 22.14% of bytes) | 2568500_755800 (-0.3%) | 568 / 317 / 636 |
| point_source_id | A | 27.5476 | 27.6932 | +0.53% | 28.0854 | +1.95% | z5GMxol0oJkH (+0.0%, 0.18% of bytes) | 752132_1153215 (+0.2%) | 961 / 1631 / 1196 |
| point_source_id | B | 21.0341 | 21.1218 | +0.42% | 19.5593 | -7.01% | z5GMxol0oJkH (+0.0%, 0.13% of bytes) | 2519000_735700 (+0.1%) | 1338 / 2174 / 1237 |
| point_source_id | C | 14.9956 | 15.0127 | +0.11% | 14.1377 | -5.72% | 2519000_735700 (-0.0%, 12.60% of bytes) | 2519000_735700 (-0.0%) | 1071 / 1597 / 866 |

Mean over the 3 splits (seed 1):

| column | spec_n vs b4tol_n | spectok_n vs b4tol_n |
|---|---|---|
| color | +0.61% | +0.74% |
| gps_time | +4.73% | +4.57% |
| intensity | +7.99% | +8.30% |
| scan_angle | +13.30% | +11.45% |
| point_source_id | +0.35% | -3.59% |

### 4.2 point_source_id diagnosis (seed 1)

| split | run | point_source_id ratio | b4tol_n control | gain | Z3_6190800 file ratio (run vs control) |
|---|---|---|---|---|---|
| B | tok_n_g250_s1 | 19.4771 | 21.0341 | -7.40% | 8.5446 vs 15.7317 (-45.7%) |
| B | spec_n_g250_s1 | 21.1218 | 21.0341 | +0.42% | 15.7762 vs 15.7317 (+0.3%) |
| B | spectok_n_g250_s1 | 19.5593 | 21.0341 | -7.01% | 8.5947 vs 15.7317 (-45.4%) |
| C | tok_n_g250_s1 | 14.1377 | 14.9956 | -5.72% | 8.5446 vs 15.7317 (-45.7%) |
| C | spec_n_g250_s1 | 15.0127 | 14.9956 | +0.11% | 15.8116 vs 15.7317 (+0.5%) |
| C | spectok_n_g250_s1 | 14.1377 | 14.9956 | -5.72% | 8.5446 vs 15.7317 (-45.7%) |
| A | spec_n_g250_s1 | 27.6932 | 27.5476 | +0.53% | Z3 is a train file in A |

The loss is one file: Z3_6190800_2067600 (a train file in A, a test file in B and C). With tok seeds its ratio drops from 15.73 to 8.54-8.59 (-45%). It has the lowest ratio of the column (the other test files are 7-31 or 1000+), so its bytes weigh a lot: 11.2% (B) and 8.5% (C) of the control's compressed bytes.

For point_source_id, `tok_n` and `spectok_n` on C give the same model file (byte-identical `.zlc`). With tok seeds, Z3 alone costs 451,037 extra bytes (536,227 -> 987,264), more than the column's whole net loss (383,369 on B, 381,620 on C).

### 4.3 Seeds 1 and 2: gps_time, intensity, scan_angle

| column | split | seed | b4tol_n | spec_n | spec gain | spectok_n | spectok gain | tok on top of spec (spectok vs spec) | worst file spec (gain, byte share) | worst file spec, share >= 1% | bench dec MB/s (ctl / spec / spectok) |
|---|---|---|---|---|---|---|---|---|---|---|---|
| gps_time | A | 1 | 10.0629 | 10.8336 | +7.66% | 10.7879 | +7.21% | -0.42% | 3828740_103001 (+0.7%, 15.86% of bytes) | 3828740_103001 (+0.7%) | 1481 / 1236 / 1385 |
| gps_time | A | 2 | 10.0662 | 10.8246 | +7.53% | 10.8235 | +7.52% | -0.01% | 3828740_103001 (+0.8%, 15.86% of bytes) | 3828740_103001 (+0.8%) | 1074 / 918 / 927 |
| gps_time | B | 1 | 7.7270 | 7.9593 | +3.01% | 7.9454 | +2.83% | -0.17% | dqKZWZa1QwBW (+0.0%, 0.02% of bytes) | Z3_6190800_206 (+1.2%) | 753 / 768 / 1192 |
| gps_time | B | 2 | 7.9513 | 7.8829 | -0.86% | 7.9240 | -0.34% | +0.52% | znHYtqPtsNgf (-13.4%, 0.02% of bytes) | 3090551_447834 (-1.7%) | 929 / 814 / 825 |
| gps_time | C | 1 | 7.0242 | 7.2711 | +3.52% | 7.2827 | +3.68% | +0.16% | 2519000_735700 (+0.0%, 15.45% of bytes) | 2519000_735700 (+0.0%) | 1117 / 840 / 1257 |
| gps_time | C | 2 | 7.0202 | 7.2765 | +3.65% | 7.2755 | +3.64% | -0.01% | 2024-ballarat_ (+0.0%, 2.80% of bytes) | 2024-ballarat_ (+0.0%) | 770 / 491 / 872 |
| intensity | A | 1 | 1.7928 | 1.8369 | +2.46% | 1.8431 | +2.80% | +0.34% | zw7rJF499NRr_3 (+0.6%, 8.41% of bytes) | zw7rJF499NRr_3 (+0.6%) | 444 / 484 / 599 |
| intensity | A | 2 | 1.8016 | 1.8398 | +2.12% | 1.8464 | +2.49% | +0.36% | 2568500_755800 (+0.2%, 9.77% of bytes) | 2568500_755800 (+0.2%) | 277 / 386 / 446 |
| intensity | B | 1 | 2.0838 | 2.5174 | +20.81% | 2.5260 | +21.22% | +0.34% | 140 (-0.3%, 6.58% of bytes) | 140 (-0.3%) | 876 / 664 / 659 |
| intensity | B | 2 | 2.0838 | 2.5179 | +20.83% | 2.5243 | +21.14% | +0.25% | 140 (-0.3%, 6.58% of bytes) | 140 (-0.3%) | 679 / 500 / 471 |
| intensity | C | 1 | 2.0447 | 2.0590 | +0.70% | 2.0629 | +0.89% | +0.19% | 533084_6793800 (+0.0%, 0.01% of bytes) | 2519000_735700 (+0.0%) | 381 / 331 / 581 |
| intensity | C | 2 | 2.0198 | 2.0596 | +1.97% | 2.0636 | +2.17% | +0.19% | 533084_6793800 (-9.9%, 0.01% of bytes) | 2519000_735700 (+0.0%) | 289 / 207 / 314 |
| scan_angle | A | 1 | 4.3921 | 5.0802 | +15.67% | 5.1609 | +17.50% | +1.59% | 2568500_755800 (+9.6%, 24.33% of bytes) | 2568500_755800 (+9.6%) | 508 / 731 / 585 |
| scan_angle | A | 2 | 4.7061 | 5.1453 | +9.33% | 5.1565 | +9.57% | +0.22% | 2568500_755800 (-0.2%, 23.48% of bytes) | 2568500_755800 (-0.2%) | 435 / 630 / 389 |
| scan_angle | B | 1 | 5.4888 | 6.4565 | +17.63% | 6.0154 | +9.59% | -6.83% | z5GMxol0oJkH (-18.1%, 0.04% of bytes) | 2519000_735700 (+3.3%) | 670 / 431 / 801 |
| scan_angle | B | 2 | 5.4886 | 5.8750 | +7.04% | 5.9320 | +8.08% | +0.97% | z5GMxol0oJkH (-11.7%, 0.04% of bytes) | 2519000_735700 (+0.3%) | 129 / 724 / 832 |
| scan_angle | C | 1 | 3.3466 | 3.5674 | +6.60% | 3.5892 | +7.25% | +0.61% | 2568500_755800 (-0.3%, 22.14% of bytes) | 2568500_755800 (-0.3%) | 568 / 317 / 636 |
| scan_angle | C | 2 | 3.3383 | 3.5813 | +7.28% | 3.5835 | +7.34% | +0.06% | 2568500_755800 (+0.2%, 22.20% of bytes) | 2568500_755800 (+0.2%) | 446 / 342 / 311 |

Mean over 6 cells (3 splits x 2 seeds), with min .. max:

| column | spec_n vs b4tol_n | spectok_n vs b4tol_n | tok seeds on top of spec |
|---|---|---|---|
| gps_time | +4.08% (-0.86 .. +7.66) | +4.09% (-0.34 .. +7.52) | +0.01% (-0.42 .. +0.52), 2/6 > 0 |
| intensity | +8.15% (+0.70 .. +20.83) | +8.45% (+0.89 .. +21.22) | +0.28% (+0.19 .. +0.36), 6/6 > 0 |
| scan_angle | +10.59% (+6.60 .. +17.63) | +9.89% (+7.25 .. +17.50) | -0.56% (-6.83 .. +1.59), 5/6 > 0 |

### 4.4 Per-file "worst" must be read with byte weight (scan_angle B)

On scan_angle B, the files with the largest losses are near-constant files: ratio 1259-1760, 5-7 KB of compressed bytes each, 0.03-0.04% of the column. The files that hold the bytes all gain.

| seed | file | ctl ratio | ctl bytes | byte share | spec gain | bytes saved |
|---|---|---|---|---|---|---|
| 1 | z5GMxol0oJkH | 1259.3 | 6,700 | 0.04% | -18.1% | -1,485 |
| 1 | znHYtqPtsNgf | 1506.8 | 5,114 | 0.03% | -13.4% | -791 |
| 1 | chunk_0_5 | 1680.9 | 5,072 | 0.03% | -11.8% | -676 |
| 1 | 533084_6793800-d | 1565.4 | 5,424 | 0.04% | -8.9% | -531 |
| 1 | dqKZWZa1QwBW | 1759.1 | 5,335 | 0.03% | -6.2% | -353 |
| 1 | 2519000_7357000 | 1.8 | 4,804,313 | 31.07% | +3.3% | +152,457 |
| 1 | 2568500_7558000- | 1.7 | 5,019,421 | 32.46% | +9.5% | +434,103 |
| 1 | zw7rJF499NRr_345 | 4.1 | 2,069,349 | 13.38% | +23.6% | +395,216 |
| 1 | 3090551_447834-C | 9.2 | 913,306 | 5.91% | +54.9% | +323,709 |
| 1 | 752132_1153215-d | 3.2 | 2,630,337 | 17.01% | +62.9% | +1,016,134 |
| 2 | z5GMxol0oJkH | 1259.3 | 6,700 | 0.04% | -11.7% | -885 |
| 2 | znHYtqPtsNgf | 1506.8 | 5,114 | 0.03% | -9.3% | -525 |
| 2 | chunk_0_5 | 1680.9 | 5,072 | 0.03% | -8.8% | -488 |
| 2 | 533084_6793800-d | 1565.4 | 5,424 | 0.04% | -7.4% | -435 |
| 2 | dqKZWZa1QwBW | 1759.1 | 5,335 | 0.03% | -6.4% | -365 |
| 2 | 2519000_7357000 | 1.8 | 4,804,382 | 31.07% | +0.3% | +12,049 |
| 2 | 2568500_7558000- | 1.7 | 5,019,478 | 32.46% | +3.7% | +181,022 |
| 2 | 752132_1153215-d | 3.2 | 2,630,305 | 17.01% | +14.8% | +338,574 |
| 2 | zw7rJF499NRr_345 | 4.1 | 2,069,270 | 13.38% | +19.3% | +334,814 |
| 2 | 3090551_447834-C | 9.2 | 913,914 | 5.91% | +20.2% | +153,344 |

The 5 near-constant files together lose 3,836 bytes (seed 1). The column saves 2,321,619 bytes on the other 5 files. The same holds for gps_time B s2 (znHYtqPtsNgf -13.4%, 0.02% of bytes) and intensity C s2 (533084 -9.9%, 0.01% of bytes). Read every "worst file" in reports 01-18 together with its byte share.

### 4.5 Per-column recipe vs upstream v0.2.3

Recipe runs (`spectok_n` for intensity, `spec_n` for the other columns; both without `ACE_NO_CTIME`) vs `runs/base`. There is no upstream scan_angle run.

| column | split | upstream | recipe seeds 1 / 2 | gain vs upstream |
|---|---|---|---|---|
| color | A | 15.0943 | 15.2252 | +0.87% |
| color | B | 12.6612 | 12.7254 | +0.51% |
| color | C | 73.1986 | 73.9550 | +1.03% |
| gps_time | A | 10.0590 | 10.8336 / 10.8246 | +7.70% / +7.61% |
| gps_time | B | 7.6039 | 7.9593 / 7.8829 | +4.67% / +3.67% |
| gps_time | C | 7.0254 | 7.2711 / 7.2765 | +3.50% / +3.57% |
| intensity | A | 1.7922 | 1.8431 / 1.8464 | +2.84% / +3.02% |
| intensity | B | 2.0527 | 2.5260 / 2.5243 | +23.06% / +22.97% |
| intensity | C | 2.0001 | 2.0629 / 2.0636 | +3.14% / +3.17% |
| scan_angle | A | no upstream run | 5.0802 / 5.1453 | - |
| scan_angle | B | no upstream run | 6.4565 / 5.8750 | - |
| scan_angle | C | no upstream run | 3.5674 / 3.5813 | - |
| point_source_id | A | 27.5476 | 27.6932 | +0.53% |
| point_source_id | B | 21.0341 | 21.1218 | +0.42% |
| point_source_id | C | 14.9956 | 15.0127 | +0.11% |

## 5. Verdict
- `ACE_SPECIALISTS` helps every secondary column. 2-seed mean vs `b4tol_n`: gps_time +4.08%, intensity +8.15%, scan_angle +10.59% (min +6.60%). Seed 1 only: color +0.61%, point_source_id +0.35%. The one cell below 0 is gps_time B s2 (-0.86%, see caveats).
- point_source_id regresses on B (-7.0%) and C (-5.7%) with `spectok`. The cause is `ACE_TOK_SEEDS`. `tok_n` alone gives the same loss (-7.4% / -5.7%). `spec_n` alone is fine (+0.42% / +0.11%, Z3 +0.3% / +0.5%). The tokenize-first seed genes were built from points graphs. They win on the u16 point_source_id training files, but they generalise badly to Z3.
- Tok seeds on top of spec, outside points: gps_time +0.01% (noise), intensity +0.28% (6/6 > 0, small but consistent), scan_angle -0.56% (one cell -6.83%, B s1).
- **Per-column recipe** (row 86):
  - points and intensity: `spectok` + `ACE_NO_CTIME=1`.
  - color, gps_time, scan_angle, point_source_id: `spec` + `ACE_NO_CTIME=1` (no `ACE_TOK_SEEDS`).
  - The points recipe was later extended with `ACE_BRUTE_K=8 ACE_BRUTE_MIN_GAIN=0.0001` (report 16). Those two flags were not tested on the secondary columns.
- Per-file "worst" numbers must be read with byte weight. The scan_angle B losses of -6..-18% are on near-constant files with no material bytes. They are not a regression.

## 6. Caveats
- Normal mode is not reproducible (report 06). Color and point_source_id have only one seed. gps_time B moves a lot between seeds: the control is 7.7270 (s1) vs 7.9513 (s2), so spec gives +3.01% on s1 and -0.86% on s2. In B s2, the file with real bytes that loses most is 3090551_447834 (-1.7%).
- `ACE_NO_CTIME` was not run on the secondary columns. The recipe assumes it is ratio-neutral there, as on points (report 14).
- For point_source_id, the `b4tol_n_g250_s1` model files are byte-identical to `runs/base` (brute picks only 1 candidate), so the "control" equals upstream for this column.
- Row 80 says Z3 "dominates" the point_source_id held-out bytes. The result files show it is 8.5-11.2% of the bytes: the single largest loss, but not most of the bytes.
- Decode MB/s here is unpinned and from a loaded machine. It says nothing reliable about decode cost on these columns.
