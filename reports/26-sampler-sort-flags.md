# 26 — What the sampler sort flags change

## Question
Production needs points in Hilbert or Morton order within each node (spatial queries). The order used to build the octree is free. Which dumps match production, and which order compresses best?

## Change
None. Code reading of `pointcloud_sampler` (branch `tbirch-npc-review`) plus a check of the dump files.

## Method
- Read `crates/partitioning/src/node.rs` (`sorted_points`) and `hierarchy.rs` (`sort_points`).
- Checked 3000 points in each of the 3 largest point chunks of file 140 for each flag: fraction of neighbouring pairs whose Morton / Hilbert key does not decrease.
- Compared compressed bytes per point on test files (zli ratios already collected), for upstream v0.2.3 `ctl_n` and our v15 `best_n`, each trained and tested on the same flag.

## Results
Code:
- `node.sorted_points()` runs on every write path. It sorts the node's points by Morton key if `--sort-by-morton`, else by Hilbert key — also when no flag is given.
- `--sort-by-hilbert` / `--sort-by-morton` also sort points before each octree level is built (`sort_points`). This only changes which point each grid cell keeps.

Dump check (fraction of non-decreasing neighbour pairs):

| dumps | Morton | Hilbert |
|---|---|---|
| no flag | 0.49–0.51 | 1.00 |
| `@hilbert` | 0.49–0.51 | 1.00 |
| `@morton` | 1.00 | 0.49–0.51 |

Bytes per point, C/A/B test files total:

| dumps | order in node | build order | ours (v15 `best_n`) | upstream |
|---|---|---|---|---|
| no flag | Hilbert | file order | **2.967** | **3.132** |
| `@hilbert` | Hilbert | Hilbert | 2.973 (+0.2%) | 3.174 (+1.3%) |
| `@morton` | Morton | Morton | 3.040 (+2.5%) | 3.298 (+5.3%) |

pbench decode on `@morton` Cm: ours 0.85× of upstream; on `@hilbert` Ch/Ah: 1.08–1.19× (upstream's `@hilbert` model is slow, ≈200 MB/s).

## Verdict
Use the unflagged dumps. They already are production format (Hilbert within node) and compress best. Hilbert within node beats Morton by 2.5% for our models. The A/B/C and 48-file results of this project stand as production results. The `@hilbert` / `@morton` splits are dropped as main evaluation.

## Caveats
- The point sets differ between flags (different build order → different kept points), so this is not a pure reorder test.
- A seeded random shuffle before the build (unbiased kept point per cell) is untested; it needs a sampler flag.
- One seed per cell.
