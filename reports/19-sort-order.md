# 19 Sort order of points (Morton / Hilbert)

Decision rows 100, 101, 102. Data: `dump_sorted.py`, `split_{A,B,C}{m,h}.json`, `runs/search/{A,B,C}{m,h}/{b4tol_n,best_n}_g250_s1/points.json` (`search_sort.log`), unsorted baselines `runs/search/{A,B,C}/{b4tol_n,best_n}_g250_s1/points.json`.

## 1. Question
The user asked two things:
1. Would Morton- or Hilbert-sorted points compress better? The idea: after a spatial sort, neighbours are close, so the entropy moves to the low bits.
2. Does the points graph still transpose, and does it still use a separate compressor per axis?

## 2. Background
- Our dumps were **not** sorted. `pointcloud_sampler` has `--sort-by-morton` and `--sort-by-hilbert`, and both default to false. We used the defaults.
- So points in a chunk are in the sampler's own order, roughly scan order. The share of Morton-monotone neighbour pairs is 0.50, the same as random order (row 100).
- Median |dx| between neighbours is 9-261 units. The axis range is 4k-32k units (row 100).

### How the points graph works (question 2)
- The SDDL description splits each `Point` into three streams: x, y and z.
- `zl.cluster` sends them to 5 ACE slots: 3 big ones (x, y, z) and 2 tiny ones.
- Each slot has its own brute-force selector with up to 8 candidates (`ACE_BRUTE_K=8`).
- Nearly every candidate ends in `transpose_split` -> fse / entropy.
- So yes: the graph still transposes, and each axis has its own compressors.

## 3. Change
No codec code. Data only.
- `dump_sorted.py <mode>` re-runs `pointcloud-sampler crunch` on the 20 corpus LAZ with `--sort-by-morton true` or `--sort-by-hilbert true`. All other flags are the same as the original dumps.
- Dump tags: `<tag>@morton` and `<tag>@hilbert`.
- Check: the `@morton` chunks are exactly sorted by the Morton code of the absolute u32 coords. The original dumps are not (row 101).
- New splits: `Am/Bm/Cm` (Morton) and `Ah/Bh/Ch` (Hilbert). Same LAZ files in train and test as A/B/C.
- The sampler sorts inside the octree build. So the node files differ, not only the point order. The test bytes differ too (points source bytes: A 109.8 MB unsorted, 114.7 MB Morton, 112.2 MB Hilbert; B 107.2 / 110.7 / 112.4 MB; C 105.4 / 112.8 / 109.2 MB).
- Binary `bins/zli_ace_v15`. Run labels `b4tol_n` and `best_n` (glossary).

## 4. Method
- Points only, normal mode, 250 gens, seed 1. One run per split, order and label.
- Each model is trained and tested on the same order. A first check that tested Morton-sorted chunks under an unsorted-trained model gave +2.8% larger output (row 100). That test is unfair, so it is not used here.
- Order-0 estimate (scratch script, row 102): on 4 hard files, zigzag delta coded per byte plane, order-0 entropy. Two streams compared:
  - `axis_delta`: delta of x, y, z separately.
  - `key_delta`: delta of the Morton key as one stream.
- No decode speed was measured. The sorted models were not kept as candidates.

## 5. Results

### 5.1 Held-out points ratio (seed 1)

| split | unsorted b4tol_n | Morton b4tol_n | Hilbert b4tol_n | unsorted best_n | Morton best_n | Hilbert best_n |
|---|---|---|---|---|---|---|
| A | 3.8481 | 3.6202 (-5.92%) | 3.7575 (-2.35%) | 4.0548 | 3.8622 (-4.75%) | 4.0094 (-1.12%) |
| B | 3.8991 | 3.7883 (-2.84%) | 3.8798 (-0.50%) | 4.0554 | 3.9784 (-1.90%) | 4.0540 (-0.03%) |
| C | 3.8128 | 3.5981 (-5.63%) | 3.7706 (-1.11%) | 4.0237 | 4.0054 (-0.45%) | 4.0473 (+0.59%) |

- Morton is worse in all 6 cells (-0.45% .. -5.92%).
- Hilbert is close to unsorted with `best_n` (-1.12% / -0.03% / +0.59%). It is worse with `b4tol_n` in all 3 cells.
- Per test file the `best_n` ratios move a lot both ways (-11.4% .. +8.6%). The node files differ, so the per-file chunks are not the same data. Only the totals are compared.
- Training time `best_n`: unsorted 1750-2195 s, sorted 2286-2527 s.

### 5.2 Order-0 estimate, 4 hard files
Bytes are estimated compressed bytes. `ratio` = raw / estimate. `orig` = unsorted dump. File names are cut to their first 10 characters.

| file | order | raw bytes | axis_delta bytes | key_delta bytes | ratio axis | ratio key |
|---|---|---|---|---|---|---|
| 2568500_75 | orig | 1,834,980 | 528,642 | 561,019 | 3.47 | 3.27 |
| 2568500_75 | @morton | 1,987,836 | 604,757 | 639,665 | 3.29 | 3.11 |
| 474000_663 | orig | 1,921,740 | 591,184 | 666,573 | 3.25 | 2.88 |
| 474000_663 | @morton | 1,396,236 | 468,036 | 488,324 | 2.98 | 2.86 |
| 533084_679 | orig | 1,749,072 | 516,469 | 576,550 | 3.39 | 3.03 |
| 533084_679 | @morton | 2,097,120 | 677,476 | 730,018 | 3.10 | 2.87 |
| zMuOt68XGo | orig | 1,151,208 | 252,644 | 288,779 | 4.56 | 3.99 |
| zMuOt68XGo | @morton | 1,028,760 | 237,584 | 268,175 | 4.33 | 3.84 |

- Per-axis delta beats Morton-key delta on all 4 files, for both orders.
- Per-axis delta on unsorted data beats per-axis delta on Morton data on all 4 files (3.25-4.56 vs 2.98-4.33).

## 6. Verdict
- Keep the writer default: **no sort**.
- Hilbert is about equal to unsorted. Morton is worse. A likely reason: Morton order jumps at octant boundaries, which gives big deltas on every axis.
- A single Morton-key stream in SDDL is not worth building. Its delta costs more than the three per-axis deltas.
- The points graph still splits x/y/z, gives each axis its own brute-force selector, and transposes before the entropy stage.

## 7. Caveats
- One seed, normal mode. Normal mode is not reproducible (report 06). The `best_n` seed spread on A is 0.29% (report 16). So Hilbert vs unsorted on B and C (-0.03%, +0.59%) is within or near noise. The Morton losses on A and B are above noise.
- The sorted dumps have different node files and test bytes. This is a test of "sampler with sort" vs "sampler without sort", not of point order alone.
- The order-0 estimate covers 4 files only, and only Morton (no Hilbert). It ignores tokenize and the other ACE stages.
- The search was not tuned for sorted data (same seeds and recipe as unsorted).
