#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#if __cplusplus >= 202002L
#include <span>
#include <string_view>
#else
#include <vector>
#endif

namespace quicr
{

namespace
{
/**
 * @brief Converts a hexadecimal character to it's decimal value.
 *
 * @tparam UInt_t The unsigned integer type to convert to.
 * @param hex The hexadecimal character to convert.
 * @returns The decimal value of the provided character.
 */
template<typename UInt_t = std::uint64_t, typename = typename std::enable_if<std::is_unsigned_v<UInt_t>, UInt_t>::type>
constexpr UInt_t hexchar_to_uint(char hex)
{
    UInt_t value = 0;
    if ('0' <= hex && hex <= '9')
        value += hex - '0';
    else if ('A' <= hex && hex <= 'F')
        value += hex - 'A' + 10;
    else if ('a' <= hex && hex <= 'f')
        value += hex - 'a' + 10;

    return value;
}

/**
 * @brief Converts an unsigned integer decimal value into a hexidecimal
 * character.
 *
 * @tparam UInt_t The unsigned integer type to convert from.
 * @param value The decimal value to convert.
 * @returns The hexadecimal character of the provided decimal value.
 */
template<typename UInt_t, typename = typename std::enable_if<std::is_unsigned_v<UInt_t>, UInt_t>::type>
constexpr char uint_to_hexchar(UInt_t value)
{
    char hex = ' ';
    if (value > 9)
        hex = value + 'A' - 10;
    else
        hex = value + '0';

    return hex;
}

/**
 * @brief Converts a hexidecimal string to an unsigned integer decimal value.
 *
 * @tparam UInt_t The unsigned integer type to convert to.
 * @param hex The hexadecimal string to convert from.
 * @returns The decimal value of the provided hexadecimal string.
 */
template<typename UInt_t, typename = typename std::enable_if<std::is_unsigned_v<UInt_t>, UInt_t>::type>
constexpr UInt_t hex_to_uint(std::string_view hex)
{
    if (hex.starts_with("0x")) hex.remove_prefix(2);

    UInt_t value = 0;
    for (std::size_t i = 0; i < hex.length(); ++i)
    {
        value *= 16ull;
        value += hexchar_to_uint<UInt_t>(hex[i]);
    }

    return value;
}

/**
 * @brief Converts an unsigned integer to a hexadecimal string.
 *
 * @tparam UInt_t The unsigned integer type to convert from.
 * @param value The decimal value to convert from.
 * @returns The hexadecimal string of the provided decimal value.
 */
template<typename UInt_t, typename = typename std::enable_if<std::is_unsigned_v<UInt_t>, UInt_t>::type>
std::string uint_to_hex(UInt_t value)
{
    char hex[sizeof(UInt_t) * 2 + 1] = "";
    for (int i = sizeof(UInt_t) * 2 - 1; i >= 0; --i)
    {
        UInt_t b = value & 0x0F;
        hex[i] = uint_to_hexchar(b);
        value -= b;
        value /= 16;
    }
    hex[sizeof(UInt_t) * 2] = '\0';

    return hex;
}
} // namespace

/**
 * @brief Unsigned 128 bit number which can be created from strings or byte arrays.
 *
 * @details A Name is defined to be 128 bits, and can almost be used fully as
 *          an integer with arithmetic and logical operators (excluding
 *          multiplication and division operators). It can be constructed from a
 *          hexadecimal string (constexpr), or from a byte array.
 */
class Name
{
    using uint_type = std::uint64_t;
    static constexpr std::size_t sizeof_type_bits = sizeof(uint_type) * 2 * 4;

  private:
    constexpr explicit Name(uint_type hi, uint_type lo) : _hi{ hi }, _lo{ lo } {}

  public:
    Name() = default;
    constexpr Name(const Name& other) = default;
    Name(Name&& other) = default;

    constexpr Name& operator=(const Name& other) = default;
    Name& operator=(Name&& other) = default;

#if __cplusplus >= 202002L
    Name(const std::string& hex_value) : Name(std::string_view(hex_value)) {}
    constexpr Name(std::string_view hex_value)
    {
        if (hex_value.starts_with("0x")) hex_value.remove_prefix(2);
#else
    Name(const std::string& hex_value)
    {
        if (hex_value.substr(0, 2) == "0x") hex_value.erase(0, 2);
#endif

        if (hex_value.length() > sizeof(Name) * 2)
            throw std::invalid_argument("Hex string cannot be longer than " + std::to_string(sizeof(Name) * 2) +
                                        " bytes");

        if (hex_value.length() > sizeof(Name)) [[likely]]
        {
            _hi = hex_to_uint<uint_type>(hex_value.substr(0, hex_value.length() - sizeof(Name)));
            _lo = hex_to_uint<uint_type>(hex_value.substr(hex_value.length() - sizeof(Name), sizeof(Name)));
        }
        else
        {
            _hi = 0;
            _lo = hex_to_uint<uint_type>(hex_value.substr(0, hex_value.length()));
        }
    }

    /**
     * @brief Constructs a Name from a byte array pointer.
     * @param data The byte array pointer to read from.
     * @param length The length of the byte array pointer. Must NOT be greater
     * than the sizeof Name.
     */
    Name(const std::uint8_t* data, std::size_t length)
    {
        if (!data) throw std::invalid_argument("Byte array data must not be null");
        if (length > sizeof(Name))
            throw std::invalid_argument("Byte array length (" + std::to_string(length) +
                                        ") cannot be longer than length of Name (" + std::to_string(sizeof(Name) * 2) +
                                        ")");

        constexpr std::size_t size_of = sizeof(uint_type);
        std::memcpy(&_lo, data, size_of);
        std::memcpy(&_hi, data + size_of, size_of);
    }

#if __cplusplus >= 202002L
    Name(const std::span<std::uint8_t>& data)
#else
    Name(const std::vector<std::uint8_t>& data)
#endif
    {
        if (data.size() > sizeof(Name))
            throw std::invalid_argument("Byte array length (" + std::to_string(data.size()) +
                                        ") cannot be longer than length of Name (" + std::to_string(sizeof(Name) * 2) +
                                        ")");

        constexpr std::size_t size_of = sizeof(uint_type);
        std::memcpy(&_lo, data.data(), size_of);
        std::memcpy(&_hi, data.data() + size_of, size_of);
    }

    /*=======================================================================*/
    // Conversion Operators
    /*=======================================================================*/

    /**
     * Returns the hexadecimal string representation of the Name, including the
     * 0x prefix.
     */
    operator std::string() const
    {
        std::string hex = "0x";
        hex += uint_to_hex(_hi);
        hex += uint_to_hex(_lo);

        return hex;
    }

    explicit constexpr operator std::uint8_t() const { return _lo; }
    explicit constexpr operator std::uint16_t() const { return _lo; }
    explicit constexpr operator std::uint32_t() const { return _lo; }
    explicit constexpr operator std::uint64_t() const { return _lo; }

    /*=======================================================================*/
    // Arithmetic Operators
    /*=======================================================================*/

    constexpr Name operator+(uint_type value) const { return Name(*this) += value; }

    constexpr Name operator+(Name value) const { return Name(*this) += value; }

    constexpr Name& operator+=(uint_type value)
    {
        if (_lo + value < _lo) ++_hi;
        _lo += value;

        return *this;
    }

    constexpr Name& operator+=(Name value)
    {
        *this += value._lo;
        _hi += value._hi;

        return *this;
    }

    constexpr Name operator-(uint_type value) const { return Name(*this) -= value; }

    constexpr Name operator-(Name value) const { return Name(*this) -= value; }

    constexpr Name& operator-=(uint_type value)
    {
        if (_lo - value > _lo) --_hi;
        _lo -= value;

        return *this;
    }

    constexpr Name& operator-=(Name value)
    {
        if (_hi - value._hi > _hi)
        {
            _hi = 0;
            --_lo;
        }
        else { _hi -= value._hi; }

        *this -= value._lo;

        return *this;
    }

    constexpr Name operator++() { return *this += 1; }

    constexpr Name operator++(int)
    {
        Name name(*this);
        ++(*this);
        return name;
    }

    constexpr Name operator--() { return *this -= 1; }

    constexpr Name operator--(int)
    {
        Name name(*this);
        --(*this);
        return name;
    }

    /*=======================================================================*/
    // Logical Arithmetic Operators
    /*=======================================================================*/

    constexpr Name operator~() const { return Name(~_hi, ~_lo); }

    constexpr Name& operator&=(uint_type value)
    {
        _lo &= value;
        return *this;
    }

    constexpr Name& operator&=(const Name& other)
    {
        _hi &= other._hi;
        _lo &= other._lo;
        return *this;
    }

    constexpr Name operator&(uint_type value) const { return Name(*this) &= value; }
    constexpr Name operator&(const Name& other) const { return Name(*this) &= other; }

    constexpr Name& operator|=(uint_type value)
    {
        _lo |= value;
        return *this;
    }

    constexpr Name& operator|=(const Name& other)
    {
        _hi |= other._hi;
        _lo |= other._lo;
        return *this;
    }

    constexpr Name operator|(uint_type value) const { return Name(*this) |= value; }
    constexpr Name operator|(const Name& other) const { return Name(*this) |= other; }

    constexpr Name& operator^=(const Name& other)
    {
        _hi ^= other._hi;
        _lo ^= other._lo;
        return *this;
    }

    constexpr Name operator^(const Name& other) const { return Name(*this) ^= other; }

    constexpr Name operator>>=(uint16_t value)
    {
        if (value == 0) return *this;

        if (value < sizeof_type_bits)
        {
            _lo = _lo >> value;
            _lo |= _hi << (sizeof_type_bits - value);
            _hi = _hi >> value;
        }
        else
        {
            _lo = _hi >> (value - sizeof_type_bits);
            _hi = 0;
        }

        return *this;
    }

    constexpr Name operator>>(uint16_t value) const { return Name(*this) >>= value; }

    constexpr Name operator<<=(uint16_t value)
    {
        if (value == 0) return *this;

        if (value < sizeof_type_bits)
        {
            _hi = _hi << value;
            _hi |= _lo >> (sizeof_type_bits - value);
            _lo = _lo << value;
        }
        else
        {
            _hi = _lo << (value - sizeof_type_bits);
            _lo = 0;
        }

        return *this;
    }

    constexpr Name operator<<(uint16_t value) const { return Name(*this) <<= value; }

    /*=======================================================================*/
    // Comparison Operators
    /*=======================================================================*/

    friend constexpr bool operator==(const Name& a, const Name& b) { return a._hi == b._hi && a._lo == b._lo; }

    friend constexpr bool operator!=(const Name& a, const Name& b) { return !(a == b); }

    friend constexpr bool operator>(const Name& a, const Name& b)
    {
        if (a._hi > b._hi) return true;
        if (b._hi > a._hi) return false;
        return a._lo > b._lo;
    }

    friend constexpr bool operator<(const Name& a, const Name& b)
    {
        if (a._hi < b._hi) return true;
        if (b._hi < a._hi) return false;
        return a._lo < b._lo;
    }

    friend constexpr bool operator>=(const Name& a, const Name& b) { return !(a < b); }

    friend constexpr bool operator<=(const Name& a, const Name& b) { return !(a > b); }

    /*=======================================================================*/
    // Access Operators
    /*=======================================================================*/

    /**
     * @brief Access the byte at the given index of the Name.
     *
     * @param index The index in the range [0, 16) to access,
     * @returns The byte at the given index.
     */
    constexpr std::uint8_t operator[](std::size_t index) const
    {
        if (index >= sizeof(Name)) throw std::out_of_range("Cannot access index outside of max size of quicr::Name");

        if (index < sizeof(uint_type)) return (_lo >> (index * 8)) & 0xFF;
        return (_hi >> ((index - sizeof(uint_type)) * 8)) & 0xFF;
    }

    /**
     * @brief Returns a Name with only the requested bits, shifted to the right.
     *
     * @param from The starting bit to access from. 0 is the least significant bit.
     * @param length The number of bits to access. If length == 0, returns 1 bit.
     * @returns A Name with the requested bits.
     */
    template<typename T = Name, typename = typename std::enable_if_t<std::is_unsigned_v<T> || std::is_class_v<Name>>>
    constexpr T bits(std::uint16_t from, std::uint16_t length = 1) const
    {
        if (length > sizeof(uint_type) * 8)
            throw std::domain_error("length is greater than 64 bits, did you mean to use Name?");

        return T((*this & (((Name(uint_type(0), uint_type(1)) << length) - 1) << from)) >> from);
    }

    [[deprecated("quicr::Name::to_hex is deprecated, use std::string or std::ostream operators")]]
    std::string to_hex() const
    {
        std::string hex = "0x";
        hex += uint_to_hex(_hi);
        hex += uint_to_hex(_lo);

        return hex;
    }

    /*=======================================================================*/
    // Stream Operators
    /*=======================================================================*/

    friend std::ostream& operator<<(std::ostream& os, const Name& name) { return os << std::string(name); }

    friend std::istream& operator>>(std::istream& is, Name& name)
    {
        std::string hex;
        is >> hex;
        name = hex;
        return is;
    }

  private:
    uint_type _hi;
    uint_type _lo;
};

template<>
constexpr Name Name::bits<Name>(std::uint16_t from, std::uint16_t length) const
{
    return *this & (((Name(uint_type(0), uint_type(1)) << length) - 1) << from);
}

template<typename T>
struct is_integral : std::is_integral<T>
{
};

template<>
struct is_integral<Name> : std::true_type
{
};

template<typename T>
constexpr bool is_integral_v = is_integral<T>::value;

} // namespace quicr

/**
 * @brief Constructs a name from a literal hexadecimal string
 *
 * @param hex The hexadecimal string to construct from.
 * @returns The newly constructed Name.
 */
constexpr quicr::Name operator""_name(const char* hex) { return { std::string_view(hex) }; }
