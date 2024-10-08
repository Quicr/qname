#include <doctest/doctest.h>

#include <quicr/name.h>

#include <cstdint>
#include <iterator>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

TEST_CASE("quicr::Name Type Tests")
{
    CHECK(std::is_trivial_v<quicr::Name>);
    CHECK(std::is_trivially_constructible_v<quicr::Name>);
    CHECK(std::is_trivially_default_constructible_v<quicr::Name>);
    CHECK(std::is_trivially_destructible_v<quicr::Name>);
    CHECK(std::is_trivially_copyable_v<quicr::Name>);
    CHECK(std::is_trivially_copy_assignable_v<quicr::Name>);
    CHECK(std::is_trivially_move_constructible_v<quicr::Name>);
    CHECK(std::is_trivially_move_assignable_v<quicr::Name>);
    CHECK_EQ(sizeof(quicr::Name), sizeof(std::uint64_t) * 2);
}

TEST_CASE("quicr::Name Constructor Tests")
{
    CHECK_EQ(0x1_name, 0x00000000000000000000000000000001_name);

    CHECK_LT(0x123_name, 0x124_name);
    CHECK_GT(0x123_name, 0x122_name);
    CHECK_NE(0x123_name, 0x122_name);

    CHECK_GT(0x20000000000000001_name, 0x10000000000000002_name);
    CHECK_LT(0x10000000000000002_name, 0x20000000000000001_name);

    CHECK_NOTHROW(0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name);
    CHECK_THROWS(0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF0_name);
}

TEST_CASE("quicr::Name To Hex Tests")
{
    {

#if __cplusplus >= 202002L
        constexpr std::string_view original_hex = "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
#else
        constexpr const char* original_hex = "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
#endif
        constexpr quicr::Name name = original_hex;

        CHECK_EQ(std::string(name), original_hex);
    }
    {
#if __cplusplus >= 202002L
        constexpr std::string_view original_hex = "0xFFFFFFFFFFFFFFFF0000000000000000";
#else
        constexpr const char* original_hex = "0xFFFFFFFFFFFFFFFF0000000000000000";
#endif
        constexpr quicr::Name name = original_hex;

        CHECK_EQ(std::string(name), original_hex);
    }
    {
#if __cplusplus >= 202002L
        constexpr std::string_view long_hex = "0x0000000000000000FFFFFFFFFFFFFFFF";
#else
        constexpr const char* long_hex = "0x0000000000000000FFFFFFFFFFFFFFFF";
#endif
        constexpr quicr::Name long_name = long_hex;

#if __cplusplus >= 202002L
        constexpr std::string_view short_hex = "0xFFFFFFFFFFFFFFFF";
#else
        constexpr const char* short_hex = "0xFFFFFFFFFFFFFFFF";
#endif
        constexpr quicr::Name not_short_name = short_hex;
        CHECK_EQ(std::string(long_name), long_hex);
        CHECK_NE(std::string(not_short_name), short_hex);
        CHECK_EQ(long_name, not_short_name);
        CHECK_EQ(long_name, not_short_name);
    }
}

TEST_CASE("quicr::Name Bit Shifting Tests")
{
    CHECK_EQ((0x1234_name >> 4), 0x123_name);
    CHECK_EQ((0x1234_name << 4), 0x12340_name);

    {
        constexpr quicr::Name unshifted_32bit = 0x123456789ABCDEFF00000000_name;
        constexpr quicr::Name shifted_32bit = 0x123456789ABCDEFF_name;
        CHECK_EQ((unshifted_32bit >> 32), shifted_32bit);
        CHECK_EQ((shifted_32bit << 32), unshifted_32bit);
    }

    {
        constexpr quicr::Name unshifted_64bit = 0x123456789ABCDEFF123456789ABCDEFF_name;
        constexpr quicr::Name shifted_64bit = 0x123456789ABCDEFF_name;
        constexpr quicr::Name shifted_72bit = 0x123456789ABCDE_name;
        CHECK_EQ((unshifted_64bit >> 64), shifted_64bit);
        CHECK_EQ((unshifted_64bit >> 72), shifted_72bit);
        CHECK_EQ((shifted_64bit >> 8), shifted_72bit);
    }

    {
        constexpr quicr::Name unshifted_64bit = 0x123456789ABCDEFF_name;
        constexpr quicr::Name shifted_64bit = 0x123456789ABCDEFF0000000000000000_name;
        constexpr quicr::Name shifted_72bit = 0x3456789ABCDEFF000000000000000000_name;
        CHECK_EQ((unshifted_64bit << 64), shifted_64bit);
        CHECK_EQ((unshifted_64bit << 72), shifted_72bit);
        CHECK_EQ((shifted_64bit << 8), shifted_72bit);
    }

    {
        constexpr quicr::Name unshifted_bits = 0x00000000000000000000000000000001_name;
        quicr::Name bits = unshifted_bits;
        for (int i = 0; i < 64; ++i)
        {
            bits <<= 1;
        }

        CHECK_EQ(bits, 0x00000000000000010000000000000000_name);

        for (int i = 0; i < 64; ++i)
        {
            bits >>= 1;
        }

        CHECK_EQ(bits, unshifted_bits);
    }
}

TEST_CASE("quicr::Name Integer Arithmetic Tests")
{
    constexpr quicr::Name val42 = 0x42_name;
    constexpr quicr::Name val41 = 0x41_name;
    constexpr quicr::Name val43 = 0x43_name;
    CHECK_EQ(val42 + 1, val43);
    CHECK_EQ(val42 - 1, val41);

    CHECK_EQ(0x00000000000000010000000000000000_name + 1, 0x00000000000000010000000000000001_name);
    CHECK_EQ(0x0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name + 1, 0x10000000000000000000000000000000_name);
    CHECK_EQ(0x0000000000000000FFFFFFFFFFFFFFFF_name + 0xFFFFFFFF, 0x000000000000000100000000FFFFFFFE_name);

    CHECK_EQ(0x00000000000000010000000000000000_name - 1, 0x0000000000000000FFFFFFFFFFFFFFFF_name);
    CHECK_EQ(0x0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name - 1, 0x0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE_name);
    CHECK_EQ(0x0000000000000000FFFFFFFFFFFFFFFF_name - 0xFFFFFFFFFFFFFFFF, 0x00000000000000000000000000000000_name);
    CHECK_EQ(0x00000000000000010000000000000000_name - 2, 0x0000000000000000FFFFFFFFFFFFFFFE_name);

    quicr::Name val42_copy(val42);
    CHECK_EQ(val42_copy, val42);
    CHECK_NE(val42_copy++, val43);
    CHECK_EQ(val42_copy, val43);
    CHECK_NE(val42_copy--, val42);
    CHECK_EQ(val42_copy, val42);
    CHECK_EQ(++val42_copy, val43);
    CHECK_EQ(--val42_copy, val42);
}

TEST_CASE("quicr::Name Name Arithmetic Tests")
{
    CHECK_EQ(0x0_name + 0xFFFFFFFFFFFFFFFF0000000000000000_name, 0xFFFFFFFFFFFFFFFF0000000000000000_name);
    CHECK_EQ(0x0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name + 0x0000000000000000FFFFFFFFFFFFFFFF_name,
             0x1000000000000000FFFFFFFFFFFFFFFE_name);
    CHECK_EQ(0xEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE_name + 0x11111111111111111111111111111111_name,
             0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name);

    CHECK_EQ(0x0000000000000000FFFFFFFFFFFFFFFF_name - 0x0000000000000000FFFFFFFFFFFFFFFF_name, 0x0_name);

    CHECK_EQ(0xFFFFFFFFFFFFFFFF0000000000000000_name - 0xFFFFFFFFFFFFFFFF0000000000000001_name,
             0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name);

    CHECK_EQ(0xFFFFFFFFFFFFFFFF0000000000000000_name - 0xFFFFFFFFFFFFFFFE0000000000000001_name,
             0x0000000000000000FFFFFFFFFFFFFFFF_name);
}

TEST_CASE("quicr::Name Bitwise Not Tests")
{
    constexpr quicr::Name zeros = 0x0_name;
    constexpr quicr::Name ones = ~zeros;

    constexpr quicr::Name expected_ones = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name;
    auto literal_ones = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name;

    CHECK_NE(ones, zeros);
    CHECK_EQ(ones, expected_ones);
    CHECK_EQ(literal_ones, expected_ones);
}

TEST_CASE("quicr::Name Full Byte Array Tests")
{
    constexpr quicr::Name name_to_bytes = 0x10000000000000000000000000000000_name;
    const std::uint8_t* bytes_ptr = reinterpret_cast<const std::uint8_t*>(&name_to_bytes);

#if __cplusplus >= 202002L
    std::vector<uint8_t> byte_arr = { bytes_ptr, std::next(bytes_ptr, sizeof(quicr::Name)) };
    CHECK_FALSE(byte_arr.empty());
    CHECK_EQ(byte_arr.size(), 16);

    const quicr::Name name_from_bytes(byte_arr);
    CHECK_EQ(name_from_bytes, name_to_bytes);
#endif

    const quicr::Name name_from_byte_ptr(bytes_ptr, sizeof(quicr::Name));
    CHECK_EQ(name_from_byte_ptr, name_to_bytes);
}

TEST_CASE("quicr::Name Medium Byte Array Tests")
{
    constexpr quicr::Name long_name = 0x10000000010000000000000000000000_name;
    constexpr quicr::Name short_name = 0x1000000001_name;

    std::vector<uint8_t> byte_arr = { 0x01, 0x00, 0x00, 0x00, 0x10 };

#if __cplusplus >= 202002L
    const quicr::Name name_from_bytes(byte_arr);
#else
    const quicr::Name name_from_bytes(byte_arr.data(), byte_arr.size());
#endif
    CHECK_NE(name_from_bytes, long_name);
    CHECK_EQ(name_from_bytes, short_name);
}

TEST_CASE("quicr::Name Short Byte Array Tests")
{
    constexpr quicr::Name long_name = 0x10000000000000000000000000000000_name;
    constexpr quicr::Name short_name = 0x10_name;

    std::vector<uint8_t> byte_arr = { 0x10 };

#if __cplusplus >= 202002L
    const quicr::Name name_from_bytes(byte_arr);
#else
    const quicr::Name name_from_bytes(byte_arr.data(), byte_arr.size());
#endif
    CHECK_NE(name_from_bytes, long_name);
    CHECK_EQ(name_from_bytes, short_name);
}

TEST_CASE("quicr::Name Integer Byte Array Tests")
{
    {
        constexpr std::uint64_t i = 0x123456;
        const std::uint8_t* i_ptr = reinterpret_cast<const std::uint8_t*>(&i);
        const quicr::Name name_from_bytes(i_ptr, 3);
        CHECK_EQ(name_from_bytes, 0x00000000000000000000000000123456_name);
    }

    {
        std::vector<uint8_t> byte_arr = { 0x56, 0x34, 0x12 };
#if __cplusplus >= 202002L
        const quicr::Name name_from_bytes(byte_arr);
#else
        const quicr::Name name_from_bytes(byte_arr.data(), byte_arr.size());
#endif
        CHECK_EQ(name_from_bytes, 0x00000000000000000000000000123456_name);
    }
}

TEST_CASE("quicr::Name Logical Arithmetic Tests")
{
    auto full_arith_and = 0x01010101010101010101010101010101_name & 0x10101010101010101010101010101010_name;
    CHECK_EQ(full_arith_and, 0x0_name);

    auto short_arith_and = 0x0101010101010101_name & 0x1010101010101010;
    CHECK_EQ(short_arith_and, 0x0_name);

    auto full_arith_or = 0x01010101010101010101010101010101_name | 0x10101010101010101010101010101010_name;
    CHECK_EQ(full_arith_or, 0x11111111111111111111111111111111_name);

    auto short_arith_or = 0x0101010101010101_name | 0x1010101010101010;
    CHECK_EQ(short_arith_or, 0x1111111111111111_name);

    auto mask = 0xFFFFFFFFFFFFFFFF_name;
    CHECK_EQ(~mask, 0xFFFFFFFFFFFFFFFF0000000000000000_name);
    mask = ~mask;

    {
        auto some_name = 0xABCDEFABCDEF01234567890123456789_name;
        some_name &= mask;
        CHECK_EQ(some_name, 0xABCDEFABCDEF01230000000000000000_name);
    }

    {
        auto some_name = 0xABCDEFABCDEF01234567890123456789_name;
        some_name |= mask;
        CHECK_EQ(some_name, 0xFFFFFFFFFFFFFFFF4567890123456789_name);
    }

    {
        auto some_name = 0xABCDEFABCDEF01234567890123456789_name;
        some_name ^= mask;
        CHECK_EQ(some_name, 0x543210543210FEDC4567890123456789_name);
    }
}

TEST_CASE("quicr::Name Conversion Tests")
{
    constexpr quicr::Name name = 0x000000000000FFFFFFFFFFFFFFFFFFFF_name;

    CHECK_EQ(std::uint8_t(name), 0xFF);
    CHECK_EQ(std::uint16_t(name), 0xFFFF);
    CHECK_EQ(std::uint32_t(name), 0xFFFFFFFF);
    CHECK_EQ(std::uint64_t(name), 0xFFFFFFFFFFFFFFFF);

    CHECK_EQ(std::string(name), "0x000000000000FFFFFFFFFFFFFFFFFFFF");
}

TEST_CASE("quicr::Name Extract Bits Tests")
{
    constexpr quicr::Name name = 0x000000000000FFFFFFFF000000000000_name;

    CHECK_EQ(name.bits<std::uint64_t>(48, 8), std::uint64_t(0xFF));
    CHECK_EQ(name.bits<std::uint64_t>(48, 16), std::uint64_t(0xFFFF));
    CHECK_EQ(name.bits<std::uint64_t>(48, 32), std::uint64_t(0xFFFFFFFF));

    CHECK_EQ(name.bits<std::uint64_t>(0, 64), std::uint64_t(0xFFFF000000000000));
    CHECK_EQ(name.bits<std::uint64_t>(16, 64), std::uint64_t(0xFFFFFFFF00000000));
    CHECK_EQ(name.bits<std::uint64_t>(64, 64), std::uint64_t(0x000000000000FFFF));

    CHECK_EQ(name.bits(48, 24), 0x00000000000000FFFFFF000000000000_name);
}
