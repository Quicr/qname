#pragma once

#include <quicr/name.h>

#include <map>
#include <ostream>

namespace quicr
{

/**
 * @brief A prefix for a quicr::Name
 */
class Namespace
{
    static constexpr Name mask(const Name &name, uint8_t sig_bits)
    {
        return name & ~(~0x0_name >> sig_bits);
    }

  public:
    Namespace() = default;
    constexpr Namespace(const Namespace &ns) = default;
    Namespace(Namespace &&ns) = default;

    constexpr Namespace(Name name, uint8_t sig_bits) : _name{mask(name, sig_bits)}, _sig_bits{sig_bits}
    {
    }

    Namespace(std::string_view str)
    {
        auto delim_pos = str.find_first_of('/');
        _name = str.substr(0, delim_pos);
        str.remove_prefix(delim_pos + 1);
        // TODO: Would be nice if constexpr
        _sig_bits = std::atoi(str.data());
    }

    constexpr Namespace &operator=(const Namespace &other) = default;
    Namespace &operator=(Namespace &&other) = default;

    constexpr bool contains(const Name &name) const
    {
        return mask(name, _sig_bits) == _name;
    }

    constexpr bool contains(const Namespace &prefix) const
    {
        return contains(prefix._name);
    }

    constexpr Name name() const
    {
        return _name;
    }

    constexpr uint8_t length() const
    {
        return _sig_bits;
    }

    std::string to_hex() const
    {
        return _name.to_hex() + "/" + std::to_string(_sig_bits);
    }

    friend constexpr bool operator==(const Namespace &a, const Namespace &b)
    {
        return a._name == b._name && a._sig_bits == b._sig_bits;
    }

    friend constexpr bool operator!=(const Namespace &a, const Namespace &b)
    {
        return !(a == b);
    }

    friend constexpr bool operator>(const Namespace &a, const Namespace &b)
    {
        return a._name > b._name;
    }

    friend constexpr bool operator>(const Namespace &a, const Name &b)
    {
        return a._name > quicr::Namespace{b, a._sig_bits};
    }

    friend constexpr bool operator>(const Name &a, const Namespace &b)
    {
        return quicr::Namespace{a, b._sig_bits} > b._name;
    }

    friend constexpr bool operator<(const Namespace &a, const Namespace &b)
    {
        return a._name < b._name;
    }

    friend constexpr bool operator<(const Namespace &a, const Name &b)
    {
        return a._name < quicr::Namespace{b, a._sig_bits};
    }

    friend constexpr bool operator<(const Name &a, const Namespace &b)
    {
        return quicr::Namespace{a, b._sig_bits} < b._name;
    }

    friend std::ostream &operator<<(std::ostream &os, const Namespace &ns)
    {
        return os << ns.to_hex();
    }

  private:
    Name _name;
    uint8_t _sig_bits;
};

template <template <typename> class Comp>
class NamespaceComparator : Comp<quicr::Namespace>
{
    using base = Comp<quicr::Namespace>;

  public:
    using is_transparent = std::true_type;

    using base::operator();

    constexpr bool operator()(const Namespace &ns, const Name &name) const
    {
        return base::operator()(ns, {name, ns.length()});
    }

    constexpr bool operator()(const Name &name, const Namespace &ns) const
    {
        return base::operator()({name, ns.length()}, ns);
    }
};

template <class T, template <typename> class Comparator = std::less,
          class Allocator = std::allocator<std::pair<const quicr::Namespace, T>>>
using namespace_map = std::map<quicr::Namespace, T, NamespaceComparator<Comparator>, Allocator>;
} // namespace quicr
