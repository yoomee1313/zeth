pragma solidity ^0.5.0;

/*
 * Reference paper:
 *
 * \[Bel07]:
 * "Two-Tier Signatures, Strongly Unforgeable Signatures and Fiat-Shamir without Random Oracles",
 * Mihir Bellare, Sarah Shoup,
 * International Workshop on Public Key Cryptography, 2007,
 * <https://eprint.iacr.org/2007/273.pdf>
**/

import "./Pairing.sol";
import "./Bytes.sol";

contract OTSchnorrVerifier {
    using Pairing for *;
    using Bytes for *;

    constructor(
    ) public {
    }

    function verify(
        uint[2][2] memory vk,
        uint sigma,
        bytes32 hash_ciphers,
        bytes32 hash_proof,
        bytes32 hash_inputs
    ) public returns (bool) {

        bytes32 h_bytes = sha256(abi.encodePacked(vk[1][0], vk[1][1], hash_ciphers, hash_proof, hash_inputs));
        uint h = uint(h_bytes);

        Pairing.G1Point memory X = Pairing.G1Point(vk[0][0], vk[0][1]);
        Pairing.G1Point memory Y = Pairing.G1Point(vk[1][0], vk[1][1]);
        Pairing.G1Point memory S = Pairing.mul(Pairing.P1(), sigma);

        Pairing.G1Point memory S_comp = Pairing.add(Y, Pairing.mul(X, h));

        return (S.X == S_comp.X && S.Y == S_comp.Y);
    }
}