// Copyright (c) 2015-2021 Clearmatics Technologies Ltd
//
// SPDX-License-Identifier: LGPL-3.0+

pragma solidity ^0.8.0;

contract TestBLS12_377
{
    // In many cases, these numbers must be used as literals in the assembly
    // code.

    uint256 private constant _SCALAR_WORDS = 1;
    uint256 private constant _G1_COORD_WORDS = 2;
    uint256 private constant _G1_POINT_WORDS = 2 * _G1_COORD_WORDS;
    uint256 private constant _G2_COORD_WORDS = 4;
    uint256 private constant _G2_POINT_WORDS = 2 * _G2_COORD_WORDS;

    /// `input` is the concatenation of 2 encoded points in G1
    function testECAdd(bytes32[2 * _G1_POINT_WORDS] memory input)
        external
        returns(bytes32[_G1_POINT_WORDS] memory)
    {
        bytes32[_G1_POINT_WORDS] memory output;
        bool success = true;
        assembly
        {
            success := call(gas(), 0xc4, 0, input, 0x100, output, 0x80)
        }

        require(success, "precompiled contract call failed (ECAdd)");
        return output;
    }

    /// `input` is an encoded point in G1, followed by an encoded scalar.
    function testECMul(bytes32[_G1_POINT_WORDS + _SCALAR_WORDS] memory input)
        external
        returns(bytes32[_G1_POINT_WORDS] memory)
    {
        bytes32[_G1_POINT_WORDS] memory output;
        bool success = true;
        assembly
        {
            success := call(gas(), 0xc5, 0, input, 0xa0, output, 0x80)
        }

        require(success, "precompiled contract call failed (ECMul)");
        return output;
    }

    /// `input` is the concatenation of 4 pairs of encoded points. Each pair is
    /// a G1 point, followed by a G2 point. For BW6-761, both of these points
    /// are 6 words, so there should be 4 * 2 * 6 = 48 words.
    function testECPairing(
        bytes32[4 * (_G1_POINT_WORDS + _G2_POINT_WORDS)] memory input
    )
        external
        returns(uint256)
    {
        uint256[1] memory output;
        bool success = true;
        assembly
        {
            success := call(gas(), 0xc6, 0, input, 0x600, output, 0x20)
        }

        require(success, "precompiled contract call failed (ECMul)");
        return output[0];
    }
}
