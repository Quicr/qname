#pragma once

#include "_utilities.h"
#include "name.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <numeric>
#include <vector>
#if __cplusplus >= 202002L
#include <concepts>
#include <span>
#include <string_view>
#endif

namespace quicr
{
/**
 * @brief Encodes/Decodes a hex string from/into a list of unsigned integers
 *        values.
 *
 * The hex string created by/passed to this class is of the format:
 *     0xXX...XYY...YZZ...Z....
 *       |____||____||____|
 *        Dist0 Dist1 Dist2   ...
 *       |_____________________|
 *                Size
 * Where Dist is the distribution of bits for each value provided. For Example:
 *   HexEndec<64, 32, 24, 8>
 * Describes a 64 bit hex value, distributed into 3 sections 32bit, 24bit, and
 * 8bit respectively.
 *
 * @tparam Size The maximum size in bits of the Hex string
 * @tparam Dist The distribution of bits for each value passed.
 */
template<std::uint16_t Size, uint16_t... Dist>
class HexEndec
{
    static_assert((Size & (Size - 1)) == 0, "Size must be a power of 2");
    static_assert(Size <= sizeof(quicr::Name) * 8, "Size must not exceed sizeof(quicr::Name) * 8");

  public:
    constexpr HexEndec() noexcept { static_assert(Size == (Dist + ...), "Total bits must be equal to Size"); }

    /**
     * @brief Encodes the last Dist bits of values in order according to distribution
     *        of Dist and builds a hex string that is the size in bits of Size.
     *
     * @tparam UInt_ts The unsigned integer types to be passed.
     * @param values The unsigned values to be encoded into the hex string.
     *
     * @returns Hex string containing the provided values distributed according
     *          to Dist in order.
     */
#if __cplusplus >= 202002L
    template<std::unsigned_integral... UInt_ts>
#else
    template<typename... UInt_ts, typename std::enable_if_t<(std::is_unsigned_v<UInt_ts> && ...), bool> = true>
#endif
    static inline std::string Encode(UInt_ts... values)
    {
        static_assert(Size == (Dist + ...), "Total bits cannot exceed specified size");
        static_assert(sizeof...(Dist) == sizeof...(UInt_ts), "Number of values should match distribution of bits");

        std::array<std::uint16_t, sizeof...(Dist)> distribution{ Dist... };
        std::array<std::uint64_t, sizeof...(UInt_ts)> vals{ values... };
#if __cplusplus >= 202002L
        return Encode(distribution, std::span<std::uint64_t>(vals));
#else
        return Encode(distribution, vals);
#endif
    }

#if __cplusplus >= 202002L
    template<std::unsigned_integral... UInt_ts>
    static inline std::string Encode(std::span<std::uint16_t> distribution, UInt_ts... values)
#else
    template<typename... UInt_ts, typename std::enable_if_t<(std::is_unsigned_v<UInt_ts> && ...), bool> = true>
    static inline std::string Encode(std::vector<std::uint16_t> distribution, UInt_ts... values)
#endif
    {
        if (Size < std::accumulate(distribution.begin(), distribution.end(), 0))
            throw std::invalid_argument("Total bits cannot exceed specified size");

        std::array<std::uint64_t, sizeof...(UInt_ts)> vals{ values... };
#if __cplusplus >= 202002L
        return Encode(distribution, std::span<std::uint64_t>(vals));
#else
        return Encode(distribution, vals);
#endif
    }

#if __cplusplus >= 202002L
    static inline std::string Encode(std::span<std::uint16_t> distribution, std::span<std::uint64_t> values)
#else
    template<std::size_t D_N, std::size_t V_N>
    static inline std::string Encode(std::array<std::uint16_t, D_N> distribution, std::array<std::uint64_t, V_N> values)
#endif
    {
        if (distribution.size() != values.size())
            throw std::invalid_argument("Number of values should match distribution of bits");

        quicr::Name bits = 0x0_name;
        for (size_t i = 0; i < values.size(); ++i)
        {
            const auto& value = values[i];
            const auto& dist = distribution[i];
            bits <<= dist;
            bits |= (dist >= sizeof(uint64_t) * 8) ? value : (value & ~(~0x0ull << dist));
        };

        if constexpr (Size == sizeof(quicr::Name) * 8) return bits;

        return "0x" + utility::unsigned_to_hex(bits.bits<std::uint64_t>(0, Size));
    }

    /**
     * @brief Decodes a hex string that has size in bits of Size into a list of
     *        values sized according to Dist in order.
     *
     * @tparam UInt_t The unsigned integer type to return.
     * @param hex The hex string to decode. Must have a length in bytes
     *            corresponding to the size in bits of Size.
     *
     * @returns Structured binding of values decoded from hex string
     *          corresponding in order to the size of Dist.
     */
#if __cplusplus >= 202002L
    template<std::unsigned_integral UInt_t = std::uint64_t>
    static constexpr std::array<UInt_t, sizeof...(Dist)> Decode(std::string_view hex)
    {
        return Decode<UInt_t>(quicr::Name(hex));
    }
#else
    template<typename UInt_t = std::uint64_t, typename std::enable_if_t<std::is_unsigned_v<UInt_t>, bool> = true>
    static constexpr std::array<UInt_t, sizeof...(Dist)> Decode(const char* hex)
    {
        return Decode<UInt_t>(quicr::Name(hex));
    }
#endif

#if __cplusplus >= 202002L
    template<std::unsigned_integral UInt_t = std::uint64_t>
#else
    template<typename UInt_t = std::uint64_t, typename std::enable_if_t<std::is_unsigned_v<UInt_t>, bool> = true>
#endif
    static constexpr std::array<UInt_t, sizeof...(Dist)> Decode(quicr::Name name)
    {
        static_assert(Size >= (Dist + ...), "Total bits cannot exceed specified size");

        std::array<std::uint16_t, sizeof...(Dist)> distribution{ Dist... };
        return Decode<sizeof...(Dist), UInt_t>(distribution, name);
    }

#if __cplusplus >= 202002L
    template<std::size_t N, std::unsigned_integral UInt_t = std::uint64_t>
    static constexpr std::array<UInt_t, N> Decode(std::span<std::uint16_t> distribution, quicr::Name name)
#else
    template<std::size_t N, typename UInt_t = std::uint64_t, typename std::enable_if_t<std::is_unsigned_v<UInt_t>, bool> = true>
    static constexpr std::array<UInt_t, N> Decode(std::array<std::uint16_t, N> distribution, quicr::Name name)
#endif
    {
        const auto dist_size = distribution.size();
        std::array<UInt_t, N> result;

        for (size_t i = 0; i < dist_size; ++i)
        {
            const auto dist = distribution[i];
            result[i] = name.bits<UInt_t>(Size - dist, dist);
            name <<= dist;
        }

        return result;
    }

#if __cplusplus >= 202002L
    template<std::unsigned_integral UInt_t = std::uint64_t>
    static inline std::vector<UInt_t> Decode(std::span<std::uint16_t> distribution, std::string_view hex)
#else
    template<typename UInt_t = std::uint64_t, typename std::enable_if_t<std::is_unsigned_v<UInt_t>, bool> = true>
    static inline std::vector<UInt_t> Decode(std::vector<std::uint16_t> distribution, const char* hex)
#endif
    {
        return Decode(distribution, quicr::Name(hex));
    }

#if __cplusplus >= 202002L
    template<std::unsigned_integral UInt_t = std::uint64_t>
    static inline std::vector<UInt_t> Decode(std::span<std::uint16_t> distribution, quicr::Name name)
#else
    template<typename UInt_t = std::uint64_t, typename std::enable_if_t<std::is_unsigned_v<UInt_t>, bool> = true>
    static inline std::vector<UInt_t> Decode(std::vector<std::uint16_t> distribution, quicr::Name name)
#endif
    {
        const auto dist_size = distribution.size();
        std::vector<UInt_t> result(dist_size);
        for (size_t i = 0; i < dist_size; ++i)
        {
            const auto dist = distribution[i];
            result[i] = name.bits<UInt_t>(Size - dist, dist);
            name <<= dist;
        }

        return result;
    }
};
} // namespace quicr
