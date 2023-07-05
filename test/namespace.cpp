#include <doctest/doctest.h>

#include <quicr/namespace.h>

#include <type_traits>

TEST_CASE("quicr::Namespace Type Tests")
{
    CHECK(std::is_trivial_v<quicr::Namespace>);
    CHECK(std::is_trivially_constructible_v<quicr::Namespace>);
    CHECK(std::is_trivially_default_constructible_v<quicr::Namespace>);
    CHECK(std::is_trivially_destructible_v<quicr::Namespace>);
    CHECK(std::is_trivially_copyable_v<quicr::Namespace>);
    CHECK(std::is_trivially_copy_assignable_v<quicr::Namespace>);
    CHECK(std::is_trivially_move_constructible_v<quicr::Namespace>);
    CHECK(std::is_trivially_move_assignable_v<quicr::Namespace>);
}

TEST_CASE("quicr::Namespace Constructor Tests")
{
    CHECK_NOTHROW(quicr::Namespace(0x11111111111111112222222222222200_name, 120));
    CHECK_NOTHROW(quicr::Namespace(std::string_view("0x11111111111111112222222222222200/120")));

    quicr::Namespace name_ns(0x11111111111111112222222222222200_name, 120);
    quicr::Namespace str_ns(std::string_view("0x11111111111111112222222222222200/120"));

    CHECK_EQ(name_ns, str_ns);
}

TEST_CASE("quicr::Namespace Contains Names Test")
{
    quicr::Namespace base_namespace(0x11111111111111112222222222222200_name, 120);

    quicr::Name valid_name = 0x111111111111111122222222222222FF_name;
    CHECK(base_namespace.contains(valid_name));

    quicr::Name another_valid_name = 0x11111111111111112222222222222211_name;
    CHECK(base_namespace.contains(another_valid_name));

    quicr::Name invalid_name = 0x11111111111111112222222222222300_name;
    CHECK_FALSE(base_namespace.contains(invalid_name));
}

TEST_CASE("quicr::Namespace Contains Namespaces Test")
{
    quicr::Namespace base_namespace(0x11111111111111112222222222220000_name, 112);

    quicr::Namespace valid_namespace(0x11111111111111112222222222222200_name, 120);
    CHECK(base_namespace.contains(valid_namespace));

    quicr::Namespace invalid_namespace(0x11111111111111112222222222000000_name, 104);
    CHECK_FALSE(base_namespace.contains(invalid_namespace));
}

TEST_CASE("quicr::Namespace String Constructor Test")
{
    quicr::Namespace ns = std::string_view("0xA11CEE00000001010007000000000001/80");
    CHECK_EQ(std::string(ns), "0xA11CEE00000001010007000000000000/80");
    CHECK_EQ(ns.name(), 0xA11CEE00000001010007000000000000_name);
    CHECK_EQ(ns.length(), 80);
}

TEST_CASE("quicr::Namespace Map Sorting Test")
{
    quicr::Name name = 0xABCDEFFFFFFFFFFFFFFFFFFFFFFFFFFF_name;
    quicr::Namespace ns(name, 24);
    quicr::Namespace ns2(name, 16);
    {
        quicr::namespace_map<int> ns_map{ { ns, 101 }, { ns2, 102 } };

        CHECK_EQ(ns_map.count(ns), 1);
        CHECK_EQ(ns_map.count(ns2), 1);
        CHECK_EQ(ns_map.size(), 2);
        CHECK_EQ(ns_map.count(name), 2);
        CHECK_EQ(ns_map.find(ns)->second, 101);
        CHECK_EQ(ns_map.find(ns2)->second, 102);
        CHECK_EQ(ns_map.find(name)->second, 102);
    }
    {
        quicr::namespace_map<int, std::greater> ns_map{ { ns, 101 }, { ns2, 102 } };

        CHECK_EQ(ns_map.count(ns), 1);
        CHECK_EQ(ns_map.count(ns2), 1);
        CHECK_EQ(ns_map.size(), 2);
        CHECK_EQ(ns_map.count(name), 2);
        CHECK_EQ(ns_map.find(ns)->second, 101);
        CHECK_EQ(ns_map.find(ns2)->second, 102);
        CHECK_EQ(ns_map.find(name)->second, 101);
    }
}
