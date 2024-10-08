#include <doctest/doctest.h>

#include <quicr/hex_endec.h>
#include <quicr/name.h>

#include <array>
#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>
#if __cplusplus >= 202002L
#include <string_view>
#endif

TEST_CASE("quicr::HexEndec Type Assertions")
{
    CHECK(std::is_trivially_destructible_v<quicr::HexEndec<128>>);
    CHECK(std::is_trivially_copyable_v<quicr::HexEndec<128>>);
    CHECK(std::is_trivially_copy_assignable_v<quicr::HexEndec<128>>);
    CHECK(std::is_trivially_move_constructible_v<quicr::HexEndec<128>>);
    CHECK(std::is_trivially_move_assignable_v<quicr::HexEndec<128>>);
}

TEST_CASE("quicr::HexEndec 128bit Encode/Decode Test")
{
    const std::string hex_value = "0x11111111111111112222222222222200";
    const std::uint64_t first_part = 0x1111111111111111;
    const std::uint64_t second_part = 0x22222222222222;
    const std::uint8_t third_part = 0x00;

    constexpr quicr::HexEndec<128, 64, 56, 8> formatter_128bit;
    const std::string formatted = formatter_128bit.Encode(first_part, second_part, third_part);
    CHECK_EQ(formatted, hex_value);

#if __cplusplus >= 202002L
    const auto [one, two, three] = formatter_128bit.Decode(std::string_view(hex_value));
#else
    const auto [one, two, three] = formatter_128bit.Decode(hex_value.c_str());
#endif
    CHECK_EQ(one, first_part);
    CHECK_EQ(two, second_part);
    CHECK_EQ(three, third_part);
}

TEST_CASE("quicr::HexEndec 128bit Encode/Decode Container Test")
{
#if __cplusplus >= 202002L
    const std::string_view hex_value = "0x11111111111111112222222222222200";
#else
    const char* hex_value = "0x11111111111111112222222222222200";
#endif
    const std::uint64_t first_part = 0x1111111111111111;
    const std::uint64_t second_part = 0x22222222222222;
    const std::uint8_t third_part = 0x00;

    std::array<uint16_t, 3> dist = { 64, 56, 8 };
    std::array<std::uint64_t, 3> vals = { first_part, second_part, third_part };
    const std::string mask = quicr::HexEndec<128>::Encode(dist, vals);
    CHECK_EQ(mask, hex_value);

    const auto out = quicr::HexEndec<128>::Decode(dist, hex_value);
    CHECK_EQ(out[0], first_part);
    CHECK_EQ(out[1], second_part);
    CHECK_EQ(out[2], third_part);
}

TEST_CASE("quicr::HexEndec 64bit Encode/Decode Test")
{
#if __cplusplus >= 202002L
    const std::string_view hex_value = "0x1111111122222200";
#else
    const char* hex_value = "0x1111111122222200";
#endif
    const std::uint64_t first_part = 0x11111111;
    const std::uint64_t second_part = 0x222222;
    const std::uint8_t third_part = 0x00;

    constexpr quicr::HexEndec<64, 32, 24, 8> formatter_64bit;
    const std::string mask = formatter_64bit.Encode(first_part, second_part, third_part);
    CHECK_EQ(mask, hex_value);

    const auto [one, two, three] = formatter_64bit.Decode(hex_value);
    CHECK_EQ(one, first_part);
    CHECK_EQ(two, second_part);
    CHECK_EQ(three, third_part);
}

TEST_CASE("quicr::HexEndec Decode Throw Test")
{
#if __cplusplus >= 202002L
    const std::string_view valid_hex_value = "0x11111111111111112222222222222200";
    const std::string_view invalid_hex_value = "0x1111111111111111222222222222220000";
#else
    const char* valid_hex_value = "0x11111111111111112222222222222200";
    const char* invalid_hex_value = "0x1111111111111111222222222222220000";
#endif

    constexpr quicr::HexEndec<128, 64, 56, 8> formatter_128bit;
    CHECK_NOTHROW(formatter_128bit.Decode(valid_hex_value));
    CHECK_THROWS(formatter_128bit.Decode(invalid_hex_value));
}

TEST_CASE("quicr::HexEndec Decode quicr::Name")
{
    const quicr::Name name = 0x11111111111111112222222222222233_name;
    std::array<std::uint64_t, 3> results{};

    constexpr quicr::HexEndec<128, 64, 56, 8> formatter_128bit;
    CHECK_NOTHROW(results = formatter_128bit.Decode(name));
    CHECK_EQ(results, std::array<std::uint64_t, 3>{ 0x1111111111111111, 0x22222222222222, 0x33 });
    CHECK_EQ(results[0], 0x1111111111111111);
}

TEST_CASE("quicr::HexEndec Name Encode/Decode Test")
{
    const quicr::Name name = 0x11111111111111112222222222222200_name;
    const std::uint64_t first_part = 0x1111111111111111;
    const std::uint64_t second_part = 0x22222222222222;
    const std::uint8_t third_part = 0x00;

    constexpr quicr::HexEndec<128, 64, 56, 8> formatter_128bit;
    const std::string mask = formatter_128bit.Encode(first_part, second_part, third_part);
    CHECK_EQ(mask, std::string(name));

    const auto [one, two, three] = formatter_128bit.Decode(name);
    CHECK_EQ(one, first_part);
    CHECK_EQ(two, second_part);
    CHECK_EQ(three, third_part);
}
