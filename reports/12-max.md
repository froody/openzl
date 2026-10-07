# 12 max: ACE_POOL=3 x 250 gens + ACE_BRUTE_K=8 + ACE_NO_CTIME

Decision rows 67, 73. Data: `runs/search/<split>/max_n_g250_s<seed>`, `runs/search/MAX_RESULTS.md`, `runs/search/speed_A1max.json`.

## 1. Question
The user said training time does not matter, compress speed is low priority, and decode speed is second after ratio (row 67). If we spend much more training compute (3 full 250-gen searches), let the selector keep more picks (K=8), and drop the compression-time objective, how much ratio do we gain, and does decode get faster?

## 2. Change
Commit fd9723b on `ar/ace`, binary `bins/zli_ace_v11`.

- `ACE_NO_CTIME=1`: set the compression-time objective to 0. The front then trades size against decompression time only.

Run label (`runs/search/<split>/max_n_g250_s<seed>`):
- `max`: `ACE_POOL=3 ACE_BRUTE_K=8 ACE_BRUTE_TOL=0.05 ACE_NO_CTIME=1`, 250 gens per pool search (3 x 250 gens).
- Controls: `b4tol_n_g250` (`ACE_BRUTE_K=4 ACE_BRUTE_TOL=0.02`, 1 x 250 gens) and `pool3b4tol_n_g83` (`ACE_POOL=3`, 3 x 83 gens; report 08).
- `_n` = normal mode. `s<N>` = `ACE_SEED`.

## 3. Method
- Points only, normal mode, seeds 1-3 x splits A/B/C, 9 cells. Run by the `run-max` subagent (MAX_RESULTS.md).
- Compute: `max` uses 3x the generations of either control.
- Decode speed: the `run-max` subagent first measured 3 files unpinned (`zli benchmark -n 5`, load ~200; MAX_RESULTS.md section 2). Then pinned with `speed_ab.py` on A seed 1, 5 files including the hard files 533084 and 474000 (`speed_A1max.json`, load ~165; method in report 08).

## 4. Results
`max` vs `b4tol_n` control, points held-out:

| split | seed | max_n_g250 | b4tol_n_g250 | gain | worst file (gain) | train s (var / ctl) | bench dec MB/s (var / ctl) |
|---|---|---|---|---|---|---|---|
| A | 1 | 3.8639 | 3.8481 | +0.41% | 220721_191602 (-1.4%) | 5042 / 1391 | 308 / 372 |
| A | 2 | 3.8941 | 3.8653 | +0.74% | 220721_191602 (-0.2%) | 4845 / 1389 | 446 / 357 |
| A | 3 | 3.8889 | 3.8455 | +1.13% | 3828740_103001 (-0.3%) | 4917 / 1294 | 444 / 520 |
| B | 1 | 3.9191 | 3.8991 | +0.51% | 533084_6793800 (-0.0%) | 5784 / 1832 | 244 / 320 |
| B | 2 | 3.9165 | 3.9002 | +0.42% | 533084_6793800 (-0.0%) | 5298 / 1735 | 288 / 266 |
| B | 3 | 3.8965 | 3.8838 | +0.33% | 533084_6793800 (-6.1%) | 4979 / 1475 | 357 / 408 |
| C | 1 | 3.8296 | 3.8128 | +0.44% | 2519000_735700 (-0.1%) | 4877 / 1432 | 453 / 312 |
| C | 2 | 3.8150 | 3.8108 | +0.11% | dqKZWZa1QwBW (-3.7%) | 4730 / 1435 | 360 / 252 |
| C | 3 | 3.8042 | 3.7755 | +0.76% | 2568500_755800 (-1.4%) | 4848 / 1225 | 426 / 472 |

Mean gain +0.54% (min +0.11%, max +1.13%), 9 of 9 cells > 0.

`max` vs `pool3b4tol_n_g83` (same pool, 3x fewer gens, K=4, tol 0.02):

| split | seed | max_n_g250 | pool3b4tol_n_g83 | gain | worst file (gain) |
|---|---|---|---|---|---|
| A | 1 | 3.8639 | 3.9270 | -1.61% | 533084_6793800 (-6.8%) |
| A | 2 | 3.8941 | 3.9470 | -1.34% | 533084_6793800 (-6.1%) |
| A | 3 | 3.8889 | 3.8896 | -0.02% | 3828740_103001 (-0.8%) |
| B | 1 | 3.9191 | 3.8989 | +0.52% | dqKZWZa1QwBW (+0.0%) |
| B | 2 | 3.9165 | 3.9037 | +0.33% | 533084_6793800 (-0.0%) |
| B | 3 | 3.8965 | 3.8989 | -0.06% | 533084_6793800 (-6.1%) |
| C | 1 | 3.8296 | 3.8140 | +0.41% | 3828740_103001 (-0.2%) |
| C | 2 | 3.8150 | 3.8182 | -0.08% | 2519000_735700 (-0.3%) |
| C | 3 | 3.8042 | 3.7902 | +0.37% | dqKZWZa1QwBW (-0.7%) |

Mean gain -0.17% (min -1.61%, max +0.52%), 4 of 9 cells > 0.

Pinned speed, split A seed 1, 5 files (`speed_A1max.json`; `ctl` = `b4tol_n_g250`, `pool` = `pool3b4tol_n_g83`, `spec` = `spec_n_g250`, `spectok` = `spectok_n_g250`):

| model | ratio (5 files) | dec MB/s | dec vs ctl | enc MB/s | enc vs ctl | dec 533084_679 | dec 474000_663 | dec znHYtqPtsN | dec 140 | dec 2568500_75 |
|---|---|---|---|---|---|---|---|---|---|---|
| ctl | 3.7833 | 62.5 | 1.00x | 9.1 | 1.00x | 69.4 | 58.0 | 58.3 | 60.2 | 70.2 |
| pool | 3.9722 | 64.2 | 1.03x | 6.3 | 0.69x | 63.5 | 53.6 | 61.2 | 72.1 | 79.3 |
| max | 3.8493 | 68.7 | 1.10x | 4.2 | 0.47x | 66.3 | 61.3 | 72.9 | 69.4 | 77.9 |
| spec | 4.1854 | 58.8 | 0.94x | 5.0 | 0.56x | 58.7 | 62.2 | 58.9 | 56.4 | 57.6 |
| spectok | 4.1708 | 62.0 | 0.99x | 4.5 | 0.49x | 66.4 | 65.4 | 61.1 | 54.5 | 64.3 |

Train time: 3.05-3.96x the control (mean 3.46x).

## 5. Verdict
Rejected for ratio. 3x more search plus K=8 gives +0.54% over the control, but it is -0.17% vs `pool3b4tol_n_g83`, which uses one third of the compute (-0.99% mean on split A, where the hard files favour pool). More training compute in this form adds nothing.

Decode: pinned, `max` decodes 1.10x faster than the control (68.7 vs 62.5 MB/s). That is real but small, and likely comes from `ACE_NO_CTIME`. It was tested on top of the final recipe (report 14: `spectokNC`).

## 6. Caveats
- MAX_RESULTS.md reports decode 1.23x control (63.9 vs 52.0 MB/s) from an unpinned run under load ~200 on 3 files. The pinned `speed_A1max.json` gives 1.10x; use the pinned number.
- The decision log row 73 gives encode 0.49x; the pinned file gives 0.47x (4.2 vs 9.1 MB/s). MAX_RESULTS.md gives 0.49x from the unpinned run.
- Speed was measured on one cell (A seed 1) and 5 files.
- Normal mode is not reproducible (report 06); each cell is one run.
