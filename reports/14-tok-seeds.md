# 14 ACE_TOK_SEEDS, alone and combined with ACE_SPECIALISTS

Decision rows 72, 76, 78. Data: `runs/search/<split>/{tok,b4tol}_g120_s*` (size-only), `runs/search/<split>/{spectok,spec,final,b4tol,spectokL,spectokNC}_n_g250_s*` (normal mode), `runs/search/speed_A1max.json`, `runs/search/speed_A1nc.json`.

## 1. Question
The searches that end badly on points are the ones that never find tokenize-first chains (an idea from the `ideas-opus` subagent, "P1", `ideas/opus.md`). If every search starts with a few such chains in its population, does the points ratio go up, and does seed variance go down? Does it add anything on top of ACE_SPECIALISTS? Which final recipe is best?

## 2. Change
Commit 2f66300 on `ar/ace`, binary `bins/zli_ace_v12` (later runs: `bins/zli_ace_v13`).

- `ACE_TOK_SEEDS=1`: add 7 prebuilt numeric genes to the starting population of every numeric ACE graph. They are the tokenize-first chains that won the big points graphs of the strongest cross-corpus model (`runs/isl/B/brute4_250_s1`). Examples: `tokenize_numeric > {delta > range_pack > transpose > entropy | delta > zigzag > transpose > entropy}`, `range_pack > tokenize_numeric > ...`, `delta > zigzag > tokenize_numeric > ...`, `divide_by > delta > zigzag > tokenize_numeric > {delta > quantize_offsets | transpose > fse}`. (Row 72 says 6 genes; the code adds 2 for each of 2 sort settings plus 3 more = 7.)

Run labels (`runs/search/<split>/<label>_g<gens>_s<seed>`):
- `tok`: `ACE_TOK_SEEDS=1 ACE_BRUTE_K=4 ACE_BRUTE_TOL=0.02`.
- `b4tol`: control, `ACE_BRUTE_K=4 ACE_BRUTE_TOL=0.02`.
- `spec`: `ACE_SPECIALISTS=1 ACE_SPEC_GENERATIONS=60 ACE_BRUTE_K=4 ACE_BRUTE_TOL=0.5` (report 11).
- `spectok`: `spec` + `ACE_TOK_SEEDS=1`.
- `final`: `spectok` + `ACE_POOL=3` (pool searches at the full 250 gens each).
- `spectokL`: `spectok` with `ACE_SPEC_GENERATIONS=150`.
- `spectokNC`: `spectok` + `ACE_NO_CTIME=1`.
- `_n` = normal mode; no suffix = size-only. `g120`, `g250` = generations; `s<N>` = `ACE_SEED`.

## 3. Method
- Points only.
- `tok` vs `b4tol`: size-only, same seed (identical search apart from the seeds), 120 gens, seeds 1-3 x A/B/C.
- `spectok`, `spec`, `final` vs `b4tol` control: normal mode, 250 gens, seeds 1-3 x A/B/C (seed 3 was added in the final batch, row 76).
- `spectokL`, `spectokNC` vs `spectok`: normal mode, seeds 1-2 x A/B/C (row 78).
- Pinned decode speed: `speed_ab.py` on split A seed 1, 5 files incl. the hard files 533084 and 474000 (`speed_A1max`, `speed_A1nc`; method in report 13).
- "bench dec MB/s" in the tables is from the unpinned one-iteration benchmark CSVs.

## 4. Results
`tok` vs `b4tol`, size-only, points held-out:

| split | seed | tok_g120 | b4tol_g120 | gain | worst file (gain) | train s (var / ctl) | bench dec MB/s (var / ctl) |
|---|---|---|---|---|---|---|---|
| A | 1 | 3.9204 | 3.8070 | +2.98% | zw7rJF499NRr_3 (-0.1%) | 1181 / 1948 | 206 / 224 |
| A | 2 | 3.9122 | 3.9115 | +0.02% | 3828740_103001 (-0.7%) | 1252 / 1723 | 236 / 255 |
| A | 3 | 3.9200 | 3.7304 | +5.08% | 264500_3290500 (-0.9%) | 1385 / 1667 | 237 / 342 |
| B | 1 | 3.9036 | 3.8946 | +0.23% | 3090551_447834 (-2.4%) | 1798 / 1742 | 180 / 328 |
| B | 2 | 3.9036 | 3.8482 | +1.44% | 2519000_735700 (-2.6%) | 1889 / 1690 | 220 / 309 |
| B | 3 | 3.8994 | 3.8682 | +0.81% | 752132_1153215 (-0.3%) | 1720 / 1775 | 216 / 324 |
| C | 1 | 3.8962 | 3.6688 | +6.20% | 2519000_735700 (-1.5%) | 1326 / 1407 | 232 / 345 |
| C | 2 | 3.8362 | 3.8255 | +0.28% | 533084_6793800 (-0.8%) | 1322 / 1408 | 207 / 270 |
| C | 3 | 3.8885 | 3.7349 | +4.11% | 2568500_755800 (-2.2%) | 1242 / 1520 | 209 / 236 |

Mean gain +2.35% (min +0.02%, max +6.20%), 9 of 9 cells > 0.

Hard files over the 9 cells: 3828740 up to +13.2%, 474000 +1.2..+16.4%, 533084 up to +18.8%.

Seed spread (size-only, 120 gens): the bad-seed searches were the ones without tokenize-first chains.

| split | tok_g120 seeds 1/2/3 | spread | b4tol_g120 seeds 1/2/3 | spread |
|---|---|---|---|---|
| A | 3.9204 / 3.9122 / 3.9200 | 0.21% | 3.8070 / 3.9115 / 3.7304 | 4.85% |
| B | 3.9036 / 3.9036 / 3.8994 | 0.11% | 3.8946 / 3.8482 / 3.8682 | 1.20% |
| C | 3.8962 / 3.8362 / 3.8885 | 1.56% | 3.6688 / 3.8255 / 3.7349 | 4.27% |

`spectok` vs `b4tol` control, normal mode:

| split | seed | spectok_n_g250 | b4tol_n_g250 | gain | worst file (gain) | train s (var / ctl) | bench dec MB/s (var / ctl) |
|---|---|---|---|---|---|---|---|
| A | 1 | 4.0421 | 3.8481 | +5.04% | 140 (+0.2%) | 2431 / 1391 | 241 / 372 |
| A | 2 | 4.0581 | 3.8653 | +4.99% | 3828740_103001 (-0.6%) | 2336 / 1389 | 319 / 357 |
| A | 3 | 4.0475 | 3.8455 | +5.25% | 3828740_103001 (+0.5%) | 2311 / 1294 | 394 / 520 |
| B | 1 | 4.0414 | 3.8991 | +3.65% | chunk_0_5 (+0.5%) | 2601 / 1832 | 308 / 320 |
| B | 2 | 4.0386 | 3.9002 | +3.55% | chunk_0_5 (+0.5%) | 2489 / 1735 | 434 / 266 |
| B | 3 | 4.0377 | 3.8838 | +3.96% | chunk_0_5 (+0.7%) | 2589 / 1475 | 438 / 408 |
| C | 1 | 3.9477 | 3.8128 | +3.54% | 3828740_103001 (+0.3%) | 2239 / 1432 | 269 / 312 |
| C | 2 | 3.9550 | 3.8108 | +3.78% | 140 (+0.4%) | 2108 / 1435 | 196 / 252 |
| C | 3 | 3.9514 | 3.7755 | +4.66% | 220721_191602 (+0.4%) | 2142 / 1225 | 327 / 472 |

Mean gain +4.27% (min +3.54%, max +5.25%), 9 of 9 cells > 0.

`spectok` vs `spec`, normal mode (what TOK_SEEDS adds on top of SPECIALISTS):

| split | seed | spectok_n_g250 | spec_n_g250 | gain | worst file (gain) |
|---|---|---|---|---|---|
| A | 1 | 4.0421 | 4.0574 | -0.38% | 220721_191602 (-0.7%) |
| A | 2 | 4.0581 | 4.0543 | +0.10% | 220721_191602 (-0.2%) |
| A | 3 | 4.0475 | 4.0591 | -0.29% | 2568500_755800 (-1.3%) |
| B | 1 | 4.0414 | 4.0357 | +0.14% | Z3_6190800_206 (+0.0%) |
| B | 2 | 4.0386 | 4.0392 | -0.01% | 2519000_735700 (-0.1%) |
| B | 3 | 4.0377 | 4.0413 | -0.09% | 533084_6793800 (-0.8%) |
| C | 1 | 3.9477 | 3.9653 | -0.45% | dqKZWZa1QwBW (-2.0%) |
| C | 2 | 3.9550 | 3.8626 | +2.39% | EX_20230607_12 (-0.1%) |
| C | 3 | 3.9514 | 3.8600 | +2.37% | 220721_191602 (-0.1%) |

Mean gain +0.42% (min -0.45%, max +2.39%), 4 of 9 cells > 0.

`final` (spectok + ACE_POOL=3) vs control, and vs `spectok`:

| split | seed | final_n_g250 | b4tol_n_g250 | gain | worst file (gain) | train s (var / ctl) | bench dec MB/s (var / ctl) |
|---|---|---|---|---|---|---|---|
| A | 1 | 4.0354 | 3.8481 | +4.87% | 2568500_755800 (-0.1%) | 4920 / 1391 | 350 / 372 |
| A | 2 | 4.0501 | 3.8653 | +4.78% | 3828740_103001 (-0.1%) | 5152 / 1389 | 318 / 357 |
| A | 3 | 4.0523 | 3.8455 | +5.38% | 3828740_103001 (+0.5%) | 5091 / 1294 | 308 / 520 |
| B | 1 | 4.0378 | 3.8991 | +3.56% | chunk_0_5 (+0.8%) | 5549 / 1832 | 445 / 320 |
| B | 2 | 4.0438 | 3.9002 | +3.68% | chunk_0_5 (+0.5%) | 5494 / 1735 | 473 / 266 |
| B | 3 | 4.0420 | 3.8838 | +4.07% | chunk_0_5 (+0.3%) | 5591 / 1475 | 455 / 408 |
| C | 1 | 3.9554 | 3.8128 | +3.74% | 3828740_103001 (+0.5%) | 4596 / 1432 | 331 / 312 |
| C | 2 | 3.9338 | 3.8108 | +3.23% | 2024-ballarat_ (-0.3%) | 4684 / 1435 | 329 / 252 |
| C | 3 | 3.9516 | 3.7755 | +4.66% | 220721_191602 (+0.1%) | 4531 / 1225 | 302 / 472 |

Mean gain +4.22% (min +3.23%, max +5.38%), 9 of 9 cells > 0.

| split | seed | final_n_g250 | spectok_n_g250 | gain | worst file (gain) |
|---|---|---|---|---|---|
| A | 1 | 4.0354 | 4.0421 | -0.17% | 2568500_755800 (-0.7%) |
| A | 2 | 4.0501 | 4.0581 | -0.20% | 2568500_755800 (-0.6%) |
| A | 3 | 4.0523 | 4.0475 | +0.12% | zw7rJF499NRr_3 (-0.1%) |
| B | 1 | 4.0378 | 4.0414 | -0.09% | 2568500_755800 (-0.7%) |
| B | 2 | 4.0438 | 4.0386 | +0.13% | 140 (-0.2%) |
| B | 3 | 4.0420 | 4.0377 | +0.11% | chunk_0_5 (-0.4%) |
| C | 1 | 3.9554 | 3.9477 | +0.20% | zMuOt68XGoj6 (-0.3%) |
| C | 2 | 3.9338 | 3.9550 | -0.54% | dqKZWZa1QwBW (-3.6%) |
| C | 3 | 3.9516 | 3.9514 | +0.00% | 2568500_755800 (-0.5%) |

Mean gain -0.05% (min -0.54%, max +0.20%), 5 of 9 cells > 0.

`spectokL` (150 specialist gens) vs `spectok`:

| split | seed | spectokL_n_g250 | spectok_n_g250 | gain | worst file (gain) | train s (var / ctl) | bench dec MB/s (var / ctl) |
|---|---|---|---|---|---|---|---|
| A | 1 | 4.0388 | 4.0421 | -0.08% | 2568500_755800 (-0.6%) | 2501 / 2431 | 362 / 241 |
| A | 2 | 4.0465 | 4.0581 | -0.29% | 2568500_755800 (-0.6%) | 2471 / 2336 | 314 / 319 |
| B | 1 | 4.0407 | 4.0414 | -0.02% | 2519000_735700 (-0.2%) | 2604 / 2601 | 413 / 308 |
| B | 2 | 4.0384 | 4.0386 | -0.00% | Z3_6190800_206 (-0.1%) | 2623 / 2489 | 445 / 434 |
| C | 1 | 3.9567 | 3.9477 | +0.23% | 140 (-0.8%) | 2433 / 2239 | 313 / 269 |
| C | 2 | 3.9516 | 3.9550 | -0.09% | dqKZWZa1QwBW (-3.5%) | 2379 / 2108 | 330 / 196 |

Mean gain -0.04% (min -0.29%, max +0.23%), 1 of 6 cells > 0.

`spectokNC` (+ ACE_NO_CTIME) vs `spectok`:

| split | seed | spectokNC_n_g250 | spectok_n_g250 | gain | worst file (gain) | train s (var / ctl) | bench dec MB/s (var / ctl) |
|---|---|---|---|---|---|---|---|
| A | 1 | 4.0420 | 4.0421 | -0.00% | 3828740_103001 (-0.5%) | 1869 / 2431 | 282 / 241 |
| A | 2 | 4.0522 | 4.0581 | -0.15% | 2568500_755800 (-0.6%) | 2009 / 2336 | 278 / 319 |
| B | 1 | 4.0436 | 4.0414 | +0.06% | 2568500_755800 (-0.1%) | 2032 / 2601 | 264 / 308 |
| B | 2 | 4.0411 | 4.0386 | +0.06% | 2568500_755800 (-0.5%) | 2013 / 2489 | 241 / 434 |
| C | 1 | 3.9722 | 3.9477 | +0.62% | zMuOt68XGoj6 (-0.4%) | 1647 / 2239 | 441 / 269 |
| C | 2 | 3.9404 | 3.9550 | -0.37% | dqKZWZa1QwBW (-3.7%) | 1715 / 2108 | 426 / 196 |

Mean gain +0.04% (min -0.37%, max +0.62%), 3 of 6 cells > 0.

Against the upstream baseline (`runs/base`, report 01), mean over seeds:

| split | upstream base (runs/base, seed 0, 900 s) | b4tol_n mean | spec_n mean | spectok_n mean | spectokNC_n mean (seeds 1-2) | final_n mean |
|---|---|---|---|---|---|---|
| A | 3.8535 | 3.8529 (-0.01%) | 4.0569 (+5.28%) | 4.0493 (+5.08%) | 4.0471 (+5.02%) | 4.0460 (+5.00%) |
| B | 3.8798 | 3.8944 (+0.38%) | 4.0387 (+4.10%) | 4.0392 (+4.11%) | 4.0423 (+4.19%) | 4.0412 (+4.16%) |
| C | 3.7605 | 3.7997 (+1.04%) | 3.8960 (+3.60%) | 3.9514 (+5.07%) | 3.9563 (+5.21%) | 3.9469 (+4.96%) |

Pinned decode, split A seed 1, 5 files (from `speed_A1max` and `speed_A1nc`, see report 13 for the full table): `spectok` 0.99x and 0.95x of the `b4tol_n` control; `spectokNC` 0.96x; `spectokL` 0.93x. Encode 0.47-0.50x.

## 5. Verdict
- `ACE_TOK_SEEDS`: kept for points. Alone it gives +2.35% (size-only, 9 of 9 cells >= 0) and removes most of the seed spread. On top of SPECIALISTS it adds +0.42% mean, mostly by fixing the weak C seeds 2 and 3 (+2.4%); the minimum over cells goes from +1.36% (`spec`) to +3.54% (`spectok`). It is points-only: on point_source_id it hurts (report 15).
- `ACE_POOL` on top (`final`): rejected, -0.05% vs `spectok` at about 2x the train time.
- `ACE_SPEC_GENERATIONS=150` (`spectokL`): rejected, -0.04%; 60 gens is enough.
- `ACE_NO_CTIME` (`spectokNC`): kept (row 79). Ratio +0.04% and pinned decode +1.8% vs `spectok`, both within noise, and it drops an objective we do not care about.
- Final recipe for points: `ACE_TOK_SEEDS=1 ACE_SPECIALISTS=1 ACE_SPEC_GENERATIONS=60 ACE_BRUTE_K=4 ACE_BRUTE_TOL=0.5 ACE_NO_CTIME=1`, `bins/zli_ace_v13` (`ar/ace` a65d5a4). `spectok` is +4.27% vs the brute control and +4.1..+5.1% vs upstream v0.2.3, with seeds within 0.4% of each other per split. Decode 0.95-0.99x of the brute control.

## 6. Caveats
- Row 72 gives `spectok` vs `spec` as +0.30% and `spectok` vs control as +4.09% mean; those were the first 6 cells (seeds 1-2). With seed 3 the result files give +0.42% and +4.27%.
- The full recipe with `ACE_NO_CTIME` (`spectokNC`) has only seeds 1-2. The 3-seed numbers above are for `spectok` without `ACE_NO_CTIME`.
- The upstream comparison uses the `runs/base` matrix: seed 0, 900 s wall clock per graph, not fixed generations, and only one run per split.
- Normal mode is not reproducible; differences under about 0.5% between normal-mode variants are noise.
- Decode was measured only for split A seed 1.
