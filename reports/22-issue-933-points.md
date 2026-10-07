# 22 Issue 933 on points: does training on more or other files hurt?

Decision rows 114, 115, 121, 124. Data: `split_I5all.json`, `split_I5self_<x>.json`, `split_I5loo_<x>.json`; upstream `runs/search_up/<split>/ctl_n_g250_s1/points.json`; ours `runs/search/<split>/best_n_g250_s1/points.json` and `runs/search/I5all/bestl3k_n_g250_s1/points.json`.

## 1. Question
Upstream issue facebook/openzl#933 says that training on more or different files can make a file compress worse than a model trained on that file alone. The worst case in the issue is dqKZWZa1QwBW: there the self-trained model wins by 65% (row 115; whole-file numbers, all columns).

For the points column only:
1. Does upstream v0.2.3 show the same effect on the issue's 5 files?
2. Does our recipe (`best_n`) show it?
3. How well does each do on a file it has never seen?

## 2. Change
No code. New data and splits.
- The 5 issue files (the "I5" files). Short names:

| short | LAZ tag | points source bytes | byte share |
|---|---|---|---|
| zn | `znHYtqPtsNgf` | 9,474,888 | 19.8% |
| dA | `dAPcA8fVvH3u_482813_4206587` | 9,005,220 | 18.8% |
| out | `output` | 11,034,060 | 23.0% |
| dq | `dqKZWZa1QwBW` | 8,773,416 | 18.3% |
| 140 | `140` | 9,631,476 | 20.1% |

- dA and out come from `~/openzl-training` and were dumped with `dump_ot.py` (same sampler flags as `dump_all.py`, row 114). zn, dq and 140 were already in our corpus.
- Splits:
  - `I5all`: train on all 5, test on all 5.
  - `I5self_<x>`: train on file x only, test on file x (train = test).
  - `I5loo_<x>`: train on the other 4, test on file x (leave one out).
- New labels:
  - `up_self`, `up_all`, `up_loo`: upstream v0.2.3 `ctl_n` (`bins/zli_upstream`, no ACE_* env vars) on `I5self_<x>`, `I5all`, `I5loo_<x>`.
  - `our_self`, `our_all`, `our_loo`: our `best_n` on the same splits (`bins/zli_ace_v17`, row 121; `best_n` does not set ACE_BRUTE_LAMBDA).
  - `bestl3k_n`: `best_n` + `ACE_BRUTE_LAMBDA=3000` (`bins/zli_ace_v17`). Run on `I5all` only.

## 3. Method
- Points only, normal mode, 250 gens, seed 1. One run per split and label (11 splits x 2 builds, plus `bestl3k_n`).
- Ratio per file = source bytes / compressed bytes of that file's test chunks. For `self` and `loo`, the test set is one file, so this is the run's total ratio. For `all`, it is the `per_laz` value of the `I5all` run (4 decimals).
- Totals are byte-weighted over the 5 files: sum of source bytes / sum of compressed bytes. For `all` this is the `I5all` run's own total. For `self` and `loo` it combines the 5 separate runs.
- Upstream oracle: for each file, the better of `up_self` and `up_all`. This is what a user would get by picking the better upstream model per file.
- "self" is an in-sample test: the model is tested on the data it was trained on.

## 4. Results

### 4.1 Per-file ratio

| file | byte share | up_self | up_all | up_loo | our_self | our_all | our_loo | our_all vs up_self | our_loo vs up_self |
|---|---|---|---|---|---|---|---|---|---|
| zn (`znHYtqPtsNgf`) | 19.8% | 4.4976 | 4.4839 | 3.8002 | 4.5378 | 4.5584 | 4.5604 | +1.35% | +1.40% |
| dA (`dAPcA8fVvH3u_482813_4206587`) | 18.8% | 2.9479 | 2.9293 | 2.9364 | 2.9513 | 2.9589 | 2.9489 | +0.37% | +0.03% |
| out (`output`) | 23.0% | 3.8335 | 3.8098 | 3.8432 | 3.8559 | 3.8850 | 3.8782 | +1.34% | +1.16% |
| dq (`dqKZWZa1QwBW`) | 18.3% | 2.7759 | 3.7084 | 3.3848 | 4.1829 | 4.1824 | 3.7315 | +50.67% | +34.43% |
| 140 (`140`) | 20.1% | 4.1783 | 4.1337 | 4.1392 | 4.1949 | 4.1798 | 4.1770 | +0.04% | -0.03% |
| **total** (byte-weighted) | 100% | 3.5481 | 3.7497 | 3.5894 | 3.8662 | 3.8756 | 3.7930 | +9.23% | +6.90% |

- Upstream oracle (best of `up_self` / `up_all` per file): total **3.7700**.
- `our_all` beats the upstream oracle by +2.80% and `up_all` by +3.36%.
- `our_loo` (each file unseen) beats `up_all` (every file seen) by +1.15% in total. It beats it on every file (zn +1.71%, dA +0.67%, out +1.80%, dq +0.62%, 140 +1.05%).
- `our_all` beats `up_self` on every file. On 140 the gain is only +0.04%.
- `bestl3k_n` on `I5all`: total 3.8730 (-0.07% vs `our_all`).

### 4.2 Effect of training set, per build
Change vs the same build's self-trained model.

| file | up: all vs self | up: loo vs self | ours: all vs self | ours: loo vs self |
|---|---|---|---|---|
| zn | -0.30% | -15.51% | +0.45% | +0.50% |
| dA | -0.63% | -0.39% | +0.26% | -0.08% |
| out | -0.62% | +0.25% | +0.75% | +0.58% |
| dq | +33.59% | +21.94% | -0.01% | -10.79% |
| 140 | -1.07% | -0.94% | -0.36% | -0.42% |
| total | +5.68% | +1.16% | +0.24% | -1.89% |

- Upstream, more files (`all`) costs 0.3-1.1% on 4 of 5 files. On dq it is the opposite: `up_all` is +33.6% over `up_self`.
- Upstream self-trained dq is the **worst** of the three (2.7759 vs 3.7084 all-5 and 3.3848 LOO). This is the opposite of the issue's whole-file result for dq.
- Upstream on an unseen file (`up_loo`) loses 15.5% on zn. The other files are within -0.9 .. +0.3%, except dq (+21.9%, because `up_self` is so bad).
- Ours: `our_all` is within -0.36 .. +0.75% of `our_self` on every file. So with our recipe, training on more files does not hurt.
- Ours on an unseen file: within -0.4 .. +0.6% on 4 files. dq loses 10.8% (3.7315 vs 4.1829). dq is the file the other 4 do not cover well.

### 4.3 Training time (s)

| split | upstream ctl_n | our best_n |
|---|---|---|
| I5all | 1144 | 1804 |
| I5self_zn / I5loo_zn | 207 / 841 | 258 / 1597 |
| I5self_dA / I5loo_dA | 174 / 923 | 184 / 1790 |
| I5self_out / I5loo_out | 251 / 849 | 295 / 1261 |
| I5self_dq / I5loo_dq | 414 / 823 | 357 / 1227 |
| I5self_140 / I5loo_140 | 346 / 864 | 266 / 1111 |

## 5. Verdict
- On points, issue 933's effect does not show for dq. Upstream self-trained dq is far worse (2.78) than the all-5 model (3.71). So the issue's dq gap must be in another column. Intensity and gps_time runs for both builds are **in progress** (`search_I5u2.log`, `search_I5o2.log`).
- Upstream does show a small version of the effect on the other 4 files: all-5 costs 0.3-1.1% vs self.
- With our recipe, more files does not hurt. One `best_n` model on all 5 files beats upstream's self-trained model on every file, and beats the upstream best-per-file oracle by +2.80%.
- Ours on an unseen file beats upstream on a seen file: `our_loo` 3.7930 vs `up_all` 3.7497 (+1.15%).

## 6. Caveats
- **No decode numbers.** This report measures ratio only. No pbench or zli benchmark run was made on the I5 models.
- Points only. The issue's numbers are whole-file (all columns). This report cannot say where the issue's dq gap comes from.
- One seed, normal mode. Normal mode is not reproducible (report 06). Differences under about 0.3% are within seed noise.
- `self` is in-sample (train = test). It is the best case for a per-file model, not a held-out score.
- Only 5 files. The upstream dq self result (2.78) is from one run. A bad search on one small training set could explain it; it was not repeated.
- Our data are `pointcloud_sampler` dumps in 256 KiB chunks (row 115). The issue may use a different setup, so absolute ratios are not compared with the issue.
- Decision row 124 gives slightly different totals (up_self 3.5475, up_loo 3.5890, our_self 3.8666, our_all 3.8760, our_loo 3.7931, oracle 3.7701). The numbers here are recomputed from the compressed byte counts in the result files.
