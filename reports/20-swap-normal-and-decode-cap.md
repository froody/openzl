# 20 x/y swap in normal mode, and a decode-time cap on brute picks (ACE_BRUTE_DEC_CAP)

Decision rows 103-106, 108, 110. Data: `runs/search/{A,B,C}sw/best_n_g250_s<seed>/points.json` (`search_bestsw.log`), `runs/search/{A,C}sw/{bestc115_n,bestc13_n}_g250_s1/points.json` (`search_cap.log`), `runs/search/{A,B,C}sw/bestc115_n_g250_s{2,3}` and `Bsw/bestc115_n_g250_s1` (`search_c115.log`), pinned speed files `runs/search/speed_C1sw.json`, `speed_C1sw_r2.json`, `speed_A1sw.json`, `speed_Ccap.json`.

## 1. Question
Report 17 found that training on x/y-swapped copies (`Asw/Bsw/Csw`) gains held-out points ratio in size-only mode, mostly on C. Two questions follow:
1. Does the swap still gain in normal mode with the points recipe `best_n`, and what does it cost in decode speed?
2. If decode gets slower, can a cap on the decode time of brute picks win the speed back and keep the ratio?

## 2. Change
- x/y swap: data only, as in report 17. Splits `Asw/Bsw/Csw`, same 12 test files as A/B/C.
- `ACE_BRUTE_DEC_CAP=f`: commit f56868a on `ar/ace`, binary `bins/zli_ace_v16`. After the first brute pick, skip candidates whose training decode time is more than f x the first pick's. Off = unchanged.

Run labels (`runs/search/<split>/<label>_g250_s<seed>/points.json`):
- `best_n` on A/B/C: control (report 16), `bins/zli_ace_v14`/`v15`.
- `best_n` on Asw/Bsw/Csw: same flags, swap-augmented training data, `bins/zli_ace_v15`. In speed files: `best_sw`.
- `bestc115_n`: `best_n` + `ACE_BRUTE_DEC_CAP=1.15`, on the swap splits, `bins/zli_ace_v16`. In speed files: `c115`.
- `bestc13_n`: `best_n` + `ACE_BRUTE_DEC_CAP=1.3`, on the swap splits, `bins/zli_ace_v16`. In speed files: `c13`.

## 3. Method
- Points only, normal mode, 250 gens.
- Swap: seeds 1-3 x A/B/C, 9 cells. Not paired (the training data differ, so the search differs).
- Cap: seed 1 on Csw and Asw for both caps. Then seeds 1-3 x Asw/Bsw/Csw for cap 1.15 (9 cells).
- Pinned decode: `speed_ab.py` (glossary), all 12 test files of the split, 5 rounds.
- "gain" = vs `best_n` on the unswapped split, same seed. "vs upstream" = vs `runs/base/<split>/points.json` (A 3.8535, B 3.8798, C 3.7605). "worst file" = the test file with the lowest ratio change vs `best_n`.

## 4. Results

### 4.1 Swap in normal mode (`best_n`, 9 cells)

| split | seed | best_n | best_n on *sw | gain | vs upstream | worst file | train s (best_n / sw) |
|---|---|---|---|---|---|---|---|
| A | 1 | 4.0548 | 4.0652 | +0.26% | +5.49% | -0.21% (3828740) | 2029 / 4413 |
| A | 2 | 4.0462 | 4.0590 | +0.32% | +5.33% | +0.03% (533084) | 1949 / 4201 |
| A | 3 | 4.0579 | 4.0485 | -0.23% | +5.06% | -0.59% (2568500) | 1956 / 4203 |
| B | 1 | 4.0554 | 4.0559 | +0.01% | +4.54% | -0.19% (dqKZWZa1QwBW) | 2195 / 4672 |
| B | 2 | 4.0459 | 4.0566 | +0.27% | +4.56% | -0.25% (Z3_6190800) | 2122 / 4501 |
| B | 3 | 4.0536 | 4.0553 | +0.04% | +4.52% | -0.14% (3090551) | 2123 / 4435 |
| C | 1 | 4.0237 | 4.0835 | +1.49% | +8.59% | -0.02% (3828740) | 1750 / 4243 |
| C | 2 | 4.0302 | 4.0872 | +1.41% | +8.69% | -0.31% (2024-ballarat) | 1898 / 3977 |
| C | 3 | 4.0275 | 4.0849 | +1.43% | +8.63% | -0.65% (2568500) | 1731 / 4034 |

- Mean gain +0.56%. 8/9 cells >= 0. Per split: A +0.12%, B +0.11%, C +1.44%. (Row 103 says +0.55% mean and A +0.11%; the numbers here are recomputed from the result files.)
- C gains in all 3 seeds (+1.41..+1.49%). A and B are within seed noise.
- Worst single file: -0.65% (C s3, 2568500).
- Swap about doubles training time (1731-2195 s -> 3977-4672 s).

### 4.2 Pinned decode, swap

| file | split, seed | core | ratio best_n / sw | dec MB/s best_n / sw | dec sw / best_n | worst file dec | enc sw / best_n |
|---|---|---|---|---|---|---|---|
| `speed_C1sw.json` | C s1 | 11 | 4.0237 / 4.0835 | 93.7 / 82.4 | 0.880x | 0.66x (533084) | 0.90x |
| `speed_C1sw_r2.json` | C s1 (repeat) | 13 | 4.0237 / 4.0835 | 91.8 / 80.6 | 0.878x | 0.82x (EX_20230607) | 0.90x |
| `speed_A1sw.json` | A s1 | - | 4.0548 / 4.0652 | 64.1 / 63.7 | 0.994x | 0.86x (3828740) | 0.84x |

- On C the swap model decodes 12% slower. The repeat on another core gives the same number, so it is real.
- On A the decode cost is small (0.994x).
- The C swap model keeps 7-8 picks per big slot (best_n: 6-7). The extra picks include `field_lz`, `num8` and double-transpose chains (row 104). This is below the 0.97x decode bar.

### 4.3 ACE_BRUTE_DEC_CAP on the swap splits, seed 1

| split | variant | ratio | vs best_n | vs upstream | worst file | picks per big slot | train s |
|---|---|---|---|---|---|---|---|
| C | best_n (sw) | 4.0835 | +1.49% | +8.59% | -0.02% | 7-8 | 4243 |
| C | bestc115_n | 4.0784 | +1.36% | +8.45% | +0.04% | 8/8/8 | 3611 |
| C | bestc13_n | 4.0767 | +1.32% | +8.41% | +0.12% | 6/7/8 | 3584 |
| A | best_n (sw) | 4.0652 | +0.26% | +5.49% | -0.21% | - | 4413 |
| A | bestc115_n | 4.0500 | -0.12% | +5.10% | -0.76% | - | 3960 |
| A | bestc13_n | 4.0543 | -0.01% | +5.21% | -0.12% | - | 3871 |

- On C s1 the cap keeps most of the swap gain (+1.36% / +1.32% vs +1.49%). This does not hold over 3 seeds (4.5). No C test file is worse than `best_n`.
- On A all three are within noise of `best_n`. `bestc115_n` has one file at -0.76% (474000).
- Picks were not read for A.

### 4.4 Pinned decode, cap, C s1 (`speed_Ccap.json`, core 11)

| variant | ratio | dec MB/s | dec vs best_n | worst file dec | enc vs best_n |
|---|---|---|---|---|---|
| best_n | 4.0237 | 102.0 | 1.00x | - | 1.00x |
| best_n (sw) | 4.0835 | 90.1 | 0.883x | 0.81x (2568500) | 0.90x |
| bestc115_n | 4.0784 | 99.8 | 0.979x | 0.85x (2519000) | 0.70x |
| bestc13_n | 4.0767 | 96.0 | 0.941x | 0.76x (2519000) | 0.76x |

- `bestc115_n` passes the 0.97x decode bar on C (0.979x). `bestc13_n` does not (0.941x).
- Encode with the cap is slower (0.70x). Encode is low priority.
- Absolute MB/s differ between speed files (best_n on C s1: 93.7, 91.8, 102.0). Only ratios within one file are compared.

### 4.5 Cap 1.15, 3 seeds (`search_c115.log`)
"vs best_n" = vs `best_n` on the unswapped split, same seed. "worst file" = vs `best_n`.

| split | seed | best_n | best_n on *sw | bestc115_n | c115 vs best_n | sw vs best_n | c115 vs upstream | worst file c115 |
|---|---|---|---|---|---|---|---|---|
| A | 1 | 4.0548 | 4.0652 | 4.0500 | -0.12% | +0.26% | +5.10% | -0.76% (474000) |
| A | 2 | 4.0462 | 4.0590 | 4.0469 | +0.02% | +0.32% | +5.02% | -0.14% (140) |
| A | 3 | 4.0579 | 4.0485 | 4.0421 | -0.39% | -0.23% | +4.89% | -1.02% (474000) |
| B | 1 | 4.0554 | 4.0559 | 4.0539 | -0.04% | +0.01% | +4.49% | -0.29% (Z3_6190800) |
| B | 2 | 4.0459 | 4.0566 | 4.0393 | -0.16% | +0.27% | +4.11% | -0.65% (dqKZWZa1QwBW) |
| B | 3 | 4.0536 | 4.0553 | 4.0496 | -0.10% | +0.04% | +4.38% | -0.20% (z5GMxol0oJkH) |
| C | 1 | 4.0237 | 4.0835 | 4.0784 | +1.36% | +1.49% | +8.45% | +0.04% (3828740) |
| C | 2 | 4.0302 | 4.0872 | 4.0090 | -0.53% | +1.41% | +6.61% | -2.46% (474000) |
| C | 3 | 4.0275 | 4.0849 | 4.0225 | -0.12% | +1.43% | +6.97% | -0.84% (474000) |

- `bestc115_n` vs `best_n`: mean -0.01%. Per split: A -0.16%, B -0.10%, C +0.24%. Only 2 of 9 cells > 0.
- Uncapped swap on the same seeds: mean +0.56%, C +1.41..+1.49% in all 3 seeds.
- The C s1 gain (+1.36%) does not repeat. C s2 is -0.53% and C s3 is -0.12%.
- C s2, `bestc115_n` vs uncapped swap: 474000 -6.3%, 533084 -5.9%, Z3_6190800 -3.9%. These are the hard files that swap fixes (report 17).

Likely cause: the cap is relative to pick 0. The candidates that fix the hard files decode slower than pick 0, so the cap usually removes them. In C s1, pick 0 was slow, so these candidates passed the cap.

## 5. Verdict
- x/y swap holds in normal mode: +0.56% mean, C +1.44% in all 3 seeds. A and B are flat.
- Uncapped, the swap model is too slow to decode on C (0.88x, measured twice).
- `ACE_BRUTE_DEC_CAP=1.15` is **rejected** (row 110). Over 3 seeds it gains nothing (-0.01% mean, C +0.24%). The seed-1 C result was luck.
- The swap gain needs the slower picks. A cap relative to pick 0 removes them.
- So swap is still not kept: it gains ratio on C but fails the 0.97x decode bar there.

## 6. Caveats
- Cap 1.3 has seed 1 only. Normal mode is not reproducible (report 06).
- The cause above (cap relative to pick 0) is a likely reading. The picks and their decode times per seed were not compared.
- Pinned decode for the cap was measured on C s1 only, the one cell where the cap kept the ratio.
- The cap uses training decode times. These are noisy (machine load), so the picks may differ between runs.
- The worst file under `bestc115_n` still decodes at 0.85x.
- Swap doubles training time. The cap cuts it by about 10-15% (C 4243 -> 3611 s, A 4413 -> 3960 s).
