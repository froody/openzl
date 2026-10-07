# 05 ACE_BRUTE_K: brute-force selector over complementary candidates

Decision rows 41-46, 48. Data: `runs/isl/{A,B,C}/brute4_250_s*` (unpaired), `runs/paired/{A,B,C}/k4_g120_s*` (paired).

## 1. Question
Different ACE candidates win on different chunks (the per-chunk minimum of two seed models beats both, report 04). If the trained model keeps a few complementary candidates per ACE graph and picks the smallest one per chunk at compression time, how much held-out ratio do we gain, and what does it cost in speed?

## 2. Change
- Commit cff58c2 on `ar/ace`, binary `bins/zli_ace_v5`. `ACE_BRUTE_K=K`: after training, for each ACE graph, take the final front and greedily pick up to K candidates. Each pick is the one that most reduces the training total of "best size so far per input". Stop when the next pick saves less than 0.2% of training bytes. The picks go under the stock brute-force selector (`SI_selector_brute_force`), registered as the private graph `!zl.private.brute_force`. At compression time each stream is compressed with every pick and the smallest is kept. The output format is unchanged: the stock upstream decoder reads it (round trip checked).
- Commit 3a01d05, binary `bins/zli_ace_v6`. `ACE_BRUTE_K1_OUT=<path>`: the same training run also writes the plain model (the single smallest candidate, what upstream would save) to `<path>`. This gives a paired control from one search.

Run labels:
- `brute4_250` (`runs/isl/<split>/brute4_250_s<seed>`): `ACE_BRUTE_K=4`, 250 gens, normal mode. Compared with `base_250` from report 04 (a different search run).
- `k4_g120_s<seed>` (`runs/paired/<split>/`): `ACE_BRUTE_K=4`, `ACE_MAX_GENERATIONS=120`, `ACE_SEED=<seed>`. `k1/` holds the benchmark of the plain model, `brute/` the benchmark of the K=4 model, both from the same search (`paired.py`).

## 3. Method
- Points only, normal mode (3 objectives).
- Unpaired: `brute4_250` vs `base_250`, seeds 1 and 2, splits A/B/C.
- Paired: one search per cell writes both models. 120 gens, seeds 1-3, splits A/B/C, 9 cells. The only difference between the two models is the selection.
- Speed: from the benchmark CSVs (`zli benchmark -n 1`, not pinned, shared machine). "Compress time brute/k1" = sum of ctimeMs ratio; same for decompress.

## 4. Results
Paired test (the deciding test), points held-out:

| split | seed | plain (k1) | brute (K=4) | gain | worst test file | picks per ACE graph | enc MB/s k1 / brute | dec MB/s k1 / brute | compress time brute/k1 | decompress time brute/k1 |
|---|---|---|---|---|---|---|---|---|---|---|
| A | 1 | 3.8140 | 3.8613 | +1.24% | +0.00% | 3/1/2/1/1 | 56 / 40 | 214 / 228 | 1.40x | 0.94x |
| A | 2 | 3.8742 | 3.8985 | +0.63% | +0.00% | 1/1/2/1/2 | 17 / 46 | 66 / 259 | 0.36x | 0.26x |
| A | 3 | 3.8123 | 3.8399 | +0.72% | +0.06% | 2/1/2/1/2 | 68 / 31 | 233 / 215 | 2.23x | 1.09x |
| B | 1 | 3.8742 | 3.9321 | +1.49% | +0.30% | 2/1/3/1/3 | 79 / 37 | 330 / 346 | 2.14x | 0.95x |
| B | 2 | 3.8790 | 3.9664 | +2.25% | +0.03% | 2/1/3/1/2 | 53 / 34 | 282 / 321 | 1.55x | 0.88x |
| B | 3 | 3.8734 | 3.9324 | +1.52% | +0.57% | 2/1/3/1/3 | 61 / 36 | 204 / 319 | 1.68x | 0.64x |
| C | 1 | 3.7250 | 3.7633 | +1.03% | +0.00% | 2/1/2/1/1 | 70 / 50 | 209 / 225 | 1.41x | 0.93x |
| C | 2 | 3.7853 | 3.7948 | +0.25% | +0.00% | 1/1/2/1/1 | 69 / 49 | 248 / 230 | 1.41x | 1.08x |
| C | 3 | 3.7307 | 3.7378 | +0.19% | +0.00% | 1/1/2/1/1 | 69 / 55 | 221 / 214 | 1.25x | 1.03x |

Mean gain +1.04%. 9 of 9 cells positive. No test file is worse in any pair (worst +0.00%). "Picks per ACE graph" is in the order the graphs appear in the training log; 1 pick means the graph kept the plain candidate.

Unpaired test (first try, confounded):

| split | seed | brute4_250 | base_250 | gain (unpaired) |
|---|---|---|---|---|
| A | 1 | 3.8612 | 3.8386 | +0.59% |
| A | 2 | 3.9437 | 3.9088 | +0.89% |
| B | 1 | 4.0189 | 3.8609 | +4.09% |
| B | 2 | 3.9314 | 3.8803 | +1.32% |
| C | 1 | 3.7231 | 3.8001 | -2.03% |
| C | 2 | 3.8717 | 3.8008 | +1.86% |

## 5. Verdict
Kept. First reliable points gain: +1.04% mean over 9 paired cells, never worse on any file. Decompression cost is unchanged, because the decoder only runs the one graph that was picked. Compression runs every pick, so it is about 1.4-2.2x slower (one cell shows 0.36x, which is load noise). `ACE_BRUTE_K=4` is part of every later control (`b4tol`).

The unpaired test could not decide: split C seed 1 was -2.0% because that run's own search ended 2% worse on graph 5 (5.73 MB vs 5.62 MB train) with the same seed. Same-seed normal-mode runs are not reproducible (report 06). This is why the paired design was built.

## 6. Caveats
- Speed numbers are from one benchmark iteration on a loaded machine (load 150-300), not pinned. Compress ratios vary from 0.36x to 2.23x and decompress from 0.26x to 1.09x, mostly noise. Pinned decode measurements came later (report 13).
- The paired test uses 120 gens, not the production 250 gens.
- Row 42 lists brute compress speed as "base 94/52 MB/s, brute 62/53" on split A; those are the unpaired runs and match the CSVs (94/62 seed 1, 52/53 seed 2).
