// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "block.h"

#include "amount.h"

#include "crypto/common.h"
#include "crypto/rinhash.h"
#include "hash.h"
#include "tinyformat.h"
#include "utilstrencodings.h"

namespace bitcoin {

uint256 CBlockHeader::GetHash() const {
    // Dispatch to the correct proof-of-work hash function for each supported coin.
    // Add a new branch here when adding support for a coin with a custom hash algorithm.
    const auto &unit = GetCurrencyUnit();
    if (unit == "RIN") {
        // Rincoin: BLAKE3 -> Argon2d -> SHA3-256.
        // RinHash() can throw on libargon2 failure (e.g. allocator error). Catch here
        // so that a transient failure in a single header does not abort the server;
        // return an all-zeroes hash so any chain-linkage check (hashPrevBlock) fails
        // and the caller treats the header as invalid rather than crashing.
        try {
            return RinHash(*this);
        } catch (const std::exception &e) {
            // Note: a zeroed uint256 is the same value used by the genesis prev-hash;
            // any non-genesis context will refuse this as a chain link.
            (void)e; // logger is not available in this translation unit
            return uint256();
        }
    } else if (unit == "BTC") {
        // Bitcoin Core / Bitcoin Knots: SHA256d
        return SerializeHash(*this);
    } else if (unit == "LTC") {
        // Litecoin: SHA256d (same algorithm, different chain params)
        return SerializeHash(*this);
    } else {
        // BCH and any other unknown coin: SHA256d
        return SerializeHash(*this);
    }
}

std::string CBlock::ToString(bool fVerbose) const {
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, "
                   "hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, "
                   "vtx=%u)\n",
                   GetHash().ToString(), nVersion, hashPrevBlock.ToString(),
                   hashMerkleRoot.ToString(), nTime, nBits, nNonce, vtx.size());
    for (const auto &tx : vtx) {
        s << "  " << tx->ToString(fVerbose) << "\n";
    }
    return s.str();
}

} // end namespace bitcoin
