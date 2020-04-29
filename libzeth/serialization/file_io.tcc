// Copyright (c) 2015-2020 Clearmatics Technologies Ltd
//
// SPDX-License-Identifier: LGPL-3.0+

#ifndef __ZETH_SERIALIZATION_FILE_IO_TCC__
#define __ZETH_SERIALIZATION_FILE_IO_TCC__

#include "libzeth/serialization/file_io.hpp"

namespace libzeth
{

/// SerializableT represents any type that overloads the operator<< and
/// operator>> of ostream and istream Note: Both r1cs_ppzksnark_proving_key and
/// r1cs_ppzksnark_verifying_key implement these overloading, so both of them
/// can easily be writen and loaded from files
template<typename serializableT>
void write_to_file(boost::filesystem::path path, const serializableT &obj)
{
    // Convert the boost path into char*
    const char *str_path = path.string().c_str();

    std::stringstream ss;
    ss << obj;

    // ofstream: Stream class to write on files
    std::ofstream fh;

    fh.open(str_path, std::ios::binary); // We open our ofstream in binary mode
    ss.rdbuf()->pubseekpos(0, std::ios_base::out);
    fh << ss.rdbuf();
    fh.flush();
    fh.close();
}

template<typename serializableT>
serializableT load_from_file(boost::filesystem::path path)
{
    // Convert the boost path into char*
    const char *str_path = path.string().c_str();

    std::stringstream ss;

    // ifstream: Stream class to read from files (opened in binary mode)
    std::ifstream fh(str_path, std::ios::binary);
    assert(fh.is_open());

    // Get a stream buffer from the ifstream and "dump" its content to the
    // stringstream
    ss << fh.rdbuf();
    fh.close();

    // Set internal position pointer to absolute position 0
    ss.rdbuf()->pubseekpos(0, std::ios_base::in);

    serializableT obj;
    ss >> obj;

    return obj;
}

template<typename snarkT>
void serialize_proving_key_to_file(
    const typename snarkT::ProvingKeyT &pk, boost::filesystem::path pk_path)
{
    write_to_file<typename snarkT::ProvingKeyT>(pk_path, pk);
}

template<typename snarkT>
typename snarkT::ProvingKeyT deserialize_proving_key_from_file(
    boost::filesystem::path pk_path)
{
    return load_from_file<snarkT::ProvingKeyT>(pk_path);
}

template<typename snarkT>
void serialize_verification_key_to_file(
    const typename snarkT::VerifKeyT &vk, boost::filesystem::path vk_path)
{
    write_to_file<typename snarkT::VerifKeyT>(vk_path, vk);
}

template<typename snarkT>
typename snarkT::VerifKeyT deserialize_verification_key_from_file(
    boost::filesystem::path vk_path)
{
    return load_from_file<snarkT::VerifKeyT>(vk_path);
}

template<typename snarkT>
void serialize_setup_to_file(
    const typename snarkT::KeypairT &keypair,
    boost::filesystem::path setup_path)
{
    if (setup_path.empty()) {
        setup_path = get_path_to_setup_directory();
    }

    boost::filesystem::path vk_json("vk.json");
    boost::filesystem::path vk_raw("vk.raw");
    boost::filesystem::path pk_raw("pk.raw");

    boost::filesystem::path path_vk_json = setup_path / vk_json;
    boost::filesystem::path path_vk_raw = setup_path / vk_raw;
    boost::filesystem::path path_pk_raw = setup_path / pk_raw;

    typename snarkT::ProvingKeyT proving_key = keypair.pk;
    typename snarkT::VerifKeyT verification_key = keypair.vk;

    // Write the verification key in json format
    snarkT::verification_key_to_json(verification_key, path_vk_json);

    // Write the verification and proving keys in raw format
    serialize_verification_key_to_file<snarkT>(verification_key, path_vk_raw);
    serialize_proving_key_to_file<snarkT>(proving_key, path_pk_raw);
}

template<typename ppT>
void fill_stringstream_with_json_constraints(
    libsnark::linear_combination<libff::Fr<ppT>> constraints,
    std::stringstream &ss)
{
    ss << "[";
    size_t count = 0;
    for (const libsnark::linear_term<libff::Fr<ppT>> &lt : constraints.terms) {
        if (count != 0) {
            ss << ",";
        }

        ss << "{";
        ss << "\"index\":" << lt.index << ",";
        ss << "\"value\":"
           << "\"0x" + bigint_to_hex<libff::Fr<ppT>>(lt.coeff.as_bigint())
           << "\"";
        ss << "}";
        count++;
    }
    ss << "]";
}

template<typename ppT>
void r1cs_to_json(
    libsnark::protoboard<libff::Fr<ppT>> pb, boost::filesystem::path r1cs_path)
{
    if (r1cs_path.empty()) {
        // Used for debugging purpose
        boost::filesystem::path tmp_path = get_path_to_debug_directory();
        boost::filesystem::path r1cs_json_file("r1cs.json");
        r1cs_path = tmp_path / r1cs_json_file;
    }
    // Convert the boost path into char*
    const char *str_path = r1cs_path.string().c_str();

    // output inputs, right now need to compile with debug flag so that the
    // `variable_annotations` exists. Having trouble setting that up so will
    // leave for now.
    libsnark::r1cs_constraint_system<libff::Fr<ppT>> constraints =
        pb.get_constraint_system();
    std::stringstream ss;
    std::ofstream fh;
    fh.open(str_path, std::ios::binary);

    ss << "{\n";
    ss << "\"scalar_field_characteristic\":"
       << "\"Not yet supported. Should be bigint in hexadecimal\""
       << ",\n";
    ss << "\"num_variables\":" << pb.num_variables() << ",\n";
    ss << "\"num_constraints\":" << pb.num_constraints() << ",\n";
    ss << "\"num_inputs\": " << pb.num_inputs() << ",\n";
    ss << "\"variables_annotations\":[";
    for (size_t i = 0; i < constraints.num_variables(); ++i) {
        ss << "{";
        ss << "\"index\":" << i << ",";
        ss << "\"annotation\":"
           << "\"" << constraints.variable_annotations[i].c_str() << "\"";
        if (i == constraints.num_variables() - 1) {
            ss << "}";
        } else {
            ss << "},";
        }
    }
    ss << "],\n";
    ss << "\"constraints\":[";
    for (size_t c = 0; c < constraints.num_constraints(); ++c) {
        ss << "{";
        ss << "\"constraint_id\": " << c << ",";
        ss << "\"constraint_annotation\": "
           << "\"" << constraints.constraint_annotations[c].c_str() << "\",";
        ss << "\"linear_combination\":";
        ss << "{";
        ss << "\"A\":";
        fill_stringstream_with_json_constraints<ppT>(
            constraints.constraints[c].a, ss);
        ss << ",";
        ss << "\"B\":";
        fill_stringstream_with_json_constraints<ppT>(
            constraints.constraints[c].b, ss);
        ss << ",";
        ss << "\"C\":";
        fill_stringstream_with_json_constraints<ppT>(
            constraints.constraints[c].c, ss);
        ss << "}";
        if (c == constraints.num_constraints() - 1) {
            ss << "}";
        } else {
            ss << "},";
        }
    }
    ss << "]\n";
    ss << "}";

    ss.rdbuf()->pubseekpos(0, std::ios_base::out);
    fh << ss.rdbuf();
    fh.flush();
    fh.close();
}

} // namespace libzeth

#endif // __ZETH_SERIALIZATION_FILE_IO_TCC__
