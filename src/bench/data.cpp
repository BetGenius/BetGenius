// Copyright (c) 2019-2021 The Bitcoin Core developers
// Copyright (c) 2024 The Betgenius Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <bench/data.h>

namespace benchmark {
namespace data {

#include <bench/data/block0.raw.h>
const std::vector<uint8_t> block0{std::begin(block0_raw), std::end(block0_raw)};

} // namespace data
} // namespace benchmark
