#include <benchmark/benchmark.h>

#include <quicr/name.h>
#include <quicr/namespace.h>

#include <string>
#include <string_view>

static void Namespace_ConstructFrom_Name(benchmark::State& state)
{
    constexpr quicr::Name name = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name;
    for ([[maybe_unused]] auto _ : state)
    {
        [[maybe_unused]] const quicr::Namespace ns(name, 80);
    }
}

#if __cplusplus >= 202002L
static void Namespace_ConstructFrom_String(benchmark::State& state)
{
    const std::string str = "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF/80";
    for ([[maybe_unused]] auto _ : state)
    {
        [[maybe_unused]] const quicr::Namespace ns(str);
    }
}

static void Namespace_ConstructFrom_StringView(benchmark::State& state)
{
    const std::string_view str = "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF/80";
    for ([[maybe_unused]] auto _ : state)
    {
        [[maybe_unused]] const quicr::Namespace ns(str);
    }
}

static void Namespace_ConstructFrom_ConstexprStringView(benchmark::State& state)
{
    constexpr std::string_view str = "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF/80";
    for ([[maybe_unused]] auto _ : state)
    {
        [[maybe_unused]] constexpr quicr::Namespace ns(str);
    }
}
#else
static void Namespace_ConstructFrom_CString(benchmark::State& state)
{
    const char* str = "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF/80";
    for ([[maybe_unused]] auto _ : state)
    {
        [[maybe_unused]] const quicr::Namespace ns(str);
    }
}

static void Namespace_ConstructFrom_ConstexprCString(benchmark::State& state)
{
    constexpr const char* str = "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF/80";
    for ([[maybe_unused]] auto _ : state)
    {
        [[maybe_unused]] constexpr quicr::Namespace ns(str);
    }
}
#endif

static void Namespace_ConvertTo_String(benchmark::State& state)
{
    constexpr quicr::Name name = 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF_name;
    for ([[maybe_unused]] auto _ : state)
    {
        [[maybe_unused]] const std::string ns = name;
    }
}

BENCHMARK(Namespace_ConstructFrom_Name);
#if __cplusplus >= 202002L
BENCHMARK(Namespace_ConstructFrom_String);
BENCHMARK(Namespace_ConstructFrom_StringView);
BENCHMARK(Namespace_ConstructFrom_ConstexprStringView);
#else
BENCHMARK(Namespace_ConstructFrom_CString);
BENCHMARK(Namespace_ConstructFrom_ConstexprCString);
#endif
BENCHMARK(Namespace_ConvertTo_String);
