// Copyright (c) 2012-2022 The Bitcoin Core developers
// Copyright (c) 2024 The Betgenius Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <common/bloom.h>

#include <clientversion.h>
#include <common/system.h>
#include <key.h>
#include <key_io.h>
#include <merkleblock.h>
#include <primitives/block.h>
#include <random.h>
#include <serialize.h>
#include <streams.h>
#include <test/util/random.h>
#include <test/util/setup_common.h>
#include <uint256.h>
#include <util/strencodings.h>

#include <vector>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(bloom_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(bloom_create_insert_serialize)
{
    CBloomFilter filter(3, 0.01, 0, BLOOM_UPDATE_ALL);

    BOOST_CHECK_MESSAGE( !filter.contains(ParseHex("99108ad8ed9bb6274d3980bab5a85c048f0950c8")), "Bloom filter should be empty!");
    filter.insert(ParseHex("99108ad8ed9bb6274d3980bab5a85c048f0950c8"));
    BOOST_CHECK_MESSAGE( filter.contains(ParseHex("99108ad8ed9bb6274d3980bab5a85c048f0950c8")), "Bloom filter doesn't contain just-inserted object!");
    // One bit different in first byte
    BOOST_CHECK_MESSAGE(!filter.contains(ParseHex("19108ad8ed9bb6274d3980bab5a85c048f0950c8")), "Bloom filter contains something it shouldn't!");

    filter.insert(ParseHex("b5a2c786d9ef4658287ced5914b37a1b4aa32eee"));
    BOOST_CHECK_MESSAGE(filter.contains(ParseHex("b5a2c786d9ef4658287ced5914b37a1b4aa32eee")), "Bloom filter doesn't contain just-inserted object (2)!");

    filter.insert(ParseHex("b9300670b4c5366e95b2699e8b18bc75e5f729c5"));
    BOOST_CHECK_MESSAGE(filter.contains(ParseHex("b9300670b4c5366e95b2699e8b18bc75e5f729c5")), "Bloom filter doesn't contain just-inserted object (3)!");

    DataStream stream{};
    stream << filter;

    std::vector<uint8_t> expected = ParseHex("03614e9b050000000000000001");
    auto result{MakeUCharSpan(stream)};

    BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), expected.begin(), expected.end());

    BOOST_CHECK_MESSAGE( filter.contains(ParseHex("99108ad8ed9bb6274d3980bab5a85c048f0950c8")), "Bloom filter doesn't contain just-inserted object!");
}

BOOST_AUTO_TEST_CASE(bloom_create_insert_serialize_with_tweak)
{
    // Same test as bloom_create_insert_serialize, but we add a nTweak of 100
    CBloomFilter filter(3, 0.01, 2147483649UL, BLOOM_UPDATE_ALL);

    filter.insert(ParseHex("99108ad8ed9bb6274d3980bab5a85c048f0950c8"));
    BOOST_CHECK_MESSAGE( filter.contains(ParseHex("99108ad8ed9bb6274d3980bab5a85c048f0950c8")), "Bloom filter doesn't contain just-inserted object!");
    // One bit different in first byte
    BOOST_CHECK_MESSAGE(!filter.contains(ParseHex("19108ad8ed9bb6274d3980bab5a85c048f0950c8")), "Bloom filter contains something it shouldn't!");

    filter.insert(ParseHex("b5a2c786d9ef4658287ced5914b37a1b4aa32eee"));
    BOOST_CHECK_MESSAGE(filter.contains(ParseHex("b5a2c786d9ef4658287ced5914b37a1b4aa32eee")), "Bloom filter doesn't contain just-inserted object (2)!");

    filter.insert(ParseHex("b9300670b4c5366e95b2699e8b18bc75e5f729c5"));
    BOOST_CHECK_MESSAGE(filter.contains(ParseHex("b9300670b4c5366e95b2699e8b18bc75e5f729c5")), "Bloom filter doesn't contain just-inserted object (3)!");

    DataStream stream{};
    stream << filter;

    std::vector<uint8_t> expected = ParseHex("03ce4299050000000100008001");
    auto result{MakeUCharSpan(stream)};

    BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(bloom_create_insert_key)
{
    std::string strSecret = std::string("L2ABwR8Fdvj6iaFHSV1ZQLBuQUh4MkMVLeExfi7o2K5cC3jpqB1M");
    CKey key = DecodeSecret(strSecret);
    CPubKey pubkey = key.GetPubKey();
    std::vector<unsigned char> vchPubKey(pubkey.begin(), pubkey.end());

    CBloomFilter filter(2, 0.001, 0, BLOOM_UPDATE_ALL);
    filter.insert(vchPubKey);
    uint160 hash = pubkey.GetID();
    filter.insert(hash);

    DataStream stream{};
    stream << filter;

    std::vector<unsigned char> expected = ParseHex("0309faf2080000000000000001");
    auto result{MakeUCharSpan(stream)};

    BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(bloom_match)
{
    // Random test transaction (0xc246c29857297012bed36fe6f3ba52adf9c29d51c776709e9e82c187c5b58801)
    DataStream stream{
        ParseHex("0200000001cd1e9af477a48a5a0fb41aa052c220d2e1b455734c5aa8e15b27b6d7a0058359000000006a473044022005a2cf604517766c75329cedd6b0ed7bba750ac7028bd6c590f986dd3641328f02207145abb1524570bbc72a4f788d4bb125e1254ddc30830a20dc14d5abfec03b8a012103dd447ceed1fbc5ba778b74171d990883857af70982a51405b1cb1ab63ed0831bfdffffff021fe77648170000001976a914c92d4150373accadee47ab81470a79838d645e3e88ac00e87648170000001976a9145fad48976a726ae26e3c9b0d5e31997c5043385088ac1e000000"),
    };
    CTransaction tx(deserialize, TX_WITH_WITNESS, stream);

    // and one which spends it (0x0edfbabc60d0a16e85e382f7d348ae410e77282c2b114becb0de247c71361cfa)
    unsigned char ch[] = {0x02, 0x00, 0x00, 0x00, 0x01, 0x01, 0x88, 0xb5, 0xc5, 0x87, 0xc1, 0x82, 0x9e, 0x9e, 0x70, 0x76, 0xc7, 0x51, 0x9d, 0xc2, 0xf9, 0xad, 0x52, 0xba, 0xf3, 0xe6, 0x6f, 0xd3, 0xbe, 0x12, 0x70, 0x29, 0x57, 0x98, 0xc2, 0x46, 0xc2, 0x01, 0x00, 0x00, 0x00, 0x6a, 0x47, 0x30, 0x44, 0x02, 0x20, 0x28, 0x90, 0x67, 0x74, 0xf7, 0xce, 0x4f, 0x4b, 0xd0, 0x55, 0x9c, 0x50, 0x5b, 0x5d, 0xd3, 0x9f, 0xb0, 0x20, 0xa2, 0x17, 0xdf, 0x32, 0xae, 0xe1, 0xff, 0x96, 0x08, 0x36, 0xd4, 0x14, 0x88, 0xdc, 0x02, 0x20, 0x28, 0x24, 0x02, 0xb7, 0x77, 0x72, 0x81, 0xe1, 0xd2, 0x06, 0xfd, 0xc6, 0x53, 0x11, 0x5a, 0x43, 0x7e, 0x12, 0x6e, 0x50, 0x94, 0x58, 0x93, 0xa0, 0x02, 0x25, 0xfa, 0x0e, 0xaf, 0x7f, 0xd4, 0x54, 0x01, 0x21, 0x03, 0x1f, 0x1c, 0x90, 0x8e, 0x07, 0xfa, 0x0a, 0xab, 0x31, 0xd1, 0xc3, 0x9a, 0x9c, 0xfb, 0xcc, 0xa2, 0x75, 0x90, 0x93, 0xfe, 0x90, 0x10, 0x23, 0x9d, 0x21, 0x49, 0xf4, 0x4c, 0x4e, 0x47, 0xfa, 0x20, 0xfd, 0xff, 0xff, 0xff, 0x02, 0x1f, 0x73, 0x3b, 0xa4, 0x0b, 0x00, 0x00, 0x00, 0x19, 0x76, 0xa9, 0x14, 0xe2, 0xf6, 0x95, 0x71, 0x56, 0x40, 0x32, 0xaa, 0x31, 0x8b, 0x6d, 0x4c, 0xee, 0x9a, 0x4c, 0x81, 0x63, 0xee, 0x04, 0x21, 0x88, 0xac, 0x00, 0x74, 0x3b, 0xa4, 0x0b, 0x00, 0x00, 0x00, 0x19, 0x76, 0xa9, 0x14, 0xd7, 0x23, 0xca, 0x47, 0x9e, 0x18, 0x7d, 0x52, 0xb3, 0x40, 0xa1, 0xe9, 0x20, 0x5f, 0x00, 0xf0, 0x3a, 0x68, 0x6c, 0xea, 0x88, 0xac, 0x6d, 0x00, 0x00, 0x00, 0x00, 0x00};
    std::vector<unsigned char> vch(ch, ch + sizeof(ch) -1);
    DataStream spendStream{vch};
    CTransaction spendingTx(deserialize, TX_WITH_WITNESS, spendStream);

    CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    filter.insert(uint256S("0xc246c29857297012bed36fe6f3ba52adf9c29d51c776709e9e82c187c5b58801"));
    BOOST_CHECK_MESSAGE(filter.IsRelevantAndUpdate(tx), "Simple Bloom filter didn't match tx hash");

    filter = CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    // byte-reversed tx hash
    filter.insert(ParseHex("0188b5c587c1829e9e7076c7519dc2f9ad52baf3e66fd3be1270295798c246c2"));
    BOOST_CHECK_MESSAGE(filter.IsRelevantAndUpdate(tx), "Simple Bloom filter didn't match manually serialized tx hash");

    filter = CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    filter.insert(ParseHex("3044022005a2cf604517766c75329cedd6b0ed7bba750ac7028bd6c590f986dd3641328f02207145abb1524570bbc72a4f788d4bb125e1254ddc30830a20dc14d5abfec03b8a01"));
    BOOST_CHECK_MESSAGE(filter.IsRelevantAndUpdate(tx), "Simple Bloom filter didn't match input signature");

    filter = CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    filter.insert(ParseHex("03dd447ceed1fbc5ba778b74171d990883857af70982a51405b1cb1ab63ed0831b"));
    BOOST_CHECK_MESSAGE(filter.IsRelevantAndUpdate(tx), "Simple Bloom filter didn't match input pub key");

    filter = CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    filter.insert(ParseHex("5fad48976a726ae26e3c9b0d5e31997c50433850"));
    BOOST_CHECK_MESSAGE(filter.IsRelevantAndUpdate(tx), "Simple Bloom filter didn't match output address");
    BOOST_CHECK_MESSAGE(filter.IsRelevantAndUpdate(spendingTx), "Simple Bloom filter didn't add output");

    filter = CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    filter.insert(ParseHex("c92d4150373accadee47ab81470a79838d645e3e"));
    BOOST_CHECK_MESSAGE(filter.IsRelevantAndUpdate(tx), "Simple Bloom filter didn't match output address");

    filter = CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    filter.insert(COutPoint(TxidFromString("0x598305a0d7b6275be1a85a4c7355b4e1d220c252a01ab40f5a8aa477f49a1ecd"), 0));
    BOOST_CHECK_MESSAGE(filter.IsRelevantAndUpdate(tx), "Simple Bloom filter didn't match COutPoint");

    filter = CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    COutPoint prevOutPoint(TxidFromString("0x598305a0d7b6275be1a85a4c7355b4e1d220c252a01ab40f5a8aa477f49a1ecd"), 0);
    {
        std::vector<unsigned char> data(32 + sizeof(unsigned int));
        memcpy(data.data(), prevOutPoint.hash.begin(), 32);
        memcpy(data.data()+32, &prevOutPoint.n, sizeof(unsigned int));
        filter.insert(data);
    }
    BOOST_CHECK_MESSAGE(filter.IsRelevantAndUpdate(tx), "Simple Bloom filter didn't match manually serialized COutPoint");

    filter = CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    filter.insert(uint256S("d5e2cf1360715ccd6675d5865d534e295a5d29e169fe97299f2a1cebbe0130f8"));
    BOOST_CHECK_MESSAGE(!filter.IsRelevantAndUpdate(tx), "Simple Bloom filter matched random tx hash");

    filter = CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    filter.insert(ParseHex("e84d111600f63fe32510f92f15a28a41a577f787"));
    BOOST_CHECK_MESSAGE(!filter.IsRelevantAndUpdate(tx), "Simple Bloom filter matched random address");

    filter = CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    filter.insert(COutPoint(TxidFromString("0x598305a0d7b6275be1a85a4c7355b4e1d220c252a01ab40f5a8aa477f49a1ecd"), 1));
    BOOST_CHECK_MESSAGE(!filter.IsRelevantAndUpdate(tx), "Simple Bloom filter matched COutPoint for an output we didn't care about");

    filter = CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    filter.insert(COutPoint(TxidFromString("d5e2cf1360715ccd6675d5865d534e295a5d29e169fe97299f2a1cebbe0130f8"), 0));
    BOOST_CHECK_MESSAGE(!filter.IsRelevantAndUpdate(tx), "Simple Bloom filter matched COutPoint for an output we didn't care about");
}

BOOST_AUTO_TEST_CASE(merkle_block_1)
{
    // Random test block (0x414e96f90a60b9dd125f0ef9a04b67f1335e9f8045d1ba1db8b52361f9ec1d89)
    // With 9 txes
    CBlock block;
    DataStream stream{
        ParseHex("00000020ca688cca33b7d47b2feb61cb604a7bc7e2175652a885238b325d252f7106157f2c60e574599c862709c165908c8a6afbbbdfb1653085837095165e1a86f7d9956a163166ffff7f206f00000000000000000000003468c93f1039dbf9d6563b99cc73e507cc7f91fb259416dee74ea423bbaccfef09020000000001010000000000000000000000000000000000000000000000000000000000000000ffffffff03016f00ffffffff0208d7ed902e0000001976a9143d43ee6b2e01b439908f07c5715ed0572668aa9088ac0000000000000000266a24aa21a9edd314965bd33edd008d7ed626972346997c8639606093d184e4f94ef392cc26160120000000000000000000000000000000000000000000000000000000000000000000000000020000000166b14613cbeb27f64f417966a736c8e737aab2d408aefafe491945e89ada8f2e000000006a473044022078eec3a2293496b5fe663c9944a032e2ef68d167ef3d02b95b266bf2834da274022043557f40c337276b775d37d6275a5412cbeb0234e054ffa9ce430920766b69c2012103dd447ceed1fbc5ba778b74171d990883857af70982a51405b1cb1ab63ed0831bfdffffff0200e40b54020000001976a914b150de59803b725f5fa2f4d7a0bf3f18634cac6d88ac1febe13c2c0000001976a9147b0d312b007dbac5cf6c25da50c01308d298e2a088ac6e0000000200000001a8bece298933f7d7773bae388151e06688f9e957e94bf8428f08a89f6dfef61f010000006a47304402200d74399557d7929473898ec1c45834bb1ce0f451f3301942402815e0ee484cc702201def63f882b0df06dccd25c1aafbbf0261859015599580890d8d2967fc730252012102ce26d9af10b21cdf964a3367932c4c35cb2e9b14583d3238689c38534e0dcae3fdffffff0200e40b54020000001976a91417e1a6f5891cdf50fd3c3255b8af17090f50479988ac3e06d6e8290000001976a914fb6e3b2576e6c6808db9eba588c0cc9ec437311488ac6e00000002000000012fa0f0f07217b3b2ca60171451b0df629b9b4ff7c9d1198a59af25220e77ab5f010000006a47304402205e44ae41859ec518ed88e0c9c7e6d7eff85f547a6aa5a306414e0a133ed83bb702204c289bca66748fbfe050867a2fcb59ec9bdb90d80d8b14939bfd976267bbedc001210207f06c4afa55071c72e93cc1b5fbabdc330b9cc06035844172596294240b07dcfdffffff0200e40b54020000001976a91438849a498d6608cbfa0f55d9c790d2eb1f972f0d88ac5d21ca94270000001976a9140c1a1690fb3d9165a750a6266432a66fc0c5b38688ac6e0000000200000001d3165ce9f148deb7f0c340ad6809fd6398089f088c5dffbbd26181d3e4674bcb010000006a473044022047381a776ce63400b08802686ba4338378109510380704d3b7a4c771b2a6d88e022006fe3df187eccba6c6c2933ee2eb43eaca8f7772635feeb8fc6cf087bcb6df4f012103c8d0d4f65b1d528ed85c4accd6c2e270191da7f7185bf9aa816be700209f9c77fdffffff027c3cbe40250000001976a9148d8b7e4b19ba89f27c61c538592c5aaad7e2e7c388ac00e40b54020000001976a9142cb487fa9bf1c89ac92a435d0eaf2c321469d2e188ac6e0000000200000001c94d74a198225a1f54c85f71f822d31fd1cc4b8f0411bf3b3fd8335e18bd9fe2000000006a47304402206ac86a6ff28c5f8c1e3156d102779139690f52c33b9a92b52df0376ebca9557802203103bf5d7fda27c710232d039e4cd1a1686cfa6cdf389e2ebbb50709310fbdd5012102ddbb6656b12b62546b311c1a1c30d3f0efb91da065fb4f0feee980baf04734a3fdffffff029b57b2ec220000001976a9147320bd04e41e10890c099ea41b861e0d38de549488ac00e40b54020000001976a9140adf6cbb0eaf625de09892d81fb7030f88aae93a88ac6e00000002000000010db5ad7f4a989b28dc1323c082bb8c532c3c76b2004c63855b2992e57dc10401000000006a47304402202512bf190e5303ac60c53f74b42f53aa48632dc4bea6102119451b52b194b16002206698634b0d440c0ea1dfe3ccf5041aabfa029cf4fd04b931efdebcfccab23fc70121034ffca23d3fb7043c20359ebfa2d45d3689663abde27c1837f0d8d4705d68998bfdffffff0200e40b54020000001976a914751297168f98a2f5d5652dfa12f26c74f633841988acba72a698200000001976a91476dc7a44ef7d79f5226ff7e0a53fe6f45954432d88ac6e00000002000000013f398349c085947f24fe279eaa5d1dda3a140c9a603205877a3ee15974e80dfb010000006a47304402200b71d6c1e05e5fb8459a010a773ef22256273bf55cd7bdc2dbf32730927a83cb022017c4e4a7d36153dbe10aeede5fb5c64620de4e9bc3c06ab8efc1837b80a9c10c01210299ad22636cd20660a32034a411cb725967728b69c9f85c40162d1f535b59a058fdffffff02d98d9a441e0000001976a9146a98fc0829619134881fd09ac56b2824417dd4f688ac00e40b54020000001976a91401606793146b3188debe78f5d46924c9465d764688ac6e0000000200000001d1bdd181075a3a231b39bd1a285fd322c468bed70349bbed346e675be5ba3ca6000000006a473044022047810101ee4c6ee53024145437ead095652042e00e4cbddeb816e715ada489a502206126540ea95339a48b6f51ba932366037899c22f07fcf7897c0a941aa7f10a5e0121026d59e75da10191bbb497d98be226fb19d63bb99ae0adac11d2acb32db66b1ffcfdffffff0200e40b54020000001976a91426bdb2343e1b3b6774a3d18305108edca85879e188acf8a88ef01b0000001976a914b2bc2d81924343e59e691abac36ec9c2e0375a9788ac6e000000"),
    };
    stream >> TX_WITH_WITNESS(block);
    CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    // Match the last transaction
    filter.insert(uint256S("0x59a50fe568f76afb68309ced3a8e2ae3b5a6da9bb8f2cbc5336e20ab8d84bea5"));

    CMerkleBlock merkleBlock(block, filter);
    BOOST_CHECK_EQUAL(merkleBlock.header.GetHash().GetHex(), block.GetHash().GetHex());

    BOOST_CHECK_EQUAL(merkleBlock.vMatchedTxn.size(), 1U);
    std::pair<unsigned int, uint256> pair = merkleBlock.vMatchedTxn[0];

    BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256S("0x59a50fe568f76afb68309ced3a8e2ae3b5a6da9bb8f2cbc5336e20ab8d84bea5"));
    BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 8);

    std::vector<uint256> vMatched;
    std::vector<unsigned int> vIndex;
    BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched, vIndex) == block.hashMerkleRoot);
    BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
    for (unsigned int i = 0; i < vMatched.size(); i++)
        BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);

    // Also match the 8th transaction
    filter.insert(uint256S("0xa63cbae55b676e34edbb4903d7be68c422d35f281abd391b233a5a0781d1bdd1"));
    merkleBlock = CMerkleBlock(block, filter);
    BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

    BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 2);

    BOOST_CHECK(merkleBlock.vMatchedTxn[1] == pair);

    BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256S("0xa63cbae55b676e34edbb4903d7be68c422d35f281abd391b233a5a0781d1bdd1"));
    BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 7);

    BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched, vIndex) == block.hashMerkleRoot);
    BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
    for (unsigned int i = 0; i < vMatched.size(); i++)
        BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);
}

BOOST_AUTO_TEST_CASE(merkle_block_2)
{
    // Random test block (0x72422453a408178ed98ab9c86840c0887e7f77e4954ff3e5c4863c1fa24bfa58)
    // With 4 txes
    CBlock block;
    DataStream stream{
        ParseHex("00000020e51ed2ee12f709ba8eb922ad4c5a3ec2515e8417dbc3c06d21876cbea3b2282c0a4bfdb8e563235cb08a135237bd2ad8e2c7a7d50dfadb4ad98b4ecce465e3f25a203166ffff7f207100000000000000000000000ab8940331ca8304926b0e4ea7ae45a7d21bebbf45ee124dfbc9edb67878ee6c04020000000001010000000000000000000000000000000000000000000000000000000000000000ffffffff03017100ffffffff025cd4ed902e0000001976a9143d43ee6b2e01b439908f07c5715ed0572668aa9088ac0000000000000000266a24aa21a9eddc0ebffdf505ecacf3271054ff019bc79cef423a9c9baab0131471557630b28f0120000000000000000000000000000000000000000000000000000000000000000000000000020000000218d1a66c518918379b3c120619ecc1b386bf740078e38e8f3af84797f76a1b3f000000006a47304402206d5c223a3f6e6170ba6db266ae37d18325c94361ac6654cac0793aac52f0419d02204e68a7657de0141a42540e73c095a2c275d2c1d735623adab7399b00a1797295012103dd447ceed1fbc5ba778b74171d990883857af70982a51405b1cb1ab63ed0831bfdffffff8bf8246f2d44fced33a3589bcb515d44c28ffda231b9030f0211126f6ccbd1d7010000006a4730440220010eec12bf0d2bf5141f34b32bf9c639b3bd7b38cb54cba39f628122b3a82f6502202818b476c5f65b164250ba32aab713a229bba89b5e45f45419511bdf2acff5fd0121031f46e2a08a2b9e3800e411de6dbbc33a5999f3fb11b567195a2e37c36897154efdffffff028cf0052a010000001976a914506ecb7be626cd8ccbf7ec25785c61a1ed91fb3788ac00d0ed902e0000001976a91409ac42368076cfa9dec268c8f2b74dd5bbf3f76e88ac700000000200000002c4fe3d4fe37f28b5961a8fb984f1b5701e4fc2502b3e14bb24b8da46ba3e0ed4010000006a47304402205159afd272efc53a712b254a01b6e6483108669c6ac5cf2dc75d6ca399faf10902201e9a548e5a5e7f37326f34d72134629e9c6ad28b93525decef22a227b7a2a4130121031f46e2a08a2b9e3800e411de6dbbc33a5999f3fb11b567195a2e37c36897154efdffffffc4fe3d4fe37f28b5961a8fb984f1b5701e4fc2502b3e14bb24b8da46ba3e0ed4000000006a4730440220299ec0b500e573bd4bc0f0e35a068bbab88fad4ae7d8236bff3a2dd5e942ed8f0220129856ced061893ec7c033ab4e0d78bfc5d519e0cec6ec0387f8cf4e5b7dc53d012103bb26267315c45f7a78abed0fee559367c7ffcc6843e6e82dded8c1100f157b75fdffffff0218ef052a010000001976a914506ecb7be626cd8ccbf7ec25785c61a1ed91fb3788ac00d0ed902e0000001976a914506ecb7be626cd8ccbf7ec25785c61a1ed91fb3788ac700000000200000002076473032f91c25e09fa0c14d049e39466e2dd6356404c04b2fdd945430ca135000000006a47304402207f1e901b2ed1c23bbabb662512f1939f2dd9d7c1d1f40ae7c4e19186e1ac632e022015e4a0c5352533e1a49693cfee780bdf060b2b1b826ab8ca036bb58588c6a931012103bb26267315c45f7a78abed0fee559367c7ffcc6843e6e82dded8c1100f157b75fdffffff076473032f91c25e09fa0c14d049e39466e2dd6356404c04b2fdd945430ca135010000006a473044022065aa53e36469140c80d60e7b5d01a8a15592ebbfba35c9abb692f83c1dc2daf70220110fdb044b2c41076197f0367a26a5081a14f814bff65eb2f6eefb9848221c0c012103bb26267315c45f7a78abed0fee559367c7ffcc6843e6e82dded8c1100f157b75fdffffff02a4ed052a010000001976a91409ac42368076cfa9dec268c8f2b74dd5bbf3f76e88ac00d0ed902e0000001976a914506ecb7be626cd8ccbf7ec25785c61a1ed91fb3788ac70000000"),
    };
    stream >> TX_WITH_WITNESS(block);

    CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    // Match the first transaction
    filter.insert(uint256S("0x12694f1cc14923ec8f228a1f4444278c37c3156979647a9f40e45cc0d9531e30"));

    CMerkleBlock merkleBlock(block, filter);
    BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

    BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 1);
    std::pair<unsigned int, uint256> pair = merkleBlock.vMatchedTxn[0];

    BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256S("0x12694f1cc14923ec8f228a1f4444278c37c3156979647a9f40e45cc0d9531e30"));
    BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 0);

    std::vector<uint256> vMatched;
    std::vector<unsigned int> vIndex;
    BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched, vIndex) == block.hashMerkleRoot);
    BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
    for (unsigned int i = 0; i < vMatched.size(); i++)
        BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);

    // Match an output from the second transaction (the script for address GJj4rbQadk2HNmS61ZZM58k9617WVQg95t)
    // This should match the third transaction because it spends the output matched
    // It also matches the fourth transaction, which spends to the script again
    filter.insert(ParseHex("09ac42368076cfa9dec268c8f2b74dd5bbf3f76e"));

    merkleBlock = CMerkleBlock(block, filter);
    BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

    BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 4);

    BOOST_CHECK(pair == merkleBlock.vMatchedTxn[0]);

    BOOST_CHECK(merkleBlock.vMatchedTxn[1].second == uint256S("0xd40e3eba46dab824bb143e2b50c24f1e70b5f184b98f1a96b5287fe34f3dfec4"));
    BOOST_CHECK(merkleBlock.vMatchedTxn[1].first == 1);

    BOOST_CHECK(merkleBlock.vMatchedTxn[2].second == uint256S("0x35a10c4345d9fdb2044c405663dde26694e349d0140cfa095ec2912f03736407"));
    BOOST_CHECK(merkleBlock.vMatchedTxn[2].first == 2);

    BOOST_CHECK(merkleBlock.vMatchedTxn[3].second == uint256S("0xc353b6cfb58b6b829a83809baa6b33b66b6f417913bdf3a759f6e463fb609cb8"));
    BOOST_CHECK(merkleBlock.vMatchedTxn[3].first == 3);

    BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched, vIndex) == block.hashMerkleRoot);
    BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
    for (unsigned int i = 0; i < vMatched.size(); i++)
        BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);
}

BOOST_AUTO_TEST_CASE(merkle_block_2_with_update_none)
{
    // Random test block (0x72422453a408178ed98ab9c86840c0887e7f77e4954ff3e5c4863c1fa24bfa58)
    // With 4 txes
    CBlock block;
    DataStream stream{
        ParseHex("00000020e51ed2ee12f709ba8eb922ad4c5a3ec2515e8417dbc3c06d21876cbea3b2282c0a4bfdb8e563235cb08a135237bd2ad8e2c7a7d50dfadb4ad98b4ecce465e3f25a203166ffff7f207100000000000000000000000ab8940331ca8304926b0e4ea7ae45a7d21bebbf45ee124dfbc9edb67878ee6c04020000000001010000000000000000000000000000000000000000000000000000000000000000ffffffff03017100ffffffff025cd4ed902e0000001976a9143d43ee6b2e01b439908f07c5715ed0572668aa9088ac0000000000000000266a24aa21a9eddc0ebffdf505ecacf3271054ff019bc79cef423a9c9baab0131471557630b28f0120000000000000000000000000000000000000000000000000000000000000000000000000020000000218d1a66c518918379b3c120619ecc1b386bf740078e38e8f3af84797f76a1b3f000000006a47304402206d5c223a3f6e6170ba6db266ae37d18325c94361ac6654cac0793aac52f0419d02204e68a7657de0141a42540e73c095a2c275d2c1d735623adab7399b00a1797295012103dd447ceed1fbc5ba778b74171d990883857af70982a51405b1cb1ab63ed0831bfdffffff8bf8246f2d44fced33a3589bcb515d44c28ffda231b9030f0211126f6ccbd1d7010000006a4730440220010eec12bf0d2bf5141f34b32bf9c639b3bd7b38cb54cba39f628122b3a82f6502202818b476c5f65b164250ba32aab713a229bba89b5e45f45419511bdf2acff5fd0121031f46e2a08a2b9e3800e411de6dbbc33a5999f3fb11b567195a2e37c36897154efdffffff028cf0052a010000001976a914506ecb7be626cd8ccbf7ec25785c61a1ed91fb3788ac00d0ed902e0000001976a91409ac42368076cfa9dec268c8f2b74dd5bbf3f76e88ac700000000200000002c4fe3d4fe37f28b5961a8fb984f1b5701e4fc2502b3e14bb24b8da46ba3e0ed4010000006a47304402205159afd272efc53a712b254a01b6e6483108669c6ac5cf2dc75d6ca399faf10902201e9a548e5a5e7f37326f34d72134629e9c6ad28b93525decef22a227b7a2a4130121031f46e2a08a2b9e3800e411de6dbbc33a5999f3fb11b567195a2e37c36897154efdffffffc4fe3d4fe37f28b5961a8fb984f1b5701e4fc2502b3e14bb24b8da46ba3e0ed4000000006a4730440220299ec0b500e573bd4bc0f0e35a068bbab88fad4ae7d8236bff3a2dd5e942ed8f0220129856ced061893ec7c033ab4e0d78bfc5d519e0cec6ec0387f8cf4e5b7dc53d012103bb26267315c45f7a78abed0fee559367c7ffcc6843e6e82dded8c1100f157b75fdffffff0218ef052a010000001976a914506ecb7be626cd8ccbf7ec25785c61a1ed91fb3788ac00d0ed902e0000001976a914506ecb7be626cd8ccbf7ec25785c61a1ed91fb3788ac700000000200000002076473032f91c25e09fa0c14d049e39466e2dd6356404c04b2fdd945430ca135000000006a47304402207f1e901b2ed1c23bbabb662512f1939f2dd9d7c1d1f40ae7c4e19186e1ac632e022015e4a0c5352533e1a49693cfee780bdf060b2b1b826ab8ca036bb58588c6a931012103bb26267315c45f7a78abed0fee559367c7ffcc6843e6e82dded8c1100f157b75fdffffff076473032f91c25e09fa0c14d049e39466e2dd6356404c04b2fdd945430ca135010000006a473044022065aa53e36469140c80d60e7b5d01a8a15592ebbfba35c9abb692f83c1dc2daf70220110fdb044b2c41076197f0367a26a5081a14f814bff65eb2f6eefb9848221c0c012103bb26267315c45f7a78abed0fee559367c7ffcc6843e6e82dded8c1100f157b75fdffffff02a4ed052a010000001976a91409ac42368076cfa9dec268c8f2b74dd5bbf3f76e88ac00d0ed902e0000001976a914506ecb7be626cd8ccbf7ec25785c61a1ed91fb3788ac70000000"),
    };
    stream >> TX_WITH_WITNESS(block);

    CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_NONE);
    // Match the first transaction
    filter.insert(uint256S("0x12694f1cc14923ec8f228a1f4444278c37c3156979647a9f40e45cc0d9531e30"));

    CMerkleBlock merkleBlock(block, filter);
    BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

    BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 1);
    std::pair<unsigned int, uint256> pair = merkleBlock.vMatchedTxn[0];

    BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256S("0x12694f1cc14923ec8f228a1f4444278c37c3156979647a9f40e45cc0d9531e30"));
    BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 0);

    std::vector<uint256> vMatched;
    std::vector<unsigned int> vIndex;
    BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched, vIndex) == block.hashMerkleRoot);
    BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
    for (unsigned int i = 0; i < vMatched.size(); i++)
        BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);

    // Match an output from the second transaction (the script for address GJj4rbQadk2HNmS61ZZM58k9617WVQg95t)
    // This should not match the third transaction though it spends the output matched
    // It will match the fourth transaction, which has another pay-to-pubkey-hash output to the same address
    filter.insert(ParseHex("09ac42368076cfa9dec268c8f2b74dd5bbf3f76e"));

    merkleBlock = CMerkleBlock(block, filter);
    BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

    BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 3);

    BOOST_CHECK(pair == merkleBlock.vMatchedTxn[0]);

    BOOST_CHECK(merkleBlock.vMatchedTxn[1].second == uint256S("0xd40e3eba46dab824bb143e2b50c24f1e70b5f184b98f1a96b5287fe34f3dfec4"));
    BOOST_CHECK(merkleBlock.vMatchedTxn[1].first == 1);

    BOOST_CHECK(merkleBlock.vMatchedTxn[2].second == uint256S("0xc353b6cfb58b6b829a83809baa6b33b66b6f417913bdf3a759f6e463fb609cb8"));
    BOOST_CHECK(merkleBlock.vMatchedTxn[2].first == 3);

    BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched, vIndex) == block.hashMerkleRoot);
    BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
    for (unsigned int i = 0; i < vMatched.size(); i++)
        BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);
}

BOOST_AUTO_TEST_CASE(merkle_block_3_and_serialize)
{
    // Random test block (0x11b7ff8ce46fa60fa4ab4d9bcbe294b5dd7787e530d751d30a9f10827fc35c94)
    // With one tx
    CBlock block;
    DataStream stream{
        ParseHex("0000002058fa4ba21f3c86c4e5f34f95e4777f7e88c04068c8b98ad98e1708a453244272b55a0ceabf2ca9c0a91ae38cc2c9c0de18e1985a10b5b03248a2cbaca3e1ef099b263166ffff7f20720000000200000000000000ab60fad1aa1a430a14e16e7f1efd110ea7cd4db0d938112417c57cd10c4b2abc01020000000001010000000000000000000000000000000000000000000000000000000000000000ffffffff03017200ffffffff0200d0ed902e0000001976a9143d43ee6b2e01b439908f07c5715ed0572668aa9088ac0000000000000000266a24aa21a9ede2f61c3f71d1defd3fa999dfa36953755c690689799962b48bebd836974e8cf90120000000000000000000000000000000000000000000000000000000000000000000000000"),
    };
    stream >> TX_WITH_WITNESS(block);

    CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    // Match the only transaction
    filter.insert(uint256S("0x09efe1a3accba24832b0b5105a98e118dec0c9c28ce31aa9c0a92cbfea0c5ab5"));

    CMerkleBlock merkleBlock(block, filter);
    BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

    BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 1);

    BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256S("0x09efe1a3accba24832b0b5105a98e118dec0c9c28ce31aa9c0a92cbfea0c5ab5"));
    BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 0);

    std::vector<uint256> vMatched;
    std::vector<unsigned int> vIndex;
    BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched, vIndex) == block.hashMerkleRoot);
    BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
    for (unsigned int i = 0; i < vMatched.size(); i++)
        BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);

    DataStream merkleStream{};
    merkleStream << merkleBlock;

    std::vector<uint8_t> expected = ParseHex("0000002058fa4ba21f3c86c4e5f34f95e4777f7e88c04068c8b98ad98e1708a453244272b55a0ceabf2ca9c0a91ae38cc2c9c0de18e1985a10b5b03248a2cbaca3e1ef099b263166ffff7f20720000000200000000000000ab60fad1aa1a430a14e16e7f1efd110ea7cd4db0d938112417c57cd10c4b2abc0100000001b55a0ceabf2ca9c0a91ae38cc2c9c0de18e1985a10b5b03248a2cbaca3e1ef090101");
    auto result{MakeUCharSpan(merkleStream)};

    BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), result.begin(), result.end());
}

BOOST_AUTO_TEST_CASE(merkle_block_4)
{
    // Random test block (0x6cd76c4719bc5648531abce846fc37e07abc0329a2ba98a392802d5800698174)
    // With 7 txes
    CBlock block;
    DataStream stream{
        ParseHex("00000020945cc37f82109f0ad351d730e58777ddb594e2cb9b4daba40fa66fe48cffb7111ea1fd722154d32fcb5c237010c2152dc8605528c75c31bb0e4ce09795914ab051283166ffff7f207300000006000000000000001bbb6a42a8766baf3fcdcd90dfd685d0e28217d172b92c22a2c948e0a87165be07020000000001010000000000000000000000000000000000000000000000000000000000000000ffffffff03017300ffffffff0207d9ed902e0000001976a9143d43ee6b2e01b439908f07c5715ed0572668aa9088ac0000000000000000266a24aa21a9ed197167d846cad7fdddba268be19702fa73aa3c305092ca0147bc699412be104201200000000000000000000000000000000000000000000000000000000000000000000000000200000001b89c60fb63e4f659a7f3bd1379416f6bb6336baa9b80839a826b8bb5cfb653c3010000006a47304402204ece81e64caec95e790f3f78e6aa1d062427f28df1322656ff8763e04964dc53022029e692a4549332d4b2185b8f8415ab46fdaab31f2fd622d078449490eab07722012103bb26267315c45f7a78abed0fee559367c7ffcc6843e6e82dded8c1100f157b75fdffffff0200e87648170000001976a9147175910e71115b404df6e77121a938c3420ef08a88ac1fe77648170000001976a914ce84f745d0cb4869f496dd7692d028b363bde13a88ac7200000002000000013e8cc434dbeda730c84f94c192a6f69c1c8c9062989985fc3b9066368242020f000000006a47304402204b8ac9648bd55f54b0fdc6f064b74f22b73e9843871610d31d291fd48b782a8f0220344c4c4e0b320a291284a9cdad61e32fdf970df0f62db0f807b2ed893d94d9a90121032d8b8512eff5174388b2ceefd7e87027a44260442cbec67f1b5d12bde56dc97bfdffffff02005847f80d0000001976a9140698b8f5e945227347e41e136554dc747f9673b588ac1f8f2f50090000001976a9142bacaec1abb1677acb23d0d141d90404781b796c88ac11000000020000000104ba794c4bd796e19595b45ac588ac802ae39e137d4391cc711a4698cc44f40e010000006a47304402206b14e4fc46af11c95e7b98b23d4509ac91c21b434c9075b73327917f3bbfcfb002201f62381d7e52262b9ff2ce88ed5c27291cade8f6eb763c50843fdad6cc9e4202012103522785b1d3c3e56341c058294fc81cc96c805141ff9137f6ffb8186e8496998cfdffffff01608e2f50090000001976a9140153485616fa57faf01aa9b39e4977346dfab3f988ac720000000200000006a8bece298933f7d7773bae388151e06688f9e957e94bf8428f08a89f6dfef61f000000006a47304402202a824a0871d5016a63e1f6ad44ec534f4e110c28fad15163230405252c020d11022062b312748723c3a115b7df88f84855bc1d274a97a24705c24ee0d09f30f1a0510121030da0dcd85608c3f8013aa82a79b5256c0b425fbab6069aec15bd41f9862dafe1fdffffff3f398349c085947f24fe279eaa5d1dda3a140c9a603205877a3ee15974e80dfb000000006a47304402202c4f38ba99903131452ac6fe88dad4d7610205844a455b357d6a4c44469dcc1e0220302ce0b0921e95b176174038628b86afd1d3da564236df4d8dc346d093b4598e012102ac6cb8f1b72d845398f1eceebf82d9b41ede28d54cfe63093e03227b607a833dfdffffffd3165ce9f148deb7f0c340ad6809fd6398089f088c5dffbbd26181d3e4674bcb000000006a473044022067f33b157cda5ee9b2701f0fd06da70a7c1253c567cd2de511ff46f3aa93d4a802201f6bfeee587ac165380cdfd2f0eabe872b7a6a33d025133be2900e8b58c7710f01210358b27ae43caa182e0c31cc2edf997c713d06485e8fb2b903ce040a069203460efdffffffc94d74a198225a1f54c85f71f822d31fd1cc4b8f0411bf3b3fd8335e18bd9fe2010000006a47304402200ee8f945515219752eb05c6e0b663e5171665fc8d3a505204b888d20126d577f022061a10cab665927915e9564a0ff5a9979b8c2b3d5c7294e1f65b49c14a221fb2001210235ab9b33c1665890ac52997df4875de3c35d786292085698c94a45826d9894c9fdffffffa5be848dab206e33c5cbf2b89bdaa6b5e32a8e3aed9c3068fb6af768e50fa559000000006a473044022061dd6d8616607ce6831561832643b34bb6d64c028d829f28aed2ee819fc8d3a702203ac5b1355b4dd4dab3877cc07ce6d28cdee44a777f7f253e1a3786fcbc63592701210266487ac37cc79e5c04f5dcc8a6272586282173af9bb1019b581bfbe913fbd00afdffffff2fa0f0f07217b3b2ca60171451b0df629b9b4ff7c9d1198a59af25220e77ab5f000000006a47304402204f2e0e5eeaf8b78987e28a29722c418693fa450de89af4a2653b0e9f65762cc302206125b3bd0656295dd543461ab211468ef91079290a41b59f8bf11b404fe8d392012102030e17b4a91d5600734273de9177b555003034ac4a509edbf390314c8509a606fdffffff0200743ba40b0000001976a914d7fa792d6c7009dc65c441dff1ed1f2a76fdd34888ac40e00b54020000001976a91438bda43d00737257e57c42b46142ad98c37e01e988ac7200000002000000020db5ad7f4a989b28dc1323c082bb8c532c3c76b2004c63855b2992e57dc10401010000006a4730440220054777ac182ddd809c06eb964a4568f61fd13ee3a65963300304cf46c53fbe0f02203eeb1abd5df3f78e51733d6bc44df57d1fe200f8c833fb47ca406604f2f01520012102c48fbf2c7b2d49c09d13618bb5e96f8fefcf23f84c5460dc94ec5c87b01ca6effdffffffd1bdd181075a3a231b39bd1a285fd322c468bed70349bbed346e675be5ba3ca6010000006a47304402203bb186efd194d663bfe1feed23e4dbce48d0b34fe1e10eb20878fd88be4379c902205514dba428f1a1c1c32788cb99814a311279a45f5f69075ecdb1402d3ee7ba650121026352805fd87cb11ed9b1fb7bd923ffe0b15453a32aaa88f81bcc0399e5fef4c0fdffffff028ce20b54020000001976a914e7b78010c23c02d6214d0a42c9503e119d95748588ac00e40b54020000001976a9142dbacd8de8c28ac1e5860cf99ca499862e0a522588ac3500000002000000020188b5c587c1829e9e7076c7519dc2f9ad52baf3e66fd3be1270295798c246c2000000006a4730440220042be7ebe4c0f01da1cf0485dc1422a074019b4395465703c87a1bb527eb379c02204333c5b0d98e197cdedd6754e2406e9ad814b6a1e9d3b011e8a57d8ff8b59bcc0121028f793773bd0c2c3d52d8cc3a0c3ea15cb12c9055f3ad7e7e7372c57180890858fdffffff3e8cc434dbeda730c84f94c192a6f69c1c8c9062989985fc3b9066368242020f010000006a4730440220209e67af18676329855f85d402cc3511776447f6842833762b1b86424d77e3c40220168cb436b2022ae35684e3f9add3bb1da53e81e7585fb14c31143947eec98ae001210219eeee6040e10231f1768bad1cf3e470fdf4619d025e985bee7d181a84948193fdffffff01eccced902e0000001976a914168e6ace5991fc68f9cdf3f65d05d815cd1b9b5488ac72000000"),
    };
    stream >> TX_WITH_WITNESS(block);

    CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    // Match the last transaction
    filter.insert(uint256S("0xeeeb9c3483da798dc68182727b7c728b3b162e9da415850241dc1fb34dc7eaf3"));

    CMerkleBlock merkleBlock(block, filter);
    BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

    BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 1);
    std::pair<unsigned int, uint256> pair = merkleBlock.vMatchedTxn[0];

    BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256S("0xeeeb9c3483da798dc68182727b7c728b3b162e9da415850241dc1fb34dc7eaf3"));
    BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 6);

    std::vector<uint256> vMatched;
    std::vector<unsigned int> vIndex;
    BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched, vIndex) == block.hashMerkleRoot);
    BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
    for (unsigned int i = 0; i < vMatched.size(); i++)
        BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);

    // Also match the 4th transaction
    filter.insert(uint256S("0x4176844c8a3e8e997b4e4e781ecb0d297463adda620b7bb562dd8b386c270d29"));
    merkleBlock = CMerkleBlock(block, filter);
    BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

    BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 2);

    BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256S("0x4176844c8a3e8e997b4e4e781ecb0d297463adda620b7bb562dd8b386c270d29"));
    BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 3);

    BOOST_CHECK(merkleBlock.vMatchedTxn[1] == pair);

    BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched, vIndex) == block.hashMerkleRoot);
    BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
    for (unsigned int i = 0; i < vMatched.size(); i++)
        BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);
}

BOOST_AUTO_TEST_CASE(merkle_block_4_test_p2pubkey_only)
{
    // Random test block (0x6cd76c4719bc5648531abce846fc37e07abc0329a2ba98a392802d5800698174)
    // With 7 txes
    CBlock block;
    DataStream stream{
        ParseHex("00000020945cc37f82109f0ad351d730e58777ddb594e2cb9b4daba40fa66fe48cffb7111ea1fd722154d32fcb5c237010c2152dc8605528c75c31bb0e4ce09795914ab051283166ffff7f207300000006000000000000001bbb6a42a8766baf3fcdcd90dfd685d0e28217d172b92c22a2c948e0a87165be07020000000001010000000000000000000000000000000000000000000000000000000000000000ffffffff03017300ffffffff0207d9ed902e0000001976a9143d43ee6b2e01b439908f07c5715ed0572668aa9088ac0000000000000000266a24aa21a9ed197167d846cad7fdddba268be19702fa73aa3c305092ca0147bc699412be104201200000000000000000000000000000000000000000000000000000000000000000000000000200000001b89c60fb63e4f659a7f3bd1379416f6bb6336baa9b80839a826b8bb5cfb653c3010000006a47304402204ece81e64caec95e790f3f78e6aa1d062427f28df1322656ff8763e04964dc53022029e692a4549332d4b2185b8f8415ab46fdaab31f2fd622d078449490eab07722012103bb26267315c45f7a78abed0fee559367c7ffcc6843e6e82dded8c1100f157b75fdffffff0200e87648170000001976a9147175910e71115b404df6e77121a938c3420ef08a88ac1fe77648170000001976a914ce84f745d0cb4869f496dd7692d028b363bde13a88ac7200000002000000013e8cc434dbeda730c84f94c192a6f69c1c8c9062989985fc3b9066368242020f000000006a47304402204b8ac9648bd55f54b0fdc6f064b74f22b73e9843871610d31d291fd48b782a8f0220344c4c4e0b320a291284a9cdad61e32fdf970df0f62db0f807b2ed893d94d9a90121032d8b8512eff5174388b2ceefd7e87027a44260442cbec67f1b5d12bde56dc97bfdffffff02005847f80d0000001976a9140698b8f5e945227347e41e136554dc747f9673b588ac1f8f2f50090000001976a9142bacaec1abb1677acb23d0d141d90404781b796c88ac11000000020000000104ba794c4bd796e19595b45ac588ac802ae39e137d4391cc711a4698cc44f40e010000006a47304402206b14e4fc46af11c95e7b98b23d4509ac91c21b434c9075b73327917f3bbfcfb002201f62381d7e52262b9ff2ce88ed5c27291cade8f6eb763c50843fdad6cc9e4202012103522785b1d3c3e56341c058294fc81cc96c805141ff9137f6ffb8186e8496998cfdffffff01608e2f50090000001976a9140153485616fa57faf01aa9b39e4977346dfab3f988ac720000000200000006a8bece298933f7d7773bae388151e06688f9e957e94bf8428f08a89f6dfef61f000000006a47304402202a824a0871d5016a63e1f6ad44ec534f4e110c28fad15163230405252c020d11022062b312748723c3a115b7df88f84855bc1d274a97a24705c24ee0d09f30f1a0510121030da0dcd85608c3f8013aa82a79b5256c0b425fbab6069aec15bd41f9862dafe1fdffffff3f398349c085947f24fe279eaa5d1dda3a140c9a603205877a3ee15974e80dfb000000006a47304402202c4f38ba99903131452ac6fe88dad4d7610205844a455b357d6a4c44469dcc1e0220302ce0b0921e95b176174038628b86afd1d3da564236df4d8dc346d093b4598e012102ac6cb8f1b72d845398f1eceebf82d9b41ede28d54cfe63093e03227b607a833dfdffffffd3165ce9f148deb7f0c340ad6809fd6398089f088c5dffbbd26181d3e4674bcb000000006a473044022067f33b157cda5ee9b2701f0fd06da70a7c1253c567cd2de511ff46f3aa93d4a802201f6bfeee587ac165380cdfd2f0eabe872b7a6a33d025133be2900e8b58c7710f01210358b27ae43caa182e0c31cc2edf997c713d06485e8fb2b903ce040a069203460efdffffffc94d74a198225a1f54c85f71f822d31fd1cc4b8f0411bf3b3fd8335e18bd9fe2010000006a47304402200ee8f945515219752eb05c6e0b663e5171665fc8d3a505204b888d20126d577f022061a10cab665927915e9564a0ff5a9979b8c2b3d5c7294e1f65b49c14a221fb2001210235ab9b33c1665890ac52997df4875de3c35d786292085698c94a45826d9894c9fdffffffa5be848dab206e33c5cbf2b89bdaa6b5e32a8e3aed9c3068fb6af768e50fa559000000006a473044022061dd6d8616607ce6831561832643b34bb6d64c028d829f28aed2ee819fc8d3a702203ac5b1355b4dd4dab3877cc07ce6d28cdee44a777f7f253e1a3786fcbc63592701210266487ac37cc79e5c04f5dcc8a6272586282173af9bb1019b581bfbe913fbd00afdffffff2fa0f0f07217b3b2ca60171451b0df629b9b4ff7c9d1198a59af25220e77ab5f000000006a47304402204f2e0e5eeaf8b78987e28a29722c418693fa450de89af4a2653b0e9f65762cc302206125b3bd0656295dd543461ab211468ef91079290a41b59f8bf11b404fe8d392012102030e17b4a91d5600734273de9177b555003034ac4a509edbf390314c8509a606fdffffff0200743ba40b0000001976a914d7fa792d6c7009dc65c441dff1ed1f2a76fdd34888ac40e00b54020000001976a91438bda43d00737257e57c42b46142ad98c37e01e988ac7200000002000000020db5ad7f4a989b28dc1323c082bb8c532c3c76b2004c63855b2992e57dc10401010000006a4730440220054777ac182ddd809c06eb964a4568f61fd13ee3a65963300304cf46c53fbe0f02203eeb1abd5df3f78e51733d6bc44df57d1fe200f8c833fb47ca406604f2f01520012102c48fbf2c7b2d49c09d13618bb5e96f8fefcf23f84c5460dc94ec5c87b01ca6effdffffffd1bdd181075a3a231b39bd1a285fd322c468bed70349bbed346e675be5ba3ca6010000006a47304402203bb186efd194d663bfe1feed23e4dbce48d0b34fe1e10eb20878fd88be4379c902205514dba428f1a1c1c32788cb99814a311279a45f5f69075ecdb1402d3ee7ba650121026352805fd87cb11ed9b1fb7bd923ffe0b15453a32aaa88f81bcc0399e5fef4c0fdffffff028ce20b54020000001976a914e7b78010c23c02d6214d0a42c9503e119d95748588ac00e40b54020000001976a9142dbacd8de8c28ac1e5860cf99ca499862e0a522588ac3500000002000000020188b5c587c1829e9e7076c7519dc2f9ad52baf3e66fd3be1270295798c246c2000000006a4730440220042be7ebe4c0f01da1cf0485dc1422a074019b4395465703c87a1bb527eb379c02204333c5b0d98e197cdedd6754e2406e9ad814b6a1e9d3b011e8a57d8ff8b59bcc0121028f793773bd0c2c3d52d8cc3a0c3ea15cb12c9055f3ad7e7e7372c57180890858fdffffff3e8cc434dbeda730c84f94c192a6f69c1c8c9062989985fc3b9066368242020f010000006a4730440220209e67af18676329855f85d402cc3511776447f6842833762b1b86424d77e3c40220168cb436b2022ae35684e3f9add3bb1da53e81e7585fb14c31143947eec98ae001210219eeee6040e10231f1768bad1cf3e470fdf4619d025e985bee7d181a84948193fdffffff01eccced902e0000001976a914168e6ace5991fc68f9cdf3f65d05d815cd1b9b5488ac72000000"),
    };
    stream >> TX_WITH_WITNESS(block);

    CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_P2PUBKEY_ONLY);
    // Match the generation script
    filter.insert(ParseHex("76a9143d43ee6b2e01b439908f07c5715ed0572668aa9088ac"));
    // ...and the output address of the 4th transaction
    filter.insert(ParseHex("0153485616fa57faf01aa9b39e4977346dfab3f9"));

    CMerkleBlock merkleBlock(block, filter);
    BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

    // We shouldn't match any outpoints (NOT_P2PUBKEY)
    BOOST_CHECK(!filter.contains(COutPoint(TxidFromString("0xc6c86ce439f63792ba68460098dc26bab41dceca3ec504f0017125db24580960"), 0)));
    BOOST_CHECK(!filter.contains(COutPoint(TxidFromString("0x4176844c8a3e8e997b4e4e781ecb0d297463adda620b7bb562dd8b386c270d29"), 0)));
}

BOOST_AUTO_TEST_CASE(merkle_block_4_test_update_none)
{
    // Random test block (0x6cd76c4719bc5648531abce846fc37e07abc0329a2ba98a392802d5800698174)
    // With 7 txes
    CBlock block;
    DataStream stream{
        ParseHex("00000020945cc37f82109f0ad351d730e58777ddb594e2cb9b4daba40fa66fe48cffb7111ea1fd722154d32fcb5c237010c2152dc8605528c75c31bb0e4ce09795914ab051283166ffff7f207300000006000000000000001bbb6a42a8766baf3fcdcd90dfd685d0e28217d172b92c22a2c948e0a87165be07020000000001010000000000000000000000000000000000000000000000000000000000000000ffffffff03017300ffffffff0207d9ed902e0000001976a9143d43ee6b2e01b439908f07c5715ed0572668aa9088ac0000000000000000266a24aa21a9ed197167d846cad7fdddba268be19702fa73aa3c305092ca0147bc699412be104201200000000000000000000000000000000000000000000000000000000000000000000000000200000001b89c60fb63e4f659a7f3bd1379416f6bb6336baa9b80839a826b8bb5cfb653c3010000006a47304402204ece81e64caec95e790f3f78e6aa1d062427f28df1322656ff8763e04964dc53022029e692a4549332d4b2185b8f8415ab46fdaab31f2fd622d078449490eab07722012103bb26267315c45f7a78abed0fee559367c7ffcc6843e6e82dded8c1100f157b75fdffffff0200e87648170000001976a9147175910e71115b404df6e77121a938c3420ef08a88ac1fe77648170000001976a914ce84f745d0cb4869f496dd7692d028b363bde13a88ac7200000002000000013e8cc434dbeda730c84f94c192a6f69c1c8c9062989985fc3b9066368242020f000000006a47304402204b8ac9648bd55f54b0fdc6f064b74f22b73e9843871610d31d291fd48b782a8f0220344c4c4e0b320a291284a9cdad61e32fdf970df0f62db0f807b2ed893d94d9a90121032d8b8512eff5174388b2ceefd7e87027a44260442cbec67f1b5d12bde56dc97bfdffffff02005847f80d0000001976a9140698b8f5e945227347e41e136554dc747f9673b588ac1f8f2f50090000001976a9142bacaec1abb1677acb23d0d141d90404781b796c88ac11000000020000000104ba794c4bd796e19595b45ac588ac802ae39e137d4391cc711a4698cc44f40e010000006a47304402206b14e4fc46af11c95e7b98b23d4509ac91c21b434c9075b73327917f3bbfcfb002201f62381d7e52262b9ff2ce88ed5c27291cade8f6eb763c50843fdad6cc9e4202012103522785b1d3c3e56341c058294fc81cc96c805141ff9137f6ffb8186e8496998cfdffffff01608e2f50090000001976a9140153485616fa57faf01aa9b39e4977346dfab3f988ac720000000200000006a8bece298933f7d7773bae388151e06688f9e957e94bf8428f08a89f6dfef61f000000006a47304402202a824a0871d5016a63e1f6ad44ec534f4e110c28fad15163230405252c020d11022062b312748723c3a115b7df88f84855bc1d274a97a24705c24ee0d09f30f1a0510121030da0dcd85608c3f8013aa82a79b5256c0b425fbab6069aec15bd41f9862dafe1fdffffff3f398349c085947f24fe279eaa5d1dda3a140c9a603205877a3ee15974e80dfb000000006a47304402202c4f38ba99903131452ac6fe88dad4d7610205844a455b357d6a4c44469dcc1e0220302ce0b0921e95b176174038628b86afd1d3da564236df4d8dc346d093b4598e012102ac6cb8f1b72d845398f1eceebf82d9b41ede28d54cfe63093e03227b607a833dfdffffffd3165ce9f148deb7f0c340ad6809fd6398089f088c5dffbbd26181d3e4674bcb000000006a473044022067f33b157cda5ee9b2701f0fd06da70a7c1253c567cd2de511ff46f3aa93d4a802201f6bfeee587ac165380cdfd2f0eabe872b7a6a33d025133be2900e8b58c7710f01210358b27ae43caa182e0c31cc2edf997c713d06485e8fb2b903ce040a069203460efdffffffc94d74a198225a1f54c85f71f822d31fd1cc4b8f0411bf3b3fd8335e18bd9fe2010000006a47304402200ee8f945515219752eb05c6e0b663e5171665fc8d3a505204b888d20126d577f022061a10cab665927915e9564a0ff5a9979b8c2b3d5c7294e1f65b49c14a221fb2001210235ab9b33c1665890ac52997df4875de3c35d786292085698c94a45826d9894c9fdffffffa5be848dab206e33c5cbf2b89bdaa6b5e32a8e3aed9c3068fb6af768e50fa559000000006a473044022061dd6d8616607ce6831561832643b34bb6d64c028d829f28aed2ee819fc8d3a702203ac5b1355b4dd4dab3877cc07ce6d28cdee44a777f7f253e1a3786fcbc63592701210266487ac37cc79e5c04f5dcc8a6272586282173af9bb1019b581bfbe913fbd00afdffffff2fa0f0f07217b3b2ca60171451b0df629b9b4ff7c9d1198a59af25220e77ab5f000000006a47304402204f2e0e5eeaf8b78987e28a29722c418693fa450de89af4a2653b0e9f65762cc302206125b3bd0656295dd543461ab211468ef91079290a41b59f8bf11b404fe8d392012102030e17b4a91d5600734273de9177b555003034ac4a509edbf390314c8509a606fdffffff0200743ba40b0000001976a914d7fa792d6c7009dc65c441dff1ed1f2a76fdd34888ac40e00b54020000001976a91438bda43d00737257e57c42b46142ad98c37e01e988ac7200000002000000020db5ad7f4a989b28dc1323c082bb8c532c3c76b2004c63855b2992e57dc10401010000006a4730440220054777ac182ddd809c06eb964a4568f61fd13ee3a65963300304cf46c53fbe0f02203eeb1abd5df3f78e51733d6bc44df57d1fe200f8c833fb47ca406604f2f01520012102c48fbf2c7b2d49c09d13618bb5e96f8fefcf23f84c5460dc94ec5c87b01ca6effdffffffd1bdd181075a3a231b39bd1a285fd322c468bed70349bbed346e675be5ba3ca6010000006a47304402203bb186efd194d663bfe1feed23e4dbce48d0b34fe1e10eb20878fd88be4379c902205514dba428f1a1c1c32788cb99814a311279a45f5f69075ecdb1402d3ee7ba650121026352805fd87cb11ed9b1fb7bd923ffe0b15453a32aaa88f81bcc0399e5fef4c0fdffffff028ce20b54020000001976a914e7b78010c23c02d6214d0a42c9503e119d95748588ac00e40b54020000001976a9142dbacd8de8c28ac1e5860cf99ca499862e0a522588ac3500000002000000020188b5c587c1829e9e7076c7519dc2f9ad52baf3e66fd3be1270295798c246c2000000006a4730440220042be7ebe4c0f01da1cf0485dc1422a074019b4395465703c87a1bb527eb379c02204333c5b0d98e197cdedd6754e2406e9ad814b6a1e9d3b011e8a57d8ff8b59bcc0121028f793773bd0c2c3d52d8cc3a0c3ea15cb12c9055f3ad7e7e7372c57180890858fdffffff3e8cc434dbeda730c84f94c192a6f69c1c8c9062989985fc3b9066368242020f010000006a4730440220209e67af18676329855f85d402cc3511776447f6842833762b1b86424d77e3c40220168cb436b2022ae35684e3f9add3bb1da53e81e7585fb14c31143947eec98ae001210219eeee6040e10231f1768bad1cf3e470fdf4619d025e985bee7d181a84948193fdffffff01eccced902e0000001976a914168e6ace5991fc68f9cdf3f65d05d815cd1b9b5488ac72000000"),
    };
    stream >> TX_WITH_WITNESS(block);

    CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_NONE);
    // Match the generation script
    filter.insert(ParseHex("76a9143d43ee6b2e01b439908f07c5715ed0572668aa9088ac"));
    // ...and the output address of the 4th transaction
    filter.insert(ParseHex("0153485616fa57faf01aa9b39e4977346dfab3f9"));

    CMerkleBlock merkleBlock(block, filter);
    BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

    // We shouldn't match any outpoints (UPDATE_NONE)
    BOOST_CHECK(!filter.contains(COutPoint(TxidFromString("0xc6c86ce439f63792ba68460098dc26bab41dceca3ec504f0017125db24580960"), 0)));
    BOOST_CHECK(!filter.contains(COutPoint(TxidFromString("0x4176844c8a3e8e997b4e4e781ecb0d297463adda620b7bb562dd8b386c270d29"), 0)));
}

static std::vector<unsigned char> RandomData()
{
    uint256 r = InsecureRand256();
    return std::vector<unsigned char>(r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(rolling_bloom)
{
    SeedInsecureRand(SeedRand::ZEROS);
    g_mock_deterministic_tests = true;

    // last-100-entry, 1% false positive:
    CRollingBloomFilter rb1(100, 0.01);

    // Overfill:
    static const int DATASIZE=399;
    std::vector<unsigned char> data[DATASIZE];
    for (int i = 0; i < DATASIZE; i++) {
        data[i] = RandomData();
        rb1.insert(data[i]);
    }
    // Last 100 guaranteed to be remembered:
    for (int i = 299; i < DATASIZE; i++) {
        BOOST_CHECK(rb1.contains(data[i]));
    }

    // false positive rate is 1%, so we should get about 100 hits if
    // testing 10,000 random keys. We get worst-case false positive
    // behavior when the filter is as full as possible, which is
    // when we've inserted one minus an integer multiple of nElement*2.
    unsigned int nHits = 0;
    for (int i = 0; i < 10000; i++) {
        if (rb1.contains(RandomData()))
            ++nHits;
    }
    // Expect about 100 hits
    BOOST_CHECK_EQUAL(nHits, 75U);

    BOOST_CHECK(rb1.contains(data[DATASIZE-1]));
    rb1.reset();
    BOOST_CHECK(!rb1.contains(data[DATASIZE-1]));

    // Now roll through data, make sure last 100 entries
    // are always remembered:
    for (int i = 0; i < DATASIZE; i++) {
        if (i >= 100)
            BOOST_CHECK(rb1.contains(data[i-100]));
        rb1.insert(data[i]);
        BOOST_CHECK(rb1.contains(data[i]));
    }

    // Insert 999 more random entries:
    for (int i = 0; i < 999; i++) {
        std::vector<unsigned char> d = RandomData();
        rb1.insert(d);
        BOOST_CHECK(rb1.contains(d));
    }
    // Sanity check to make sure the filter isn't just filling up:
    nHits = 0;
    for (int i = 0; i < DATASIZE; i++) {
        if (rb1.contains(data[i]))
            ++nHits;
    }
    // Expect about 5 false positives
    BOOST_CHECK_EQUAL(nHits, 6U);

    // last-1000-entry, 0.01% false positive:
    CRollingBloomFilter rb2(1000, 0.001);
    for (int i = 0; i < DATASIZE; i++) {
        rb2.insert(data[i]);
    }
    // ... room for all of them:
    for (int i = 0; i < DATASIZE; i++) {
        BOOST_CHECK(rb2.contains(data[i]));
    }
    g_mock_deterministic_tests = false;
}

BOOST_AUTO_TEST_SUITE_END()
