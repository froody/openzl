# Experiment reports

One report per experiment. Each has: Question, Change, Method, Results, Verdict, Caveats. Numbers are recomputed from the result files under `runs/` and `scratch/`. Where they differ from `decision.tsv`, the report says so. Points held-out ratio is the main score; other columns are secondary checks.

| file | question | verdict | headline number |
|---|---|---|---|
| [01-baseline](01-baseline.md) | Stock held-out ratio; does froody/dev beat upstream v0.2.3? | `base` kept as reference; froody rejected | overall base A/B/C 5.6795 / 5.6885 / 5.5885; froody -0.64% / +0.24% / -0.23% |
| [02-census-and-sizeonly](02-census-and-sizeonly.md) | Do non-compressing nodes crowd the population? Does ranking on size alone help? | premise rejected; ACE_SIZE_ONLY rejected for production (kept as test bed) | nogain only 3-11/100; sizeonly gps_time 9.9410 vs 9.9485 |
| [03-family](03-family.md) | Does picking node families uniformly (ACE_FAMILY) find better graphs? | rejected | points -3.18% (upstream, 30 gens, 2 seeds) |
| [04-islands](04-islands.md) | Can independent island searches turn seed variance into gain at equal compute? | rejected | +0.82% at 30 gens, +0.18% at production compute |
| [05-brute-k](05-brute-k.md) | Does a brute-force selector over complementary candidates (ACE_BRUTE_K) gain held-out? | kept | +1.04% paired mean, 9/9 cells > 0, no file worse |
| [06-determinism](06-determinism.md) | Why do same-seed runs differ? | confirmed: time objectives cause it; size-only runs are reproducible | normal mode 3.2% apart, size-only identical |
| [07-polish](07-polish.md) | Does best-backend polish of top genes (ACE_POLISH) help points? | rejected | +0.21% (noise) |
| [08-pool](08-pool.md) | Does pooling independent searches into brute selection (ACE_POOL) help at equal compute? | kept (small), then superseded by ACE_SPECIALISTS | +0.75% normal mode (12 cells); +2.31% size-only |
| [09-codec-probe](09-codec-probe.md) | Do froody codecs or a bucket+raw-bits codec beat the trained points model? | froody codecs dropped; bucket codec not built | best hand chain +1.4..+4.0% larger than ref; oracle gain at most 1-2% |
| [10-specialist-ceiling](10-specialist-ceiling.md) | How much would a choice among single-file models gain over one generalist? | confirms a selector over corpus-specific graphs is the lever | per-file +2.8..+4.5%, per-chunk +2.9..+4.8% |
| [11-specialists](11-specialists.md) | Can ACE build per-file specialists itself (ACE_SPECIALISTS)? | kept | +3.84% mean (9 cells, min +1.36%), decode 0.94-0.97x pinned |
| [12-max](12-max.md) | Does 3x more search + K=8 + ACE_NO_CTIME gain ratio or decode speed? | rejected for ratio | +0.54% vs control, -0.17% vs pool3b4tol_n_g83; decode 1.10x pinned |
| [13-decode-speed](13-decode-speed.md) | What do the kept changes cost in pinned decode speed? Does the slack tie-break help? | spectok decode cost accepted; ACE_BRUTE_SLACK / ACE_DEC_EPS rejected | spectok 0.95-0.99x; slack changes the ratio by < 1 ppm |
| [14-tok-seeds](14-tok-seeds.md) | Do tokenize-first seed genes (ACE_TOK_SEEDS) help, alone and with ACE_SPECIALISTS? Final recipe? | kept (points only); final recipe = spectok + ACE_NO_CTIME | spectok +4.27% vs brute control (min +3.54%), +4.1..+5.1% vs upstream |
| [15-other-columns](15-other-columns.md) | Does spectok help the secondary columns? Why does point_source_id regress? | per column: points + intensity = spectok + NO_CTIME, others = spec + NO_CTIME | point_source_id B/C -7.0% / -5.7% (Z3 -45%), spec alone +0.42% / +0.11%; spec 2-seed: gps_time +4.1%, intensity +8.2%, scan_angle +10.6% |
| [16-brute-stop-and-balance](16-brute-stop-and-balance.md) | Does a lower brute min-gain threshold (ACE_BRUTE_MIN_GAIN), more picks (K=8) or file-balancing (ACE_BRUTE_BALANCE) improve selection? | K=8 + ACE_BRUTE_MIN_GAIN=0.0001 kept (`best`); ACE_BRUTE_BALANCE rejected | spectokG +0.21%, spectokG8 +0.19% more (9/9 > 0); best_n +0.77% vs spectok_n, +5.57% vs upstream, decode 0.98-1.00x |
| [17-cross-slot-and-xy-swap](17-cross-slot-and-xy-swap.md) | Does letting x/y/z brute picks share candidates (ACE_BRUTE_CROSS_SLOT) or training on x/y-swapped copies fix the per-axis slack? | both help, mostly on C; combined run and normal-mode check open | vs spectokG8: cross +0.55%, swap +0.69% (9/9 > 0; C +1.6% / +1.9%) |
| [18-bigger-corpus](18-bigger-corpus.md) | Does training on 24 or 48 files (corpus2) beat 8 files on the same test sets? Is the gain from same-source leakage? Decode cost? | ratio: yes for the specialist recipes, leakage not the cause; not kept yet: decode fails the bar; slack test running | best_n 48 clean files: A +0.48%, C +1.76%; decode C 0.838x; generalist -1.9% / -2.6% |
| [19-sort-order](19-sort-order.md) | Do Morton- or Hilbert-sorted points compress better? Does the points graph still transpose per axis? | keep no sort (writer default); Morton-key stream not built | best_n Hilbert -1.12% / -0.03% / +0.59%, Morton -4.75% / -1.90% / -0.45% (A/B/C, 1 seed) |
| [20-swap-normal-and-decode-cap](20-swap-normal-and-decode-cap.md) | Does x/y swap hold in normal mode? Can a decode-time cap on brute picks (ACE_BRUTE_DEC_CAP) fix its decode cost? | swap gains ratio but fails decode on C; cap 1.15 rejected | swap +0.56% (C +1.44%), decode C 0.88x; bestc115_n 3 seeds -0.01% (C s1 +1.36% was luck) |
| [21-port-to-dev](21-port-to-dev.md) | Do the kept ACE features keep their gain when ported to upstream dev 5545627c? How fast do the models decode? | port keeps the ratio; decode fails the 0.97x bar; pbench is now the decode metric; lambda runs on the port running | port vs `ar/ace` best_n +0.10..+0.21%; vs dev +3.57% / +3.86% / +6.07% (A/B/C); pbench C: port 0.89-0.90x, `ar/ace` best_n 0.92-0.93x, dev 0.71-0.74x of v0.2.3 |
| [22-issue-933-points](22-issue-933-points.md) | Issue facebook/openzl#933: does training on more or other files hurt (points only, 5 issue files)? | not for our recipe; upstream dq self-trained is the worst, so the issue's dq gap is in another column (runs in progress) | totals: up_self / up_all / up_loo 3.5481 / 3.7497 / 3.5894; ours 3.8662 / 3.8756 / 3.7930; our_all +2.80% vs upstream oracle |
| [23-iwt](23-iwt.md) | Does froody's integer wavelet (`zl.iwt_int`) help any column, sorted or unsorted? Can a changed iwt help? | no, on every column; kernel has a sign bug for 1/2/4-byte widths; no changed lifting beats delta | real codec vs delta>zigzag>transpose>entropy: points Hilbert +25.4% larger (1 level), points_u16 +15%, intensity +7.7%, gps_time +26%; decode 0.61x; best numpy fix still +3.0% larger |
| [24-no-transformer-lambda](24-no-transformer-lambda.md) | Is dev's Compression Transformer graph why λ cannot reach the decode bar on the ported build? | yes: without it, λ 50000 passes the bar (KEEP-candidate); trained-λ runs on 9 splits incl. Hilbert/Morton pending | C: bestnt_n +7.04% at 0.85x; λ 50k (edited) +5.83% at 0.971x; λ 30k +5.98% at 0.959x; with transformer λ tops at 0.90x |
| [25-lambda-48-files](25-lambda-48-files.md) | Can λ recover decode speed on the 48-file corpus? | KEEP-candidate (seeds 2,3 pending) | e20k: C +7.89% at 0.98×, A +4.41% at 1.01× |
| [26-sampler-sort-flags](26-sampler-sort-flags.md) | Which sampler sort flags match production, and which compress best? | use unflagged dumps (already Hilbert within node) | Morton within node +2.5% bytes (ours), +5.3% (upstream) |

Final recipe (points): `ACE_TOK_SEEDS=1 ACE_SPECIALISTS=1 ACE_SPEC_GENERATIONS=60 ACE_BRUTE_K=8 ACE_BRUTE_TOL=0.5 ACE_NO_CTIME=1 ACE_BRUTE_MIN_GAIN=0.0001` (label `best`), binary `bins/zli_ace_v14`, branch `ar/ace` commit 3aa7587. Intensity: `spectok` + `ACE_NO_CTIME=1`. Color, gps_time, scan_angle, point_source_id: `spec` + `ACE_NO_CTIME=1` (report 15; K=8 / MIN_GAIN not tested there). ACE_BRUTE_CROSS_SLOT rejected (row 99). `ACE_BRUTE_DEC_CAP=1.15` rejected (row 110). Open: x/y swap and more training files both gain ratio but fail the 0.97x decode bar on C (reports 20, 18). The `ACE_BRUTE_SLACK` runs on A48c/C48c were stopped: slack never reached the selector before v17 (row 112). Next: `ACE_BRUTE_LAMBDA` (v17, and on the dev port, report 21). Decode speed is now measured with pbench (report 21).

## Glossary

### Splits, modes, suffixes
- **A / B / C**: train/test splits of the 20 LAZ files, 8 train and 12 test each. A = original corpus split (`split_A.json`). B, C = random re-splits with seed 7.
- **held-out ratio**: sum of source bytes / sum of compressed bytes over the test LAZ chunks (256 KiB chunks, at most 8 MiB per LAZ per column).
- **normal mode**: ACE ranks on 3 objectives (compressed size, compression time, decompression time). Not reproducible with the same seed (report 06).
- **size-only**: `ACE_SIZE_ONLY=1`. Ranks on size only. Reproducible with the same seed; used as a test bed for search changes.
- **`_n` suffix** (e.g. `spec_n`): normal mode. **No suffix** (e.g. `tok_g120`): size-only.
- **`g<N>`**: `ACE_MAX_GENERATIONS=N` (per search; with `ACE_POOL` per pool search).
- **`s<N>`**: `ACE_SEED=N`.
- **Asw / Bsw / Csw**: A / B / C with x/y-swapped copies of every train file added to the train list (`split_<X>sw.json`, chunks in `chunks/points/<laz>_swxy`, columns [1,0,2]). Same test files.
- **Am / Bm / Cm**, **Ah / Bh / Ch**: A / B / C with Morton- or Hilbert-sorted dumps of the same LAZ files (tags `<tag>@morton` / `<tag>@hilbert`, `dump_sorted.py`, report 19).
- **A24 / A48 / C24 / C48**: A or C train files + 16 or 40 new files from `corpus2.json` (79 new LAZ, 3 per `~/plp-data` source dir). Same test files as A or C.
- **I5 files**: the 5 files of issue facebook/openzl#933: zn = `znHYtqPtsNgf`, dA = `dAPcA8fVvH3u_482813_4206587`, out = `output`, dq = `dqKZWZa1QwBW`, 140 = `140`. **I5all** = train and test on all 5. **I5self_<x>** = train and test on x only. **I5loo_<x>** = train on the other 4, test on x (report 22).
- **self / all / loo** (report 22): ratio on a file from its I5self, I5all or I5loo model; `up_` = upstream v0.2.3 `ctl_n`, `our_` = `best_n`.
- **A48c / C48c**: A or C train files + 40 new corpus2 files from source dirs with no test file (leakage check, row 98). Same test files as A or C.
- **pbench**: `bins/pbench_dev` (source `ideas/round3_scratch/pbench.cpp`, dev decoder lib). Per chunk, decodes every model's frame back to back, 7 iterations, order flipped each iteration, pinned to core 13. MB/s = source bytes / sum of per-chunk min time. Frames made with `pb_frames.py` (`runs/pb/<set>/`). The decode metric from report 21 on.
- **pinned speed**: `speed_ab.py`: `zli benchmark -n 5` on one pinned core, 5 interleaved rounds, median per file, single-thread MB/s. Not repeatable (report 21): use pbench. **bench MB/s**: from the one-iteration benchmark CSVs written by `bench.py`, unpinned, on a shared machine (rough only).

### Run labels
- **base**: upstream v0.2.3 matrix, 900 s per ACE graph (`runs/base`).
- **froody**: froody/dev (quars removed) matrix, 900 s per ACE graph (`runs/froody`).
- **census**, **sizeonly**, **oracle**, **fr_census**, **fr_sizeonly** (`scratch/`): 300 s split A runs: upstream with census log; with `ACE_SIZE_ONLY`; trained on the test LAZ files; same two on the froody node set.
- **fam_up**, **fam_fr** (`scratch/`): `ACE_FAMILY=1` on upstream / froody, 300 s.
- **up_/fr_ + base/fam + _s<N>** (`runs/gen30/A`): upstream or froody, without or with `ACE_FAMILY`, seed N, 30 gens.
- **isl3_30**, **isl3_83**: `ACE_ISLANDS=3`, 30 or 83 gens per search. **base_30**, **base_90**, **base_250**: one plain search of 30, 90, 250 gens (`runs/isl`).
- **brute4_250**: `ACE_BRUTE_K=4`, 250 gens, unpaired (`runs/isl`).
- **k4_g120** (`runs/paired`): `ACE_BRUTE_K=4`, 120 gens; `k1/` = plain model and `brute/` = K=4 model from the same search.
- **ctl**: no change (size-only control for `pol5`). In speed files, `ctl` = `b4tol_n_g250`.
- **pol5**: `ACE_POLISH=5`.
- **b4tol**: `ACE_BRUTE_K=4 ACE_BRUTE_TOL=0.02`. The brute control for reports 08-15.
- **pool3b4tol**: `ACE_POOL=3 ACE_BRUTE_K=4 ACE_BRUTE_TOL=0.02`. In speed files: `pool`.
- **spec**: `ACE_SPECIALISTS=1 ACE_SPEC_GENERATIONS=60 ACE_BRUTE_K=4 ACE_BRUTE_TOL=0.5`.
- **spec8**: `spec` with `ACE_BRUTE_K=8`.
- **specs20**, **specs50**: `spec` + `ACE_BRUTE_SLACK=20` / `50` (0.2% / 0.5%).
- **ctl_n** (in `runs/search_up`, `runs/search_updev`): plain upstream training (v0.2.3 or dev 5545627c), no ACE_* env vars, normal mode.
- **bestl3k**, **bestl5k**: `best` + `ACE_BRUTE_LAMBDA=3000` / `5000` (`zli_ace_v17` or `zli_acedev_v1`).
- **v17 best_n** (decision log): the `runs/search/<split>/best_n_g250_s1` models, i.e. `best_n` on `ar/ace`.
- **bests100**, **bests300**: `best` + `ACE_BRUTE_SLACK=100` / `300` (1% / 3%), run on A48c / C48c (report 18).
- **specd**: `spec` + `ACE_BRUTE_SLACK=20 ACE_DEC_EPS=0.002`.
- **tok**: `ACE_TOK_SEEDS=1 ACE_BRUTE_K=4 ACE_BRUTE_TOL=0.02`.
- **spectok**: `spec` + `ACE_TOK_SEEDS=1`.
- **spectokL**: `spectok` with `ACE_SPEC_GENERATIONS=150`.
- **spectokNC**: `spectok` + `ACE_NO_CTIME=1` (the final recipe).
- **spectokG**: `spectok` + `ACE_BRUTE_MIN_GAIN=0.0005`.
- **spectokB**: `spectokG` + `ACE_BRUTE_BALANCE=1`.
- **spectokG0**: `spectok` + `ACE_BRUTE_MIN_GAIN=0.0001`.
- **spectokG8**: `spectok` with `ACE_BRUTE_K=8 ACE_BRUTE_MIN_GAIN=0.0001`.
- **spectokG8x**: `spectokG8` + `ACE_BRUTE_CROSS_SLOT=1` (`zli_ace_v15`).
- **best**: `spectok` + `ACE_NO_CTIME=1 ACE_BRUTE_K=8 ACE_BRUTE_MIN_GAIN=0.0001` (the points recipe, `zli_ace_v14`). In speed files: `best`.
- **bestc115**, **bestc13**: `best` + `ACE_BRUTE_DEC_CAP=1.15` / `1.3` (`zli_ace_v16`), run on the swap splits (`Asw/Bsw/Csw`). In speed files: `c115`, `c13`. `best` on a swap split is `best_sw` in speed files.
- **final**: `spectok` + `ACE_POOL=3`.
- **max**: `ACE_POOL=3 ACE_BRUTE_K=8 ACE_BRUTE_TOL=0.05 ACE_NO_CTIME=1`, 3 x 250 gens.
- **runs/spec/<laz>**: specialist ceiling model trained on one LAZ only (`ACE_BRUTE_K=4`, 120 gens, seed 1).
- **refA / refB / refC**, **dz_te**, **dz_pcM**, **E1-E4**: codec probe names, see report 09.

### Environment variables (branch `ar/ace` in `wt/ace`; binary where first available)
- **ACE_CENSUS** (aafeaad, `zli_ace_v1`): print one population summary line per generation (`front0`, `best`, `nogain`, `within1pct`, `mean_components`), and the node list (`ACE_NODES`).
- **ACE_MAX_GENERATIONS=N** (aafeaad, `zli_ace_v1`): stop each ACE search after N generations.
- **ACE_SIZE_ONLY=1** (aafeaad, `zli_ace_v1`): set both time objectives to 0; rank on size only.
- **ACE_FAMILY=1** (bd31eb5, `zli_ace_v2`): mutation and random build pick a node name uniformly, then a variant of it.
- **ACE_SEED=N** (20a584c, `zli_ace_v3`): ACE random seed.
- **ACE_ISLANDS=K** (3ebc6ac, `zli_ace_v4`): run K-1 independent searches first, then load their populations into the main search.
- **ACE_BRUTE_K=K** (cff58c2, `zli_ace_v5`): per ACE graph, greedily pick up to K complementary candidates (each must save >= 0.2% of training bytes) and put them under the stock brute-force selector; each stream uses its smallest pick. Stock decoder reads the output.
- **ACE_BRUTE_K1_OUT=path** (3a01d05, `zli_ace_v6`): also write the plain single-candidate model from the same search (paired control).
- **ACE_POLISH=N** (ce3681c, `zli_ace_v7`): each generation, try every backend on each leaf of the N smallest unpolished front-0 genes, keep the smallest, add it as a child.
- **ACE_POOL=K** (a93556e, `zli_ace_v8`): after the main search, run K-1 more independent searches (same gens each) and add their populations to the brute selection.
- **ACE_BRUTE_TOL=t** (a93556e, `zli_ace_v8`): brute candidates = all genes within (1+t) of the smallest size, not only front 0.
- (e9b2c7a, `zli_ace_v9`): no env var; raises the gene CBOR decode depth limit from 100 to 4096 (needed for merged pools).
- **ACE_SPECIALISTS=1** (39a0875, `zli_ace_v10`): after the main search, one extra search per training file; the 10 smallest genes of each go into the brute selection.
- **ACE_SPEC_GENERATIONS=N** (39a0875, `zli_ace_v10`): generations per file search (default 60).
- **ACE_NO_CTIME=1** (fd9723b, `zli_ace_v11`): set the compression-time objective to 0.
- **ACE_TOK_SEEDS=1** (2f66300, `zli_ace_v12`): add 7 tokenize-first prebuilt genes to every numeric search's starting population. Points only.
- **ACE_BRUTE_SLACK=n** (a65d5a4, `zli_ace_v13`): brute picks listed fastest-decoding first; the selector takes the first pick within (1 + n x 1e-4) of the smallest (`ZL_BRUTE_FORCE_SLACK_PARAM`).
- **ACE_DEC_EPS=e** (a65d5a4, `zli_ace_v13`): when picking brute candidates, prefer a faster-decoding candidate within (1+e) of the best next training total.
- **ACE_BRUTE_MIN_GAIN=g** (3aa7587, `zli_ace_v14`): stop adding brute candidates when the next candidate saves less than fraction g of training bytes (replaces the fixed 0.2% threshold; default 0.002).
- **ACE_BRUTE_BALANCE=1** (3aa7587, `zli_ace_v14`): weight each training input chunk by 1/(its file's bytes under the best single candidate), so every training file counts equally in greedy brute selection.
- **ACE_BRUTE_CROSS_SLOT=1** (9a7dff2 + fix 8391fa6, `zli_ace_v15`): each ACE graph's brute picks may also use the front candidates of the other graphs with the same input type (x/y/z of points). Off = v14 output bit for bit.
- **ACE_BRUTE_LAMBDA=l** (71f688f, `zli_ace_v17`): the brute selector picks argmin of size + l [bytes/ms] x rate_i [ns/KiB] x KiB, with per-pick decode rates from training. Params on the `zl.ace` wrapper; this also makes ACE_BRUTE_SLACK reach the selector. Encoder-only.
- **ACE_NO_TRANSFORMER=1** (`ar/ace-dev-port`, `zli_acedev_v2`): drop `graphs::TransformerNumeric` from the ACE graph universe and prebuilt seeds. Unset = dev. Labels: `bestnt_n` = best_n + this; `bestntl3k_n`/`bestntl30k_n`/`bestntl50k_n` = bestnt_n + λ 3000/30000/50000; `ctlnt_n` = upstream settings + this.
- **eNk**: a `bestl5k_n` model with λ set to N×1000 after training (param 7312).
- **C48c / A48c**: 48 clean training files; test files = C / A test files. `C48ch`/`C48cm` etc.: same with `--sort-by-hilbert`/`--sort-by-morton` dumps.
- **ACE_BRUTE_DEC_CAP=f** (f56868a, `zli_ace_v16`): after the first brute pick, skip candidates whose training decode time is more than f x the first pick's. Off = unchanged.

### Binaries (`bins/`)
- `zli_upstream`: v0.2.3 (1730823). `zli_froody`: froody/dev with quars removed (5f94c43).
- `zli_ace_v1` ... `zli_ace_v17`: `ar/ace` builds, listed with the env vars above. `zli_ace_v17` = 71f688f, contains everything; `zli_ace_v14` = 3aa7587 is the recipe binary.
- `zli_updev_5545627`: upstream dev 5545627c. `zli_acedev_v1`: kept features ported to dev (branch `ar/ace-dev-port`, `wt/acedev`, report 21). `pbench_dev`: see pbench.
- `zli_acefr_v1`/`v2`/`v3`: the census / family / seed changes on froody (`wt/acefr`, branch `ar/ace-froody`).
