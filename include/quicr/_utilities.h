#pragma once

#include <concepts>
#include <string>
#include <string_view>
#include <type_traits>

namespace quicr::utility
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
template<std::unsigned_integral UInt_t = std::uint64_t>
constexpr UInt_t hexchar_to_unsigned(char hex) noexcept
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
 * @brief Converts an unsigned integer decimal value into a hexidecimal character.
 *
 * @tparam UInt_t The unsigned integer type to convert from.
 * @param value The decimal value to convert.
 * @returns The hexadecimal character of the provided decimal value.
 */
template<std::unsigned_integral UInt_t>
constexpr char unsigned_to_hexchar(UInt_t value) noexcept
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
template<std::unsigned_integral UInt_t>
constexpr UInt_t hex_to_unsigned(std::string_view hex) noexcept
{
    if (hex.starts_with("0x")) hex.remove_prefix(2);

    UInt_t value = 0;
    for (std::size_t i = 0; i < hex.length(); ++i)
    {
        value *= 16ull;
        value += hexchar_to_unsigned<UInt_t>(hex[i]);
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
template<std::unsigned_integral UInt_t>
std::string unsigned_to_hex(UInt_t value) noexcept
{
    char hex[sizeof(UInt_t) * 2 + 1] = "";
    for (int i = sizeof(UInt_t) * 2 - 1; i >= 0; --i)
    {
        UInt_t b = value & 0x0F;
        hex[i] = unsigned_to_hexchar(b);
        value -= b;
        value /= 16;
    }
    hex[sizeof(UInt_t) * 2] = '\0';

    return hex;
}

}
}
