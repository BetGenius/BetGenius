// Copyright (c) 2024 The Bitcoin Core developers
// Copyright (c) 2024 The Betgenius Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>
#include <test/util/setup_common.h>

#include <crypto/ethash/lib/ethash/endianness.hpp>
#include <crypto/ethash/include/ethash/progpow.hpp>

#include <crypto/ethash/helpers.hpp>
#include <crypto/ethash/ethash_test_vectors.hpp>

#include <array>

BOOST_FIXTURE_TEST_SUITE(ethash_tests, TestingSetup)

BOOST_AUTO_TEST_CASE(ethash_l1_cache)
{
    auto& context = get_ethash_epoch_context_0();

    constexpr auto test_size = 20;
    std::array<uint32_t, test_size> cache_slice;
    for (size_t i = 0; i < cache_slice.size(); ++i)
    cache_slice[i] = ethash::le::uint32(context.l1_cache[i]);

    const std::array<uint32_t, test_size> expected{
        {2492749011, 430724829, 2029256771, 3095580433, 3583790154, 3025086503,
         805985885, 4121693337, 2320382801, 3763444918, 1006127899, 1480743010,
         2592936015, 2598973744, 3038068233, 2754267228, 2867798800, 2342573634,
         467767296, 246004123}};
    int i = 0;
    for (auto item : cache_slice) {
        BOOST_CHECK(item == expected[i]);
        i++;
    }
}

BOOST_AUTO_TEST_CASE(ethash_hash_empty)
{
    auto& context = get_ethash_epoch_context_0();

    int count = 1000;
    ethash_result result;
    while (count > 0) {
        result = progpow::hash(context, count, {}, 0);
        --count;
    }

    const auto mix_hex = "6e97b47b134fda0c7888802988e1a373affeb28bcd813b6e9a0fc669c935d03a";
    const auto final_hex = "e601a7257a70dc48fccc97a7330d704d776047623b92883d77111fb36870f3d1";
    BOOST_CHECK_EQUAL(to_hex(result.hashMix), mix_hex);
    BOOST_CHECK_EQUAL(to_hex(result.final_hash), final_hex);
}

BOOST_AUTO_TEST_CASE(ethash_hash_30000)
{
    const int blockNumber = 30000;
    const auto header =
            to_hash256("ffeeddccbbaa9988776655443322110000112233445566778899aabbccddeeff");
    const uint64_t nonce = 0x123456789abcdef0;

    auto context = ethash::create_epoch_context(ethash::get_epoch_number(blockNumber));

    const auto result = progpow::hash(*context, blockNumber, header, nonce);
    const auto mix_hex = "177b565752a375501e11b6d9d3679c2df6197b2cab3a1ba2d6b10b8c71a3d459";
    const auto final_hex = "c824bee0418e3cfb7fae56e0d5b3b8b14ba895777feea81c70c0ba947146da69";
    BOOST_CHECK_EQUAL(to_hex(result.hashMix), mix_hex);
    BOOST_CHECK_EQUAL(to_hex(result.final_hash), final_hex);

}

BOOST_AUTO_TEST_CASE(ethash_hash_and_verify)
{
    ethash::epoch_context_ptr context{nullptr, nullptr};

    for (auto& t : ethash_hash_test_cases)
    {
        const auto epoch_number = ethash::get_epoch_number(t.blockNumber);
        if (!context || context->epoch_number != epoch_number)
            context = ethash::create_epoch_context(epoch_number);

        const auto header_hash = to_hash256(t.headerHash);
        const auto nonce = std::stoull(t.nonce, nullptr, 16);
        const auto result = progpow::hash(*context, t.blockNumber, header_hash, nonce);
        BOOST_CHECK_EQUAL(to_hex(result.hashMix), t.hashMix);
        BOOST_CHECK_EQUAL(to_hex(result.final_hash), t.finalHash);

        auto success = progpow::verify(
                *context, t.blockNumber, header_hash, result.hashMix, nonce, result.final_hash);
        BOOST_CHECK(success);

        auto lower_boundary = result.final_hash;
        --lower_boundary.bytes[31];
        auto final_failure = progpow::verify(
                *context, t.blockNumber, header_hash, result.hashMix, nonce, lower_boundary);
        BOOST_CHECK(!final_failure);

        auto different_mix = result.hashMix;
        ++different_mix.bytes[7];
        auto mix_failure = progpow::verify(
                *context, t.blockNumber, header_hash, different_mix, nonce, result.final_hash);
        BOOST_CHECK(!mix_failure);
    }
}

BOOST_AUTO_TEST_CASE(ethash_search)
{
    auto ctxp = ethash::create_epoch_context_full(0);
    auto& ctx = *ctxp;
    auto& ctxl = reinterpret_cast<const ethash::epoch_context&>(ctx);

    auto boundary = to_hash256("00ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    auto sr = progpow::search(ctx, 0, {}, boundary, 700, 100);
    auto srl = progpow::search_light(ctxl, 0, {}, boundary, 700, 100);

    BOOST_CHECK(sr.hashMix == ethash::hash256{});
    BOOST_CHECK(sr.final_hash == ethash::hash256{});
    BOOST_CHECK(sr.nonce == 0x0);
    BOOST_CHECK(sr.hashMix == srl.hashMix);
    BOOST_CHECK(sr.final_hash == srl.final_hash);
    BOOST_CHECK(sr.nonce == srl.nonce);

    // Switch it to a different starting nonce and find another solution
    sr = progpow::search(ctx, 0, {}, boundary, 300, 100);
    srl = progpow::search_light(ctxl, 0, {}, boundary, 300, 100);

    BOOST_CHECK(sr.hashMix != ethash::hash256{});
    BOOST_CHECK(sr.final_hash != ethash::hash256{});
    BOOST_CHECK(sr.nonce == 395);
    BOOST_CHECK(sr.hashMix == srl.hashMix);
    BOOST_CHECK(sr.final_hash == srl.final_hash);
    BOOST_CHECK(sr.nonce == srl.nonce);

    auto r = progpow::hash(ctx, 0, {}, 395);
    BOOST_CHECK(sr.final_hash == r.final_hash);
    BOOST_CHECK(sr.hashMix == r.hashMix);
}

BOOST_AUTO_TEST_SUITE_END()