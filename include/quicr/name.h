#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <istream>
#include <ostream>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

namespace quicr
{

/**
 * @brief A new definition for checking is_integral in the QUICR API.
 * @tparam T The type to check.
 */
template<typename T>
struct is_integral : std::is_integral<T> {};

/**
 * @brief Unique to QUICR, defines Name to be an integral type to the QUICR API.
 */
template<>
struct is_integral<class Name> : std::true_type {};

template<typename T>
constexpr bool is_integral_v = is_integral<T>::value;

template<typename T>
concept Unsigned = std::is_unsigned_v<T>;

template<typename T>
concept Integral = quicr::is_integral_v<T>;

namespace
{
/**
 * @brief Converts a hexadecimal character to it's decimal value.
 *
 * @tparam UInt_t The unsigned integer type to convert to.
 * @param hex The hexadecimal character to convert.
 * @returns The decimal value of the provided character.
 */
template<Unsigned UInt_t = std::uint64_t>
constexpr UInt_t hexchar_to_uint(char hex) noexcept
{
    if ('0' <= hex && hex <= '9')
        return hex - '0';
    else if ('A' <= hex && hex <= 'F')
        return hex - 'A' + 10;
    else if ('a' <= hex && hex <= 'f')
        return hex - 'a' + 10;

    return 0;
}

/**
 * @brief Converts an unsigned integer decimal value into a hexidecimal
 * character.
 *
 * @tparam UInt_t The unsigned integer type to convert from.
 * @param value The decimal value to convert.
 * @returns The hexadecimal character of the provided decimal value.
 */
template<Unsigned UInt_t>
constexpr char uint_to_hexchar(UInt_t value) noexcept
{
    if (value > 9) return value + 'A' - 10;
    return value + '0';
}

/**
 * @brief Converts a hexidecimal string to an unsigned integer decimal value.
 *
 * @tparam UInt_t The unsigned integer type to convert to.
 * @param hex The hexadecimal string to convert from.
 * @returns The decimal value of the provided hexadecimal string.
 */
template<Unsigned UInt_t>
constexpr UInt_t hex_to_uint(std::string_view hex) noexcept
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
template<Unsigned UInt_t>
std::string uint_to_hex(UInt_t value) noexcept
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
    using uint_t = std::uint64_t;

  private:
    constexpr explicit Name(uint_t hi, uint_t lo) noexcept : _hi{ hi }, _lo{ lo } {}

  public:
    Name() noexcept = default;
    constexpr Name(const Name& other) noexcept = default;
    Name(Name&& other) noexcept = default;

    constexpr Name& operator=(const Name& other) noexcept = default;
    Name& operator=(Name&& other) noexcept = default;

    constexpr Name(std::string_view hex_value) noexcept(false)
    {
        if (hex_value.starts_with("0x")) hex_value.remove_prefix(2);

        if (hex_value.length() > sizeof(Name) * 2)
            throw std::invalid_argument("Hex string cannot be longer than " + std::to_string(sizeof(Name) * 2) +
                                        " bytes");

        if (hex_value.length() > sizeof(Name))
        {
            _hi = hex_to_uint<uint_t>(hex_value.substr(0, hex_value.length() - sizeof(Name)));
            _lo = hex_to_uint<uint_t>(hex_value.substr(hex_value.length() - sizeof(Name), sizeof(Name)));
        }
        else
        {
            _hi = 0;
            _lo = hex_to_uint<uint_t>(hex_value.substr(0, hex_value.length()));
        }
    }

    constexpr Name& operator=(std::string_view hex) { return *this = Name(hex); }

    /**
     * @brief Constructs a Name from a byte array pointer.
     *
     * @param data The byte array pointer to read from.
     * @param length The length of the byte array pointer. Must NOT be greater.
     * @param align_left Aligns the bytes to the left (higher bytes). than the sizeof Name.
     */
    Name(const std::uint8_t* data, std::size_t length, bool align_left = false) noexcept(false)
    {
        if (!data) throw std::invalid_argument("Byte array data must not be null");
        if (length > sizeof(Name))
            throw std::invalid_argument("Byte array length (" + std::to_string(length) +
                                        ") cannot be longer than length of Name (" + std::to_string(sizeof(Name)) +
                                        ")");

        constexpr std::size_t size_of = sizeof(uint_t);

        if (length > size_of)
        {
            std::memcpy(&_hi, data, length - size_of);
            std::memcpy(&_lo, data + (length - size_of), size_of);
        }
        else if (align_left)
        {
            std::memcpy(&_hi, data, size_of);
            _lo = 0ull;
        }
        else
        {
            _hi = 0ull;
            std::memcpy(&_lo, data, size_of);
        }

        if (align_left) *this <<= (size_of - length) * 8;
    }

    Name(std::span<std::uint8_t> data, bool align_left = false) noexcept(false)
      : Name(data.data(), data.size(), align_left)
    {
    }

    /*=======================================================================*/
    // Conversion Operators
    /*=======================================================================*/

    /**
     * Returns the hexadecimal string representation of the Name, with 0x prefix.
     */
    operator std::string() const noexcept
    {
        std::string hex = "0x";
        hex += uint_to_hex(_hi);
        hex += uint_to_hex(_lo);

        return hex;
    }

    explicit constexpr operator std::uint8_t() const noexcept { return _lo; }
    explicit constexpr operator std::uint16_t() const noexcept { return _lo; }
    explicit constexpr operator std::uint32_t() const noexcept { return _lo; }
    explicit constexpr operator std::uint64_t() const noexcept { return _lo; }

    /*=======================================================================*/
    // Arithmetic Operators
    /*=======================================================================*/

    constexpr Name& operator+=(uint_t value) noexcept
    {
        if (_lo + value < _lo) ++_hi;
        _lo += value;

        return *this;
    }

    constexpr Name& operator+=(Name value) noexcept
    {
        *this += value._lo;
        _hi += value._hi;

        return *this;
    }

    constexpr Name& operator-=(uint_t value) noexcept
    {
        if (_lo - value > _lo) --_hi;
        _lo -= value;

        return *this;
    }

    constexpr Name& operator-=(Name value) noexcept
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

    constexpr Name operator+(uint_t value) const noexcept { return Name(*this) += value; }
    constexpr Name operator+(Name value) const noexcept { return Name(*this) += value; }
    constexpr Name operator-(uint_t value) const noexcept { return Name(*this) -= value; }
    constexpr Name operator-(Name value) const noexcept { return Name(*this) -= value; }

    constexpr Name& operator++() noexcept { return *this += 1; }
    constexpr Name operator++(int) noexcept
    {
        Name name(*this);
        ++(*this);
        return name;
    }

    constexpr Name& operator--() noexcept { return *this -= 1; }
    constexpr Name operator--(int) noexcept
    {
        Name name(*this);
        --(*this);
        return name;
    }

    /*=======================================================================*/
    // Logical Arithmetic Operators
    /*=======================================================================*/

    constexpr Name operator&(uint_t value) const noexcept { return Name(_hi, _lo & value); }
    constexpr Name operator&(const Name& other) const noexcept { return Name(_hi & other._hi, _lo & other._lo); }
    constexpr Name operator|(uint_t value) const noexcept { return Name(_hi, _lo | value); }
    constexpr Name operator|(const Name& other) const noexcept { return Name(_hi | other._hi, _lo | other._lo); }
    constexpr Name operator^(const Name& other) const noexcept { return Name(_hi ^ other._hi, _lo ^ other._lo); }
    constexpr Name operator~() const noexcept { return Name(~_hi, ~_lo); }

    constexpr Name& operator&=(uint_t value) noexcept { return *this = *this | value; }
    constexpr Name& operator&=(const Name& other) noexcept { return *this = *this | other; }
    constexpr Name& operator|=(uint_t value) noexcept { return *this = *this | value; }
    constexpr Name& operator|=(const Name& other) noexcept { return *this = *this | other; }
    constexpr Name& operator^=(const Name& other) noexcept { return *this = *this ^ other; }

    constexpr Name& operator>>=(uint16_t value) noexcept
    {
        if (value == 0) return *this;

        if (value < sizeof(uint_t) * 8)
        {
            _lo = _lo >> value;
            _lo |= _hi << (sizeof(uint_t) * 8 - value);
            _hi = _hi >> value;
        }
        else
        {
            _lo = _hi >> (value - sizeof(uint_t) * 8);
            _hi = 0;
        }

        return *this;
    }

    constexpr Name operator>>(uint16_t value) const noexcept { return Name(*this) >>= value; }

    constexpr Name& operator<<=(uint16_t value) noexcept
    {
        if (value == 0) return *this;

        if (value < sizeof(uint_t) * 8)
        {
            _hi = _hi << value;
            _hi |= _lo >> (sizeof(uint_t) * 8 - value);
            _lo = _lo << value;
        }
        else
        {
            _hi = _lo << (value - sizeof(uint_t) * 8);
            _lo = 0;
        }

        return *this;
    }

    constexpr Name operator<<(uint16_t value) const noexcept { return Name(*this) <<= value; }

    /*=======================================================================*/
    // Comparison Operators
    /*=======================================================================*/

    friend constexpr bool operator==(const Name& a, const Name& b) noexcept { return a._hi == b._hi && a._lo == b._lo; }

    friend constexpr bool operator!=(const Name& a, const Name& b) noexcept { return !(a == b); }

    friend constexpr bool operator>(const Name& a, const Name& b) noexcept
    {
        if (a._hi > b._hi) return true;
        if (b._hi > a._hi) return false;
        return a._lo > b._lo;
    }

    friend constexpr bool operator<(const Name& a, const Name& b) noexcept
    {
        if (a._hi < b._hi) return true;
        if (b._hi < a._hi) return false;
        return a._lo < b._lo;
    }

    friend constexpr bool operator>=(const Name& a, const Name& b) noexcept { return !(a < b); }

    friend constexpr bool operator<=(const Name& a, const Name& b) noexcept { return !(a > b); }

    friend constexpr bool operator==(const Name& a, std::string_view b) { return a == Name(b); }

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

        if (index < sizeof(uint_t)) return (_lo >> (index * 8)) & 0xFF;
        return (_hi >> ((index - sizeof(uint_t)) * 8)) & 0xFF;
    }

    /**
     * @brief Returns a Name with only the requested bits, shifted to the right.
     *
     * @param from The starting bit to access from. 0 is the least significant bit.
     * @param length The number of bits to access. If length == 0, returns 1 bit.
     * @returns The requested bits.
     */
    template<Integral T = Name>
    constexpr T bits(std::uint16_t from, std::uint16_t length = 1) const
    {
        if (length == 0) return 0;

        if (length > sizeof(uint_t) * 8)
            throw std::domain_error("length is greater than 64 bits, did you mean to use Name?");

        return T((*this & (((Name(uint_t(0), uint_t(1)) << length) - 1) << from)) >> from);
    }

    [[deprecated("quicr::Name::to_hex is deprecated, use std::string or std::ostream operators")]]
    std::string to_hex() const noexcept
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
    uint_t _hi;
    uint_t _lo;
};

/**
 * @brief Full specialization returning Name, but creates a mask.
 *
 * @details Unlike the uint versions, this one does not shift into the lower
 *          bits, instead only highlighting the bits the were requested, and
 *          zeroing the rest out, effectively creating a mask.
 *
 * @param from The starting bit to access from. 0 is the least significant bit.
 * @param length The number of bits to access. If length == 0, returns 1 bit.
 * @returns A Name with the requested bits.
 */
template<>
constexpr Name Name::bits<Name>(std::uint16_t from, std::uint16_t length) const
{
    if (length == 0) return Name(uint_t(0), uint_t(0));
    if (length == sizeof(Name) * 8) return *this;

    return *this & (((Name(uint_t(0), uint_t(1)) << length) - 1) << from);
}
} // namespace quicr

/**
 * @brief Constructs a name from a literal hexadecimal string
 *
 * @param hex The hexadecimal string to construct from.
 * @returns The newly constructed Name.
 */
constexpr quicr::Name operator""_name(const char* hex) { return { std::string_view(hex) }; }
