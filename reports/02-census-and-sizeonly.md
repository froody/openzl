# 02 Population census and ACE_SIZE_ONLY

Decision rows 5, 6, 8-14, 19, 21. Data: `scratch/census`, `scratch/sizeonly`, `scratch/oracle`, `scratch/fr_census`, `scratch/fr_sizeonly`, `scratch/generic_A.txt`.

## 1. Question
The starting premise was: "nodes that only reorder or shuffle data (and do not compress) crowd the ACE population, so the search wastes effort". Is that true? If the time objectives keep useless genes alive, does ranking on size alone (`ACE_SIZE_ONLY`) find smaller graphs?

## 2. Change
Worktree `wt/ace`, branch `ar/ace` off v0.2.3, commit aafeaad (binary `bins/zli_ace_v1`). Same change on froody: worktree `wt/acefr`, branch `ar/ace-froody` (commits ff468e4, 470e9c6; binary `bins/zli_acefr_v1`).

- `ACE_CENSUS=1`: print one `ACE_CENSUS` line per generation to stderr. It reports: `front0` (genes on the first Pareto front), `uniq_size`, `best` (smallest training size), `nogain` (genes whose size is at least 95% of the source, so they do not compress), `within1pct` (genes within 1% of the best), and `mean_components`.
- `ACE_MAX_GENERATIONS=N`: stop each ACE search after N generations.
- `ACE_SIZE_ONLY=1`: set the compression-time and decompression-time objectives to 0. NSGA-II then ranks on size alone (`computeFitness` / `adjustResults` in `automated_compressor_explorer.cpp`).
- `ACE_NODES` (printed when `ACE_CENSUS` is set, commit bd31eb5): the list of nodes that mutation and random build can pick, counted by name.

Run labels (all in `scratch/`):
- `census`: upstream + census log, normal mode (equal to upstream behaviour).
- `sizeonly`: upstream + `ACE_SIZE_ONLY=1`.
- `oracle`: upstream, trained on the 12 test LAZ files themselves (`split_Aoracle.json`), tested on the same files.
- `fr_census`, `fr_sizeonly`: the same on the froody node set.

## 3. Method
- Split A only. One run per cell, default seed.
- Budget: `--max-time-secs 300` per ACE graph (wall clock, not fixed generations).
- Columns: intensity and gps_time; points on froody and for the upstream census.
- Generic reference: zstd -19 and xz -9 on the same split A test chunks (`generic_ref.py`).
- No speed measurement in this experiment.

## 4. Results
Held-out ratio on split A, recomputed from the per-LAZ CSVs. Generations per ACE graph in brackets (from the `ACE_CENSUS` lines; `oracle` ran without the census log):

| run | points | gps_time | intensity |
|---|---|---|---|
| census (upstream, normal) | 3.8182 (154/250/250/183/101 gens) | 9.9485 (31 gens) | 1.7931 (49 gens) |
| sizeonly (upstream, ACE_SIZE_ONLY) | - | 9.9410 (18 gens) | 1.7932 (17 gens) |
| oracle (upstream, train on test LAZ) | - | 9.9499 (? gens) | 1.7932 (? gens) |
| fr_census (froody, normal) | 3.7996 (208/244/18/23/30 gens) | 9.7824 (33 gens) | 1.7932 (32 gens) |
| fr_sizeonly (froody, ACE_SIZE_ONLY) | 3.6702 (159/250/8/15/11 gens) | 9.8999 (18 gens) | 1.7916 (15 gens) |

Best-case check (rows 11, 12): training on the test files themselves (`oracle`) gives intensity 1.7932 and gps_time 9.9499. The normal train-on-train runs give 1.7931 and 9.9485. So neither search nor generalisation limits the ratio on these two columns with the upstream node set.

Generic coders on the same test chunks (row 13):

| column | zstd -19 | xz -9 | OpenZL (census) |
|---|---|---|---|
| intensity | 1.5507 | 1.6813 | 1.7931 |
| gps_time | 7.5183 | 9.6337 | 9.9485 |
| points | 2.6371 | 3.2461 | 3.8182 |

Population census (last `ACE_CENSUS` line of each graph; points graphs 2 and 3 are tiny streams and are left out):

| run | column | ACE graph | gens | nogain/100 (gen>=15) | front0 max | front0 last | within1pct max | mean components (last) |
|---|---|---|---|---|---|---|---|---|
| census | intensity | 1 | 49 | 7-8 | 63 | 62 | 29 | 5.39 |
| census | gps_time | 1 | 31 | 5-8 | 52 | 52 | 13 | 6.67 |
| census | points | 1 | 154 | 5-9 | 84 | 83 | 19 | 5.19 |
| census | points | 4 | 183 | 4-7 | 64 | 64 | 16 | 4.92 |
| census | points | 5 | 101 | 4-11 | 97 | 82 | 24 | 6.17 |
| fr_census | intensity | 1 | 32 | 4-7 | 58 | 58 | 28 | 4.70 |
| fr_census | gps_time | 1 | 33 | 3-3 | 69 | 68 | 13 | 7.65 |
| fr_census | points | 1 | 208 | 14-25 | 22 | 22 | 21 | 2.48 |
| fr_census | points | 4 | 23 | 4-8 | 43 | 43 | 27 | 5.16 |
| fr_census | points | 5 | 30 | 9-12 | 56 | 56 | 16 | 4.32 |
| sizeonly | intensity | 1 | 17 | 0-0 | 3 | 1 | 100 | 8.61 |
| sizeonly | gps_time | 1 | 18 | 0-0 | 2 | 1 | 93 | 18.81 |
| fr_sizeonly | intensity | 1 | 15 | 0-0 | 2 | 1 | 100 | 11.30 |
| fr_sizeonly | gps_time | 1 | 18 | 0-0 | 6 | 1 | 100 | 10.55 |
| fr_sizeonly | points | 1 | 159 | 0-0 | 12 | 12 | 27 | 2.02 |
| fr_sizeonly | points | 4 | 15 | 0-0 | 1 | 1 | 100 | 4.19 |
| fr_sizeonly | points | 5 | 11 | 0-0 | 2 | 1 | 100 | 7.11 |

Node universe (row 21, from `ACE_NODES`): the numeric input type has 131 node entries upstream and 140 on froody. `zl.bitunpack` has 63 entries (one per bit width), `zl.convert_serial_to_struct` 32, `zl.field_lz` 10. froody adds 9 numeric and 3 struct nodes with 1 entry each. Mutation picks uniformly over entries, so bitunpack is 63/131 = 48% of numeric picks, and each new froody codec gets about 1/140 = 0.7%.

## 5. Verdict
- Premise rejected. Non-compressing genes are only 3-11 of 100 after generation 15, on upstream and on froody. They do not crowd the population.
- What does fill the population is the size of front 0, caused by the two time objectives (52-97 of 100 genes on front 0).
- `ACE_SIZE_ONLY` rejected for production. It gives no gain on upstream (gps_time 9.9410 vs 9.9485, intensity 1.7932 vs 1.7931). On froody it helps gps_time a little (9.8999 vs 9.7824) and hurts points (3.6702 vs 3.7996). It collapses diversity (`within1pct` reaches 91-100 of 100) and graphs grow to 9-19 components, so fewer generations fit in the time budget.
- Kept as tools: `ACE_CENSUS` and `ACE_MAX_GENERATIONS`. `ACE_SIZE_ONLY` was later kept as a test bed only, because it makes runs reproducible (report 06).
- New premise after this experiment (row 14): the upstream search already finds its own optimum on these columns, so gains need a bigger node set or a different selection. The skewed node pick (row 21) led to report 03.

## 6. Caveats
- 300 s wall-clock budgets on a loaded machine. Generation counts vary a lot (for example sizeonly ran 17-18 gens where census ran 31-49). Many differences here may be fewer generations, not a worse objective (row 25).
- One seed per cell. Seed noise was measured later at about 3% on gps_time (report 06), larger than every difference here except the froody points loss.
- `oracle` was only run for intensity and gps_time, not for points.
- In row 18 the upstream census points value 3.8182 is used for comparison; it is from the same 300 s setup.
