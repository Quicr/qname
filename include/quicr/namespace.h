#pragma once

#include <quicr/name.h>

#include <istream>
#include <map>
#include <optional>
#include <ostream>

namespace quicr
{

/**
 * @brief A prefix for a Name
 */
class Namespace
{
  public:
    Namespace() noexcept = default;
    constexpr Namespace(const Namespace& ns) noexcept = default;
    Namespace(Namespace&& ns) noexcept = default;

    constexpr Namespace& operator=(const Namespace& other) noexcept = default;
    Namespace& operator=(Namespace&& other) noexcept = default;

    /**
     * @brief Construct a namespace from a Name with significant bits to extract.
     * @param name The name that will form the base of the namespace
     * @param sig_bits The amount of bits (from right to left) that are significant.
     */
    constexpr Namespace(Name name, uint8_t sig_bits) noexcept
      : _name{ name.bits((sizeof(Name) * 8) - sig_bits, sig_bits) }, _sig_bits{ sig_bits }
    {
    }

    /**
     * @brief Constructs a namespace from a string.
     * @param str A string of the form '0xXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX/X'.
     */
    constexpr Namespace(std::string_view str)
    {
        auto delim_pos = str.find_first_of('/');
        _name = str.substr(0, delim_pos);

        str.remove_prefix(delim_pos + 1);
        _sig_bits = str_to_uint(str);

        _name = _name.bits((sizeof(Name) * 8) - _sig_bits, _sig_bits);
    }

    constexpr Namespace& operator=(std::string_view hex) { return *this = Namespace(hex); }

    /**
     * @brief Checks if the given name falls within the namespace.
     * @param name The name to check.
     * @returns True if the given name is contained within the namespace. False otherwise.
     */
    constexpr bool contains(const Name& name) const noexcept
    {
        return name.bits((sizeof(Name) * 8) - _sig_bits, _sig_bits) == _name;
    }

    /**
     * @brief Checks if the given namespace falls within the current namespace.
     * @param prefix The namespace to check.
     * @returns True if the sub-namespace is contained within the current namespace. False otherwise.
     */
    constexpr bool contains(const Namespace& prefix) const noexcept { return contains(prefix._name); }

    /**
     * @brief The name of the namespace.
     * @returns The masked name of the namespace, with the insignificant bits set to 0.
     */
    constexpr Name name() const noexcept { return _name; }

    /**
     * @brief The length of the namespace, corresponding directly to the number of significant bits.
     * @returns The significant bits.
     */
    constexpr uint8_t length() const noexcept { return _sig_bits; }

    [[deprecated("Namespace::to_hex is deprecated, use std::string or std::ostream operators")]]
    std::string to_hex() const
    {
        return std::string(_name) + "/" + std::to_string(_sig_bits);
    }

    /*=======================================================================*/
    // Conversion Operators
    /*=======================================================================*/

    /**
     * Returns a string in the form '0xXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX'.
     */
    operator std::string() const { return std::string(_name) + "/" + std::to_string(_sig_bits); }

    constexpr operator Name() const noexcept { return name(); }

    /*=======================================================================*/
    // Comparison Operators
    /*=======================================================================*/

    friend constexpr bool operator==(const Namespace& a, const Namespace& b) noexcept
    {
        return a._name == b._name && a._sig_bits == b._sig_bits;
    }

    friend constexpr bool operator!=(const Namespace& a, const Namespace& b) noexcept { return !(a == b); }

    friend constexpr bool operator>(const Namespace& a, const Namespace& b) noexcept { return a._name > b._name; }

    friend constexpr bool operator>(const Namespace& a, const Name& b) noexcept
    {
        return a._name > Namespace{ b, a._sig_bits };
    }

    friend constexpr bool operator>(const Name& a, const Namespace& b) noexcept
    {
        return Namespace{ a, b._sig_bits } > b._name;
    }

    friend constexpr bool operator<(const Namespace& a, const Namespace& b) noexcept { return a._name < b._name; }

    friend constexpr bool operator<(const Namespace& a, const Name& b) noexcept
    {
        return a._name < Namespace{ b, a._sig_bits };
    }

    friend constexpr bool operator<(const Name& a, const Namespace& b) noexcept
    {
        return Namespace{ a, b._sig_bits } < b._name;
    }

    friend constexpr bool operator>=(const Namespace& a, const Namespace& b) noexcept { return !(a < b); }

    friend constexpr bool operator>=(const Namespace& a, const Name& b) noexcept { return !(a < b); }

    friend constexpr bool operator<=(const Namespace& a, const Namespace& b) noexcept { return !(a > b); }

    friend constexpr bool operator<=(const Name& a, const Namespace& b) noexcept { return !(a > b); }

    /*=======================================================================*/
    // Stream Operators
    /*=======================================================================*/

    /**
     * Outputs a string in the form '0xXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX/X'.
     */
    friend std::ostream& operator<<(std::ostream& os, const Namespace& ns) { return os << std::string(ns); }

    /**
     * Inputs a string in the form '0xXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX/X' into Name
     */
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
class namespace_comparator_wrapper : Comp<Namespace>
{
    using base = Comp<Namespace>;

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
 * @brief A map keyed on Namespace.
 *
 * @details A map keyed by Namespace, and searchable by Name as well. It is thus
 *          possible to use a Name to retrieve an entry. When indexing using Name,
 *          if the given comparator is std::greater, the entry returned is that
 *          which has a key (Namespace) whose value matches Name the longest (i.e.
 *          most specific namespace). When the comparator is std::less, the entry
 *          returned is that which has a key (Namespace) whose value matches Name
 *          the shortest (i.e. the shortest match).
 *
 * @tparam T The value type
 * @tparam Comparator The STL comparator to use inside the namespace_comparator_wrapper. Defaults to std::greater<T>.
 * @tparam Allocator The allocator type to use. Default is same as std::map.
 */
template<class T,
         template<typename> class Comparator = std::greater,
         class Allocator = std::allocator<std::pair<const Namespace, T>>>
class namespace_map : public std::map<Namespace, T, namespace_comparator_wrapper<Comparator>, Allocator>
{
    using base_t = std::map<Namespace, T, namespace_comparator_wrapper<Comparator>, Allocator>;

  public:
    using base_t::base_t;
};
} // namespace quicr
