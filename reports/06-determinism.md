# 06 Determinism: same-seed reproducibility, normal vs size-only

Decision rows 47, 49-51. Data: `scratch/det/so{0,1}_r{1,2}`.

## 1. Question
Two normal-mode runs with the same seed gave different searches (report 05, split C seed 1). Why? Is it the measured compress and decompress times that enter the NSGA-II ranking? Can we get runs that repeat exactly, so A/B tests can be paired by seed?

## 2. Change
No new code. The test uses `ACE_SIZE_ONLY` (report 02) and `ACE_SEED` (report 03), with binary `bins/zli_ace_v6`.

Run labels (`scratch/det/so<X>_r<N>`):
- `so0`: `ACE_SIZE_ONLY` not set (normal mode: size, compression time and decompression time are all objectives).
- `so1`: `ACE_SIZE_ONLY=1` (time objectives set to 0, ranking on size only).
- `r1`, `r2`: first and second run with identical settings.

## 3. Method
- gps_time, split A, `ACE_SEED=1`, `ACE_MAX_GENERATIONS=30`, no time limit. Each setting run twice.
- Compare the per-generation best training size (`ACE_CENSUS` `best=`) and the held-out ratio.
- No speed measurement.

## 4. Results

| run | mode | gen 1 best (train bytes) | gen 2 best | gens | final best (train bytes) | held-out ratio | train s |
|---|---|---|---|---|---|---|---|
| so0_r1 | normal | 12063327 | 12053821 | 30 | 11000326 | 9.9390 | 202 |
| so0_r2 | normal | 12093748 | 12085331 | 30 | 11305524 | 9.6312 | 183 |
| so1_r1 | size-only | 12093748 | 12063327 | 30 | 10930872 | 10.0259 | 314 |
| so1_r2 | size-only | 12093748 | 12063327 | 30 | 10930872 | 10.0259 | 320 |

- Normal mode: the two runs differ from generation 1 (12063327 vs 12093748 bytes). They end 2.8% apart on training bytes (11000326 vs 11305524) and 3.2% apart on held-out ratio (9.9390 vs 9.6312). Per test file, the gap goes both ways: -15% on 4 files, +2..5% on others.
- Size-only: the two runs have the same best size in all 30 generations and the same held-out ratio (10.0259), bit for bit.

## 5. Verdict
Confirmed: the measured times in the NSGA-II ranking make normal-mode search nondeterministic (time measurements change from run to run, so ranking and survivors change). Size-only runs are reproducible.

Kept as test method (row 51): search changes are A/B-tested in size-only mode, with the same seed with and without the change, 3 seeds x 3 splits (`search_ab.py`). Changes that pass are checked again in normal mode, with several seeds, because size-only was rejected for production (report 02). Decode and encode time are judged separately.

## 6. Caveats
- One column (gps_time), one split, one seed, 30 generations. The 3% noise is for a single column; on points the seed-to-seed spread was 2-4% (reports 03, 04).
- Row 50 gives the held-out gap as 3.1%; the result files give 9.9390 vs 9.6312 = 3.2%.
- Size-only runs are slower in wall-clock here (314-320 s vs 183-202 s for 30 gens), because size-only graphs grow bigger (report 02).
