// Copyright (c) 2015-2021 Clearmatics Technologies Ltd
//
// SPDX-License-Identifier: LGPL-3.0+

#include "libzeth/core/bits.hpp"

#include "libzeth/core/include_libff.hpp"
#include "libzeth/core/utils.hpp"

namespace libzeth
{

std::vector<bool> bit_vector_from_hex(const std::string &hex_str)
{
    std::vector<bool> result;
    result.reserve(4 * hex_str.size());
    for (char c : hex_str) {
        const uint8_t nibble = char_to_nibble(c);
        result.push_back(nibble & 8);
        result.push_back(nibble & 4);
        result.push_back(nibble & 2);
        result.push_back(nibble & 1);
    }

    return result;
}

std::vector<bool> bit_vector_from_size_t_le(size_t x)
{
    std::vector<bool> ret;
    while (x) {
        ret.push_back((x & 1) != 0);
        x >>= 1;
    }

    return ret;
}

std::vector<bool> bit_vector_from_size_t_be(size_t x)
{
    std::vector<bool> res;
    size_t num_bits = 8 * sizeof(size_t);
    const size_t mask = 1ULL << (num_bits - 1);

    // Remove 0-bits at the front
    while (num_bits > 0) {
        if ((x & mask) != 0) {
            break;
        }
        x = x << 1;
        --num_bits;
    }

    // Pre-allocate and fill the vector with remaining bits
    res.reserve(num_bits);
    while (num_bits > 0) {
        res.push_back((x & mask) != 0);
        x = x << 1;
        --num_bits;
    }

    return res;
}

} // namespace libzeth
