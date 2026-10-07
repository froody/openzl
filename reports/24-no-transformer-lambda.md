# 24 Removing the Compression Transformer from ACE, plus λ (ported build)

Decision rows: "2026-10-07T19:55 transformer", "2026-10-07T21:30 transformer". Data: `runs/search_acedev/{C,A}/{bestnt_n,bestntl3k_n,ctlnt_n}_g250_s1/points.json` (`search_nt.log`), pbench `runs/pb/Cnt/pbench_core13.json`, `pbench_core27.json`, `pbench_ntlam_sweep.json`, `pbench_ntlam_sweep2.json`, per-chunk `perchunk.csv`.

## 1. Question
The ported build (report 21) decodes at 0.89x of v0.2.3, below the 0.97x bar. λ did not help on it (λ up to 3e6 stayed at 0.90x). Upstream dev added `zl.transformer_numeric` (the Compression Transformer, #1008) to the ACE graph universe and to the prebuilt seeds. Is that graph the reason λ cannot reach the bar?

## 2. Change
- `ACE_NO_TRANSFORMER=1` (commit on `ar/ace-dev-port`, binary `bins/zli_acedev_v2`): drops `graphs::TransformerNumeric` from `makeAllGraphs()` and from the prebuilt numeric seeds in `tools/training/ace/ace_compressors.cpp`. Unset = dev behaviour.
- New run labels (glossary): `bestnt_n` = `best_n` + `ACE_NO_TRANSFORMER`; `bestntl3k_n` = that + λ 3000; `ctlnt_n` = plain upstream settings + `ACE_NO_TRANSFORMER`; `bestntl30k_n` / `bestntl50k_n` = `bestnt_n` + λ 30000 / 50000.
- λ (`ACE_BRUTE_LAMBDA`, selector param 7312) only changes the brute-force selector's pick at compress time. So one trained model can be re-scored at any λ by editing param 7312 in the `.zlc` (`set_lambda`). The candidate graphs stay the same.

## 3. Method
- Points, unsorted split C (and A for ratio), seed 1, 250 gens.
- Decode: pbench only (`bins/pbench_dev`), 563 C test chunks, 7 iterations, min time per chunk, all models decoded back to back per chunk. Two cores (13, 27) for the main table.
- λ sweep: the `bestntl3k_n` model with param 7312 set to 0 .. 1e6, frames re-made with `pb_frames.py`.

## 4. Results

### 4.1 Trained models (ratio on held-out test files)

| split | model | ratio | vs best_n | vs v0.2.3 |
|---|---|---|---|---|
| C | best_n | 4.0313 | 0 | +7.20% |
| C | bestnt_n | 4.0251 | -0.15% | +7.04% |
| C | bestntl3k_n | 4.0227 | -0.21% | +6.97% |
| C | ctlnt_n | 3.8473 | -4.56% | +2.31% |
| A | best_n | 4.0633 | 0 | +5.44% |
| A | bestnt_n | 4.0472 | -0.40% | +5.03% |

### 4.2 pbench decode, C (vs v0.2.3; MB/s on core 13 / core 27)

| model | ratio | core 13 | core 27 |
|---|---|---|---|
| v0.2.3 | 3.7606 | 1.000x (303) | 1.000x (295) |
| dev | 3.8008 | 0.718x | 0.719x |
| best_n | 4.0314 | 0.891x | 0.898x |
| bestnt | 4.0252 | 0.848x | 0.845x |
| bestntl3k | 4.0228 | 0.907x | 0.904x |
| ctlnt | 3.8474 | 0.943x | 0.942x |

Removing the transformer alone does not make `best_n` faster (0.85x). The brute-force selector then picks other slow candidates.

### 4.3 λ sweep on one no-transformer model (C, pbench core 27)

| λ | ratio | vs v0.2.3 | decode vs v0.2.3 |
|---|---|---|---|
| (best_n, with transformer) | 4.0314 | +7.20% | 0.900x |
| 3000 | 4.0228 | +6.97% | 0.911x |
| 10000 | 4.0089 | +6.60% | 0.941x |
| 15000 | 4.0001 | +6.37% | 0.950x |
| 20000 | 3.9951 | +6.24% | 0.951x |
| 30000 | 3.9853 | +5.98% | 0.959x |
| **50000** | **3.9797** | **+5.83%** | **0.971x** |
| 70000 | 3.9552 | +5.18% | 0.975x |
| 100000 | 3.8657 | +2.80% | 0.988x |

For comparison, the same sweep on a model with the transformer (report 21 era, `pbench_sweep.json`) topped out at 0.90x even at λ 3e6.

### 4.4 Where the decode time goes (best_n vs v0.2.3, per file)
Slowdown is spread over the flat airborne files: 220721 0.80x, 2519000 0.78x, 2568500 0.84x, ballarat 0.81x. Each is 10-17% of the extra time. The hard files (474000, 533084, dq) carry most of the ratio gain (+17..24%) at 0.88-0.99x.

## 5. Verdict
- **KEEP-candidate.** `ACE_NO_TRANSFORMER` + λ 50000 is the first setting that passes the 0.97x decode bar: +5.83% ratio vs v0.2.3 at 0.971x on C.
- `ACE_NO_TRANSFORMER` alone is not enough; it is what lets λ work.
- Pending: λ 30k/50k trained directly (not edited after training) on 9 splits: unsorted A/B/C, Hilbert, Morton (`search_ntlam.log`). Production order is Hilbert/Morton, so the sorted splits decide.

## 6. Caveats
- One split (C), seed 1, unsorted order. Sorted results pending.
- λ was edited after training. Training with λ may pick other candidates.
- pbench numbers move about ±1% between cores; 0.971x is close to the bar.
