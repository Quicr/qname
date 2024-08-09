#include <benchmark/benchmark.h>

#include <quicr/hex_endec.h>
#include <quicr/name.h>

#include <cstdint>

static void HexEndec_Encode4x32_to_128(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        quicr::HexEndec<128, 32, 32, 32, 32>::Encode(0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU);
    }
}

static void HexEndec_Decode128_to_4x32(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        quicr::HexEndec<128, 32, 32, 32, 32>::Decode("0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    }
}

static void HexEndec_Encode4x16_to_64(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        quicr::HexEndec<64, 16, 16, 16, 16>::Encode(0xFFFFU, 0xFFFFU, 0xFFFFU, 0xFFFFU);
    }
}

static void HexEndec_Decode64_to_4x16(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        quicr::HexEndec<64, 16, 16, 16, 16>::Decode("0xFFFFFFFFFFFFFFFF");
    }
}

static void HexEndec_RealEncode(benchmark::State& state)
{
    const uint32_t orgId = 0x00A11CEE;
    const uint8_t appId = 0x00;
    const uint32_t confId = 0x00F00001;
    const uint8_t mediaType = 0x00U | 0x1U;
    const uint16_t clientId = 0xFFFF;
    const uint64_t uniqueId = 0U;
    for ([[maybe_unused]] auto _ : state)
    {
        quicr::HexEndec<128, 24, 8, 24, 8, 16, 48>::Encode(orgId, appId, confId, mediaType, clientId, uniqueId);
    }
}

static void HexEndec_RealDecode_Name(benchmark::State& state)
{
    const quicr::Name qname = 0xA11CEE00F00001000000000000000000_name;
    for ([[maybe_unused]] auto _ : state)
    {
        [[maybe_unused]] auto s = quicr::HexEndec<128, 24, 8, 24, 8, 16, 48>::Decode(qname);
    }
}
static void HexEndec_RealDecode_String(benchmark::State& state)
{
    const quicr::Name qname = 0xA11CEE00F00001000000000000000000_name;
    for ([[maybe_unused]] auto _ : state)
    {
        [[maybe_unused]] auto s = quicr::HexEndec<128, 24, 8, 24, 8, 16, 48>::Decode(qname);
    }
}

BENCHMARK(HexEndec_Encode4x32_to_128);
BENCHMARK(HexEndec_Decode128_to_4x32);
BENCHMARK(HexEndec_Encode4x16_to_64);
BENCHMARK(HexEndec_Decode64_to_4x16);
BENCHMARK(HexEndec_RealEncode);
BENCHMARK(HexEndec_RealDecode_Name);
BENCHMARK(HexEndec_RealDecode_String);
