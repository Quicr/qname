#include <benchmark/benchmark.h>

#include <quicr/name.h>

#include <cstdint>
#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>

#if __cplusplus >= 202002L
static void Name_ConstructFrom_String(benchmark::State& state)
{
    const std::string str = "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
    for ([[maybe_unused]] auto _ : state)
    {
        [[maybe_unused]] const quicr::Name n(str);
    }
}

static void Name_ConstructFrom_StringView(benchmark::State& state)
{
    const std::string_view str = "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
    for ([[maybe_unused]] auto _ : state)
    {
        [[maybe_unused]] const quicr::Name n(str);
    }
}

static void Name_ConstructFrom_ConstexprStringView(benchmark::State& state)
{
    constexpr std::string_view str = "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
    for ([[maybe_unused]] auto _ : state)
    {
        [[maybe_unused]] const quicr::Name n(str);
    }
}
#else
static void Name_ConstructFrom_CString(benchmark::State& state)
{
    const std::string str = "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
    for ([[maybe_unused]] auto _ : state)
    {
        [[maybe_unused]] const quicr::Name n(str.c_str());
    }
}

static void Name_ConstructFrom_ConstexprCString(benchmark::State& state)
{
    constexpr const char* str = "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
    for ([[maybe_unused]] auto _ : state)
    {
        [[maybe_unused]] const quicr::Name n(str);
    }
}
#endif

static void Name_ConstructFrom_Vector(benchmark::State& state)
{
    std::vector<uint8_t> data = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    };
    for ([[maybe_unused]] auto _ : state)
    {
        [[maybe_unused]] const quicr::Name n(data.data(), data.size());
    }
}

static void Name_ConstructFrom_BytePointer(benchmark::State& state)
{
    const std::vector<uint8_t> vec_data = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    };

    const std::uint8_t* data = vec_data.data();
    const std::size_t length = vec_data.size();

    for ([[maybe_unused]] auto _ : state)
    {
        [[maybe_unused]] const quicr::Name n(data, length);
    }
}

static void Name_ConstructFrom_Copy(benchmark::State& state)
{
    constexpr const quicr::Name name = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name;
    for ([[maybe_unused]] auto _ : state)
    {
        [[maybe_unused]] const quicr::Name n(name);
    }
}

static void Name_Arithmetic_LeftShift(benchmark::State& state)
{
    constexpr const quicr::Name name = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name;
    for ([[maybe_unused]] auto _ : state)
    {
        name << 64;
    }
}

static void Name_Arithmetic_RightShift(benchmark::State& state)
{
    constexpr const quicr::Name name = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name;
    for ([[maybe_unused]] auto _ : state)
    {
        name >> 64;
    }
}

static void Name_Arithmetic_Add(benchmark::State& state)
{
    quicr::Name name = 0x0_name;
    for ([[maybe_unused]] auto _ : state)
    {
        ++name;
    }
}

static void Name_Arithmetic_Sub(benchmark::State& state)
{
    quicr::Name name = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name;
    for ([[maybe_unused]] auto _ : state)
    {
        --name;
    }
}

constexpr quicr::Name object_id_mask = 0x00000000000000000000000000001111_name;
constexpr quicr::Name group_id_mask = 0x00000000000000000000111111110000_name;
static void Name_RealArithmetic(benchmark::State& state)
{
    quicr::Name name = 0xA11CEE00F00001000000000000000000_name;
    for ([[maybe_unused]] auto _ : state)
    {
        name = (name & ~object_id_mask) | (++name & object_id_mask);

        auto group_id_bits = (++(name >> 16) << 16) & group_id_mask;
        name = ((name & ~group_id_mask) | group_id_bits) & ~object_id_mask;
    }
}

static void Name_ExtractBits(benchmark::State& state)
{
    constexpr quicr::Name name = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name;
    for ([[maybe_unused]] auto _ : state)
    {
        [[maybe_unused]] auto n = name.bits(64, 64);
    }
}

static void Name_ConvertTo_UInt64(benchmark::State& state)
{
    constexpr quicr::Name name = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name;
    for ([[maybe_unused]] auto _ : state)
    {
        [[maybe_unused]] auto n = std::uint64_t(name);
    }
}

static void Name_ConvertTo_String(benchmark::State& state)
{
    constexpr quicr::Name name = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name;
    for ([[maybe_unused]] auto _ : state)
    {
        [[maybe_unused]] const std::string n = name;
    }
}

#if __cplusplus >= 202002L
BENCHMARK(Name_ConstructFrom_String);
BENCHMARK(Name_ConstructFrom_StringView);
BENCHMARK(Name_ConstructFrom_ConstexprStringView);
#else
BENCHMARK(Name_ConstructFrom_CString);
BENCHMARK(Name_ConstructFrom_ConstexprCString);
#endif
BENCHMARK(Name_ConstructFrom_Vector);
BENCHMARK(Name_ConstructFrom_BytePointer);
BENCHMARK(Name_ConstructFrom_Copy);
BENCHMARK(Name_Arithmetic_LeftShift);
BENCHMARK(Name_Arithmetic_RightShift);
BENCHMARK(Name_Arithmetic_Add);
BENCHMARK(Name_Arithmetic_Sub);
BENCHMARK(Name_RealArithmetic);
BENCHMARK(Name_ExtractBits);
BENCHMARK(Name_ConvertTo_UInt64);
BENCHMARK(Name_ConvertTo_String);
