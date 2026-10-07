// Copyright (c) Meta Platforms, Inc. and affiliates.

#ifndef ZSTRONG_COMPRESS_SELECTORS_SELECTOR_BRUTE_FORCE_H
#define ZSTRONG_COMPRESS_SELECTORS_SELECTOR_BRUTE_FORCE_H

#include "openzl/shared/portability.h"
#include "openzl/zl_selector.h"

ZL_BEGIN_C_DECLS

/// Optional int local params of the graph running the brute force selector.
/// SLACK (units of 1e-4): successors are listed fastest-decoding first, so take
/// the first one whose size is within (1 + slack) of the smallest.
/// LAMBDA (bytes per ms of decode time) with RATE_PARAM_BASE + i (successor i's
/// decode cost in ns per KiB of input): take argmin size + lambda * time.
/// LAMBDA takes precedence over SLACK.
#define ZL_BRUTE_FORCE_SLACK_PARAM 7311
#define ZL_BRUTE_FORCE_LAMBDA_PARAM 7312
#define ZL_BRUTE_FORCE_RATE_PARAM_BASE 7320

ZL_GraphID SI_selector_brute_force(
        const ZL_Selector* selCtx,
        const ZL_Input* inputStream,
        const ZL_GraphID* customGraphs,
        size_t nbCustomGraphs);

ZL_END_C_DECLS

#endif
