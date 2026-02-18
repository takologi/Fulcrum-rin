// Copyright (c) 2024-2025 The Rincoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "../uint256.h"

namespace bitcoin {

class CBlockHeader;

uint256 RinHash(const CBlockHeader& block);

} // namespace bitcoin
