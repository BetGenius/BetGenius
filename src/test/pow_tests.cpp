// Copyright (c) 2015-2022 The Bitcoin Core developers
// Copyright (c) 2024 The Betgenius Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chain.h>
#include <chainparams.h>
#include <pow.h>
#include <test/util/random.h>
#include <test/util/setup_common.h>
#include <util/chaintype.h>

#include <boost/test/unit_test.hpp>

const std::vector<std::pair<uint32_t, uint32_t>> blockIndexData = {
    {1712232000, 0x1f0affff}, {1712232030, 0x1f0affff}, {1712232060, 0x1f0affff}, {1712232090, 0x1f0affff}, {1712232120, 0x1f0affff}, {1712232150, 0x1f0affff}, 
    {1712232180, 0x1f0affff}, {1712232210, 0x1f0affff}, {1712232240, 0x1f0affff}, {1712232270, 0x1f0affff}, {1712232300, 0x1f0affff}, {1712232330, 0x1f0affff},
    {1712232360, 0x1f0affff}, {1712232390, 0x1f0affff}, {1712232420, 0x1f0affff}, {1712232450, 0x1f0affff}, {1712232480, 0x1f0affff}, {1712232510, 0x1f0affff},
    {1712232540, 0x1f0affff}, {1712232570, 0x1f0affff}, {1712232600, 0x1f0affff}, {1712232630, 0x1f053999}, {1712232660, 0x1f04f6b7}, {1712232690, 0x1f04d240},
    {1712232720, 0x1f04ada6}, {1712232750, 0x1f048837}, {1712232780, 0x1f0461ea}, {1712232810, 0x1f043abb}, {1712232840, 0x1f0412a4}, {1712232870, 0x1f03e99f},
    {1712232900, 0x1f03bfa8}, {1712232930, 0x1f0394b7}, {1712232960, 0x1f0368c9}, {1712232990, 0x1f033bd7}, {1712233020, 0x1f030dda}, {1712233050, 0x1f02decd},
    {1712233080, 0x1f02aea9}, {1712233110, 0x1f027d69}, {1712233140, 0x1f024b05}, {1712233170, 0x1f021776}, {1712233200, 0x1f01e2b6}, {1712233230, 0x1f01acbe},
    {1712233260, 0x1f0196f6}, {1712233290, 0x1f0182ef}, {1712233320, 0x1f016f50}, {1712233350, 0x1f015c17}, {1712233380, 0x1f014949}, {1712233410, 0x1f0136ee},
    {1712233440, 0x1f01250f}, {1712233470, 0x1f0113b3}, {1712233500, 0x1f0102e3}, {1712233530, 0x1f00f2a8}, {1712233560, 0x1f00e30b}, {1712233590, 0x1f00d415},
    {1712233620, 0x1f00c5d1}, {1712233650, 0x1f00b849}, {1712233680, 0x1f00ab87}, {1712233710, 0x1f009f96}, {1712233740, 0x1f009482}, {1712233770, 0x1f008a57},
    {1712233800, 0x1f008120}, {1712233830, 0x1e78ebd7}, {1712233860, 0x1e71c5e8}, {1712233890, 0x1e6afad1}, {1712233920, 0x1e647e6a}, {1712233950, 0x1e5e4fda},
    {1712233980, 0x1e586e8f}, {1712234010, 0x1e52d9db}, {1712234040, 0x1e4d90db}, {1712234070, 0x1e489272}, {1712234100, 0x1e43dd54}, {1712234130, 0x1e3f6ff5},
    {1712234160, 0x1e3b488f}, {1712234190, 0x1e376519}, {1712234220, 0x1e33c34a}, {1712234250, 0x1e30608a}, {1712234280, 0x1e2d39f6}, {1712234310, 0x1e2a4c60},
    {1712234340, 0x1e279445}, {1712234370, 0x1e250dc6}, {1712234400, 0x1e22b4a8}, {1712234430, 0x1e208454}, {1712234460, 0x1e1e77c0}, {1712234490, 0x1e1c8982},
    {1712234520, 0x1e1ab81c}, {1712234550, 0x1e190264}, {1712234580, 0x1e176732}, {1712234610, 0x1e15e559}, {1712234640, 0x1e147ba9}, {1712234670, 0x1e1328f1},
    {1712234700, 0x1e11ec00}, {1712234730, 0x1e10c3a4}, {1712234760, 0x1e0faead}, {1712234790, 0x1e0eabf1}, {1712234820, 0x1e0dba48}, {1712234850, 0x1e0cd893},
    {1712234880, 0x1e0c05ba}, {1712234910, 0x1e0b40b0}, {1712234940, 0x1e0a8877}, {1712234970, 0x1e09dc1b}, {1712235000, 0x1e093abe}, {1712235030, 0x1e08a392},
    {1712235060, 0x1e0815e3}, {1712235090, 0x1e079114}, {1712235120, 0x1e0714a5}, {1712235150, 0x1e06a01d}, {1712235180, 0x1e063307}, {1712235210, 0x1e05ccf1},
    {1712235240, 0x1e056d6f}, {1712235270, 0x1e051418}, {1712235300, 0x1e04c089}
};

const std::vector<std::unique_ptr<CBlockIndex>> GenerateBlockIndexes(const std::vector<std::pair<uint32_t, uint32_t>>& blockIndexData)
{
    std::vector<std::unique_ptr<CBlockIndex>> blockIndexes;

    for (size_t i = 0; i < blockIndexData.size(); ++i) {
        auto height = i;
        auto time = blockIndexData[i].first;
        auto bits = blockIndexData[i].second;
        CBlockIndex* prevBlock = (i == 0) ? nullptr : blockIndexes.back().get();

        auto blockIndex = std::make_unique<CBlockIndex>();
        blockIndex->nHeight = height;
        blockIndex->nTime = time;
        blockIndex->nBits = bits;
        blockIndex->pprev = prevBlock;

        blockIndexes.push_back(std::move(blockIndex));
    }

    return blockIndexes;
}

BOOST_FIXTURE_TEST_SUITE(pow_tests, BasicTestingSetup)

/* Test calculation of next difficulty target with 30 second block times */
BOOST_AUTO_TEST_CASE(get_next_work)
{
    const auto chainParams = CreateChainParams(ChainType::MAIN);
    const auto blockIndexes = GenerateBlockIndexes(blockIndexData);

    for (const auto &blockIndex : blockIndexes) {
        uint32_t nBits = CalculateNextWorkRequired(blockIndex->pprev, chainParams->GetConsensus());

        BOOST_CHECK_EQUAL(nBits, blockIndex->nBits);
        BOOST_CHECK(PermittedDifficultyTransition(chainParams->GetConsensus(), blockIndex->nBits, nBits));
    }
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_negative_target)
{
    const auto consensus = CreateChainParams(ChainType::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nBits;
    nBits = UintToArith256(consensus.powLimit).GetCompact(true);
    hash.SetHex("0x1");
    BOOST_CHECK(!CheckProofOfWork(hash, nBits, consensus));
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_overflow_target)
{
    const auto consensus = CreateChainParams(ChainType::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nBits{~0x00800000U};
    hash.SetHex("0x1");
    BOOST_CHECK(!CheckProofOfWork(hash, nBits, consensus));
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_too_easy_target)
{
    const auto consensus = CreateChainParams(ChainType::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nBits;
    arith_uint256 nBits_arith = UintToArith256(consensus.powLimit);
    nBits_arith *= 2;
    nBits = nBits_arith.GetCompact();
    hash.SetHex("0x1");
    BOOST_CHECK(!CheckProofOfWork(hash, nBits, consensus));
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_biger_hash_than_target)
{
    const auto consensus = CreateChainParams(ChainType::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nBits;
    arith_uint256 hash_arith = UintToArith256(consensus.powLimit);
    nBits = hash_arith.GetCompact();
    hash_arith *= 2; // hash > nBits
    hash = ArithToUint256(hash_arith);
    BOOST_CHECK(!CheckProofOfWork(hash, nBits, consensus));
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_zero_target)
{
    const auto consensus = CreateChainParams(ChainType::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nBits;
    arith_uint256 hash_arith{0};
    nBits = hash_arith.GetCompact();
    hash = ArithToUint256(hash_arith);
    BOOST_CHECK(!CheckProofOfWork(hash, nBits, consensus));
}

BOOST_AUTO_TEST_CASE(GetBlockProofEquivalentTime_test)
{
    const auto chainParams = CreateChainParams(ChainType::MAIN);
    std::vector<CBlockIndex> blocks(10000);
    for (int i = 0; i < 10000; i++) {
        blocks[i].pprev = i ? &blocks[i - 1] : nullptr;
        blocks[i].nHeight = i;
        blocks[i].nTime = 1712232000 + i * chainParams->GetConsensus().nPowTargetSpacing;
        blocks[i].nBits = 0x207fffff; /* target 0x7fffff000... */
        blocks[i].nChainWork = i ? blocks[i - 1].nChainWork + GetBlockProof(blocks[i - 1]) : arith_uint256(0);
    }

    for (int j = 0; j < 1000; j++) {
        CBlockIndex *p1 = &blocks[InsecureRandRange(10000)];
        CBlockIndex *p2 = &blocks[InsecureRandRange(10000)];
        CBlockIndex *p3 = &blocks[InsecureRandRange(10000)];

        int64_t tdiff = GetBlockProofEquivalentTime(*p1, *p2, *p3, chainParams->GetConsensus());
        BOOST_CHECK_EQUAL(tdiff, p1->GetBlockTime() - p2->GetBlockTime());
    }
}

void sanity_check_chainparams(ChainType chain_type)
{
    const auto chainParams = CreateChainParams(chain_type);
    const auto consensus = chainParams->GetConsensus();

    // hash genesis is correct
    BOOST_CHECK_EQUAL(consensus.hashGenesisBlock, chainParams->GenesisBlock().GetHash());

    // genesis nBits is positive, doesn't overflow and is lower than powLimit
    arith_uint256 pow_compact;
    bool neg, over;
    pow_compact.SetCompact(chainParams->GenesisBlock().nBits, &neg, &over);
    BOOST_CHECK(!neg && pow_compact != 0);
    BOOST_CHECK(!over);
    BOOST_CHECK(UintToArith256(consensus.powLimit) >= pow_compact);

    if (!consensus.fPowNoRetargeting) {
        arith_uint256 targ_max{UintToArith256(uint256S("0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"))};
        targ_max /= consensus.nPowTargetWindow*3;
        BOOST_CHECK(UintToArith256(consensus.powLimit) < targ_max);
    }
}

BOOST_AUTO_TEST_CASE(ChainParams_MAIN_sanity)
{
    sanity_check_chainparams(ChainType::MAIN);
}

BOOST_AUTO_TEST_CASE(ChainParams_TESTNET_sanity)
{
    sanity_check_chainparams(ChainType::TESTNET);
}

BOOST_AUTO_TEST_CASE(ChainParams_REGTEST_sanity)
{
    sanity_check_chainparams(ChainType::REGTEST);
}

BOOST_AUTO_TEST_SUITE_END()