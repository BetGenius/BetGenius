// Copyright (c) 2023 The Bitcoin Core developers
// Copyright (c) 2024 The Betgenius Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BETGENIUS_TEST_UTIL_JSON_H
#define BETGENIUS_TEST_UTIL_JSON_H

#include <string>

#include <univalue.h>

UniValue read_json(const std::string& jsondata);

#endif // BETGENIUS_TEST_UTIL_JSON_H
