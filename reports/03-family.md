# 03 ACE_FAMILY: pick a node family first, then a variant

Decision rows 17, 18, 20, 22-25, 28-31. Data: `scratch/fam_up`, `scratch/fam_fr`, `scratch/census`, `scratch/fr_census`, `runs/gen30/A`.

## 1. Question
Mutation and random build pick uniformly over node entries. `zl.bitunpack` alone is 48% of numeric picks, and each new froody codec gets only 0.7% (report 02). If we pick uniformly over node names instead, does the search find better graphs, on upstream and on the bigger froody node set?

## 2. Change
Commit bd31eb5 on `ar/ace` (`pickRandomNode` in `tools/training/ace/ace_compressors.cpp`, used by random build and by add-pipeline mutation). Same change on `wt/acefr` (commit 43d0b48).

- `ACE_FAMILY=1`: first pick a node name uniformly among the compatible names, then pick one parameter variant of that name uniformly. Without it, ACE picks uniformly over all entries.
- `ACE_SEED=N` (commit 20a584c, row 25): sets the ACE random seed, so different seeds can be compared.

Binaries: `bins/zli_ace_v2` / `bins/zli_acefr_v2` (300 s runs), `bins/zli_ace_v3` / `bins/zli_acefr_v3` (fixed-generation runs, with `ACE_SEED`).

Run labels:
- `fam_up`, `fam_fr` (in `scratch/`): `ACE_FAMILY=1` on upstream / froody, 300 s per graph. Compared with `census` / `fr_census` from report 02.
- `runs/gen30/A/<build>_<fam|base>_s<seed>`: `<build>` is `up` (upstream) or `fr` (froody); `fam` = `ACE_FAMILY=1`, `base` = without it; `s<seed>` = `ACE_SEED`. All with `ACE_MAX_GENERATIONS=30` and no time limit (`gen_ab.py`).

## 3. Method
- Split A only. Normal mode (3 objectives).
- First round: 300 s wall clock per ACE graph, one seed, points / gps_time / intensity.
- Row 25: generation counts in 300 s ranged from 12 to 49 for the same column, because machine load reached 550 from other users' jobs. So the second round used fixed generations: 30 generations, seeds 1 and 2, the same 3 columns, both builds.
- Train ratio of points graph 5 (the biggest stream) is read from the last `ACE_CENSUS` line.
- Decode MB/s below is from the benchmark CSVs (one iteration, not pinned).

## 4. Results
Fixed 30 generations, split A (`runs/gen30/A`). Held-out ratio from `<column>.json`:

| run | points held-out | points graph 5 train ratio | gps_time | intensity | points train s | points dec MB/s |
|---|---|---|---|---|---|---|
| up_base_s1 | 3.7613 | 4.1561 | 9.9868 | 1.7932 | 686 | 230 |
| up_base_s2 | 3.9007 | 4.2926 | 10.0167 | 1.7932 | 726 | 169 |
| up_fam_s1 | 3.6746 | 4.0385 | 9.6475 | 1.7922 | 696 | 111 |
| up_fam_s2 | 3.7441 | 4.1228 | 9.9803 | 1.7932 | 683 | 104 |
| fr_base_s1 | 3.5651 | 3.5512 | 9.9925 | 1.7875 | 816 | 179 |
| fr_base_s2 | 3.8420 | 3.6726 | 9.9410 | 1.7932 | 714 | 66 |
| fr_fam_s1 | 3.6507 | 3.6667 | 9.9416 | 1.7815 | 709 | 44 |
| fr_fam_s2 | 3.6890 | 3.5675 | 9.9824 | 1.7932 | 813 | 96 |

Mean of the 2 seeds, `ACE_FAMILY` vs without:
- upstream: points -3.18%, gps_time -1.88%, intensity -0.03%.
- froody: points -0.91%, gps_time -0.05%, intensity -0.17%.
- froody base vs upstream base: points -3.33%.

The seed spread without any change is large: upstream points 3.7613 vs 3.9007 (3.7%), and graph 5 train ratio 4.1561 vs 4.2926 (3.3%) (rows 30, 31).

First round, 300 s per graph, split A, one seed (gens per ACE graph in brackets):

| run | points (gens per graph) | gps_time (gens) | intensity (gens) |
|---|---|---|---|
| census (upstream) | 3.8182 (154/250/250/183/101) | 9.9485 (31) | 1.7931 (49) |
| fam_up (upstream + ACE_FAMILY) | 3.7633 (74/250/250/99/81) | 9.9485 (24) | 1.7932 (19) |
| fr_census (froody) | 3.7996 (208/244/18/23/30) | 9.7824 (33) | 1.7932 (32) |
| fam_fr (froody + ACE_FAMILY) | 3.5912 (110/101/10/14/13) | 9.9580 (21) | 1.7825 (12) |

## 5. Verdict
Rejected. With equal generations `ACE_FAMILY` is worse on upstream for points (both seeds: 3.6746 vs 3.7613, 3.7441 vs 3.9007) and for gps_time on seed 1 (-3.4%). Points graph 5 train ratio drops 2.8% and 4.0%. On froody it does not help either. The promising 300 s gps_time result on froody (9.9580) only matched upstream, and the time-budget runs were confounded by generation counts.

Side result: froody is not better than upstream at equal generations (points -3.3% mean over 2 seeds).

## 6. Caveats
- Only 2 seeds and one split. The seed spread on points (3.7%) is about the same size as the family effect. The direction is the same on both seeds for upstream points, which is why the verdict is still "reject".
- The 300 s runs are confounded by generation counts (load up to 550) and by froody training the 5 points graphs in parallel inside one time budget (row 20).
- The graph 5 train ratios for the froody runs may not be the same stream as upstream graph 5, so only compare graph 5 within one build.
- Decode MB/s in the table is from a loaded, unpinned machine. Do not read speed conclusions from it.
