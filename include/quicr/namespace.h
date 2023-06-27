#pragma once

#include <quicr/name.h>

#include <istream>
#include <map>
#include <ostream>

namespace quicr
{

/**
 * @brief A prefix for a quicr::Name
 */
class Namespace
{
  public:
    Namespace() = default;
    constexpr Namespace(const Namespace& ns) = default;
    Namespace(Namespace&& ns) = default;

    constexpr Namespace& operator=(const Namespace& other) = default;
    Namespace& operator=(Namespace&& other) = default;

    /**
     * @brief Construct a namespace from a Name with significant bits to extract.
     * @param name The name that will form the base of the namespace
     * @param sig_bits The amount of bits (from right to left) that are significant.
     */
    constexpr Namespace(Name name, uint8_t sig_bits)
      : _name{ name.bits((sizeof(Name) * 8) - sig_bits, sig_bits) }, _sig_bits{ sig_bits }
    {
    }

    /**
     * @brief Constructs a namespace from a string.
     * @param str A string of the form '0xXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX/X'.
     */
    Namespace(const std::string& str) : Namespace(std::string_view(str)) {}
    constexpr Namespace(std::string_view str)
    {
        auto delim_pos = str.find_first_of('/');
        _name = str.substr(0, delim_pos);
        str.remove_prefix(delim_pos + 1);
        _sig_bits = str_to_uint(str);
    }

    /**
     * @brief Checks if the given name falls within the namespace.
     * @param name The name to check.
     * @returns True if the given name is contained within the namespace. False otherwise.
     */
    constexpr bool contains(const Name& name) const
    {
        return name.bits((sizeof(Name) * 8) - _sig_bits, _sig_bits) == _name;
    }

    /**
     * @brief Checks if the given namespace falls within the current namespace.
     * @param prefix The namespace to check.
     * @returns True if the sub-namespace is contained within the current namespace. False otherwise.
     */
    constexpr bool contains(const Namespace& prefix) const { return contains(prefix._name); }

    /**
     * @brief The name of the namespace.
     * @returns The masked name of the namespace, with the insignificant bits set to 0.
     */
    constexpr Name name() const { return _name; }

    /**
     * @brief The length of the namespace, corresponding directly to the number of significant bits.
     * @returns The significant bits.
     */
    constexpr uint8_t length() const { return _sig_bits; }

    [[deprecated("quicr::Namespace::to_hex is deprecated, use std::string or std::ostream operators")]]
    std::string to_hex() const
    {
        return std::string(_name) + "/" + std::to_string(_sig_bits);
    }

    /*=======================================================================*/
    // Conversion Operators
    /*=======================================================================*/

    /**
     * Outputs the string version of the Namespace, in the form '0xXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX/X'.
     */
    constexpr operator std::string() const { return std::string(_name) + "/" + std::to_string(_sig_bits); }

    constexpr operator Name() const { return name(); }

    /*=======================================================================*/
    // Comparison Operators
    /*=======================================================================*/

    friend constexpr bool operator==(const Namespace& a, const Namespace& b)
    {
        return a._name == b._name && a._sig_bits == b._sig_bits;
    }

    friend constexpr bool operator!=(const Namespace& a, const Namespace& b) { return !(a == b); }

    friend constexpr bool operator>(const Namespace& a, const Namespace& b) { return a._name > b._name; }

    friend constexpr bool operator>(const Namespace& a, const Name& b)
    {
        return a._name > quicr::Namespace{ b, a._sig_bits };
    }

    friend constexpr bool operator>(const Name& a, const Namespace& b)
    {
        return quicr::Namespace{ a, b._sig_bits } > b._name;
    }

    friend constexpr bool operator>=(const Namespace& a, const Namespace& b) { return !(a < b); }
    friend constexpr bool operator>=(const Namespace& a, const Name& b) { return !(a < b); }

    friend constexpr bool operator<(const Namespace& a, const Namespace& b) { return a._name < b._name; }

    friend constexpr bool operator<(const Namespace& a, const Name& b)
    {
        return a._name < quicr::Namespace{ b, a._sig_bits };
    }

    friend constexpr bool operator<(const Name& a, const Namespace& b)
    {
        return quicr::Namespace{ a, b._sig_bits } < b._name;
    }

    friend constexpr bool operator<=(const Namespace& a, const Namespace& b) { return !(a > b); }
    friend constexpr bool operator<=(const Name& a, const Namespace& b) { return !(a > b); }

    /*=======================================================================*/
    // Stream Operators
    /*=======================================================================*/

    friend std::ostream& operator<<(std::ostream& os, const Namespace& ns) { return os << std::string(ns); }

    friend std::istream& operator>>(std::istream& is, Namespace& name)
    {
        std::string hex;
        is >> hex;
        name = hex;
        return is;
    }

  private:
    /**
     * @brief Static helper method for converting charater to decimal value at compile-time.
     * @param dec The decimal character to convert.
     * @returns The decimal value of the digit character.
     */
    static constexpr uint8_t char_to_uint(char dec)
    {
        if ('0' <= dec && dec <= '9') return dec - '0';
        return 0;
    }

    /**
     * @brief Static helper function to convert a string to a decimal value at compile-time.
     * @param dec The decimal string to convert.
     * @returns The decimal value of the given string.
     */
    static constexpr uint8_t str_to_uint(std::string_view dec)
    {
        uint8_t value = 0;
        for (std::size_t i = 0; i < dec.length(); ++i)
        {
            value *= 10u;
            value += char_to_uint(dec[i]);
        }

        return value;
    }

  private:
    Name _name;
    uint8_t _sig_bits;
};

/**
 * @brief Namespace comparator capable of comparing Namespaces against Names.
 */
template<template<typename> class Comp>
class namespace_comparator : Comp<quicr::Namespace>
{
    using base = Comp<quicr::Namespace>;

  public:
    using is_transparent = std::true_type;

    /**
     * The default Namespace/Namespace comparator.
     */
    using base::operator();

    /**
     * @brief Compares a Namespace against a Name from the lhs.
     * @param ns The namespace to compare.
     * @param name The name to compare.
     * @returns True if the namespace is less than the name.
     */
    constexpr bool operator()(const Namespace& ns, const Name& name) const
    {
        return base::operator()(ns, { name, ns.length() });
    }

    /**
     * @brief Compares a Name against a Namespace from the rhs.
     * @param name The name to compare.
     * @param ns The namespace to compare.
     * @returns True if the name is less than the namespace.
     */
    constexpr bool operator()(const Name& name, const Namespace& ns) const
    {
        return base::operator()({ name, ns.length() }, ns);
    }
};

/**
 * @brief A map keyed on quicr::Namespace.
 * @tparam T The value type
 * @tparam Comparator The STL comparator to use inside the namespace_comparator. Defaults to std::less<T>.
 * @tparam Allocator The allocator type to use. Defaults to the same as std::map.
 */
template<class T,
         template<typename> class Comparator = std::less,
         class Allocator = std::allocator<std::pair<const quicr::Namespace, T>>>
using namespace_map = std::map<quicr::Namespace, T, namespace_comparator<Comparator>, Allocator>;

/**
 * @brief Unique to QUICR, defines Name to be an integral type to the QUICR API.
 */
template<>
struct is_integral<Namespace> : std::true_type
{
};

} // namespace quicr
