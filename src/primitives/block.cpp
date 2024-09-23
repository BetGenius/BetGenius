// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2019 The Bitcoin Core developers
// Copyright (c) 2024 The Betgenius Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/block.h>

#include <hash.h>
#include <tinyformat.h>


uint256 CBlockHeader::GetHash() const
{
    return ETHash(*this);
}

uint256 CBlockHeader::GetHash(uint256& hashMix) const
{
    return ETHash(*this, hashMix);
}

uint256 CBlockHeader::GetHeaderHash() const
{
    CHashInput input{*this};
    return (HashWriter{} << input).GetHash();
}

std::string CBlockHeader::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlockHeader(ver=0x%08x, nHeight=%u, hashMix=%s, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u)\n",
                   nVersion,
                   nHeight,
                   hashMix.ToString(),
                   hashPrevBlock.ToString(),
                   hashMerkleRoot.ToString(),
                   nTime, nBits, nNonce);
    return s.str();
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(ver=0x%08x, nHeight=%u, hash=%s, hashHeader=%s, hashMix=%s, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
        nVersion,
        nHeight,
        GetHash().ToString(),
        GetHeaderHash().ToString(),
        hashMix.ToString(),
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}