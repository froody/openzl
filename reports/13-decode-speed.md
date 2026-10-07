# 13 Decode speed: pinned speed_ab.py measurements and the slack tie-break

Decision rows 69-71, 74, 75, 79. Data: `runs/search/speed_*.json`, `runs/search/<split>/spec{,s20,s50}_g120_s1`, `runs/search/<split>/spec{s20,s50,d}_n_g250_s1`.

## 1. Question
Decompression runs hourly in a browser or on mobile, so decode speed is the second priority after ratio (row 67). How much decode speed do the kept changes cost, measured so that machine load does not decide the answer? Can a decode-time tie-break in the brute selection win back the cost of ACE_SPECIALISTS?

## 2. Change
Measurement tool: `speed_ab.py`. For a list of models and LAZ files it runs `zli benchmark -n 5` on one pinned core (`taskset -c <core>`), for 5 rounds. Each round runs every model on every file once, and the model order flips every round, so load drift hits all models alike. Per file it takes the median round. Summary per model: ratio, `dec_mbs` = sum of srcSize / sum of median dtimeMs, `enc_mbs` the same with ctimeMs, and `per_laz_dec`.

Tie-break: commit a65d5a4 on `ar/ace`, binary `bins/zli_ace_v13`.
- `ACE_BRUTE_SLACK=<n>`: sets the new brute-force selector int param `ZL_BRUTE_FORCE_SLACK_PARAM` to n (units of 1e-4, so 20 = 0.2%). At training, the brute picks are listed fastest-decoding first. At compression, the selector takes the first pick whose size is within (1 + n x 1e-4) of the smallest. Encoder-only choice: the stock upstream decoder reads the output (checked).
- `ACE_DEC_EPS=e`: when ACE picks brute candidates, among candidates whose training total is within (1+e) of the best next total, take the one that decodes fastest.

Run labels (`runs/search/<split>/<label>_g<gens>_s<seed>`; all add these to `spec` = `ACE_SPECIALISTS=1 ACE_SPEC_GENERATIONS=60 ACE_BRUTE_K=4 ACE_BRUTE_TOL=0.5`):
- `specs20`: `ACE_BRUTE_SLACK=20` (0.2%).
- `specs50`: `ACE_BRUTE_SLACK=50` (0.5%).
- `specd`: `ACE_BRUTE_SLACK=20 ACE_DEC_EPS=0.002`.
- `_n` = normal mode; no suffix = size-only. `g120`, `g250` = generations; `s1` = seed 1.

Speed files (model names as stored in the JSON):
- `speed_A1`, `speed_B1`, `speed_C2`: `ctl` = `b4tol_n_g250`, `spec` = `spec_n_g250`, `pool` = `pool3b4tol_n_g83`, for A seed 1, B seed 1, C seed 2. 5 ordinary test files.
- `speed_A1max`: A seed 1, adds `max` = `max_n_g250` and `spectok` = `spectok_n_g250`, and uses the hard files 533084 and 474000 plus 3 others.
- `speed_A1nc`: A seed 1, same 5 files, `b4tol_n`, `spectok_n`, `spectokNC_n`, `spectokL_n` (report 14).

## 3. Method
- Points only. All speed runs on one pinned core, 5 rounds of `-n 5`, median per file. Single-thread MB/s of raw points input.
- Load: speed_A1/B1/C2 ran first (row 69); speed_A1max at load ~165 with 28 trainings running (row 74); speed_A1nc at load ~110 (row 79). Absolute MB/s differ between files, so compare models only inside one file.
- Slack, first test (row 71): normal mode, 250 gens, seed 1 x A/B/C, vs `spec_n_g250_s1`.
- Slack, second test (row 75): size-only, 120 gens, seed 1 x A/B/C. Size-only runs with the same seed give the identical search (report 06), so only the selector order and slack differ.

## 4. Results
All pinned speed measurements (ratio over the same 5 files; "ctl" is the first model of each file):

| speed file | cell | model | run label | ratio (5 files) | ratio vs ctl | dec MB/s | dec vs ctl | per-file dec vs ctl | enc MB/s | enc vs ctl |
|---|---|---|---|---|---|---|---|---|---|---|
| speed_A1.json | A s1 | ctl | b4tol_n_g250 | 4.1677 | +0.0% | 86.5 | 1.00x | 1.00-1.00x | 12.4 | 1.00x |
| speed_A1.json | A s1 | spec | spec_n_g250 | 4.2141 | +1.1% | 81.5 | 0.94x | 0.78-1.06x | 7.1 | 0.57x |
| speed_A1.json | A s1 | pool | pool3b4tol_n_g83 | 4.1499 | -0.4% | 94.2 | 1.09x | 0.98-1.19x | 9.3 | 0.75x |
| speed_B1.json | B s1 | ctl | b4tol_n_g250 | 3.6715 | +0.0% | 87.8 | 1.00x | 1.00-1.00x | 6.8 | 1.00x |
| speed_B1.json | B s1 | spec | spec_n_g250 | 3.7893 | +3.2% | 82.4 | 0.94x | 0.81-1.06x | 5.9 | 0.86x |
| speed_B1.json | B s1 | pool | pool3b4tol_n_g83 | 3.6722 | +0.0% | 88.7 | 1.01x | 0.99-1.05x | 6.8 | 1.00x |
| speed_C2.json | C s2 | ctl | b4tol_n_g250 | 3.8377 | +0.0% | 81.1 | 1.00x | 1.00-1.00x | 12.0 | 1.00x |
| speed_C2.json | C s2 | spec | spec_n_g250 | 3.8833 | +1.2% | 78.7 | 0.97x | 0.91-1.04x | 9.4 | 0.78x |
| speed_C2.json | C s2 | pool | pool3b4tol_n_g83 | 3.7918 | -1.2% | 91.3 | 1.13x | 1.02-1.25x | 13.4 | 1.12x |
| speed_A1max.json | A s1 | ctl | b4tol_n_g250 | 3.7833 | +0.0% | 62.5 | 1.00x | 1.00-1.00x | 9.1 | 1.00x |
| speed_A1max.json | A s1 | max | max_n_g250 | 3.8493 | +1.7% | 68.7 | 1.10x | 0.95-1.25x | 4.2 | 0.47x |
| speed_A1max.json | A s1 | pool | pool3b4tol_n_g83 | 3.9722 | +5.0% | 64.2 | 1.03x | 0.91-1.20x | 6.3 | 0.69x |
| speed_A1max.json | A s1 | spec | spec_n_g250 | 4.1854 | +10.6% | 58.8 | 0.94x | 0.82-1.07x | 5.0 | 0.56x |
| speed_A1max.json | A s1 | spectok | spectok_n_g250 | 4.1708 | +10.2% | 62.0 | 0.99x | 0.91-1.13x | 4.5 | 0.49x |
| speed_A1nc.json | A s1 | b4tol_n | b4tol_n_g250 | 3.7833 | +0.0% | 89.7 | 1.00x | 1.00-1.00x | 12.5 | 1.00x |
| speed_A1nc.json | A s1 | spectok_n | spectok_n_g250 | 4.1708 | +10.2% | 84.8 | 0.95x | 0.87-1.05x | 5.9 | 0.47x |
| speed_A1nc.json | A s1 | spectokNC_n | spectokNC_n_g250 | 4.1750 | +10.4% | 86.3 | 0.96x | 0.88-1.04x | 6.0 | 0.48x |
| speed_A1nc.json | A s1 | spectokL_n | spectokL_n_g250 | 4.1663 | +10.1% | 83.2 | 0.93x | 0.86-1.02x | 6.3 | 0.50x |

Hard files in `speed_A1max` (row 74): on 533084 the ratio goes from 3.659 (ctl) to 4.634 (spec) / 4.640 (spectok), +27%; decode 0.85x (spec) and 0.96x (spectok). On 474000 ratio +24%, decode 1.07x / 1.13x.

Slack tie-break, size-only, same seed, same search (held-out ratio and number of test chunks whose compressed size changed vs `spec_g120_s1`):

| split | spec_g120_s1 | specs20_g120_s1 | chunks changed | specs50_g120_s1 | chunks changed |
|---|---|---|---|---|---|
| A | 4.031373 | 4.031375 (+0.6 ppm) | 2/591 | 4.031374 (+0.4 ppm) | 1/591 |
| B | 4.023693 | 4.023694 (+0.3 ppm) | 1/530 | 4.023694 (+0.3 ppm) | 1/530 |
| C | 4.047241 | 4.047240 (-0.2 ppm) | 1/575 | 4.047240 (-0.2 ppm) | 1/575 |

Brute pick counts per ACE graph are identical in all three (A 3/1/2/1/3, B 3/1/2/1/3, C 2/1/3/1/2).

Slack tie-break, normal mode (different search trajectories), vs `spec_n_g250_s1`:

| split | seed | specs20_n | spec_n | gain | worst file (gain) | bench dec MB/s (var / ctl) |
|---|---|---|---|---|---|---|
| A | 1 | 4.0086 | 4.0574 | -1.20% | 533084_6793800 (-5.6%) | 417 / 210 |
| B | 1 | 4.0436 | 4.0357 | +0.20% | 140 (-0.1%) | 404 / 267 |
| C | 1 | 4.0155 | 3.9653 | +1.26% | Z3_6190800_206 (-0.4%) | 342 / 280 |

Mean gain +0.09% (min -1.20%, max +1.26%), 2 of 3 cells > 0.

| split | seed | specs50_n | spec_n | gain | worst file (gain) | bench dec MB/s (var / ctl) |
|---|---|---|---|---|---|---|
| A | 1 | 4.0458 | 4.0574 | -0.28% | 474000_6632000 (-1.2%) | 333 / 210 |
| B | 1 | 4.0378 | 4.0357 | +0.05% | 140 (-0.1%) | 394 / 267 |
| C | 1 | 4.0587 | 3.9653 | +2.35% | 2519000_735700 (-0.1%) | 278 / 280 |

Mean gain +0.71% (min -0.28%, max +2.35%), 2 of 3 cells > 0.

| split | seed | specd_n | spec_n | gain | worst file (gain) | bench dec MB/s (var / ctl) |
|---|---|---|---|---|---|---|
| A | 1 | 4.0330 | 4.0574 | -0.60% | 2568500_755800 (-1.4%) | 394 / 210 |
| B | 1 | 4.0457 | 4.0357 | +0.25% | zw7rJF499NRr_3 (+0.0%) | 379 / 267 |
| C | 1 | 3.9176 | 3.9653 | -1.20% | 474000_6632000 (-4.2%) | 312 / 280 |

Mean gain -0.52% (min -1.20%, max +0.25%), 1 of 3 cells > 0.

## 5. Verdict
- Decode cost of the kept changes, pinned: `spec` 0.94-0.97x, `spectok` 0.95-0.99x, `pool` 1.01-1.13x, `max` 1.10x. The spectok cost is 1-5%, about the run-to-run spread of the measurement. Accepted for +4% ratio.
- Slack tie-break (`ACE_BRUTE_SLACK`, `ACE_DEC_EPS`): rejected. In the identical-search test the ratio is the same to within 1 ppm and only 1-2 chunks per split change. On real data the picks are almost never within 0.5% of each other on a chunk, so the selector almost never switches and there is no decode gain to get. The normal-mode test (-1.2..+2.4%) was only search noise. The slack param is not in the final recipe, so the selector stays stock.

## 6. Caveats
- Speed was measured on 5 files per cell, for 3 cells (A s1, B s1, C s2), points only.
- The machine was shared during all speed runs. The same spectok model measured 0.99x in `speed_A1max` and 0.95x in `speed_A1nc`: that is the spread of the method.
- No pinned speed run was done on the slack models, because their ratio and chunk choices are nearly identical to `spec`.
- The "bench dec MB/s" columns in the normal-mode slack tables are from unpinned one-iteration benchmark CSVs and are not usable.
