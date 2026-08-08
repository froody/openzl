# Spherical Range Image Projection Transform Specification

## Description
The Spherical Range Image Projection Transform converts 3D Cartesian coordinates $(x, y, z)$ into fixed-point spherical coordinates (range $r$, azimuth angle $\theta$, elevation angle $\phi$) and decodes them back to Cartesian coordinates losslessly using a lifting-based reversible integer rotation scheme.

## Inputs
- **Numeric Input Stream**: Contains 3D Cartesian points stored in contiguous triplets $(x_i, y_i, z_i)$ for $i = 0 \dots N/3 - 1$. Supports element widths of 1, 2, 4, or 8 bytes.

## Codec Header
- **1 Byte**: `elementWidth` (1, 2, 4, or 8).

## Outputs
- **Numeric Output Stream**: Contains transformed spherical range projection triplets $(o_{r,i}, o_{\theta,i}, o_{\phi,i})$ for $i = 0 \dots N/3 - 1$, with element width matching the input.
