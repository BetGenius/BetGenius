// Copyright (c) 2021-2022 The Raven Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

union ethash_hash256
{
    uint64_t word64s[4];
    uint32_t word32s[8];
    uint8_t bytes[32];
    char str[32];
};

union ethash_hash512
{
    uint64_t word64s[8];
    uint32_t word32s[16];
    uint8_t bytes[64];
    char str[64];
};

union ethash_hash1024
{
    union ethash_hash512 hash512s[2];
    uint64_t word64s[16];
    uint32_t word32s[32];
    uint8_t bytes[128];
    char str[128];
};

union ethash_hash2048
{
    union ethash_hash512 hash512s[4];
    uint64_t word64s[32];
    uint32_t word32s[64];
    uint8_t bytes[256];
    char str[256];
};

#ifdef __cplusplus
}
#endif
