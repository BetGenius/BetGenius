// Copyright (c) 2021-2022 The Raven Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <crypto/ethash/include/ethash/hash_types.h>

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define KAWPOW_REVISION "23"

#define KAWPOW_EPOCH_LENGTH 7500
#define KAWPOW_LIGHT_CACHE_ITEM_SIZE 64
#define KAWPOW_FULL_DATASET_ITEM_SIZE 128
#define KAWPOW_NUM_DATASET_ACCESSES 64


struct ethash_epoch_context
{
    const int epoch_number;
    const int light_cache_num_items;
    const union ethash_hash512* const light_cache;
    const uint32_t* const l1_cache;
    const int full_dataset_num_items;
};


struct ethash_epoch_context_full;


struct ethash_result
{
    union ethash_hash256 final_hash;
    union ethash_hash256 hashMix;
};


/**
 * Calculates the number of items in the light cache for given epoch.
 *
 * This function will search for a prime number matching the criteria given
 * by the KAWPoW so the execution time is not constant. It takes ~ 0.01 ms.
 *
 * @param epoch_number  The epoch number.
 * @return              The number items in the light cache.
 */
int ethash_calculate_light_cache_num_items(int epoch_number) NOEXCEPT;


/**
 * Calculates the number of items in the full dataset for given epoch.
 *
 * This function will search for a prime number matching the criteria given
 * by the KAWPoW so the execution time is not constant. It takes ~ 0.05 ms.
 *
 * @param epoch_number  The epoch number.
 * @return              The number items in the full dataset.
 */
int ethash_calculate_full_dataset_num_items(int epoch_number) NOEXCEPT;

/**
 * Calculates the epoch seed hash.
 * @param epoch_number  The epoch number.
 * @return              The epoch seed hash.
 */
union ethash_hash256 ethash_calculate_epoch_seed(int epoch_number) NOEXCEPT;


struct ethash_epoch_context* ethash_create_epoch_context(int epoch_number) NOEXCEPT;

/**
 * Creates the epoch context with the full dataset initialized.
 *
 * The memory for the full dataset is only allocated and marked as "not-generated".
 * The items of the full dataset are generated on the fly when hit for the first time.
 *
 * The memory allocated in the context MUST be freed with ethash_destroy_epoch_context_full().
 *
 * @param epoch_number  The epoch number.
 * @return  Pointer to the context or null in case of memory allocation failure.
 */
struct ethash_epoch_context_full* ethash_create_epoch_context_full(int epoch_number) NOEXCEPT;

void ethash_destroy_epoch_context(struct ethash_epoch_context* context) NOEXCEPT;

void ethash_destroy_epoch_context_full(struct ethash_epoch_context_full* context) NOEXCEPT;


/**
 * Get global shared epoch context.
 */
const struct ethash_epoch_context* ethash_get_global_epoch_context(int epoch_number) NOEXCEPT;

/**
 * Get global shared epoch context with full dataset initialized.
 */
const struct ethash_epoch_context_full* ethash_get_global_epoch_context_full(
    int epoch_number) NOEXCEPT;


struct ethash_result ethash_hash(const struct ethash_epoch_context* context,
    const union ethash_hash256* header_hash, uint64_t nonce) NOEXCEPT;

bool ethash_verify(const struct ethash_epoch_context* context,
    const union ethash_hash256* header_hash, const union ethash_hash256* hashMix, uint64_t nonce,
    const union ethash_hash256* boundary) NOEXCEPT;

bool ethash_verify_final_hash(const union ethash_hash256* header_hash,
    const union ethash_hash256* hashMix, uint64_t nonce,
    const union ethash_hash256* boundary) NOEXCEPT;

#ifdef __cplusplus
}
#endif
