# 25 — λ on the clean 48-file corpus (v17)

## Question
Training on 48 files raises ratio but slows decode (report 18: fails the 0.97× bar). Can the decode-time selector term λ (`ACE_BRUTE_LAMBDA`, report 24) recover decode speed while keeping the extra ratio?

## Change
No code change. Binary `zli_ace_v17` (v0.2.3 base, no Compression Transformer). λ set at training (`bestl3k_n`, `bestl5k_n`) and, for higher values, edited after training in the `bestl5k_n` model (param 7312, `set_lambda`). Label `eNk` = `bestl5k_n` model with λ = N×1000.

## Method
- Splits `C48c` / `A48c`: 48 clean training files, test files = the C / A test files. Unflagged dumps (production format: Hilbert order within each node).
- Points only, 250 generations, seed 1.
- Size and decode speed with pbench (`bins/pbench_dev`) on the test frames: C 563 chunks, A 579 chunks. Decode speed = v0.2.3 time / model time. Output: `runs/pb/C48lam/pbench{1,2_sweep}.json`, `runs/pb/A48lam/pbench1.json`.

## Results
Size gain vs upstream v0.2.3 model (trained on the C / A train set), decode speed vs v0.2.3.

| model | C size gain | C decode | A size gain | A decode |
|---|---|---|---|---|
| 8-file `best_n` (current setup) | +7.00% | 0.925× | +5.22% | 0.865× |
| 48-file `best_n` | +8.88% | 0.763–0.768× | +5.73% | 0.847× |
| 48-file `bestl3k_n` | +8.67% | 0.90–0.91× | – | – |
| 48-file `bestl5k_n` | +8.51% | 0.94× | +5.46% | 0.929× |
| e10k | +8.23% | 0.965× | +4.80% | 0.984× |
| **e20k** | **+7.89%** | **0.982×** | **+4.41%** | **1.011×** |
| e30k | +7.29% | 0.994× | +4.31% | 1.011× |
| e50k | +6.97% | 0.999× | – | – |

Training cost of λ (zli ratio on the test set): `bestl3k_n` −0.20% (C) / −0.18% (A) vs `best_n`; `bestl5k_n` −0.34% / −0.25%.

Decode in MB/s: v0.2.3 on C ≈ 300 MB/s (core 13); e20k ≈ 295 MB/s.

## Verdict
KEEP-candidate. 48-file + λ 20k passes the decode bar on both splits. Mean size gain over C and A is +6.15% at ≥0.98×, vs +6.11% at 0.89× for the current 8-file `best_n`. It is the first setup that matches the current ratio without the decode loss. Adopt after seeds 2 and 3 (`search_48seed.log`) confirm.

## Caveats
- One seed. Seed spread on points was ±0.1–0.2% earlier (report 06).
- A gains less than C (+4.41% vs +7.89%); A's test files are mostly the hard airborne ones.
- λ edited after training is not the same as λ set at training: training uses λ when picking candidates. A model trained at λ 20k may do slightly better.
- B not run on 48 files (B's test files overlap the 48-file train set).
