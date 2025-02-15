// Copyright (c) 2015-2021 Clearmatics Technologies Ltd
//
// SPDX-License-Identifier: LGPL-3.0+

#ifndef __ZETH_SNARKS_GROTH16_GROTH16_SNARK_HPP__
#define __ZETH_SNARKS_GROTH16_GROTH16_SNARK_HPP__

#include <libsnark/gadgetlib1/protoboard.hpp>
#include <libsnark/zk_proof_systems/ppzksnark/r1cs_gg_ppzksnark/r1cs_gg_ppzksnark.hpp>

namespace libzeth
{

/// Core types and operations for the GROTH16 snark
template<typename ppT> class groth16_snark
{
public:
    using proving_key = libsnark::r1cs_gg_ppzksnark_proving_key<ppT>;
    using verification_key = libsnark::r1cs_gg_ppzksnark_verification_key<ppT>;
    using keypair = libsnark::r1cs_gg_ppzksnark_keypair<ppT>;
    using proof = libsnark::r1cs_gg_ppzksnark_proof<ppT>;

    /// String name of this snark, corresponding to <SNARK> in the
    /// ZETH_SNARK_<SNARK> configuration variable.
    static const std::string name;

    /// Run the trusted setup and return the keypair for the circuit
    static keypair generate_setup(
        const libsnark::protoboard<libff::Fr<ppT>> &pb);

    /// Generate the proof (from the values set to the protoboard)
    static proof generate_proof(
        const proving_key &proving_key,
        const libsnark::protoboard<libff::Fr<ppT>> &pb);

    /// Generate the proof (from given primary and auxiliary values)
    static proof generate_proof(
        const proving_key &proving_key,
        const libsnark::r1cs_primary_input<libff::Fr<ppT>> &primary_input,
        const libsnark::r1cs_auxiliary_input<libff::Fr<ppT>> auxiliary_input);

    /// Verify proof
    static bool verify(
        const libsnark::r1cs_primary_input<libff::Fr<ppT>> &primary_inputs,
        const proof &proof,
        const verification_key &verification_key);

    /// Write verification as json
    static void verification_key_write_json(
        const verification_key &, std::ostream &);

    /// Write verification key as bytes
    static void verification_key_write_bytes(
        const verification_key &, std::ostream &);

    /// Read a verification key as bytes
    static void verification_key_read_bytes(verification_key &, std::istream &);

    /// Write proving key as bytes
    static void proving_key_write_bytes(const proving_key &, std::ostream &);

    /// Read proving key as bytes
    static void proving_key_read_bytes(proving_key &, std::istream &);

    /// Write proof as json.
    static void proof_write_json(const proof &, std::ostream &);

    /// Write proof as bytes
    static void proof_write_bytes(const proof &, std::ostream &);

    /// Read proof as bytes
    static void proof_read_bytes(proof &, std::istream &);

    /// Write a keypair as bytes
    static void keypair_write_bytes(const keypair &, std::ostream &);

    /// Read a keypair from a stream.
    static void keypair_read_bytes(keypair &, std::istream &);
};

/// Check well-formedness of a proving key
template<typename ppT>
static bool is_well_formed(const typename groth16_snark<ppT>::proving_key &pk);

/// Check well-formedness of a verification key
template<typename ppT>
static bool is_well_formed(
    const typename groth16_snark<ppT>::verification_key &vk);

} // namespace libzeth

#include "libzeth/snarks/groth16/groth16_snark.tcc"

#endif // __ZETH_SNARKS_GROTH16_GROTH16_SNARK_HPP__
