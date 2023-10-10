#pragma once

#include "_utilities.h"

#include <concepts>
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

template<typename T>
concept UnsignedOrName = std::unsigned_integral<T> || std::is_same_v<T, class Name>;

/**
 * @brief Unsigned 128 bit number which can be created from strings or byte arrays.
 *
 * @details A Name is defined to be 128 bits, and can almost be used fully as
 *          an integer with arithmetic and logical operators (excluding
 *          multiplication and division operators). It can be constructed from a
 *          hexadecimal string or a byte array.
 */
class Name
{
    using uint_t = std::uint64_t;

  private:
    /**
     * @brief Private constructor for creating a name from 2 unsigned integers.
     *
     * @param hi The high bits of the name.
     * @param lo The low bits of the name.
     */
    constexpr explicit Name(uint_t hi, uint_t lo) noexcept : _lo{ lo }, _hi{ hi } {}

  public:
    Name() noexcept = default;
    constexpr Name(const Name& other) noexcept = default;
    constexpr Name(Name&& other) noexcept = default;

    constexpr Name(std::string_view hex_value) noexcept(false)
    {
        if (hex_value.starts_with("0x")) hex_value.remove_prefix(2);

        if (hex_value.length() > sizeof(Name) * 2)
            throw std::invalid_argument("Hex string cannot be longer than " + std::to_string(sizeof(Name) * 2) +
                                        " bytes");

        if (hex_value.length() > sizeof(Name))
        {
            _hi = utility::hex_to_unsigned<uint_t>(hex_value.substr(0, hex_value.length() - sizeof(Name)));
            _lo = utility::hex_to_unsigned<uint_t>(hex_value.substr(hex_value.length() - sizeof(Name), sizeof(Name)));
        }
        else
        {
            _hi = 0;
            _lo = utility::hex_to_unsigned<uint_t>(hex_value.substr(0, hex_value.length()));
        }
    }

    /**
     * @brief Constructs a Name from a byte array pointer.
     *
     * @param data The byte array pointer to read from.
     * @param size The length of the byte array pointer. Must NOT be greater.
     *
     * Note: The ordering of the byte array MUST conform to the endianness of the machine.
     */
    Name(const std::uint8_t* data, std::size_t size) noexcept(false) : _lo{ 0 }, _hi{ 0 }
    {
        if (!data) throw std::invalid_argument("Byte array data must not be null");

        if (size > sizeof(Name))
        {
            throw std::invalid_argument("Byte array size (" + std::to_string(size) + ") cannot exceed size of Name (" +
                                        std::to_string(sizeof(Name)) + ")");
        }

        if (size > sizeof(uint_t))
        {
            std::memcpy(&_hi, data + sizeof(uint_t), size - sizeof(uint_t));
            std::memcpy(&_lo, data, sizeof(uint_t));
        }
        else
            std::memcpy(&_lo, data, size);
    }

    /**
     * @brief Constructs a Name from a byte range.
     *
     * @param data The byte range to read from.
     */
    Name(std::span<std::uint8_t> data) noexcept(false) : Name(data.data(), data.size()) {}

    /*=======================================================================*/
    // Assignment Operators
    /*=======================================================================*/

    constexpr Name& operator=(const Name& other) noexcept = default;
    constexpr Name& operator=(Name&& other) noexcept = default;
    constexpr Name& operator=(std::string_view hex) noexcept(false) { return *this = Name(hex); }

    /*=======================================================================*/
    // Conversion Operators
    /*=======================================================================*/

    /**
     * Returns the hexadecimal string representation of the Name, with 0x prefix.
     */
    operator std::string() const noexcept
    {
        return "0x" + utility::unsigned_to_hex(_hi) + utility::unsigned_to_hex(_lo);
    }

    explicit constexpr operator std::uint8_t() const noexcept { return static_cast<std::uint8_t>(_lo); }
    explicit constexpr operator std::uint16_t() const noexcept { return static_cast<std::uint16_t>(_lo); }
    explicit constexpr operator std::uint32_t() const noexcept { return static_cast<std::uint32_t>(_lo); }
    explicit constexpr operator std::uint64_t() const noexcept { return _lo; }

    /*=======================================================================*/
    // Arithmetic Operators
    /*=======================================================================*/

    constexpr Name& operator+=(uint_t value) noexcept { return *this = Name(_hi + !!(_lo + value < _lo), _lo + value); }
    constexpr Name& operator+=(Name value) noexcept
    {
        *this += value._lo;
        _hi += value._hi;

        return *this;
    }

    constexpr Name& operator-=(uint_t value) noexcept { return *this = Name(_hi - !!(_lo - value > _lo), _lo - value); }
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

    constexpr Name& operator&=(uint_t value) noexcept { return *this = *this & value; }
    constexpr Name& operator&=(const Name& other) noexcept { return *this = *this & other; }
    constexpr Name& operator|=(uint_t value) noexcept { return *this = *this | value; }
    constexpr Name& operator|=(const Name& other) noexcept { return *this = *this | other; }
    constexpr Name& operator^=(const Name& other) noexcept { return *this = *this ^ other; }

    constexpr Name& operator>>=(uint16_t value) noexcept
    {
        if (value == 0) return *this;

        if (value < sizeof(uint_t) * 8)
        {
            _lo = (_lo >> value) | (_hi << (sizeof(uint_t) * 8 - value));
            _hi = _hi >> value;
        }
        else
        {
            _lo = _hi >> (value - sizeof(uint_t) * 8);
            _hi = 0;
        }

        return *this;
    }

    constexpr Name& operator<<=(uint16_t value) noexcept
    {
        if (value == 0) return *this;

        if (value < sizeof(uint_t) * 8)
        {
            _hi = (_hi << value) | (_lo >> (sizeof(uint_t) * 8 - value));
            _lo = _lo << value;
        }
        else
        {
            _hi = _lo << (value - sizeof(uint_t) * 8);
            _lo = 0;
        }

        return *this;
    }

    constexpr Name operator>>(uint16_t value) const noexcept { return Name(*this) >>= value; }
    constexpr Name operator<<(uint16_t value) const noexcept { return Name(*this) <<= value; }

    /*=======================================================================*/
    // Comparison Operators
    /*=======================================================================*/

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

    friend constexpr bool operator==(const Name& a, const Name& b) noexcept { return a._hi == b._hi && a._lo == b._lo; }
    friend constexpr bool operator!=(const Name& a, const Name& b) noexcept { return !(a == b); }
    friend constexpr bool operator>=(const Name& a, const Name& b) noexcept { return !(a < b); }
    friend constexpr bool operator<=(const Name& a, const Name& b) noexcept { return !(a > b); }

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
    template<UnsignedOrName T = Name>
    constexpr T bits(std::uint16_t from, std::uint16_t length = 1) const
    {
        if (length == 0) return 0;

        if (length > sizeof(uint_t) * 8)
            throw std::domain_error("length is greater than 64 bits, did you mean to use Name?");

        return T((*this & (((Name(uint_t(0), uint_t(1)) << length) - 1) << from)) >> from);
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
    uint_t _lo;
    uint_t _hi;
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
