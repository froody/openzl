# 10 Specialist ceiling: 20 single-LAZ models, offline per-file and per-chunk choice

Decision rows 62, 64. Data: `runs/spec/<laz>/` (one model per LAZ, benchmarked on all 20 LAZ), `runs/paired/<split>/k4_g120_s1/brute` (generalist), `specialist.py`.

## 1. Question
The user never found one compressor that is best on all data. Is that because one static graph cannot express what different files need (then a selector over corpus-specific graphs would fix it), or because a model trained on some files does not generalise? How much would an ideal choice among file-specific models gain over one generalist?

## 2. Change
No new code. `specialist.py` with binary `bins/zli_ace_v9`.

Run label:
- `runs/spec/<laz>/`: a points model trained on ONE LAZ file only ("specialist"), with `ACE_BRUTE_K=4`, `ACE_MAX_GENERATIONS=120`, `ACE_SEED=1`, normal mode. Each specialist is benchmarked on all 20 LAZ files (`points.<laz>.csv`, per chunk).

The generalist is the `runs/paired/<split>/k4_g120_s1` brute model (report 05): the same settings, but trained on the split's 8 train files.

## 3. Method
- Points only. 20 trainings run in parallel at night (20 x 8 threads). Train time per specialist 124-241 s.
- For each split, over its 12 test files, computed offline from the per-chunk CSVs (chunk source sizes match the generalist's CSVs exactly):
  - best single specialist: the one train-file specialist with the best total on the test files.
  - per-file choice: for each test file, the smallest of the 8 train-file specialists and the generalist.
  - per-chunk choice: the same, per chunk.
  - model trained on the test file itself: each test file compressed by its own specialist (a case that cannot happen in practice; for scale only).
- No speed measurement.

## 4. Results
Points held-out ratio over each split's 12 test files (gain vs the generalist):

| split | generalist (paired brute K4 s1) | best single train-file specialist | per-file choice (8 specialists + generalist) | per-chunk choice | model trained on the test file itself |
|---|---|---|---|---|---|
| A | 3.8613 | 3.8886 (+0.7%, dqKZWZa1QwBW) | 4.0366 (+4.5%) | 4.0475 (+4.8%) | 4.0191 (+4.1%) |
| B | 3.9321 | 3.8807 (-1.3%, 474000_6632000) | 4.0406 (+2.8%) | 4.0448 (+2.9%) | 4.0110 (+2.0%) |
| C | 3.7633 | 3.8056 (+1.1%, 297000_2153000) | 3.8936 (+3.5%) | 3.9016 (+3.7%) | 4.0409 (+7.4%) |

Per-file gain of the per-file choice over the generalist, and which train-file specialist won ("train" = not a test file in that split):

| LAZ | A per-file gain (picked specialist) | B | C |
|---|---|---|---|
| 140 | +0.3% (2024-bal) | +1.3% (zMuOt68X) | +1.0% (znHYtqPt) |
| 2024-ballarat_a-LiDAR_24 | train | train | +0.2% (752132_1) |
| 220721_191602 | +0.5% (2024-bal) | train | +0.1% (znHYtqPt) |
| 2519000_7357000 | train | +0.6% (zMuOt68X) | +0.0% |
| 2568500_7558000-CuG4mpzj | +0.7% (2024-bal) | +1.5% (zMuOt68X) | +0.0% (znHYtqPt) |
| 264500_3290500-Cvzq7Q5dp | +0.4% (2024-bal) | train | train |
| 297000_2153000-dc3snqhBR | train | train | train |
| 3090551_447834-C0LyGoANX | train | +0.0% | train |
| 3828740_10300196-dcPTQ5d | +0.0% (297000_2) | train | +0.0% |
| 474000_6632000-dcpb7qGtn | +22.8% (dqKZWZa1) | train | +14.5% (chunk_0_) |
| 533084_6793800-dcmGP6Y9h | +26.2% (dqKZWZa1) | +12.4% (474000_6) | +13.6% (chunk_0_) |
| 752132_1153215-dcVOpD5fi | +0.6% (2024-bal) | +1.3% (zMuOt68X) | train |
| EX_20230607_124634__2301 | train | train | +0.3% (znHYtqPt) |
| Z3_6190800_2067600 | train | +7.2% (474000_6) | +2.9% (chunk_0_) |
| chunk_0_5 | train | +0.0% | train |
| dqKZWZa1QwBW | train | +8.7% (474000_6) | +4.4% (297000_2) |
| z5GMxol0oJkH | +0.1% (EX_20230) | +0.2% (zMuOt68X) | train |
| zMuOt68XGoj6 | +0.0% (2024-bal) | train | +0.6% (znHYtqPt) |
| znHYtqPtsNgf | +0.2% (chunk_0_) | +0.6% (zMuOt68X) | train |
| zw7rJF499NRr_345856_6365 | +3.7% (chunk_0_) | +3.0% (474000_6) | train |

Most of the gain is on a few files: 474000 (+14.5..+22.8%), 533084 (+12.4..+26.2%), dqKZ (+4.4..+8.7%), Z3 (+2.9..+7.2%), zw7r (+3.0..+3.7%). The others gain 0-1.5%.

## 5. Verdict
Confirms the user's finding. One best single specialist is not better than the generalist (-1.3..+1.1%). But a choice among corpus-specific graphs is worth +2.8..+4.5% per file and +2.9..+4.8% per chunk, more than any search change so far. Most of it is already there with a per-file (coarse) choice, so a cheap selector is enough. This led to ACE_SPECIALISTS (report 11).

## 6. Caveats
- Offline oracle: choice is free and perfect (picked using the test data). A real selector must pick by trial compression or by a rule.
- One seed per specialist, normal mode (not reproducible, report 06), 120 gens.
- Row 64 lists the per-file and per-chunk values exactly as recomputed here. Row 62 says the night batch was "relaunched" after a first launch failed; only the second launch's results exist.
