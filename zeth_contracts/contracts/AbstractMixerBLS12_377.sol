// Copyright (c) 2015-2021 Clearmatics Technologies Ltd
//
// SPDX-License-Identifier: LGPL-3.0+

pragma solidity ^0.8.0;

import "./AbstractMixer.sol";
import "./LibMiMC.sol";

/// Partial implementation of AbstractMixer which implements the
/// curve-specific methods to use the BLS12-377 pairing.
abstract contract AbstractMixerBLS12_377 is AbstractMixer
{
    // TODO: Code here is very similar to AbstractMixerAltBN128, with only the
    // constants changed. Look into sharing more code (possibly by making some
    // of these constants dynamic).

    // Constants regarding the hash digest length, the prime number used and
    // its associated length in bits and the max values (v_in and v_out)

    // Number of bits that can be reliably represented by a single field
    // element:
    //   _FIELD_CAPACITY = floor( log_2(r) )
    // Denoted FIELDCAP in Zeth specifications.
    uint256 internal constant _FIELD_CAPACITY = 252;

    // Number of residual bits per bytes32
    uint256 internal constant _NUM_RESIDUAL_BITS =
        _DIGEST_LENGTH - _FIELD_CAPACITY;

    // Shift to move residual bits from lowest order to highest order
    uint256 internal constant _RESIDUAL_BITS_SHIFT = 256 - _NUM_RESIDUAL_BITS;

    // Mask to extract the residual bits in the high-order position
    uint256 internal constant _RESIDUAL_BITS_MASK =
        ((1 << _NUM_RESIDUAL_BITS) - 1) << _RESIDUAL_BITS_SHIFT;

    constructor(
        uint256 mkDepth,
        address token,
        uint256[] memory vk,
        address permittedDispatcher,
        uint256[2] memory vkHash
    )
        AbstractMixer(mkDepth, token, vk, permittedDispatcher, vkHash)
    {
    }

    function _hash(bytes32 left, bytes32 right)
        internal
        pure
        override
        returns(bytes32)
    {
        return LibMiMC._hashBLS12_377(left, right);
    }

    /// Extract a full uint256 from a field element and the n-th set of
    /// residual bits from `residual`.
    function _extractBytes32(
        uint256 fieldElement,
        uint256 residual,
        uint256 residualBitsSetIdx
    )
        internal
        pure
        override
        returns(bytes32)
    {
        // The residualBitsSetIdx-th set of residual bits (denoted r_i
        // below) start at bit:
        //   (2*_PUBLIC_VALUE_BITS) + (residualBitsSetIdx*num_residual_bits)
        //
        // Shift r_i to occupy the highest order bits:
        // 255                                       128        64           0
        //  |  bitsToShift  |     |  residualBitsIdx  |          |           |
        //  | <------------ |<r_i>|                   |<v_pub_in>|<v_pub_out>|

        // Number of bits AFTER public values
        uint256 residualBitsIdx = residualBitsSetIdx * _NUM_RESIDUAL_BITS;
        uint256 bitsToShift =
        _RESIDUAL_BITS_SHIFT - _TOTAL_PUBLIC_VALUE_BITS - residualBitsIdx;
        uint256 residualBits =
            (residual << bitsToShift) & _RESIDUAL_BITS_MASK;
        return bytes32(fieldElement | residualBits);
    }
}
