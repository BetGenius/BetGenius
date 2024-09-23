// Copyright (c) 2021-2022 The Raven Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#ifdef _MSC_VER
#include <intrin.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns the number of leading 0-bits in `x`, starting at the most significant bit position.
 * If `x` is 0, the result is undefined.
 */
static inline int __builtin_clz(unsigned int x)
{
    unsigned long most_significant_bit;
    _BitScanReverse(&most_significant_bit, x);
    return 31 - (int)most_significant_bit;
}

/**
 * Returns the number of 1-bits in `x`.
 */
static inline int __builtin_popcount(unsigned int x)
{
    return (int)__popcnt(x);
}

#ifdef __cplusplus
}
#endif

#endif
