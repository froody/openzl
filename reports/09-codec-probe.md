# 09 Codec probe: froody codecs and a bit-length bucket estimate

Decision rows 59, 60. Data: `codec_probe/RESULTS.md`, `codec_probe/BUCKET_ESTIMATE.md`, `codec_probe/per_laz.csv`, `codec_probe/bucket_estimate_per_laz.csv`, `codec_probe/raw/`.

## 1. Question
Is the points ratio limited by the codecs? Two checks:
1. Do the froody/dev codecs (pcodec_int, linear_predict, morton_3d, spherical_range), placed by hand on the real points ACE streams, beat the trained model?
2. Would a new codec that codes the bit length of each delta (bucket) with entropy coding and stores the low bits raw beat the trained model? This is checked with an ideal code-length estimate before writing any codec.

## 2. Change
No change to ACE or to any OpenZL worktree. Both checks were done by the `codec-probe` subagent.

- `codec_probe/probe.cpp`, linked against `wt/froody/build/libopenzl.a`. It compresses every points chunk with each hand chain, decompresses it, and checks the bytes match. The ACE inputs for points are the per-axis int32 x/y/z columns of a chunk (checked with `zli --trace-streams-dir`), so chains run on those columns.
- Reference graphs `refA`, `refB`, `refC`: the trained ACE graph per axis, from `runs/paired/<split>/k4_g120_s1/points.k1.zlc` (the plain model of report 05). They reproduce `zli compress` sizes to the byte.
- Bucket estimate: inline numpy. Residual = range_pack > divide_by > delta > zigzag (the common core of the ref graphs). Estimators E1-E4:
  - E1: order-0 entropy of the bit length b, plus b-1 raw bits, plus 1 byte per used symbol.
  - E2: entropy of b given the previous value's b.
  - E3: E2 plus the second-highest bit coded with an adaptive binary model.
  - E4: E2 with the other axis's bucket as context.

Chain names (from RESULTS.md): `dz_te` = delta_int > zigzag > num_to_struct > transpose_split > entropy (stock). `dz_pcM` = delta > zigzag > pcodec_int(M) > transpose > entropy. `lpMsS_te` = linear_predict(mode M, stride S) > zigzag > transpose > entropy. `dz_flz` = delta > zigzag > field_lz.

## 3. Method
- All 20 LAZ files, 913 points chunks, 27 chains, all round-trip checked (2739 chunk-streams for the estimate).
- Totals over each split's 12 test LAZ files, as bytes relative to that split's ref.
- "Per-chunk oracle" = for each chunk and axis, take the smallest of {ref, stock chains, new codecs}. This is an upper bound for a selector.
- Speed: single thread, all 913 chunks, 12 probes in parallel (noisy).
- Implementation bar for the bucket codec (set before the estimate): E2 or E3 at least 2% smaller than the ref on 2 of 3 splits.

## 4. Results
Hand chains vs the trained ref, xyz bytes over test files (lower is better; ratio vs that split's ref, recomputed from `per_laz.csv`):

| chain | A | B | C |
|---|---|---|---|
| ref of the split | 1.000 | 1.000 | 1.000 |
| dz_te (stock) | 1.027 | 1.040 | 1.023 |
| dz_pc4 (best new chain) | 1.014 | 1.040 | 1.017 |
| lp3s1_te | 1.029 | 1.042 | 1.025 |
| dz_flz | 1.212 | 1.231 | 1.191 |
| refB (model trained on split B) | 0.977 | 1.000 | 0.958 |

- No single hand chain beats the trained ref.
- pcodec_int is 0-1.3% smaller than the same stock chain, at about 2.5x encode time and 1.8x decode time (dz_pc4 145 / 346 MB/s vs dz_te 372 / 621 MB/s).
- linear_predict does not help (+0.1% to +13% vs dz_te).
- morton_3d: decode fails on 99% of chunks in froody/dev. Encoded size is +12..15% vs ref anyway.
- spherical_range: not lossless (20 of 913 chunks fail the checksum), and 2.2x the size where it does round-trip.
- Per-chunk oracle with the new codecs as extra candidates: ref+stock 0.993 / 0.987 / 0.994 -> ref+stock+new 0.981 / 0.985 / 0.981 (A/B/C). At most 1-1.9%.
- Side finding: the model trained on split B (refB) is 2.3% (A) and 4.2% (C) smaller than those splits' own refs, and 10-11% smaller on the hard airborne files. The train-split variance is bigger than any codec gain.

Bucket estimate vs the trained ref, xyz over test files (recomputed from `bucket_estimate_per_laz.csv`):

| estimate | A | B | C |
|---|---|---|---|
| E1 | 1.009 | 1.048 | 1.026 |
| E2 | 1.012 | 1.051 | 1.029 |
| E3 | 1.006 | 1.045 | 1.023 |
| E4 | 1.008 | 1.047 | 1.024 |
| dz_te (actual bytes) | 1.027 | 1.040 | 1.023 |
| per-chunk oracle min(ref, E3) | 0.977 | 0.982 | 0.981 |

- E3 is about equal to the stock dz_te chain (0.979 / 1.004 / 1.000 of dz_te). The low bits are close to uniform.
- It wins clearly on one file only (3828740: -13%, mostly on y). It loses +28% on 533084, where the ref uses tokenize to exploit repeated values.

## 5. Verdict
- froody codecs: dropped. The ceiling is small (at most 1-1.9% as free per-chunk candidates), the cost is high (pcodec 2.5x encode, 1.8x decode), and two codecs are broken.
- Bucket + raw-bits codec: not implemented. It fails the bar on all 3 splits, and an ideal estimate is a lower bound for a real codec.
- Conclusion for points: the deltas are already near their entropy. The remaining gain is in choosing the right graph per chunk or per file (search diversity and selection), not in a new entropy stage. This led to reports 10 and 11.

## 6. Caveats
- The refs are the 120-gen plain models from `runs/paired` seed 1, not production models.
- Speed numbers were measured with 12 probes in parallel and include graph setup per call on small chunks (~87 KB per axis), so they are rough.
- pcodec_int's multiplier was set by hand (M in {1, 2, 4, 8, 16, 32, 64, 256}), because its registration function is declared but not implemented in froody/dev.
- The per-chunk oracles assume free and perfect selection.
