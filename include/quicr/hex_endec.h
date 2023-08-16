#pragma once

#include <quicr/name.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <numeric>
#include <span>
#include <string_view>
#include <vector>

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
 * @tparam ...Dist The distribution of bits for each value passed.
 */
template<uint16_t Size, uint16_t... Dist>
class HexEndec
{
    static_assert((Size & (Size - 1)) == 0, "Size must be a power of 2");
    static_assert(Size <= sizeof(quicr::Name) * 8, "Size must not exceed sizeof(quicr::Name) * 8");

  public:
    HexEndec() { static_assert(Size == (Dist + ...), "Total bits must be equal to Size"); }

    /**
     * @brief Encodes the last Dist bits of values in order according to
     * distribution of Dist and builds a hex string that is the size in bits of
     * Size.
     *
     * @tparam ...UInt_ts The unsigned integer types to be passed.
     * @param ...values The unsigned values to be encoded into the hex string.
     *
     * @returns Hex string containing the provided values distributed according
     * to Dist in order.
     */
    template<Unsigned... UInt_ts>
    static inline std::string Encode(UInt_ts... values)
    {
        static_assert(Size == (Dist + ...), "Total bits cannot exceed specified size");
        static_assert(sizeof...(Dist) == sizeof...(UInt_ts), "Number of values should match distribution of bits");

        std::array<uint16_t, sizeof...(Dist)> distribution{ Dist... };
        std::array<uint64_t, sizeof...(UInt_ts)> vals{ values... };
        return Encode(distribution, std::span<uint64_t>(vals));
    }

    template<Unsigned... UInt_ts>
    static inline std::string Encode(std::span<uint16_t> distribution, UInt_ts... values)
    {
        if(Size < std::accumulate(distribution.begin(), distribution.end(), 0))
            throw std::invalid_argument("Total bits cannot exceed specified size");

        std::array<uint64_t, sizeof...(UInt_ts)> vals{ values... };
        return Encode(distribution, std::span<uint64_t>(vals));
    }

    static inline std::string Encode(std::span<uint16_t> distribution, std::span<uint64_t> values)
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

        return "0x" + uint_to_hex(bits.bits<uint64_t>(0, Size));
    }

    /**
     * @brief Decodes a hex string that has size in bits of Size into a list of
     *        values sized according to Dist in order.
     *
     * @tparam Uint_t The unsigned integer type to return.
     * @param hex The hex string to decode. Must have a length in bytes
     *            corresponding to the size in bits of Size.
     *
     * @returns Structured binding of values decoded from hex string
     * corresponding in order to the size of Dist.
     */
    template<Unsigned Uint_t = uint64_t>
    static constexpr std::array<Uint_t, sizeof...(Dist)> Decode(std::string_view hex)
    {
        return Decode<Uint_t>(quicr::Name(hex));
    }

    template<Unsigned Uint_t = uint64_t>
    static constexpr std::array<Uint_t, sizeof...(Dist)> Decode(quicr::Name name)
    {
        static_assert(Size >= (Dist + ...), "Total bits cannot exceed specified size");

        std::array<uint16_t, sizeof...(Dist)> distribution{ Dist... };
        return Decode<sizeof...(Dist), Uint_t>(distribution, name);
    }

    template<size_t N, Unsigned Uint_t = uint64_t>
    static constexpr std::array<Uint_t, N> Decode(std::span<uint16_t> distribution, quicr::Name name)
    {
        const auto dist_size = distribution.size();
        std::array<Uint_t, N> result;

        for (size_t i = 0; i < dist_size; ++i)
        {
            const auto dist = distribution[i];
            result[i] = name.bits<Uint_t>(Size - dist, dist);
            name <<= dist;
        }

        return result;
    }

    template<Unsigned Uint_t = uint64_t>
    static inline std::vector<Uint_t> Decode(std::span<uint16_t> distribution, std::string_view hex)
    {
        return Decode(distribution, quicr::Name(hex));
    }

    template<Unsigned Uint_t = uint64_t>
    static inline std::vector<Uint_t> Decode(std::span<uint16_t> distribution, quicr::Name name)
    {
        const auto dist_size = distribution.size();
        std::vector<Uint_t> result(dist_size);
        for (size_t i = 0; i < dist_size; ++i)
        {
            const auto dist = distribution[i];
            result[i] = name.bits<Uint_t>(Size - dist, dist);
            name <<= dist;
        }

        return result;
    }
};
} // namespace quicr
