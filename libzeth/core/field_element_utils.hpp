// Copyright (c) 2015-2021 Clearmatics Technologies Ltd
//
// SPDX-License-Identifier: LGPL-3.0+

#ifndef __ZETH_CORE_FIELD_ELEMENT_UTILS_HPP__
#define __ZETH_CORE_FIELD_ELEMENT_UTILS_HPP__

#include "include_libff.hpp"

namespace libzeth
{

/// WARNING: The following function assumes that the platform is little-endian
/// and that NAILS are NOT used. See Section 8.2 (page 68):
/// https://gmplib.org/gmp-man-6.2.0.pdf
///
/// In other words, we assume that:
/// - GMP_NUMB_BITS = GMP_LIMB_BITS and thus,
/// - GMP_NAIL_BITS = 0
///
/// Under these assumptions, bigints are layed out in memory with low-order
/// bytes first, occupying `FieldT::num_limbs * GMP_LIMB_BITS / 8` bytes. For
/// readability, this function returns the big-endian hex representation
/// (high-order byte first). If `prefix` is true, add the "0x" prefix.
template<typename FieldT>
std::string bigint_to_hex(
    const libff::bigint<FieldT::num_limbs> &limbs, bool prefix = false);

/// WARNING: The following function assumes that the platform is little-endian
/// and that NAILS are NOT used. See Section 8.2 (page 68):
/// https://gmplib.org/gmp-man-6.2.0.pdf
///
/// In other words, we assume that:
/// - GMP_NUMB_BITS = GMP_LIMB_BITS and thus,
/// - GMP_NAIL_BITS = 0
///
/// Hex is read as most-significant-character-first (the natural ordering for
/// numbers), as generated by `bigint_to_hex`.
template<typename FieldT>
libff::bigint<FieldT::num_limbs> bigint_from_hex(const std::string &hex);

/// Convert a base/ground field element to a hexadecimal string.
template<typename FieldT>
std::string base_field_element_to_hex(const FieldT &field_el);

/// Convert an hexadecimal string to a base/ground field element
template<typename FieldT>
FieldT base_field_element_from_hex(const std::string &field_str);

template<typename FieldT>
void field_element_write_json(const FieldT &el, std::ostream &out_s);

template<typename FieldT>
void field_element_read_json(FieldT &el, std::istream &in_s);

template<typename FieldT> std::string field_element_to_json(const FieldT &el);

template<typename FieldT>
FieldT field_element_from_json(const std::string &json);

/// Write a field element as bytes. Base field elements are written in plain
/// (non-Montgomery) form as fixed-size big-endian integers. Extension field
/// elements are written as a series of components.
template<typename FieldT>
void field_element_write_bytes(const FieldT &el, std::ostream &out_s);

/// Read a field element as bytes, in the format described for
/// field_element_write_bytes.
template<typename FieldT>
void field_element_read_bytes(FieldT &el, std::istream &in_s);

} // namespace libzeth

#include "libzeth/core/field_element_utils.tcc"

#endif // __ZETH_CORE_FIELD_ELEMENT_UTILS_HPP__
