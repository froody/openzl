# Octree Occupancy Byte-Mask Encoding Specification

## Overview

The Octree Occupancy transform encodes 3D point cloud geometry by recursively subdividing the 3D bounding box into an 8-child octree structure. For every occupied node in the octree, an 8-bit occupancy mask byte is emitted, indicating which child octants contain one or more points. Upon reaching leaf voxels, the 3D voxel coordinates and point ordering are reconstructed losslessly.

## Wire Format

### Header

| Offset | Width | Description |
|--------|-------|-------------|
| 0 | 4 | Magic Number (`0x4F435452` / `"OCTR"`) |
| 4 | 1 | Component Width $W \in \{1, 2, 4, 8\}$ |
| 5 | 1 | Flags (bit 0: `is_identity_order`) |
| 6.. | Varint | Point Count $N$ |
| .. | Varint | Remainder Byte Count $R$ |

### Geometry Data (if $N > 0$)

| Width | Description |
|-------|-------------|
| $W$ | Minimum $X$ coordinate ($X_{min}$) |
| $W$ | Minimum $Y$ coordinate ($Y_{min}$) |
| $W$ | Minimum $Z$ coordinate ($Z_{min}$) |
| 1 | Octree Depth $D$ |
| Varint | Number of Occupancy Masks $M_{cnt}$ |
| $M_{cnt}$ bytes | Array of 8-bit occupancy mask bytes |

### Index Mapping (if $N > 0$ and `is_identity_order == 0`)

| Width | Description |
|-------|-------------|
| 1 | Index Element Width $I_W \in \{1, 2, 4\}$ |
| $N \times I_W$ | Array of leaf voxel indices $\pi(i) \in [0, K-1]$ for each point $i$ |

### Remainder Data (if $R > 0$)

| Width | Description |
|-------|-------------|
| $R$ bytes | Unaligned trailing bytes |

## Decoding Algorithm

1. Verify magic number `0x4F435452`.
2. Read $W$, $N$, $R$.
3. If $N > 0$, read bounding box ($X_{min}, Y_{min}, Z_{min}$), octree depth $D$, $M_{cnt}$, and occupancy masks.
4. Perform BFS traversal starting from root node $(0,0,0)$ of size $2^D$. For each node, read occupancy mask byte and expand active child octants.
5. Collect leaf voxel relative coordinates $(vx_k, vy_k, vz_k)$ for $k = 0 \dots K-1$.
6. Reconstruct point $i$ coordinates $(X_i, Y_i, Z_i) = (X_{min} + vx_{\pi(i)}, Y_{min} + vy_{\pi(i)}, Z_{min} + vz_{\pi(i)})$.
7. Append $R$ trailing bytes.
