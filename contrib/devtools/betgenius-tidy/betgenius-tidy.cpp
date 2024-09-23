// Copyright (c) 2023 BetGenius Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "logprintf.h"

#include <clang-tidy/ClangTidyModule.h>
#include <clang-tidy/ClangTidyModuleRegistry.h>

class BetGeniusModule final : public clang::tidy::ClangTidyModule
{
public:
    void addCheckFactories(clang::tidy::ClangTidyCheckFactories& CheckFactories) override
    {
        CheckFactories.registerCheck<betgenius::LogPrintfCheck>("betgenius-unterminated-logprintf");
    }
};

static clang::tidy::ClangTidyModuleRegistry::Add<BetGeniusModule>
    X("betgenius-module", "Adds betgenius checks.");

volatile int BetGeniusModuleAnchorSource = 0;
