#include "gtest/gtest.h"

#include <libff/common/default_types/ec_pp.hpp>
#include <libsnark/common/default_types/r1cs_gg_ppzksnark_pp.hpp>
#include <libsnark/zk_proof_systems/ppzksnark/r1cs_gg_ppzksnark/r1cs_gg_ppzksnark.hpp>

// Header to use the merkle tree data structure
#include <libsnark/common/data_structures/merkle_tree.hpp>

// Used to instantiate our templates
#include <libsnark/common/default_types/r1cs_ppzksnark_pp.hpp>
#include <libff/algebra/curves/public_params.hpp>
#include <libff/algebra/curves/alt_bn128/alt_bn128_pp.hpp>

// Header to use the sha256_ethereum gadget
#include "sha256_ethereum.hpp"

#include "util.hpp"

using namespace libsnark;

typedef libff::default_ec_pp ppT;
typedef libff::Fr<ppT> FieldT; // Should be alt_bn128 in the CMakeLists.txt
typedef sha256_ethereum<FieldT> HashT; // We use our hash function to do the tests

// Note on the instantiation of the FieldT template type
//
// We use the alt_bn128_pp public params, with a field instantiated with libff::Fr<ppT>
// which corresponds (according to libff/algebra/curves/public_params.hpp) to 
// the typedef 'typedef alt_bn128_Fr Fp_type;' (see: libff/algebra/curves/alt_bn128/alt_bn128_pp.hpp)
// 'alt_bn128_Fr' being itself defined in 'libff/algebra/curves/alt_bn128/alt_bn128_init.hpp'
// as 'typedef Fp_model<alt_bn128_r_limbs, alt_bn128_modulus_r> alt_bn128_Fr;'
//
// The Fp_model class is defined in 'libff/algebra/fields/fp.hpp' and implements
// arithmetic in the finite field F[p], for prime p of fixed length. (p being passed as a template)
// like:
// ```
// template<mp_size_t n, const bigint<n>& modulus>
// class Fp_model {
// ```
//
// In our case, the modulus is 'alt_bn128_modulus_r' is initialized to the value:
// ` alt_bn128_modulus_r = bigint_r("21888242871839275222246405745257275088548364400416034343698204186575808495617");`
// in the 'libff/algebra/curves/alt_bn128/alt_bn128_init.hpp' file

namespace {

void dump_bit_vector(std::ostream &out, const libff::bit_vector &v)
{
    out << "{";
    for (size_t i = 0; i < v.size() - 1; ++i)
    {
        out << v[i] << ", ";
    }
    out << v[v.size() - 1] << "}\n";
}

TEST(TestSHA256, TestHash) {
    libsnark::protoboard<FieldT> pb;
    libsnark::pb_variable<FieldT> ZERO;
    ZERO.allocate(pb);
    pb.val(ZERO) = FieldT::zero();

    // hex: 0x0F000000000000FF00000000000000FF00000000000000FF00000000000000FF
    libsnark::pb_variable_array<FieldT> left = from_bits(
        {
            0, 0, 0, 0, 1, 1, 1, 1, // 0, 0, 0, 0, 1, 1, 1, 1,
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            1, 1, 1, 1, 1, 1, 1, 1, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            1, 1, 1, 1, 1, 1, 1, 1, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            1, 1, 1, 1, 1, 1, 1, 1, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            1, 1, 1, 1, 1, 1, 1, 1
        }, ZERO
    );

    // hex: 0x43C000000000003FC00000000000003FC00000000000003FC00000000000003F
    libsnark::pb_variable_array<FieldT> right = from_bits(
        {
            0, 1, 0, 0, 0, 0, 1, 1, //  (0, 1 is the right prefix here)
            1, 1, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 1, 1, 1, 1, 1, 1, 
            1, 1, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 1, 1, 1, 1, 1, 1, 
            1, 1, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 1, 1, 1, 1, 1, 1, 
            1, 1, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0, 
            0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 1, 1, 1, 1, 1, 1
        }, ZERO
    );

    std::shared_ptr<libsnark::digest_variable<FieldT>> result;
    result.reset(new digest_variable<FieldT>(pb, HashT::get_digest_len(), "result"));

    std::shared_ptr<libsnark::block_variable<FieldT>> input_block;
    input_block.reset(new libsnark::block_variable<FieldT>(pb, {
            left,
            right
        }, "Block_variable")
    );
    
    std::shared_ptr<sha256_ethereum<FieldT>> hasher;
    hasher.reset(new sha256_ethereum<FieldT>(
        pb, 
        libsnark::SHA256_block_size,
        *input_block,
        *result,
        "Sha256_ethereum")
    );

    // result should equal: 0xa4cc8f23d1dfeab58d7af00b3422f22dd60b9c608af5f30744073653236562c3
    // Since result = sha256(left || right), where:
    // - left = 0x0F000000000000FF00000000000000FF00000000000000FF00000000000000FF
    // - right = 0x43C000000000003FC00000000000003FC00000000000003FC00000000000003F
    //
    // 0x0F000000000000FF00000000000000FF00000000000000FF00000000000000FF43C000000000003FC00000000000003FC00000000000003FC00000000000003F
    // 
    // Note: This test vector has been generated by using the solidity sha256 function
    // (we want to make sure that we generate the same digests both on-chain and off-chain)
    char* test_vector_res_str = "a4cc8f23d1dfeab58d7af00b3422f22dd60b9c608af5f30744073653236562c3";
    libsnark::pb_variable_array<FieldT> expected = from_bits(
        hexadecimal_digest_to_binary_vector(test_vector_res_str), 
        ZERO
    );

    hasher->generate_r1cs_constraints(true);
    hasher->generate_r1cs_witness();

    bool is_valid_witness = pb.is_satisfied();
    ASSERT_TRUE(is_valid_witness);

    std::cout << " -- Input block -- " << std::endl;
    std::ostream &stream = std::cout;
    dump_bit_vector(stream, input_block->get_block());
    std::cout << " -- Expected input block -- " << std::endl;
    dump_bit_vector(stream, left.get_bits(pb));
    dump_bit_vector(stream, right.get_bits(pb));

    std::cout << " -- Result digest -- " << std::endl;
    dump_bit_vector(stream, result->get_digest());
    std::cout << " -- Expected digest -- " << std::endl;
    dump_bit_vector(stream, expected.get_bits(pb));

    ASSERT_EQ(result->get_digest(), expected.get_bits(pb));
};

} // namespace

int main(int argc, char **argv) {
    ppT::init_public_params(); // /!\ WARNING: Do once for all tests. Do not forget to do this !!!!
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}