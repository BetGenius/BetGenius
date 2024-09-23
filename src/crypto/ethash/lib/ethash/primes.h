// Copyright (c) 2021-2022 The Raven Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <crypto/ethash/include/ethash/ethash.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Finds the largest prime number not greater than the provided upper bound.
 *
 * @param upper_bound  The upper bound. SHOULD be greater than 1.
 * @return  The largest prime number `p` such `p <= upper_bound`.
 *          In case `upper_bound <= 1`, returns 0.
 */
int ethash_find_largest_prime(int upper_bound) NOEXCEPT;

#ifdef __cplusplus
}
#endif
