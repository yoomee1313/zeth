// DISCLAIMER:
// Content taken and adapted from:
// https://github.com/HarryR/ethsnarks/blob/master/src/gadgets/merkle_tree.hpp
// https://github.com/HarryR/ethsnarks/blob/master/src/gadgets/merkle_tree.cpp

#ifndef __ZETH_MERKLE_PATH_SELECTOR_TCC__
#define __ZETH_MERKLE_PATH_SELECTOR_TCC__

namespace libzeth {

template<typename FieldT>
merkle_path_selector<FieldT>::merkle_path_selector(
    libsnark::protoboard<FieldT> &pb,
    const libsnark::pb_variable<FieldT>& input,
    const libsnark::pb_variable<FieldT>& pathvar,
    const libsnark::pb_variable<FieldT>& is_right,
    const std::string &annotation_prefix
) :
    libsnark::gadget<FieldT>(pb, annotation_prefix),
    input(input),
    pathvar(pathvar),
    is_right(is_right)
{
    left.allocate(pb, FMT(this->annotation_prefix, ".left"));

    right.allocate(pb, FMT(this->annotation_prefix, ".right"));
}

template<typename FieldT>
void merkle_path_selector<FieldT>::generate_r1cs_constraints()
{
    generate_boolean_r1cs_constraint<FieldT>(this->pb, is_right, FMT(this->annotation_prefix, " is_right"));

    this->pb.add_r1cs_constraint(libsnark::r1cs_constraint<FieldT>(is_right, pathvar - input, left - input),
        FMT(this->annotation_prefix, "is_right*pathvar + 1-is_right * input = left"));

    this->pb.add_r1cs_constraint(libsnark::r1cs_constraint<FieldT>(is_right, input - pathvar, right - pathvar),
        FMT(this->annotation_prefix, "is_right*input + 1-is_right * pathvar = right"));
}

template<typename FieldT>
void merkle_path_selector<FieldT>::generate_r1cs_witness()
{
    this->pb.val(left) = this->pb.val(input) +  this->pb.val(is_right) * ( this->pb.val(pathvar) -this->pb.val(input)  );

    this->pb.val(right) = this->pb.val(pathvar) +  this->pb.val(is_right) * ( this->pb.val(input) -this->pb.val(pathvar)  );
}

template<typename FieldT>
const libsnark::pb_variable<FieldT>& merkle_path_selector<FieldT>::get_left() {
    return left;
}

template<typename FieldT>
const libsnark::pb_variable<FieldT>& merkle_path_selector<FieldT>::get_right() {
    return right;
}


} // libzeth

// __ZETH_MERKLE_PATH_SELECTOR_TCC__
#endif
