#include <benchmark/benchmark.h>

#include <quicr_name>
#include <vector>

static void Name_ConstructFrom_String(benchmark::State& state)
{
    std::string str = "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
    for (auto _ : state)
    {
        [[maybe_unused]] quicr::Name __(str);
    }
}

static void Name_ConstructFrom_StringView(benchmark::State& state)
{
    std::string_view str = "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
    for (auto _ : state)
    {
        [[maybe_unused]] quicr::Name __(str);
    }
}

static void Name_ConstructFrom_ConstexprStringView(benchmark::State& state)
{
    constexpr std::string_view str = "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
    for (auto _ : state)
    {
        [[maybe_unused]] quicr::Name __(str);
    }
}

static void Name_ConstructFrom_Vector(benchmark::State& state)
{
    std::vector<uint8_t> data = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    };
    for (auto _ : state)
    {
        [[maybe_unused]] quicr::Name __(data);
    }
}

static void Name_ConstructFrom_BytePointer(benchmark::State& state)
{
    std::vector<uint8_t> vec_data = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    };

    uint8_t* data = vec_data.data();
    size_t length = vec_data.size();

    for (auto _ : state)
    {
        [[maybe_unused]] quicr::Name __(data, length);
    }
}

static void Name_ConstructFrom_Copy(benchmark::State& state)
{
    quicr::Name name = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name;
    for (auto _ : state)
    {
        [[maybe_unused]] quicr::Name __(name);
    }
}

static void Name_Arithmetic_LeftShift(benchmark::State& state)
{
    quicr::Name name = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name;
    for (auto _ : state)
    {
        name << 64;
    }
}

static void Name_Arithmetic_RightShift(benchmark::State& state)
{
    quicr::Name name = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name;
    for (auto _ : state)
    {
        name >> 64;
    }
}

static void Name_Arithmetic_Add(benchmark::State& state)
{
    quicr::Name name = 0x0_name;
    for (auto _ : state)
    {
        ++name;
    }
}

static void Name_Arithmetic_Sub(benchmark::State& state)
{
    quicr::Name name = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name;
    for (auto _ : state)
    {
        --name;
    }
}

constexpr quicr::Name object_id_mask = 0x00000000000000000000000000001111_name;
constexpr quicr::Name group_id_mask = 0x00000000000000000000111111110000_name;
static void Name_RealArithmetic(benchmark::State& state)
{
    quicr::Name name = 0xA11CEE00F00001000000000000000000_name;
    for (auto _ : state)
    {
        name = (name & ~object_id_mask) | (++name & object_id_mask);

        auto group_id_bits = (++(name >> 16) << 16) & group_id_mask;
        name = ((name & ~group_id_mask) | group_id_bits) & ~object_id_mask;
    }
}

static void Name_ExtractBits(benchmark::State& state)
{
    quicr::Name name = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name;
    for (auto _ : state)
    {
        [[maybe_unused]] auto __ = name.bits(64, 64);
    }
}

static void Name_ConvertTo_UInt64(benchmark::State& state)
{
    quicr::Name name = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name;
    for (auto _ : state)
    {
        [[maybe_unused]] auto __ = std::uint64_t(name);
    }
}

static void Name_ConvertTo_String(benchmark::State& state)
{
    quicr::Name name = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name;
    for (auto _ : state)
    {
        [[maybe_unused]] std::string __ = name;
    }
}

BENCHMARK(Name_ConstructFrom_String);
BENCHMARK(Name_ConstructFrom_StringView);
BENCHMARK(Name_ConstructFrom_ConstexprStringView);
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
