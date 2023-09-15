#include <benchmark/benchmark.h>

#include <qname>
#include <vector>

static void Namespace_ConstructFrom_Name(benchmark::State& state)
{
    quicr::Name name = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name;
    for (auto _ : state)
    {
        [[maybe_unused]] quicr::Namespace __(name, 80);
    }
}

static void Namespace_ConstructFrom_String(benchmark::State& state)
{
    std::string str = "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF/80";
    for (auto _ : state)
    {
        [[maybe_unused]] quicr::Namespace __(str);
    }
}

static void Namespace_ConstructFrom_StringView(benchmark::State& state)
{
    std::string_view str = "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF/80";
    for (auto _ : state)
    {
        [[maybe_unused]] quicr::Namespace __(str);
    }
}

static void Namespace_ConstructFrom_ConstexprStringView(benchmark::State& state)
{
    constexpr std::string_view str = "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF/80";
    for (auto _ : state)
    {
        [[maybe_unused]] constexpr quicr::Namespace __(str);
    }
}

static void Namespace_ConvertTo_String(benchmark::State& state)
{
    quicr::Name name = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name;
    for (auto _ : state)
    {
        [[maybe_unused]] std::string __ = name;
    }
}

BENCHMARK(Namespace_ConstructFrom_Name);
BENCHMARK(Namespace_ConstructFrom_String);
BENCHMARK(Namespace_ConstructFrom_StringView);
BENCHMARK(Namespace_ConstructFrom_ConstexprStringView);
BENCHMARK(Namespace_ConvertTo_String);
