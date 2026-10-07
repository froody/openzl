# 17 Cross-slot brute picks (ACE_BRUTE_CROSS_SLOT) and x/y swap training augmentation

Decision rows 93, 97. Data: `ideas/round2.md`, `runs/search/{A,B,C}/spectokG8{,x}_g120_s<seed>/points.json`, `runs/search/{Asw,Bsw,Csw}/spectokG8_g120_s<seed>/points.json` (`search_X.log`).

## 1. Question
The `ideas-round2` subagent (`ideas/round2.md`) looked for where the points bytes still go, using the `spectokNC_n_g250_s1` models. Its main finding: the slack is in model choice per axis, not in the codecs. On C s1, the **y** graph costs +31..42% more than the same model's **x** graph on the hard airborne files 474000 and 533084. Two cheap fixes follow:
1. Let each slot's brute picks also use the other same-type slots' candidates (`ACE_BRUTE_CROSS_SLOT=1`).
2. Train on x/y-swapped copies of the training chunks, so the x and y searches see both horizontal axes (data only, no code).

Does either gain held-out points ratio over `spectokG8` (report 16)? Is either worse on any file?

## 2. Round-2 analysis summary (`ideas/round2.md`)
Sample: every 3rd full chunk of each A/B/C test file (294 chunks), models `spectokNC_n_g250_s1`. Numbers below are from that file; this report did not recompute them.
- Entropy leaves are near order-0: 1.008x of order-0 entropy after `transpose_split`, 0.8% of bytes in total.
- No gain from adaptive order-0 (0.998x), hi->lo byte context (+0.9% worse with real costs), bucket/raw-bits (+0.2% worse), lag-L context (+1..3% worse), LS4 predictor or switched delta/delta2 (no file better).
- The lattice structure (533084: x = 10q + r; 3828740: values = 5 mod 10) is already captured by tokenize (exact lattice-divide estimate 1.00x of actual).
- Small slots and header: 0.4-1.4% of bytes per chunk. Not worth a specialist pass.
- Per-axis model choice is the real slack. Chunk-level oracle vs own model:

| candidate set | A | B | C |
|---|---|---|---|
| own model, best of x/y slot graph per axis | +0.36% | +0.25% | +2.73% |
| own model, any of 3 slot graphs per axis | +0.50% | +0.44% | +2.92% |
| own + other 2 split generalists, any slot, per axis | +0.88% | +0.52% | +3.12% |
| own + train-file specialists, per axis + x/y share | +1.35% | +0.94% | +3.38% |
| best axis permutation of the input (whole chunk) | +0.14% | +0.20% | +0.40% |

- Other ideas ranked lower: a 4-chunk compression unit (hard files +0.8%, all files -0.2..-0.4%; needs a product decision), a tighter entropy leaf (ceiling about 0.1-0.5%), a per-axis library from other splits' models.

## 3. Change
- `ACE_BRUTE_CROSS_SLOT=1`: commit 9a7dff2 on `ar/ace`, with use-after-free fix 8391fa6 (the explorers now outlive their loop iteration, so their training inputs are kept alive in `aceInputsOwned`). Binary `bins/zli_ace_v15`. In `getCombinedCompressors` (`tools/training/ace/ace_combination.cpp`), brute picks now run after all slot searches. For each slot, the front candidates of every other slot with the same input type are added to its candidate list before `pickComplementary`. They count against the K cap like any other pick. Each candidate is scored on the slot's own training inputs. The output is still a stock frame. With the flag off, output equals v14 bit for bit (smoke test, row 93).
- x/y swap augmentation (no code): for each training LAZ, a copy of its points chunks with columns [1,0,2] (x and y swapped, z kept) is written to `chunks/points/<laz>_swxy`. `split_Asw.json`, `split_Bsw.json`, `split_Csw.json` = the train list of A/B/C plus the `_swxy` copies (16 entries), same 12 test files. Training bytes double (A 68.0 -> 136.1 MB, B 70.7 -> 141.5 MB, C 72.5 -> 145.0 MB).

Run labels (`runs/search/<split>/<label>_g120_s<seed>/points.json`):
- `spectokG8`: control. `spectok` with `ACE_BRUTE_K=8 ACE_BRUTE_MIN_GAIN=0.0001` (report 16).
- `spectokG8x`: `spectokG8` + `ACE_BRUTE_CROSS_SLOT=1` (`bins/zli_ace_v15`).
- `spectokG8` on split `Asw`/`Bsw`/`Csw`: same flags, trained on the swap-augmented split.

## 4. Method
- Points only, size-only mode (`ACE_SIZE_ONLY=1`), 120 gens, seeds 1-3 x A/B/C, 9 cells per variant.
- `spectokG8x` vs `spectokG8`: same seed, same search; only the brute candidate list differs (paired).
- Swap vs `spectokG8`: same seed, but the training data differ, so the search differs too. This is not a paired selection test.
- Test sets are the same 12 files as A/B/C. Swapped copies are never in the test set.
- "picks" = brute picks per ACE graph (5 graphs) from `points.zlc.log`. "bench dec MB/s" = unpinned one-iteration benchmark CSVs. No pinned speed run yet.

## 5. Results

### 5.1 Held-out points ratio (9 cells each)

| split | seed | spectokG8 | spectokG8x | cross gain | worst file cross | spectokG8 on *sw | swap gain | worst file swap | picks (G8 / G8x / sw) | train s (G8 / G8x / sw) | bench dec MB/s (G8 / G8x / sw) |
|---|---|---|---|---|---|---|---|---|---|---|---|
| A | 1 | 4.0615 | 4.0633 | +0.04% | 140 (+0.00%) | 4.0703 | +0.22% | 3828740_103001 (-0.10%) | 7/1/4/1/7 / 7/1/4/1/7 / 8/1/4/1/8 | 1540 / 1830 / 3553 | 377 / 347 / 255 |
| A | 2 | 4.0623 | 4.0632 | +0.02% | 140 (+0.00%) | 4.0726 | +0.26% | zw7rJF499NRr_3 (-0.01%) | 5/1/5/1/7 / 6/1/5/1/7 / 7/1/5/1/7 | 1575 / 1878 / 3270 | 373 / 359 / 411 |
| A | 3 | 4.0732 | 4.0753 | +0.05% | 3828740_103001 (-0.00%) | 4.0744 | +0.03% | zw7rJF499NRr_3 (-0.00%) | 7/1/4/1/8 / 7/1/4/1/7 / 8/1/4/1/8 | 1641 / 1971 / 3493 | 406 / 344 / 416 |
| B | 1 | 4.0602 | 4.0601 | -0.00% | 752132_1153215 (-0.02%) | 4.0634 | +0.08% | chunk_0_5 (-0.08%) | 8/1/7/1/8 / 8/1/7/1/8 / 8/1/7/1/8 | 1975 / 2260 / 3953 | 364 / 344 / 337 |
| B | 2 | 4.0442 | 4.0442 | +0.00% | 140 (+0.00%) | 4.0464 | +0.05% | Z3_6190800_206 (-0.10%) | 7/1/7/1/8 / 7/1/7/1/8 / 8/1/7/1/8 | 1973 / 2324 / 4023 | 396 / 337 / 373 |
| B | 3 | 4.0609 | 4.0609 | +0.00% | 533084_6793800 (-0.01%) | 4.0630 | +0.05% | 3090551_447834 (+0.00%) | 8/1/7/1/8 / 8/1/7/1/8 / 7/1/7/1/7 | 1827 / 2196 / 3532 | 414 / 353 / 420 |
| C | 1 | 4.0197 | 4.0859 | +1.65% | 2568500_755800 (+0.01%) | 4.0924 | +1.81% | 2568500_755800 (+0.00%) | 6/1/5/1/6 / 6/1/5/1/7 / 8/1/5/1/8 | 1590 / 1872 / 3660 | 360 / 358 / 253 |
| C | 2 | 4.0222 | 4.0800 | +1.44% | 2568500_755800 (+0.01%) | 4.0932 | +1.77% | EX_20230607_12 (-0.01%) | 5/1/5/1/6 / 5/1/6/1/7 / 8/1/5/1/8 | 1579 / 1901 / 3428 | 377 / 330 / 363 |
| C | 3 | 4.0121 | 4.0839 | +1.79% | 220721_191602 (+0.03%) | 4.0918 | +1.99% | EX_20230607_12 (+0.06%) | 4/1/7/1/6 / 5/1/8/1/7 / 8/1/7/1/8 | 1478 / 1834 / 3255 | 399 / 381 / 397 |

| variant vs spectokG8 | mean | min | max | > 0 | mean per split A / B / C | worst file over all cells |
|---|---|---|---|---|---|---|
| `spectokG8x` (ACE_BRUTE_CROSS_SLOT=1) | +0.55% | -0.00% | +1.79% | 7/9 | +0.04% / -0.00% / +1.62% | -0.02% |
| `spectokG8` on `*sw` (x/y swap) | +0.69% | +0.03% | +1.99% | 9/9 | +0.17% / +0.06% / +1.85% | -0.10% |

- Swap gain >= cross gain in 8 of 9 cells (the exception is A s3: +0.03% vs +0.05%).
- On B, cross-slot changes nothing: B s2 gives the same ratio as the control to all digits, B s1 -0.003%, B s3 +0.00001%. The B y graph already uses good candidates.

### 5.2 Per-file detail, split C (seeds 1 / 2 / 3)
Byte share = the file's share of the control's compressed bytes (mean over the 3 seeds).

| test file | byte share | spectokG8 ratio | cross gain | swap gain |
|---|---|---|---|---|
| 140 | 8.9% | 4.1495 / 4.1453 / 4.1423 | +0.10% / +0.01% / +0.08% | +0.11% / +0.21% / +0.29% |
| 2024-ballarat_a-LiDAR_2439500_2459500 | 6.4% | 3.4853 / 3.4851 / 3.4827 | +0.03% / +0.01% / +0.05% | +0.05% / +0.11% / +0.20% |
| 220721_191602 | 9.3% | 3.5289 / 3.5138 / 3.5132 | +0.02% / +0.04% / +0.03% | +0.02% / +0.44% / +0.53% |
| 2519000_7357000 | 10.9% | 3.0455 / 3.0397 / 3.0381 | +0.01% / +0.02% / +0.05% | +0.02% / +0.21% / +0.25% |
| 2568500_7558000-CuG4mpzj0rop | 9.3% | 3.4886 / 3.4695 / 3.4670 | +0.01% / +0.01% / +0.05% | +0.00% / +0.57% / +0.61% |
| 3828740_10300196-dcPTQ5dVqIkfK | 9.1% | 4.0286 / 4.0418 / 4.0305 | +0.33% / +0.08% / +0.25% | +0.29% / +0.18% / +0.44% |
| 474000_6632000-dcpb7qGtnTTLP-trees.json- | 10.1% | 4.0198 / 4.0663 / 4.0114 | +6.11% / +4.69% / +6.36% | +6.41% / +4.99% / +6.40% |
| 533084_6793800-dcmGP6Y9h97vy | 8.1% | 4.3369 / 4.3553 / 4.3305 | +6.45% / +6.06% / +7.27% | +7.33% / +6.98% / +7.35% |
| EX_20230607_124634__230131_112528_L00_WG | 6.1% | 5.3109 / 5.3163 / 5.3117 | +0.08% / +0.02% / +0.07% | +0.09% / -0.01% / +0.06% |
| Z3_6190800_2067600 | 6.2% | 5.4220 / 5.4221 / 5.4143 | +3.61% / +3.62% / +3.92% | +3.94% / +3.98% / +3.94% |
| dqKZWZa1QwBW | 8.3% | 4.0485 / 4.0474 / 4.0487 | +3.00% / +2.99% / +3.51% | +3.56% / +3.62% / +3.58% |
| zMuOt68XGoj6 | 7.3% | 4.4041 / 4.4022 / 4.3994 | +0.45% / +0.39% / +0.45% | +0.52% / +0.60% / +0.58% |

On C, 4 files carry the gain: 474000, 533084, Z3_6190800 and dqKZWZa1QwBW. They give 95-97% of the bytes saved by cross-slot (371,142-461,768 bytes per cell) and 87-95% of the bytes saved by swap (454,749-511,550 bytes). This is the C y-graph failure found in round 2. Swap also gains +0.2..+0.6% on the other C files in seeds 2 and 3; cross-slot gains under +0.1% there.

### 5.3 Both together
`spectokG8x` on the `*sw` splits (`runs/search/{A,B,C}sw/spectokG8x_g120_s*`, `search_XS.log`) is **still running**. When this report was written, no cell had a result file. The runs were in the last ACE graph (graph 5 of 5).

## 6. Verdict
- Both changes fix C's y graph. Swap is the stronger: +0.69% mean, 9/9 > 0 (A +0.17%, B +0.06%, C +1.85%). Cross-slot: +0.55% mean, 7/9 > 0 (A +0.04%, B 0.00%, C +1.62%).
- No file is worse by more than 0.10% with either change.
- Decision (row 97): **open**. Next steps are both together (`search_XS.log`), then normal-mode confirmation and pinned decode.

## 7. Caveats
- Size-only, 120 gens. Not yet confirmed in normal mode at 250 gens.
- The swap test is not paired: its gain includes search noise. Size-only runs are reproducible for the same data and seed, but the data differ here.
- Swap doubles the training bytes and about doubles training time (1478-1975 s -> 3255-4023 s). Cross-slot adds about 15-20% (1830-2324 s).
- No pinned decode measurement yet. Bench dec MB/s is unpinned and noisy (for example, C s1 swap 253 vs control 360 MB/s, while C s2 and s3 are close).
- Cross-slot gene portability depends on all 3 big points slots having the same input type (Int32, width 4). The code checks `inputType()` only.
- The round-2 oracle numbers are from a sample of chunks and the `spectokNC_n` models. They were not recomputed here.
