// Copyright (c) Meta Platforms, Inc. and affiliates.

#include "decode_spherical_range_kernel.h"

#include <math.h>
#include <string.h>

static inline int64_t floor_div(int64_t a, int64_t b)
{
    int64_t q = a / b;
    int64_t r = a % b;
    if (r != 0 && ((a < 0) ^ (b < 0))) {
        q--;
    }
    return q;
}

static inline void decode_point_3d(
        int64_t o_r,
        int64_t o_theta,
        int64_t o_phi,
        int64_t* out_x,
        int64_t* out_y,
        int64_t* out_z,
        double S_theta,
        int64_t M_theta,
        double S_phi,
        int64_t M_phi)
{
    // 1. Unpack triplet
    int64_t r = o_r;

    int64_t y1 = floor_div(o_theta + M_theta / 2, M_theta);
    int64_t theta = o_theta - y1 * M_theta;

    int64_t z1 = floor_div(o_phi + M_phi / 2, M_phi);
    int64_t phi = o_phi - z1 * M_phi;

    // 2. Reverse lifting rotation in XZ plane by phi
    double s_phi = sin((double)phi / S_phi);
    double c_phi = cos((double)phi / S_phi);
    double u_phi = (fabs(s_phi) < 1e-12) ? 0.0 : (1.0 - c_phi) / s_phi;

    int64_t x3 = r + (int64_t)llround((double)z1 * u_phi);
    int64_t z = z1 - (int64_t)llround((double)x3 * s_phi);
    int64_t x2 = x3 + (int64_t)llround((double)z * u_phi);

    // 3. Reverse lifting rotation in XY plane by theta
    double s_theta = sin((double)theta / S_theta);
    double c_theta = cos((double)theta / S_theta);
    double u_theta = (fabs(s_theta) < 1e-12) ? 0.0 : (1.0 - c_theta) / s_theta;

    int64_t x1 = x2 + (int64_t)llround((double)y1 * u_theta);
    int64_t y = y1 - (int64_t)llround((double)x1 * s_theta);
    int64_t x = x1 + (int64_t)llround((double)y * u_theta);

    *out_x = x;
    *out_y = y;
    *out_z = z;
}

void ZS_sphericalRangeDecode8(uint8_t* dst, uint8_t const* src, size_t nbElts)
{
    if (nbElts == 0) return;
    size_t const nbPts = nbElts / 3;
    double const S_theta = 10.0;
    int64_t const M_theta = 64;
    double const S_phi = 10.0;
    int64_t const M_phi = 64;

    for (size_t i = 0; i < nbPts; ++i) {
        int64_t o_r = (int8_t)src[3 * i + 0];
        int64_t o_theta = (int8_t)src[3 * i + 1];
        int64_t o_phi = (int8_t)src[3 * i + 2];
        int64_t x, y, z;
        decode_point_3d(o_r, o_theta, o_phi, &x, &y, &z, S_theta, M_theta, S_phi, M_phi);
        dst[3 * i + 0] = (uint8_t)(int8_t)x;
        dst[3 * i + 1] = (uint8_t)(int8_t)y;
        dst[3 * i + 2] = (uint8_t)(int8_t)z;
    }
    for (size_t i = nbPts * 3; i < nbElts; ++i) {
        dst[i] = src[i];
    }
}

void ZS_sphericalRangeDecode16(uint16_t* dst, uint16_t const* src, size_t nbElts)
{
    if (nbElts == 0) return;
    size_t const nbPts = nbElts / 3;
    double const S_theta = 100.0;
    int64_t const M_theta = 1000;
    double const S_phi = 100.0;
    int64_t const M_phi = 1000;

    for (size_t i = 0; i < nbPts; ++i) {
        int64_t o_r = (int16_t)src[3 * i + 0];
        int64_t o_theta = (int16_t)src[3 * i + 1];
        int64_t o_phi = (int16_t)src[3 * i + 2];
        int64_t x, y, z;
        decode_point_3d(o_r, o_theta, o_phi, &x, &y, &z, S_theta, M_theta, S_phi, M_phi);
        dst[3 * i + 0] = (uint16_t)(int16_t)x;
        dst[3 * i + 1] = (uint16_t)(int16_t)y;
        dst[3 * i + 2] = (uint16_t)(int16_t)z;
    }
    for (size_t i = nbPts * 3; i < nbElts; ++i) {
        dst[i] = src[i];
    }
}

void ZS_sphericalRangeDecode32(uint32_t* dst, uint32_t const* src, size_t nbElts)
{
    if (nbElts == 0) return;
    size_t const nbPts = nbElts / 3;
    double const S_theta = 1000.0;
    int64_t const M_theta = 20000;
    double const S_phi = 1000.0;
    int64_t const M_phi = 20000;

    for (size_t i = 0; i < nbPts; ++i) {
        int64_t o_r = (int32_t)src[3 * i + 0];
        int64_t o_theta = (int32_t)src[3 * i + 1];
        int64_t o_phi = (int32_t)src[3 * i + 2];
        int64_t x, y, z;
        decode_point_3d(o_r, o_theta, o_phi, &x, &y, &z, S_theta, M_theta, S_phi, M_phi);
        dst[3 * i + 0] = (uint32_t)(int32_t)x;
        dst[3 * i + 1] = (uint32_t)(int32_t)y;
        dst[3 * i + 2] = (uint32_t)(int32_t)z;
    }
    for (size_t i = nbPts * 3; i < nbElts; ++i) {
        dst[i] = src[i];
    }
}

void ZS_sphericalRangeDecode64(uint64_t* dst, uint64_t const* src, size_t nbElts)
{
    if (nbElts == 0) return;
    size_t const nbPts = nbElts / 3;
    double const S_theta = 1000.0;
    int64_t const M_theta = 20000;
    double const S_phi = 1000.0;
    int64_t const M_phi = 20000;

    for (size_t i = 0; i < nbPts; ++i) {
        int64_t o_r = (int64_t)src[3 * i + 0];
        int64_t o_theta = (int64_t)src[3 * i + 1];
        int64_t o_phi = (int64_t)src[3 * i + 2];
        int64_t x, y, z;
        decode_point_3d(o_r, o_theta, o_phi, &x, &y, &z, S_theta, M_theta, S_phi, M_phi);
        dst[3 * i + 0] = (uint64_t)x;
        dst[3 * i + 1] = (uint64_t)y;
        dst[3 * i + 2] = (uint64_t)z;
    }
    for (size_t i = nbPts * 3; i < nbElts; ++i) {
        dst[i] = src[i];
    }
}

void ZS_sphericalRangeDecode(
        void* dst,
        void const* src,
        size_t nbElts,
        size_t eltWidth)
{
    if (nbElts == 0) return;
    switch (eltWidth) {
        case 1:
            ZS_sphericalRangeDecode8((uint8_t*)dst, (uint8_t const*)src, nbElts);
            break;
        case 2:
            ZS_sphericalRangeDecode16((uint16_t*)dst, (uint16_t const*)src, nbElts);
            break;
        case 4:
            ZS_sphericalRangeDecode32((uint32_t*)dst, (uint32_t const*)src, nbElts);
            break;
        case 8:
            ZS_sphericalRangeDecode64((uint64_t*)dst, (uint64_t const*)src, nbElts);
            break;
        default:
            memcpy(dst, src, nbElts * eltWidth);
            break;
    }
}
