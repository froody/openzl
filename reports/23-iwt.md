# 23 Integer wavelet transform (froody `zl.iwt_int`)

Data: `scratch/iwt/headroom_chunks.csv` (numpy, exact kernel, all columns, unsorted points), `scratch/iwt/headroom_points_orders.csv` (numpy, points in Hilbert / Morton / unsorted order, 25 transform variants), `scratch/iwt/q2_points_chunks.csv` and `q2_points_totals.csv` (real codec, points), `q2_side_totals.csv` (real codec, other columns), `points_hilbert_perfile.csv`. Probe: `scratch/iwt/probe/probe_min.cpp`. Binaries: `pz_froody` (linked to `wt/froody/build/libopenzl.a`, has iwt) and `pz_ace` (linked to `wt/ace/build/libopenzl.a`, has `zl.private.brute_force`, which the trained models need).

## 1. Question
froody/dev has `zl.iwt_int`: one level of an integer 5/3 lifting wavelet. It splits a numeric stream into approx ((n+1)/2) and detail (n/2) streams. It is in froody's ACE node list (`tools/training/ace/ace_compressors.cpp:81`).
1. Is there headroom vs delta > zigzag on points, points_u16, intensity and gps_time?
2. How does the real codec do vs `dz_te` and the trained model, in bytes and speed?
3. Is there a modified wavelet worth building (bar: at least 2% gain on points bytes in numpy)?
4. Should we register it in our ported build (`ar/ace-dev-port`)?

## 2. Change
No change to any repo or binary. Everything is in `scratch/iwt/`.
- numpy port of `ZS_iwtEncode{16,32,64}`. It matches the C kernel byte for byte (checked through a small C harness `kcheck.c` on real int32, u16 and i64 chunks plus random data, and the kernel round-trips).
- New probe `probe_min.cpp`. It has the same per-axis method as `codec_probe/probe.cpp`, but without the froody-only codecs, so it also builds against `wt/ace`. Each chunk is compressed and decompressed 3 times, and the min time is kept. Every chunk round-trips.
- Chains (per axis, numeric; iwt output 0 = approx, output 1 = detail):
  - `dz_te`: delta > zigzag > num_to_struct > transpose_split > entropy.
  - `iwt1`: iwt > (approx: `dz_te`; detail: zigzag > te).
  - `iwt2` / `iwt3`: iwt recursing on the approx, 2 / 3 levels; each detail goes to zigzag > te.
  - `d_iwt1`: delta > iwt > both outputs zigzag > te.
  - `ref`: the trained per-axis selectors of `runs/search/{Ch,Cm,C}/best_n_g250_s1/points.zlc`.
- Ref check: graph names are renumbered on load. So the axis-to-selector mapping was found by trying all 6 orders on 3 files and keeping the one that matches zli. Ref totals match `points.json` to within 0.013% (Hilbert 26,967,878 vs 26,971,279 B; Morton 28,146,529 vs 28,149,290 B; unsorted 26,187,496 vs 26,189,782 B). The small gap is frame overhead.

## 3. Method
- Test files: the 12 split C test LAZ. For points, the main data is the Hilbert order (`<tag>@hilbert`, split `Ch`), since production is sorted. Morton and unsorted orders are side checks.
- Headroom estimate (bytes): the transpose_split order-0 estimate (sum over byte planes of order-0 entropy, per chunk and stream) and plain order-0 on values. Gains are byte-weighted totals vs delta+zigzag.
- Real codec: the same chunks. Speeds are single-thread, on cores 0-3 at nice 19, in MB/s of source. They include CCtx setup per call, as in report 09. `dz_te` and the iwt chains were timed in the same `pz_froody` process. The ref and a second `dz_te` (`dz_te@ace`) were timed in the `pz_ace` process.
- Only the real `zl.iwt_int` was run through OpenZL. The other variants (signed-fixed 5/3, predict-only 5/3, Haar/S-transform, (4,2) "9/7-int", (4,4), 2/6 S+P, delta-of-delta, raw approx) were run in numpy only.

## 4. Results

### 4.1 Kernel bug: no sign extension for widths below 8 bytes
For 1, 2 and 4 byte inputs, the kernel reads values as unsigned (`(int64_t)(uint32_t)x`) and also reads the detail as unsigned in the update step. So any negative coordinate, or any negative detail, gives a huge wrong-sign "prediction". Example: int32 x = [5, -3, 7, -1, 4, 2] gives approx [-2147483647, -2147483645, -2147483646] (signed lifting gives [1, 3, 2]). Even for positive data, one negative detail adds about 2^30 to the approx. It is still lossless, because the arithmetic wraps the same way on decode, but the approx stream is garbage. The 8-byte path uses int64 and is correct. This bug explains most of the real codec's loss below. The "fixed" rows in 4.2 are what a signed 5/3 would give.

### 4.2 Headroom (numpy), size vs delta+zigzag, test files, byte-weighted

transpose_split order-0 estimate (plain order-0 in brackets). "exact" = the froody kernel as is; "fixed" = the same 5/3 lifting with signed reads.

| column | iwt1 exact | iwt2 exact | iwt3 exact | delta>iwt exact | iwt1 fixed | iwt3 fixed | delta>iwt fixed |
|---|---|---|---|---|---|---|---|
| points, Hilbert | - | - | - | - | +5.6% (+9.9%) | +5.9% (+8.4%) | +7.0% (+11.8%) |
| points, Morton | - | - | - | - | +6.0% (+10.7%) | +6.7% (+9.2%) | +7.6% (+12.8%) |
| points, unsorted | +25.2% (+17.2%) | +38.5% (+20.8%) | +44.9% (+18.8%) | +52.3% (+27.6%) | +5.7% (+9.7%) | +6.0% (+8.1%) | +7.1% (+11.5%) |
| points_u16 | +15.0% (+16.0%) | +26.1% (+21.9%) | +30.0% (+21.0%) | +29.8% (+27.0%) | +4.9% (+7.6%) | +5.5% (+7.0%) | +5.9% (+9.3%) |
| intensity | +7.6% (+12.5%) | +12.2% (+14.4%) | +13.8% (+13.9%) | +16.1% (+19.6%) | +2.7% (+8.5%) | +3.0% (+6.4%) | +3.0% (+9.4%) |
| gps_time (i64, no bug) | +26.1% (+13.6%) | +30.8% (+12.4%) | +30.8% (+10.4%) | +29.9% (+15.3%) | same | same | same |

Other liftings on points (numpy, signed, then delta+zigzag on the approx and zigzag on each detail; transpose_split estimate vs delta+zigzag):

| variant | Hilbert | Morton | unsorted |
|---|---|---|---|
| 5/3 predict only (no update), 1 level | +3.0% | +3.5% | +3.0% |
| Haar / S-transform, 1 level | +3.0% | +2.9% | +3.0% |
| 5/3 signed, 1 / 3 levels | +5.6% / +5.9% | +6.0% / +6.7% | +5.7% / +6.0% |
| (4,2) "9/7-int", 1 level | +6.9% | +7.3% | +7.0% |
| (4,4), 1 level | +7.0% | +7.4% | +7.1% |
| 2/6 (S+P), 1 level | +7.7% | +7.4% | +7.8% |
| delta of delta | +10.9% | +11.7% | +11.1% |
| 5/3 with raw approx (no delta on approx) | +27% | +26% | +27% |

- Not one variant beats delta+zigzag on any file in total. The best per file (`53nuL1` or `haar`) is still +1.3% (EX) to +9.6% (Z3) on Hilbert.
- Per-chunk oracle (chunk and axis pick the smallest of delta+zigzag and all 23 transforms): -0.04% (Hilbert), -0.05% (Morton), -0.05% (unsorted) points bytes. A transform wins on only 7-10% of chunk-axes. For the other columns, the oracle vs `dz` is points_u16 -0.05%, intensity -0.9%, gps_time -0.005%.
- One exception: on intensity, a signed 5/3 with 3 levels is smaller than delta+zigzag on 4 of 10 files (EX -4.2%, zMuOt68XGoj6 -4.0%, 3828740 -2.0%, 474000 -1.6%). It is much worse on 140 (+38%), so the total is +3.0%. The real (unsigned) kernel loses on intensity in total too (+7.7%). For these columns the per-chunk oracle used only the 5/3 variants (signed 1-3 levels and delta>iwt).
- Why: after a spatial sort, each axis is close to a random walk. The deltas are close to white. Their lag-1 autocorrelation has median -0.08 (10th-90th percentile -0.18..-0.04, Hilbert). For a random walk, delta is the right predictor. The 5/3 detail predicts from neighbours two steps apart, so its residual is larger than a delta. The approx is a stride-2 sequence, so its delta is larger too. Sorting does not make the axes smoother at the scale the wavelet uses: Hilbert is no better than unsorted.
- Across axes: the deltas of x, y and z are uncorrelated (median pair correlation -0.006..0.012, Hilbert). Predicting one axis delta from another makes it 13% worse (median). So an iwt or predictor across axes has nothing to exploit.

### 4.3 Real codec, points (12 test files, all chunks round-trip)

Hilbert (main):

| chain | bytes | ratio | vs dz_te | vs ref | enc MB/s | dec MB/s |
|---|---|---|---|---|---|---|
| ref (`Ch/best_n`) | 26,967,878 | 4.0479 | -11.1% | 0 | 11 | 396 |
| dz_te | 30,333,299 | 3.5988 | 0 | +12.5% | 387 | 669 |
| iwt1 | 38,027,308 | 2.8706 | +25.4% | +41.0% | 230 | 408 |
| iwt2 | 42,191,342 | 2.5873 | +39.1% | +56.5% | 171 | 334 |
| iwt3 | 44,419,460 | 2.4575 | +46.4% | +64.7% | 149 | 314 |
| d_iwt1 (delta > iwt) | 45,988,102 | 2.3737 | +51.6% | +70.5% | 165 | 308 |

- Ref and `dz_te@ace` were timed in the `pz_ace` process: `dz_te@ace` ran at 288 / 560 MB/s there, so the ref decode is 0.71x of `dz_te` in the same process.
- Morton: iwt1 +25.4%, iwt2 +39.4%, iwt3 +46.7%, d_iwt1 +51.4% vs dz_te; ref -11.6%.
- Unsorted: iwt1 +25.9%, iwt2 +40.0%, iwt3 +47.4%, d_iwt1 +52.7%; ref -9.5%.
- The real iwt1 (+25.4% Hilbert, +25.9% unsorted) agrees with the numpy estimate for the exact kernel (+25.2% unsorted). So the estimate is a good proxy here.
- Per file (Hilbert), iwt1 is worse on all 12 files: +18.7% (2519000) to +47.3% (Z3). Per axis it is x +24.0%, y +24.0%, z +28.5%.
- Decode, in the same process as dz_te: iwt1 is 0.61x, iwt2 0.50x, iwt3 0.47x the speed of dz_te. Encode is 0.59x for iwt1. The cost is the extra streams (2 to 4 entropy backends instead of 1) more than the lifting itself.

### 4.4 Real codec, other columns (iwt vs dz_te, 12 test files)

| column | iwt1 | iwt3 | dz_te dec MB/s | iwt1 dec MB/s |
|---|---|---|---|---|
| points_u16 (3x u16) | +15.4% | +30.9% | 398 | 295 |
| intensity (u16) | +7.7% | +14.2% | 411 | 268 |
| gps_time (i64) | +26.1% | +31.6% | 540 | 723 |

gps_time has no sign bug and still loses 26%, so the loss there is the transform itself, not the bug. iwt1 decodes faster than dz_te on gps_time (723 vs 540 MB/s). This is not explained here; the min-of-3 timings on small chunks are noisy.

### 4.5 Modifications (question 3)
- The bar was at least 2% gain on points bytes in numpy. No variant reaches 0%. The best is the 5/3 predict-only or Haar transform at +3.0% (Hilbert). The best per-chunk choice of predictor is -0.04%. Ideas that were checked:
  - multi-level: worse than 1 level for every lifting (4.2).
  - other liftings, 2/6, 9/7-int, 4/4: worse than 5/3.
  - per-chunk choice of predictor: -0.04%.
  - Morton / Hilbert order: same as unsorted, no help.
  - across axes: the deltas are uncorrelated.
  - fixing the boundary or the sign bug: the sign fix takes the loss from +25% to +5.6%, which is still a loss. The boundary handling only touches 1-2 samples per chunk.
- So nothing was implemented.

## 5. Verdict
- **Do not register `zl.iwt_int` in `ar/ace-dev-port`.** Not on any column. On every column it is +7.7% to +47% (per file) larger than plain delta>zigzag>transpose>entropy in the real codec (points Hilbert +25.4%, points_u16 +15.4%, intensity +7.7%, gps_time +26.1%). Even a correct signed version would be +3..+6% on points. Its per-chunk oracle gain is below 0.1% on points. Adding it to the ACE node list would only add search space that ACE has to learn to avoid.
- Decode cost if used: 0.61x the decode speed of dz_te for 1 level, 0.50x for 2 levels and 0.47x for 3 levels (points, same process). Encode is 0.59x for 1 level. On u16 columns decode is 0.65-0.74x; gps_time was not slower.
- If froody keeps it, the kernel needs a fix: sign-extend for 1, 2 and 4 byte widths (read `src` and `dstDetail` as the signed type). The current output is lossless but useless as a transform for signed or mixed-sign data.

## 6. Caveats
- The numpy estimates are ideal order-0 code lengths without table cost. For the exact kernel they matched the real codec within 1 point, but small differences (< 1%) between variants are not resolved by this estimate.
- Speeds are per chunk with CCtx setup on small chunks (~87 KB per axis), on a shared machine at nice 19, min of 3 reps. Use them as ratios within one process, not as absolute numbers. `pz_froody` and `pz_ace` are different libraries, so cross-process speed comparisons are rough.
- The ref is `best_n_g250_s1` per order (one seed), not a production model. It only serves as the "trained" yardstick; iwt loses to plain dz_te already, so the ref choice does not change the verdict.
- Points chunks of fewer than 8 points were skipped in numpy; the real codec ran on every chunk.
- Side columns ran on unsorted dumps only (only points has sorted dumps).
- `reports/README.md` was not updated (outside the allowed write paths).
