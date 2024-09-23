// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2021 The Bitcoin Core developers
// Copyright (c) 2024 The Betgenius Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <kernel/chainparams.h>

#include <chainparamsseeds.h>
#include <consensus/amount.h>
#include <consensus/merkle.h>
#include <consensus/params.h>
#include <hash.h>
#include <kernel/messagestartchars.h>
#include <logging.h>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <script/interpreter.h>
#include <script/script.h>
#include <uint256.h>
#include <util/chaintype.h>
#include <util/strencodings.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <type_traits>

static CBlock CreateGenesisBlock(const char* pszTimestamp, uint32_t nTime, uint64_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward, const CScript& genesisOutputScript)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << CScriptNum(0) << 0x1f0affff << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=0004e677dfcb64, ver=4, hashPrevBlock=00000000000000, hashMerkleRoot=6d04f0, nTime=1723680000, nBits=0x1f0affff, nNonce=59F5, vtx=1)
 *   CTransaction(hash=6d04f0, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 0004ffff0a1f01044c5a424243202d2031352f4175672f32303234202d2054686520576f726c64204865616c7468204f7267616e697a6174696f6e206465636c61726573204d504f58206120676c6f62616c206865616c746820656d657267656e63792e)
 *     CTxOut(nValue=5000.00000000, scriptPubKey=00142f2469c0f81ccd53e6046cf2153868c2ec42f0e5)
 *   vMerkleTree: 6d04f0
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint64_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward, const std::vector<uint8_t>& genesisOutputScriptHex)
{
    const char* pszTimestamp = "BBC - 15/Aug/2024 - The World Health Organization declares MPOX a global health emergency.";
    const CScript genesisOutputScript = CScript(genesisOutputScriptHex.begin(), genesisOutputScriptHex.end());
    return CreateGenesisBlock(pszTimestamp, nTime, nNonce, nBits, nVersion, genesisReward, genesisOutputScript);
}

/**
 * Main network on which people trade goods and services.
 */
class CMainParams : public CChainParams {
public:
    CMainParams() {
        m_chain_type = ChainType::MAIN;
        consensus.nSubsidyHalvingInterval = 2102400;
        consensus.BIP16Active = true;
        consensus.BIP34Active = true;
        consensus.BIP65Active = true;
        consensus.BIP66Active = true;
        consensus.CSVActive = true;
        consensus.SegwitActive = true;
        consensus.TaprootActive = true;
        consensus.powLimit = uint256S("0x000affffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetWindow = 20;
        consensus.nPowTargetSpacing = 1 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1815;
        consensus.nMinerConfirmationWindow = 2016;

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0;

        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000001745");
        consensus.defaultAssumeValid = uint256S("0x0004e677dfcb6417c32a26ab9a2e1e4b810103a6e609b1f9d6168d8dcd4e3273");

        consensus.genesisOutputScriptHex = {0x00,0x14,0x2f,0x24,0x69,0xc0,0xf8,0x1c,0xcd,0x53,0xe6,0x04,0x6c,0xf2,0x15,0x38,0x68,0xc2,0xec,0x42,0xf0,0xe5};

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xee;
        pchMessageStart[1] = 0xf8;
        pchMessageStart[2] = 0xde;
        pchMessageStart[3] = 0xd8;

        nDefaultPort = 5870;
        nPruneAfterHeight = 100000;
        m_assumed_blockchain_size = 8;
        m_assumed_chain_state_size = 2;

        genesis = CreateGenesisBlock(1723680000, 0x59F5, 0x1f0affff, 4, 5000 * COIN, consensus.genesisOutputScriptHex);
        consensus.hashGenesisBlock = genesis.GetHash(genesis.hashMix);

        assert(consensus.hashGenesisBlock == uint256S("0x0004e677dfcb6417c32a26ab9a2e1e4b810103a6e609b1f9d6168d8dcd4e3273"));
        assert(genesis.hashMerkleRoot == uint256S("0x6d04f03791b5a6115d7ac24b6600163eebbc2d4c370380e4d7a267b7f9700314"));
        assert(genesis.hashMix == uint256S("0x1947407f2e772c6fb5717556ca23fe8aea17ece9a0595ba870dae3f60b6f98a3"));

        vSeeds.emplace_back("dns-seed-mainnet-1.betgenius.cc");
        vSeeds.emplace_back("dns-seed-mainnet-2.betgenius.cc");
        vSeeds.emplace_back("dns-seed-mainnet-3.betgenius.cc");
        vSeeds.emplace_back("dns-seed-mainnet-4.betgenius.cc");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,26);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,38);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "btg";

        vFixedSeeds = std::vector<uint8_t>(std::begin(chainparams_seed_main), std::end(chainparams_seed_main));

        fDefaultConsistencyChecks = false;
        m_is_mockable_chain = false;

        checkpointData = {
            {
                { 0, uint256S("0x0004e677dfcb6417c32a26ab9a2e1e4b810103a6e609b1f9d6168d8dcd4e3273")},
            }
        };

        m_assumeutxo_data = {
            
        };

        chainTxData = ChainTxData{
            .nTime    = 1723680000,
            .nTxCount = 1,
            .dTxRate  = 0
        };
    }
};

/**
 * Testnet (v3): public test network which is reset from time to time.
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        m_chain_type = ChainType::TESTNET;
        consensus.nSubsidyHalvingInterval = 2102400;
        consensus.BIP16Active = true;
        consensus.BIP34Active = true;
        consensus.BIP65Active = true;
        consensus.BIP66Active = true;
        consensus.CSVActive = true;
        consensus.SegwitActive = true;
        consensus.TaprootActive = true;
        consensus.powLimit = uint256S("0x000affffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetWindow = 20;
        consensus.nPowTargetSpacing = 1 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512;
        consensus.nMinerConfirmationWindow = 2016;

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0;

        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000001745");
        consensus.defaultAssumeValid = uint256S("0x000707100603bccfdf80f6fc667368fb904a90e0b393bf3dd79690a36b83f674");

        consensus.genesisOutputScriptHex = {0x00,0x14,0x94,0x8f,0xec,0xef,0xd5,0xfa,0x8f,0xd2,0x4a,0x69,0x16,0xb9,0x3d,0x1e,0x36,0x64,0x90,0xc2,0x6e,0x55};

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xb6;
        pchMessageStart[1] = 0xf2;
        pchMessageStart[2] = 0xac;
        pchMessageStart[3] = 0xf2;

        nDefaultPort = 15870;
        nPruneAfterHeight = 1000;
        m_assumed_blockchain_size = 8;
        m_assumed_chain_state_size = 1;

        genesis = CreateGenesisBlock(1723680000, 0x1E6D, 0x1f0affff, 4, 5000 * COIN, consensus.genesisOutputScriptHex);
        consensus.hashGenesisBlock = genesis.GetHash(genesis.hashMix);

        assert(consensus.hashGenesisBlock == uint256S("0x000707100603bccfdf80f6fc667368fb904a90e0b393bf3dd79690a36b83f674"));
        assert(genesis.hashMerkleRoot == uint256S("0x3f255d954c63cfd041bf656d197927d2b7c711a11c1f6cc7aebb848b84a53c8e"));
        assert(genesis.hashMix == uint256S("0xf2bd9c6e950647455413bdbb67435855529f17d43c2805beb78ce323cbfba219"));

        vFixedSeeds.clear();
        vSeeds.clear();

        vSeeds.emplace_back("dns-seed-mainnet-1.betgenius.cc");
        vSeeds.emplace_back("dns-seed-mainnet-2.betgenius.cc");
        vSeeds.emplace_back("dns-seed-mainnet-3.betgenius.cc");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,66);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,63);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tbtg";

        vFixedSeeds = std::vector<uint8_t>(std::begin(chainparams_seed_test), std::end(chainparams_seed_test));

        fDefaultConsistencyChecks = false;
        m_is_mockable_chain = false;

        checkpointData = {
            {
                {0, uint256S("0x000707100603bccfdf80f6fc667368fb904a90e0b393bf3dd79690a36b83f674")},
            }
        };

        m_assumeutxo_data = {
        };

        chainTxData = ChainTxData{
            .nTime    = 1723680000,
            .nTxCount = 1,
            .dTxRate  = 0
        };
    }
};

/**
 * Regression test: intended for private networks only. Has minimal difficulty to ensure that
 * blocks can be found instantly.
 */
class CRegTestParams : public CChainParams
{
public:
    explicit CRegTestParams()
    {
        m_chain_type = ChainType::REGTEST;
        consensus.nSubsidyHalvingInterval = 2102;
        consensus.BIP16Active = true;
        consensus.BIP34Active = true;
        consensus.BIP65Active = true;
        consensus.BIP66Active = true;
        consensus.CSVActive = true;
        consensus.SegwitActive = true;
        consensus.TaprootActive = true;
        consensus.powLimit = uint256S("0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetWindow = 20;
        consensus.nPowTargetSpacing = 1 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108;
        consensus.nMinerConfirmationWindow = 144;

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0;

        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000000002");
        consensus.defaultAssumeValid = uint256S("0x252e663339fd8bd121e4559b343806038bd7354f9a8c48b9959f619c2451f9d7");

        consensus.genesisOutputScriptHex = {0x00,0x14,0x31,0x5f,0x9e,0xdf,0xad,0x9b,0xf1,0x2b,0x99,0x1d,0xbe,0x40,0x8a,0xaa,0x17,0xc1,0xf3,0xd5,0x96,0xd5};

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xab;
        pchMessageStart[1] = 0xbb;
        pchMessageStart[2] = 0xbf;
        pchMessageStart[3] = 0xf7;

        nDefaultPort = 25870;
        nPruneAfterHeight = 1000;
        m_assumed_blockchain_size = 0;
        m_assumed_chain_state_size = 0;

        genesis = CreateGenesisBlock(1723680000, 0x00, 0x207fffff, 4, 5000 * COIN, consensus.genesisOutputScriptHex);
        consensus.hashGenesisBlock = genesis.GetHash(genesis.hashMix);

        assert(consensus.hashGenesisBlock == uint256S("0x252e663339fd8bd121e4559b343806038bd7354f9a8c48b9959f619c2451f9d7"));
        assert(genesis.hashMerkleRoot == uint256S("0x1826554d45b40934fc3218b09b13aaa507cea197f03e5cbeac37bd685e60ee6a"));
        assert(genesis.hashMix == uint256S("0x4f6502de51ea6444b8de92284d356bf3de7c3abeecb8a2eeced384ff098301ac"));

        vSeeds.clear();
        vSeeds.emplace_back("dummySeed.invalid.");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,66);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,63);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "rbtg";

        vFixedSeeds.clear();

        fDefaultConsistencyChecks = true;
        m_is_mockable_chain = true;

        checkpointData = {
            {
                {0, uint256S("0x252e663339fd8bd121e4559b343806038bd7354f9a8c48b9959f619c2451f9d7")},
            }
        };

        m_assumeutxo_data = {
            {
                .height = 110,
                .hash_serialized = AssumeutxoHash{uint256S("0x6657b736d4fe4db0cbc796789e812d5dba7f5c143764b1b6905612f1830609d1")},
                .nChainTx = 111,
                .blockhash = uint256S("0x696e92821f65549c7ee134edceeeeaaa4105647a3c4fd9f298c0aec0ab50425c")
            },
            {
                .height = 299,
                .hash_serialized = AssumeutxoHash{uint256S("0xa4bf3407ccb2cc0145c49ebba8fa91199f8a3903daf0883875941497d2493c27")},
                .nChainTx = 334,
                .blockhash = uint256S("0x3bb7ce5eba0be48939b7a521ac1ba9316afee2c7bada3a0cca24188e6d7d96c0")
            }
        };

        chainTxData = ChainTxData{
            .nTime    = 1723680000,
            .nTxCount = 1,
            .dTxRate  = 0
        };
    }
};

std::unique_ptr<const CChainParams> CChainParams::Main()
{
    return std::make_unique<const CMainParams>();
}

std::unique_ptr<const CChainParams> CChainParams::TestNet()
{
    return std::make_unique<const CTestNetParams>();
}

std::unique_ptr<const CChainParams> CChainParams::RegTest()
{
    return std::make_unique<const CRegTestParams>();
}
