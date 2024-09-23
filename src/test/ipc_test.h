// Copyright (c) 2023 The Bitcoin Core developers
// Copyright (c) 2024 The Betgenius Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BETGENIUS_TEST_IPC_TEST_H
#define BETGENIUS_TEST_IPC_TEST_H

#include <primitives/transaction.h>
#include <univalue.h>

class FooImplementation
{
public:
    int add(int a, int b) { return a + b; }
    COutPoint passOutPoint(COutPoint o) { return o; }
    UniValue passUniValue(UniValue v) { return v; }
};

void IpcTest();

#endif // BETGENIUS_TEST_IPC_TEST_H
