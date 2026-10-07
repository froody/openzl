# Port of ACE training changes onto upstream dev

Branch `ar/ace-dev-port` in `/mnt/data-3/tbirch/home/ozl-ar/wt/acedev`, based on `origin/dev` 5545627c. Not pushed.
Binary: `/mnt/data-3/tbirch/home/ozl-ar/bins/zli_acedev_v1` (built from HEAD).

## Commits (oldest last)
```
d49e1eb8 ace: ACE_SPECIALISTS per-training-file searches merged into the brute pool; raise ACECompressor CBOR decode depth
cd1530d9 ace: ACE_BRUTE_K complementary picks under the brute-force selector (ACE_BRUTE_TOL, ACE_BRUTE_MIN_GAIN, ACE_BRUTE_K1_OUT, ACE_BRUTE_SLACK, ACE_BRUTE_LAMBDA); params on the zl.ace wrapper
8c7fcc94 brute_force selector: optional slack (7311) and decode-cost lambda (7312, rates 7320+i) local params
09b439cf ace: ACE_TOK_SEEDS tokenize-first prebuilt seed genes
cb63d10d ace: ACE_SEED, ACE_MAX_GENERATIONS, ACE_CENSUS, ACE_SIZE_ONLY, ACE_NO_CTIME search knobs
```
cb63d10d and 09b439cf come from the earlier run that died. I reviewed them and kept them unchanged.

## Feature-to-file map
| Feature | Files |
|---|---|
| ACE_SEED, ACE_MAX_GENERATIONS | `tools/training/ace/ace.cpp` (`aceParameters`) |
| ACE_CENSUS | `ace.cpp` (`logCensus`, called from `runTraining`) |
| ACE_SIZE_ONLY, ACE_NO_CTIME | `tools/training/ace/automated_compressor_explorer.cpp` (`computeFitness`) |
| ACE_TOK_SEEDS | `tools/training/ace/ace_compressors.cpp` (`makePrebuiltNumericCompressors`) |
| ACE_BRUTE_K / TOL / MIN_GAIN | `ace.cpp` (`pickComplementary`, `makeBruteMutations`, `ACEBruteMutation`) |
| ACE_BRUTE_K1_OUT | `ace.cpp` (`ACETrainer::train`: a second `MergedParetoFrontier` over the plain smallest candidates, written to the path) |
| ACE_BRUTE_SLACK / ACE_BRUTE_LAMBDA | selector: `src/openzl/compress/selectors/selector_brute_force.{c,h}` (params 7311, 7312, 7320+i); training: `ace.cpp` |
| ACE_SPECIALISTS / ACE_SPEC_GENERATIONS | `ace.cpp` (`addSpecialists`); `automated_compressor_explorer.{h,cpp}` (`savePopulation(limit)`); `tools/training/utils/utils.h` (`MultiInput::group`); `tools/training/sample_collection/training_sample_collector.cpp` (keeps the group on collected streams) |
| CBOR decode depth (e9b2c7a1) | `tools/training/ace/ace_compressor.cpp` (maxDepth 100 -> 4096) |
| Use-after-free fix (8391fa61) | Not needed as code: see below |

## Decisions where upstream overlapped
- **Brute graph (#1011).** Upstream now has a public, serializable `ZL_GRAPH_BRUTE_FORCE` (`!zl.brute_force`) and `graphs::BruteForce`. I use it in place of our private `ZL_PrivateStandardGraphID_brute_force`, so `graph_registry.c` and `private_nodes.h` are not touched. The selector reads params with `ZL_Selector_getLocalIntParam`. Upstream's `GM_overrideBaseGraph` copies the new base's description into the `zl.ace` graph. `BackendGraphMutation::mutate` then calls `ZL_Compressor_overrideGraphParams` on that wrapper. So `ACEBruteMutation::newGraphParams` attaches the slack, lambda and rate ints to the **`zl.ace` wrapper** (with the ACE state when `saveAceState` is set). Checked in the serialized model: `zl.ace#0/3/4` have `base: zl.brute_force#N`, and their params hold `{7312: 3000, 7320..7323: rates}`. The selector change is encoder-only: the frame format does not change.
- **Selector scratch.** Our old selector used a fixed `size_t sizes[64]`. The port uses `ZL_Selector_getScratchSpace`, and only when slack or lambda is set. With no params, the code path is upstream's (plus two param lookups).
- **ace_combination.cpp is gone.** Upstream moved the generic code to `utils/pareto_combination.cpp`, which uses `BackendGraphMutation`s. Our `runBruteReplacements` is now a `BackendGraphMutation` subclass (`ACEBruteMutation`), placed in `ace.cpp` and fed into the normal `MergedParetoFrontier`. Brute mode is used only without `--pareto-frontier`, as before. A slot with exactly one pick gets a normal `ACEBackendGraphMutation`. A slot with no pick falls back to the normal candidates.
- **`--max-num-candidates` (#988)** only limits the number of final Pareto-frontier compressors. It does not put several genes under one selector, so it is not the same as ACE_BRUTE_K. I ported ours.
- **Shared ACE time budget (#976).** Kept. ACE_SPECIALISTS searches run with `maxTime = nullopt` and are capped only by ACE_SPEC_GENERATIONS (default 60). They do not use the shared budget.
- **Use-after-free (8391fa61).** In upstream's new `ACETrainer::train`, the explorer, its flattened inputs and the brute picking all live in the same loop iteration. Picks are copied out as `ACECompressor` values before the iteration ends. The old bug (explorers outliving their inputs) cannot happen, so no extra ownership code is needed. Specialist sub-inputs live as long as their explorer.
- **Benchmarks.** `pickComplementary` benchmarks with the explorer's `formatVersion()` and `compressionLevel()` (upstream #946), so picks respect the format version.

## Smoke results (20 chunks of `points/533084_6793800-dcmGP6Y9h97vy`, raw 4,657,848 B)
Every compressed chunk round-trip below was bit-exact with **both** stock decoders: upstream v0.2.3 (`wt/upstream/build/cli/zli`) and upstream dev (`bins/zli_updev_5545627`).

| Model | Compressed total | Round-trip v0.2.3 | Round-trip dev |
|---|---|---|---|
| Full smoke env (ACE_BRUTE_LAMBDA=3000), `/tmp/port/smoke.zlc` | 991,450 | 20/20 | 20/20 |
| Same model, lambda set to 0 (param 7312 edited in the CBOR) | 991,206 | 20/20 | 20/20 |
| Same model, lambda set to 1e8 | 1,399,997 | 20/20 | 20/20 |
| Same model, lambda replaced by slack 7311=2000 | 991,558 | 20/20 | 20/20 |
| Every env var unset (plain dev), `--max-time-secs 240` | 1,302,435 | 20/20 | 20/20 |
| 2 generations, SEED/CENSUS/SIZE_ONLY/BRUTE_K=3/SLACK=50 | 1,302,424 | 20/20 | 20/20 |
| ...its ACE_BRUTE_K1_OUT control | 1,302,424 | 20/20 | 20/20 |

- Smoke training took 161 s. Its log shows ACE_SPEC lines for both training-file groups in each of the 5 slots, and ACE_BRUTE picks for every ACE slot:
  - `zl.ace#0`: 4 picks (1,700,846 -> 1,679,205 train bytes);
  - `zl.ace#3`: 4 picks (1,693,902 -> 1,670,718);
  - `zl.ace#4`: 4 picks (1,648,552 -> 1,604,258);
  - `zl.ace#1` and `#2` (small slots): 1 pick each. They stop because a second pick gains nothing.
- For the lambda test I edited only param 7312 in the same model, so the search result is identical across the lambda runs. Lambda 1e8 grows the output by 41% compared with lambda 0.
- **Plain mode note:** the default is 250 generations, which is too long for a smoke run. So the plain run used upstream's own `--max-time-secs 240` flag and no ACE_* env vars. It trained (no ACE_* log lines) and round-tripped. I did not compare plain mode byte-for-byte against `zli_updev_5545627` training: the GA uses timing objectives, so two runs are not deterministic.
- With SIZE_ONLY the front has a single gene, so the 2-generation run picked 1 gene per slot. That output equals its K1 control, as expected.

## Not ported
- As instructed: ACE_NODES/ACE_FAMILY, ACE_ISLANDS, ACE_POLISH, ACE_POOL, ACE_DEC_EPS, ACE_BRUTE_BALANCE, ACE_BRUTE_CROSS_SLOT, ACE_BRUTE_DEC_CAP, and `.bak` files.
- `pickComplementary` therefore lost its `groupOf` (BALANCE) and `extra` (CROSS_SLOT) arguments.
- Our private `!zl.private.brute_force` standard graph was replaced by upstream's public one (see above).
- Intermediate commits were not built separately. Only HEAD was built and tested.
