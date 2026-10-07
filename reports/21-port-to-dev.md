# 21 Port of the kept ACE features to upstream dev

Decision rows 116-120, 122, 123. Data: `reports/port-dev.md` (port details), `runs/search_updev/{A,B,C}/ctl_n_g250_s1/points.json`, `runs/search_acedev/{A,B,C}/best_n_g250_s1/points.json`, `runs/search/{A,B,C}/best_n_g250_s1/points.json`, `runs/base/{A,B,C}/points.json`. Decode: frames in `runs/pb/Cacedev`, pbench re-runs `scratch/pbench_Cacedev_rerun{,2,3}.csv`, zli-benchmark files `runs/search/speed_Cdev.json` and `runs/search/speed_Cacedev.json`.

## 1. Question
Upstream OpenZL moved on from v0.2.3. Its `dev` branch has a new training code layout and a public brute-force graph. Two questions:
1. Do our kept features still give the same held-out points ratio when they are re-implemented on upstream dev?
2. How fast do the models decode, compared with upstream v0.2.3 and upstream dev?

## 2. Change
- Branch `ar/ace-dev-port` in `wt/acedev`, on upstream `origin/dev` 5545627c. 5 commits, cb63d10d .. d49e1eb8. Binary `bins/zli_acedev_v1`. Not pushed.
- Ported: ACE_SEED, ACE_MAX_GENERATIONS, ACE_CENSUS, ACE_SIZE_ONLY, ACE_NO_CTIME, ACE_TOK_SEEDS, ACE_BRUTE_K / TOL / MIN_GAIN / K1_OUT, ACE_BRUTE_SLACK, ACE_BRUTE_LAMBDA, ACE_SPECIALISTS / ACE_SPEC_GENERATIONS, and the CBOR decode depth change.
- Not ported: ACE_FAMILY, ACE_ISLANDS, ACE_POLISH, ACE_POOL, ACE_DEC_EPS, ACE_BRUTE_BALANCE, ACE_BRUTE_CROSS_SLOT, ACE_BRUTE_DEC_CAP. All of these were rejected or superseded in earlier reports.
- Main differences from `ar/ace` (see `reports/port-dev.md`):
  - The port uses upstream's public, serializable `zl.brute_force` graph (#1011), not our private one.
  - Selector params (slack, lambda, rates) are attached to the `zl.ace` wrapper.
  - Our brute replacement step is now a `BackendGraphMutation` subclass (`ACEBruteMutation`) in `ace.cpp`. Upstream removed `ace_combination.cpp`.
  - The use-after-free fix is not needed: in upstream's new `ACETrainer::train`, the inputs live as long as the explorer.
- Smoke test (`reports/port-dev.md`): frames from the port decode bit-exact with both stock decoders (v0.2.3 and dev), 20/20 chunks for every tested model.

Run labels:
- `ctl_n` on dev: plain upstream dev training (`bins/zli_updev_5545627`), no ACE_* env vars, normal mode.
- ported `best_n`: the `best` recipe (glossary) with `bins/zli_acedev_v1`.
- `ar/ace` `best_n`: the existing `runs/search/<split>/best_n_g250_s1` models (report 16). The decision log calls them "v17 best_n".
- v0.2.3: `runs/base` (upstream v0.2.3, 900 s per ACE graph).

## 3. Method
- Points only, normal mode, 250 gens, seed 1, splits A/B/C. One run per split and label.
- Decode speed metric: **pbench** (`bins/pbench_dev`). Source: `ideas/round3_scratch/pbench.cpp`, linked to the dev decoder library.
  - Frames are made once with `pb_frames.py`. Each model compresses every C test chunk with its own `zli`. Frames are in `runs/pb/Cacedev/<model>/`. All 4 lists have the same 563 chunks in the same order.
  - For each chunk, pbench decodes every model's frame back to back, 7 iterations. The model order flips every iteration. One process, pinned to core 13.
  - It reports, per model, the sum over chunks of the minimum time and of the median time. MB/s = raw source bytes / summed time.
  - All 4 frame sets are decoded with the same (dev) decoder. So pbench compares models, not decoders.
- The old metric, `speed_ab.py` (`zli benchmark` per file, pinned core, interleaved rounds), was also run twice on C (section 4.3).
- Decode bar: 0.97x of the v0.2.3 model's decode speed.

## 4. Results

### 4.1 Held-out points ratio

| split | v0.2.3 (`base`) | dev `ctl_n` | dev vs v0.2.3 | `ar/ace` best_n | ported best_n | port vs `ar/ace` | port vs dev | port vs v0.2.3 | port: worst / best file vs `ar/ace` | train s (dev / port / `ar/ace`) |
|---|---|---|---|---|---|---|---|---|---|---|
| A | 3.8535 | 3.9234 | +1.81% | 4.0548 | 4.0633 | +0.21% | +3.57% | +5.44% | +0.01% (2568500_755800) / +0.41% | 2586 / 3681 / 2029 |
| B | 3.8798 | 3.9086 | +0.74% | 4.0554 | 4.0593 | +0.10% | +3.86% | +4.63% | -0.20% (chunk_0_5) / +0.33% | 3212 / 4061 / 2195 |
| C | 3.7605 | 3.8008 | +1.07% | 4.0237 | 4.0313 | +0.19% | +6.07% | +7.20% | -2.52% (dqKZWZa1QwBW) / +1.40% | 2531 / 3209 / 1750 |

- The port matches or beats `ar/ace` `best_n` on all 3 splits: +0.10% .. +0.21%.
- The port beats upstream dev by +3.57% (A), +3.86% (B), +6.07% (C). Every test file gains over dev (worst +0.71% on B, +0.73% on C, +1.86% on A).
- Upstream dev beats v0.2.3 by +0.74% .. +1.81%.
- On C, single files move both ways vs `ar/ace`: dqKZWZa1QwBW -2.52%, 2568500 +1.40%. Normal mode is not reproducible (report 06), so this may be seed noise.
- Training is slower on dev: port 3209-4061 s vs `ar/ace` 1750-2195 s. The two ran at different times on a shared machine.

### 4.2 Decode speed, pbench (C test, 563 chunks)
The first run (decision row 123) did not save its output. It gave: v0.2.3 model 535.7 MB/s (1.000x), dev model 0.743x, `ar/ace` best_n 0.933x, ported best_n 0.904x.

For this report I re-ran it 3 times, same frames, same core, nice 19. The machine was busier (load 290-410), so the absolute MB/s are lower. The range over the 3 re-runs:

| model | ratio (frames) | dec MB/s, sum of per-chunk min | vs v0.2.3 (min) | vs v0.2.3 (median) |
|---|---|---|---|---|
| v0.2.3 model | 3.7606 | 305.5-318.3 | 1.000-1.000x | 1.000-1.000x |
| dev model (ctl_n) | 3.8008 | 217.0-231.2 | 0.710-0.726x | 0.697-0.719x |
| v17 best_n | 4.0238 | 281.4-296.0 | 0.921-0.931x | 0.917-0.926x |
| ported best_n | 4.0314 | 271.3-285.6 | 0.888-0.897x | 0.884-0.898x |

- Ratios from the frame sizes match `points.json` (C: 3.7605 / 3.8008 / 4.0237 / 4.0313) within rounding.
- The order is the same in all 4 runs: v0.2.3 > `ar/ace` best_n > ported best_n > dev.
- `ar/ace` best_n is at 0.92-0.93x. The port is at 0.89-0.90x. Both fail the 0.97x bar.
- The upstream dev model decodes at 0.71-0.74x. Its +1.07% ratio on C costs over 25% decode speed. Decision row 118 says dev's C model uses `zl.transformer_numeric`.
- The port is about 3% slower to decode than `ar/ace` best_n. This report does not explain why.

### 4.3 Decode speed, zli benchmark (`speed_ab.py`)

| model | `speed_Cdev.json` dec MB/s | vs v0.2.3 | `speed_Cacedev.json` dec MB/s | vs v0.2.3 |
|---|---|---|---|---|
| v0.2.3 model, v0.2.3 zli | 76.4 | 1.000x | 68.6 | 1.000x |
| v0.2.3 model, dev zli | 78.6 | 1.029x | - | - |
| dev model, dev zli | 62.6 | 0.819x | 49.8 | 0.725x |
| v17 best_n | 76.3 | 0.998x | 58.6 | 0.854x |
| ported best_n, acedev zli | - | - | 57.8 | 0.842x |
| ported best_n, dev zli | - | - | 59.0 | 0.860x |

- The two runs are about an hour apart. `ar/ace` best_n went from 0.998x to 0.854x, and the dev model from 0.819x to 0.725x. Row 123 says other rounds gave 1.00 / 0.82 / 0.85 / 0.84 for best_n.
- With the same frames (the v0.2.3 model), the dev decoder is 1.029x as fast as the v0.2.3 decoder. So the dev decoder itself is not slower.
- `speed_ab.py` times whole files, one model after another. Machine load changes between them, and the numbers do not repeat. pbench interleaves models per chunk, so load changes hit all models alike.

**pbench is now the decode metric.** The pinned `zli benchmark` (`speed_ab.py`) decode numbers in earlier reports (11-14, 16, 18, 20) are unreliable. The unpinned bench MB/s are rougher still. Their ranking may still hold, but differences of a few percent are not trustworthy. Those reports have not been re-measured.

## 5. Verdict
- The port keeps the ratio: +0.10..+0.21% vs `ar/ace` best_n, and +3.6% / +3.9% / +6.1% vs upstream dev on A / B / C.
- Decode fails the 0.97x bar. The port is at 0.89-0.90x of the v0.2.3 model on C (pbench); `ar/ace` best_n is at 0.92-0.93x.
- Next: `ACE_BRUTE_LAMBDA` on the port (`bestl3k_n` on A/B/C, `bestl5k_n` on C, `search_acedev_lam.log`). These runs are **still running**. When this report was written, none had a result file.

## 6. Caveats
- One seed per split. Normal mode is not reproducible (report 06). The `best_n` seed spread on A is 0.29% (report 16), so port vs `ar/ace` (+0.1..+0.2%) is within noise.
- pbench covers the C test files only. A and B decode speeds were not measured.
- pbench decodes all frames with the dev decoder library. Speeds with the v0.2.3 decoder library may differ a little (zli benchmark says the dev decoder is about 3% faster).
- The 535.7 MB/s first pbench run has no saved output. The re-runs here agree with its ratios within 0.01-0.03x, but not with its absolute MB/s.
- The `ar/ace` best_n models in `runs/search/{A,B,C}` were written at 04:11-04:15, before `bins/zli_ace_v17` existed (13:18). They come from an earlier `ar/ace` binary (report 16 lists `zli_ace_v14`). The decision log calls them "v17 best_n". `best_n` does not set ACE_BRUTE_LAMBDA.
- Encode speed was not measured with pbench. zli benchmark shows the ACE models encode far slower than upstream (1.8-3.3 MB/s vs 12.2-23.0 MB/s for the upstream models), but those numbers have the same reliability problem.
