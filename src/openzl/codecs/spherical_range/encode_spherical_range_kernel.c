// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "encode_spherical_range_kernel.h"

#include <math.h>
#include <string.h>

static inline void encode_point_3d(
        int64_t x,
        int64_t y,
        int64_t z,
        int64_t* out_r,
        int64_t* out_theta,
        int64_t* out_phi,
        double S_theta,
        int64_t M_theta,
        double S_phi,
        int64_t M_phi)
{
    // 1. Azimuth angle theta
    double theta_rad = atan2((double)y, (double)x);
    int64_t theta = (int64_t)llround(theta_rad * S_theta);

    // 2. Lifting rotation in XY plane by theta
    double s_theta = sin((double)theta / S_theta);
    double c_theta = cos((double)theta / S_theta);
    double u_theta = (fabs(s_theta) < 1e-12) ? 0.0 : (1.0 - c_theta) / s_theta;

    int64_t x1 = x - (int64_t)llround((double)y * u_theta);
    int64_t y1 = y + (int64_t)llround((double)x1 * s_theta);
    int64_t x2 = x1 - (int64_t)llround((double)y1 * u_theta);

    // 3. Elevation angle phi
    double phi_rad = atan2((double)z, (double)x2);
    int64_t phi = (int64_t)llround(phi_rad * S_phi);

    // 4. Lifting rotation in XZ plane by phi
    double s_phi = sin((double)phi / S_phi);
    double c_phi = cos((double)phi / S_phi);
    double u_phi = (fabs(s_phi) < 1e-12) ? 0.0 : (1.0 - c_phi) / s_phi;

    int64_t x3 = x2 - (int64_t)llround((double)z * u_phi);
    int64_t z1 = z + (int64_t)llround((double)x3 * s_phi);
    int64_t r = x3 - (int64_t)llround((double)z1 * u_phi);

    // 5. Output triplet
    *out_r = r;
    *out_theta = theta + y1 * M_theta;
    *out_phi = phi + z1 * M_phi;
}

void ZS_sphericalRangeEncode8(uint8_t* dst, uint8_t const* src, size_t nbElts)
{
    if (nbElts == 0) return;
    size_t const nbPts = nbElts / 3;
    double const S_theta = 10.0;
    int64_t const M_theta = 64;
    double const S_phi = 10.0;
    int64_t const M_phi = 64;

    for (size_t i = 0; i < nbPts; ++i) {
        int64_t x = (int8_t)src[3 * i + 0];
        int64_t y = (int8_t)src[3 * i + 1];
        int64_t z = (int8_t)src[3 * i + 2];
        int64_t r, theta, phi;
        encode_point_3d(x, y, z, &r, &theta, &phi, S_theta, M_theta, S_phi, M_phi);
        dst[3 * i + 0] = (uint8_t)(int8_t)r;
        dst[3 * i + 1] = (uint8_t)(int8_t)theta;
        dst[3 * i + 2] = (uint8_t)(int8_t)phi;
    }
    for (size_t i = nbPts * 3; i < nbElts; ++i) {
        dst[i] = src[i];
    }
}

void ZS_sphericalRangeEncode16(uint16_t* dst, uint16_t const* src, size_t nbElts)
{
    if (nbElts == 0) return;
    size_t const nbPts = nbElts / 3;
    double const S_theta = 100.0;
    int64_t const M_theta = 1000;
    double const S_phi = 100.0;
    int64_t const M_phi = 1000;

    for (size_t i = 0; i < nbPts; ++i) {
        int64_t x = (int16_t)src[3 * i + 0];
        int64_t y = (int16_t)src[3 * i + 1];
        int64_t z = (int16_t)src[3 * i + 2];
        int64_t r, theta, phi;
        encode_point_3d(x, y, z, &r, &theta, &phi, S_theta, M_theta, S_phi, M_phi);
        dst[3 * i + 0] = (uint16_t)(int16_t)r;
        dst[3 * i + 1] = (uint16_t)(int16_t)theta;
        dst[3 * i + 2] = (uint16_t)(int16_t)phi;
    }
    for (size_t i = nbPts * 3; i < nbElts; ++i) {
        dst[i] = src[i];
    }
}

void ZS_sphericalRangeEncode32(uint32_t* dst, uint32_t const* src, size_t nbElts)
{
    if (nbElts == 0) return;
    size_t const nbPts = nbElts / 3;
    double const S_theta = 1000.0;
    int64_t const M_theta = 20000;
    double const S_phi = 1000.0;
    int64_t const M_phi = 20000;

    for (size_t i = 0; i < nbPts; ++i) {
        int64_t x = (int32_t)src[3 * i + 0];
        int64_t y = (int32_t)src[3 * i + 1];
        int64_t z = (int32_t)src[3 * i + 2];
        int64_t r, theta, phi;
        encode_point_3d(x, y, z, &r, &theta, &phi, S_theta, M_theta, S_phi, M_phi);
        dst[3 * i + 0] = (uint32_t)(int32_t)r;
        dst[3 * i + 1] = (uint32_t)(int32_t)theta;
        dst[3 * i + 2] = (uint32_t)(int32_t)phi;
    }
    for (size_t i = nbPts * 3; i < nbElts; ++i) {
        dst[i] = src[i];
    }
}

void ZS_sphericalRangeEncode64(uint64_t* dst, uint64_t const* src, size_t nbElts)
{
    if (nbElts == 0) return;
    size_t const nbPts = nbElts / 3;
    double const S_theta = 1000.0;
    int64_t const M_theta = 20000;
    double const S_phi = 1000.0;
    int64_t const M_phi = 20000;

    for (size_t i = 0; i < nbPts; ++i) {
        int64_t x = (int64_t)src[3 * i + 0];
        int64_t y = (int64_t)src[3 * i + 1];
        int64_t z = (int64_t)src[3 * i + 2];
        int64_t r, theta, phi;
        encode_point_3d(x, y, z, &r, &theta, &phi, S_theta, M_theta, S_phi, M_phi);
        dst[3 * i + 0] = (uint64_t)r;
        dst[3 * i + 1] = (uint64_t)theta;
        dst[3 * i + 2] = (uint64_t)phi;
    }
    for (size_t i = nbPts * 3; i < nbElts; ++i) {
        dst[i] = src[i];
    }
}

void ZS_sphericalRangeEncode(
        void* dst,
        void const* src,
        size_t nbElts,
        size_t eltWidth)
{
    if (nbElts == 0) return;
    switch (eltWidth) {
        case 1:
            ZS_sphericalRangeEncode8((uint8_t*)dst, (uint8_t const*)src, nbElts);
            break;
        case 2:
            ZS_sphericalRangeEncode16((uint16_t*)dst, (uint16_t const*)src, nbElts);
            break;
        case 4:
            ZS_sphericalRangeEncode32((uint32_t*)dst, (uint32_t const*)src, nbElts);
            break;
        case 8:
            ZS_sphericalRangeEncode64((uint64_t*)dst, (uint64_t const*)src, nbElts);
            break;
        default:
            memcpy(dst, src, nbElts * eltWidth);
            break;
    }
}
