#pragma once

#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace quicr {

constexpr uint64_t
hexchar_to_uint(char x)
{
    uint64_t y = 0;
    if ('0' <= x && x <= '9')
        y += x - '0';
    else if ('A' <= x && x <= 'F')
        y += x - 'A' + 10;
    else if ('a' <= x && x <= 'f')
        y += x - 'a' + 10;

    return y;
}

template<typename T,
         typename = typename std::enable_if<std::is_unsigned_v<T>, T>::type>
constexpr char
uint_to_hexchar(T b)
{
    char x = ' ';
    if (b > 9)
        x = b + 'A' - 10;
    else
        x = b + '0';

    return x;
}

template<typename T,
         typename = typename std::enable_if<std::is_unsigned_v<T>, T>::type>
constexpr T
hex_to_uint(std::string_view x)
{
    if (x.starts_with("0x"))
        x.remove_prefix(2);

    T y = 0;
    for (size_t i = 0; i < x.length(); ++i) {
        y *= 16ull;
        y += hexchar_to_uint(x[i]);
    }

    return y;
}

template<typename T,
         typename = typename std::enable_if<std::is_unsigned_v<T>, T>::type>
std::string
uint_to_hex(T y)
{
    char x[sizeof(T) * 2 + 1] = "";
    for (int i = sizeof(T) * 2 - 1; i >= 0; --i) {
        T b = y & 0x0F;
        x[i] = uint_to_hexchar(b);
        y -= b;
        y /= 16;
    }
    x[sizeof(T) * 2] = '\0';

    return x;
}

/**
 * @brief Name class used for passing data in bits.
 */
class Name
{
    static constexpr size_t sizeof_type_bits = sizeof(uint64_t) * 2 * 4;

  public:
    Name() = default;
    constexpr Name(const Name& other) = default;
    Name(Name&& other) = default;

    constexpr Name(uint8_t* data, size_t length)
    {
        if (length > sizeof(Name) * 2)
            throw std::invalid_argument(
              "Byte array length cannot be longer than length of Name: " +
              std::to_string(sizeof(Name) * 2));

        constexpr size_t size_of = sizeof(Name) / 2;
        std::memcpy(&_low, data, size_of);
        std::memcpy(&_hi, data + size_of, size_of);
    }

    constexpr Name(const uint8_t* data, size_t length)
    {
        if (length > sizeof(Name) * 2)
            throw std::invalid_argument(
              "Byte array length cannot be longer than length of Name: " +
              std::to_string(sizeof(Name) * 2));

        constexpr size_t size_of = sizeof(Name) / 2;
        std::memcpy(&_low, data, size_of);
        std::memcpy(&_hi, data + size_of, size_of);
    }

    constexpr Name(const std::vector<uint8_t>& data)
    {
        if (data.size() > sizeof(Name))
            throw std::invalid_argument(
              "Byte array length cannot be longer than length of Name: " +
              std::to_string(sizeof(Name)));

        constexpr size_t size_of = sizeof(Name) / 2;
        std::memcpy(&_low, data.data(), size_of);
        std::memcpy(&_hi, data.data() + size_of, size_of);
    }

    constexpr Name(std::string_view hex_value)
    {
        if (hex_value.starts_with("0x"))
            hex_value.remove_prefix(2);

        if (hex_value.length() > sizeof(Name) * 2)
            throw std::invalid_argument("Hex string cannot be longer than " +
                                        std::to_string(sizeof(Name) * 2) +
                                        " bytes");

        if (hex_value.length() > sizeof(Name)) {
            _hi = hex_to_uint<uint64_t>(
              hex_value.substr(0, hex_value.length() - sizeof(Name)));
            _low = hex_to_uint<uint64_t>(hex_value.substr(
              hex_value.length() - sizeof(Name), sizeof(Name)));
        } else {
            _hi = 0;
            _low =
              hex_to_uint<uint64_t>(hex_value.substr(0, hex_value.length()));
        }
    }

    constexpr Name& operator=(const Name& other) = default;
    constexpr Name& operator=(Name&& other) = default;

    std::string to_hex() const
    {
        std::string hex = "0x";
        hex += uint_to_hex(_hi);
        hex += uint_to_hex(_low);

        return hex;
    }

    std::uint8_t operator[](std::size_t index) const
    {
        if (index >= sizeof(Name))
            throw std::out_of_range(
              "Cannot access index outside of max size of quicr::Name");

        if (index < sizeof(uint64_t))
            return (_low >> (index * 8)) & 0xff;
        return (_hi >> ((index - sizeof(uint64_t)) * 8)) & 0xff;
    }

    constexpr Name operator>>(uint16_t value) const
    {
        Name name(*this);
        name >>= value;
        return name;
    }

    constexpr Name operator>>=(uint16_t value)
    {
        if (value == 0)
            return *this;

        if (value < sizeof_type_bits) {
            _low = _low >> value;
            _low |= _hi << (sizeof_type_bits - value);
            _hi = _hi >> value;
        } else {
            _low = _hi >> (value - sizeof_type_bits);
            _hi = 0;
        }

        return *this;
    }

    constexpr Name operator<<(uint16_t value) const
    {
        Name name(*this);
        name <<= value;
        return name;
    }

    constexpr Name operator<<=(uint16_t value)
    {
        if (value == 0)
            return *this;

        if (value < sizeof_type_bits) {
            _hi = _hi << value;
            _hi |= _low >> (sizeof_type_bits - value);
            _low = _low << value;
        } else {
            _hi = _low << (value - sizeof_type_bits);
            _low = 0;
        }

        return *this;
    }

    constexpr Name operator+(uint64_t value) const
    {
        Name name(*this);
        name += value;
        return name;
    }

    constexpr void operator+=(uint64_t value)
    {
        if (_low + value < _low) {
            ++_hi;
        }
        _low += value;
    }

    constexpr Name operator+(Name value) const
    {
        Name name(*this);
        name += value;
        return name;
    }

    constexpr void operator+=(Name value)
    {
        if (_low + value._low < _low) {
            ++_hi;
        }
        _low += value._low;
        _hi += value._hi;
    }

    constexpr Name operator-(uint64_t value) const
    {
        Name name(*this);
        name -= value;
        return name;
    }

    constexpr void operator-=(uint64_t value)
    {
        if (_low - value > _low) {
            --_hi;
        }
        _low -= value;
    }

    constexpr Name operator-(Name value) const
    {
        Name name(*this);
        name -= value;
        return name;
    }

    constexpr void operator-=(Name value)
    {
        if (_hi - value._hi > _hi) {
            _hi = 0;
            --_low;
        } else {
            _hi -= value._hi;
        }
        *this -= value._low;
    }

    constexpr Name operator++()
    {
        *this += 1;
        return *this;
    }

    constexpr Name operator++(int)
    {
        Name name(*this);
        ++(*this);
        return name;
    }

    constexpr Name operator--()
    {
        *this -= 1;
        return *this;
    }

    constexpr Name operator--(int)
    {
        Name name(*this);
        --(*this);
        return name;
    }

    constexpr Name operator&(uint64_t value) const
    {
        Name name(*this);
        name &= value;
        return name;
    }

    constexpr void operator&=(uint64_t value) { _low &= value; }

    constexpr Name operator|(uint64_t value) const
    {
        Name name(*this);
        name |= value;
        return name;
    }

    constexpr void operator|=(uint64_t value) { _low |= value; }

    constexpr Name operator&(const Name& other) const
    {
        Name name = *this;
        name &= other;
        return name;
    }

    constexpr void operator&=(const Name& other)
    {
        _hi &= other._hi;
        _low &= other._low;
    }

    constexpr Name operator|(const Name& other) const
    {
        Name name = *this;
        name |= other;
        return name;
    }

    constexpr void operator|=(const Name& other)
    {
        _hi |= other._hi;
        _low |= other._low;
    }

    constexpr Name operator^(const Name& other) const
    {
        Name name = *this;
        name ^= other;
        return name;
    }

    constexpr void operator^=(const Name& other)
    {
        _hi ^= other._hi;
        _low ^= other._low;
    }

    friend constexpr bool operator==(const Name& a, const Name& b)
    {
        return a._hi == b._hi && a._low == b._low;
    }

    friend constexpr bool operator!=(const Name& a, const Name& b)
    {
        return !(a == b);
    }

    friend constexpr bool operator>(const Name& a, const Name& b)
    {
        if (a._hi > b._hi)
            return true;
        if (b._hi > a._hi)
            return false;
        return a._low > b._low;
    }

    friend constexpr bool operator<(const Name& a, const Name& b)
    {
        if (a._hi < b._hi)
            return true;
        if (b._hi < a._hi)
            return false;
        return a._low < b._low;
    }

    constexpr Name operator~() const
    {
        Name name(*this);
        name._hi = ~_hi;
        name._low = ~_low;
        return name;
    }

    friend std::ostream& operator<<(std::ostream& os, const Name& name)
    {
        os << name.to_hex();
        return os;
    }

  private:
    uint64_t _hi;
    uint64_t _low;
};
} // namespace quicr

constexpr quicr::Name
operator""_name(const char* x)
{
    return { std::string_view(x) };
}
