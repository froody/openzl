# 11 ACE_SPECIALISTS: per-training-file searches merged into the brute selection

Decision rows 65, 68. Data: `runs/search/<split>/{spec,spec8,b4tol}_n_g250_s<seed>`, `runs/search/SPEC_RESULTS.md`, `runs/search/speed_{A1,B1,C2,A1max}.json`.

## 1. Question
The offline ceiling (report 10) says a choice among file-specific graphs is worth +2.8..+4.8% on points. Can ACE build this itself during training, and how much of the ceiling does it reach? What does it cost in compress and decompress speed?

## 2. Change
Commit 39a0875 on `ar/ace`, binary `bins/zli_ace_v10` (later runs: `bins/zli_ace_v13`, which contains the same code).

- `ACE_SPECIALISTS=1`: for each ACE graph, after the main search, run one extra search per training file, on that file's streams only. File identity comes from the input file name (`<laz>__<chunk>.raw` from `bench.py link_dir`), carried through sample collection as the `MultiInput` group. Files with fewer than 4 inputs are skipped. The 10 smallest genes of each file search are added to the population. The brute selection (`ACE_BRUTE_K`, with a wide `ACE_BRUTE_TOL`) can then put file-specialist graphs under the brute-force selector next to the generalist.
- `ACE_SPEC_GENERATIONS=N`: generations per file search (default 60).

Run labels (`runs/search/<split>/<label>_g250_s<seed>`):
- `spec`: `ACE_SPECIALISTS=1 ACE_SPEC_GENERATIONS=60 ACE_BRUTE_K=4 ACE_BRUTE_TOL=0.5`.
- `spec8`: the same with `ACE_BRUTE_K=8`.
- `b4tol`: control, `ACE_BRUTE_K=4 ACE_BRUTE_TOL=0.02` (report 08).
- `_n` = normal mode. `g250` = 250 gens in the main search. `s<N>` = `ACE_SEED`.

## 3. Method
- Points only, normal mode, 250 gens main search, 60 gens per file search (8 files x 5 ACE graphs = 40 extra searches per model).
- `spec_n` vs `b4tol_n` control: seeds 1 and 2 x A/B/C in the first batch (run by the `run-specialists` subagent, row 68); seed 3 added with the final batch (row 76). `spec8_n`: seed 1 x A/B/C.
- Compute: row 65 estimated about 2.9x the control. Measured wall-clock train time is 1.12-1.70x (mean 1.41x), because each file search runs on about 1/8 of the data.
- Pinned speed with `speed_ab.py` (see report 08 for the method): 5 test files, pinned core, 5 interleaved rounds, median. `speed_A1` / `speed_B1` / `speed_C2` use seed 1, 1, 2 models. `speed_A1max` adds the hard files 533084 and 474000.
- "bench dec MB/s" in the tables is from the one-iteration benchmark CSVs (not pinned).

## 4. Results
`spec_n` vs `b4tol_n` control, points held-out:

| split | seed | spec_n_g250 | b4tol_n_g250 | gain | worst file (gain) | train s (var / ctl) | bench dec MB/s (var / ctl) |
|---|---|---|---|---|---|---|---|
| A | 1 | 4.0574 | 3.8481 | +5.44% | z5GMxol0oJkH (+0.8%) | 1886 / 1391 | 210 / 372 |
| A | 2 | 4.0543 | 3.8653 | +4.89% | 3828740_103001 (-0.5%) | 1888 / 1389 | 238 / 357 |
| A | 3 | 4.0591 | 3.8455 | +5.56% | z5GMxol0oJkH (+0.7%) | 2197 / 1294 | 326 / 520 |
| B | 1 | 4.0357 | 3.8991 | +3.50% | chunk_0_5 (+0.2%) | 2053 / 1832 | 267 / 320 |
| B | 2 | 4.0392 | 3.9002 | +3.56% | chunk_0_5 (+0.6%) | 2192 / 1735 | 245 / 266 |
| B | 3 | 4.0413 | 3.8838 | +4.06% | chunk_0_5 (+0.3%) | 2484 / 1475 | 430 / 408 |
| C | 1 | 3.9653 | 3.8128 | +4.00% | 3828740_103001 (+0.3%) | 1849 / 1432 | 280 / 312 |
| C | 2 | 3.8626 | 3.8108 | +1.36% | Z3_6190800_206 (-0.1%) | 1872 / 1435 | 199 / 252 |
| C | 3 | 3.8600 | 3.7755 | +2.24% | 2519000_735700 (+0.4%) | 1943 / 1225 | 282 / 472 |

Mean gain +3.84% (min +1.36%, max +5.56%), 9 of 9 cells > 0.

`spec8_n` (K=8) vs control:

| split | seed | spec8_n_g250 | b4tol_n_g250 | gain | worst file (gain) | train s (var / ctl) | bench dec MB/s (var / ctl) |
|---|---|---|---|---|---|---|---|
| A | 1 | 4.0049 | 3.8481 | +4.08% | zw7rJF499NRr_3 (+0.4%) | 2086 / 1391 | 239 / 372 |
| B | 1 | 4.0354 | 3.8991 | +3.49% | chunk_0_5 (+0.5%) | 2322 / 1832 | 376 / 320 |
| C | 1 | 3.9506 | 3.8128 | +3.62% | EX_20230607_12 (-0.0%) | 1842 / 1432 | 263 / 312 |

Mean gain +3.73% (min +3.49%, max +4.08%), 3 of 3 cells > 0.

Per-file gains on the hard files over the 9 `spec_n` cells: 533084 up to +26.9%, 474000 up to +23.9%, dqKZ +4.3..+9.7%, Z3 up to +8.0%. Worst single file over all cells: -0.46% (3828740, A seed 2).

Pinned speed (`ctl` = `b4tol_n_g250`, `spec` = `spec_n_g250`):

| file | cell | ratio ctl | ratio spec | dec MB/s ctl | dec MB/s spec | dec spec/ctl | per-file dec spec/ctl | enc MB/s ctl | enc MB/s spec | enc spec/ctl |
|---|---|---|---|---|---|---|---|---|---|---|
| speed_A1.json | A s1 | 4.1677 | 4.2141 | 86.5 | 81.5 | 0.94x | 0.78x-1.06x | 12.4 | 7.1 | 0.57x |
| speed_B1.json | B s1 | 3.6715 | 3.7893 | 87.8 | 82.4 | 0.94x | 0.81x-1.06x | 6.8 | 5.9 | 0.86x |
| speed_C2.json | C s2 | 3.8377 | 3.8833 | 81.1 | 78.7 | 0.97x | 0.91x-1.04x | 12.0 | 9.4 | 0.78x |
| speed_A1max.json | A s1 (incl. 533084, 474000) | 3.7833 | 4.1854 | 62.5 | 58.8 | 0.94x | 0.82x-1.07x | 9.1 | 5.0 | 0.56x |

## 5. Verdict
Kept. Biggest gain so far: +3.84% mean over 9 cells (min +1.36%), 9 of 9 positive. It reaches the offline per-file ceiling of report 10 (+2.8..+4.5%). K=8 is not better than K=4: on the same seed-1 cells `spec8_n` gives +3.73% mean and `spec_n` +4.31%.

Cost: train time 1.1-1.7x. Compress speed 0.56-0.86x (pinned; brute tries more picks: 10-12 picks over the 5 graphs vs 6-8 for the control, SPEC_RESULTS.md). Decode speed 0.94-0.97x pinned, 0.78-1.07x per file. This led to the decode tie-break test (report 13) and to ACE_TOK_SEEDS on top (report 14).

## 6. Caveats
- Row 68 reports decode 0.57-0.92x of control; that came from the unpinned benchmark CSVs under load ~150 (SPEC_RESULTS.md section 2). The pinned measurement (row 69) gives 0.94-0.97x and is the one to use.
- Row 68 gives the mean over 6 cells as +3.79%; with seed 3 added the mean over 9 cells is +3.84%.
- Normal mode is not reproducible, so each cell carries single-run noise (C seed 2 +1.36% and C seed 3 +2.24% vs C seed 1 +4.00%).
- Pinned speed runs shared the machine with other trainings (load ~110-200). Compare inside one file only.
