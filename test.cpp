#include "any.h"

#include <string>
#include <vector>
#include <typeinfo>
#include <ranges>
#include <utility>
#include <iostream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>

using Catch::Matchers::Equals;

namespace {
    using utils::any;

    template<class T>
    void require_eq(const any &any, const T &value) {
        REQUIRE_FALSE(any.empty());
        REQUIRE(any.getValue<T>() == value);
    }

    template<class T>
    void require_eq(const any &lhs, const any &rhs) {
        REQUIRE_FALSE(lhs.empty());
        REQUIRE_FALSE(rhs.empty());
        REQUIRE(lhs.getValue<T>() == rhs.getValue<T>());
    }

}  // namespace

using utils::any_cast;
using utils::bad_any_cast;

using std::string;
using std::cout;
using std::cerr;
using std::endl;

void contruct_test() {
    any def;
    any copy_on_type(42);
    def = 3.14;
    def = string("2.71");
    any def_copy(def);
    def = copy_on_type;
    any e;
    assert(e.empty());
}

template<class T>
void check_cast(any &a, bool should_throw) {
    bool thrown = false;
    try {
        auto res = any_cast<T>(a);
        std::cout << res;
    } catch (bad_any_cast const &err) {
        thrown = true;
        std::cerr << err.what() << std::endl;
    }
    assert(should_throw == thrown);
}

void retrieve_value_test() {
    any ia(42);
    auto res = any_cast<double>(&ia);
    assert(res == nullptr);
    check_cast<double>(ia, true);
    check_cast<int>(ia, false);
}

void swap_test(any &a, any &b) {
    swap(a, b);
}

TEST_CASE("Given") {
    contruct_test();
    retrieve_value_test();
    any a(5), b(string("6"));
    swap_test(a, b);
}

TEST_CASE("Simple") {
    any a = 5;
    require_eq(a, 5);

    any b = std::string{"abacaba"};
    require_eq(b, std::string{"abacaba"});

    any c;
    REQUIRE(c.empty());
    c = 7.;
    require_eq(c, 7.);

    any d;
    int *ptr = nullptr;
    d = ptr;
    require_eq(d, ptr);
}

TEST_CASE("empty") {
    any a;
    REQUIRE(a.empty());

    std::vector t = {1, 2};
    any b = t;
    require_eq(b, t);
    a.swap(b);

    require_eq(a, t);
    REQUIRE(b.empty());

    a.clear();
    REQUIRE(a.empty());

    a.swap(b);
    REQUIRE(a.empty());
    REQUIRE(b.empty());

    any c;
    auto d = c;
    REQUIRE(d.empty());

    a = 5;
    require_eq(a, 5);
    a = c;
    REQUIRE(a.empty());

    any e = nullptr;
    require_eq(e, nullptr);
    REQUIRE_THROWS_AS(e.getValue<void *>(), std::bad_cast);
}

TEST_CASE("Copy") {
    struct SomeStruct {
        int x;

        bool operator==(const SomeStruct &) const = default;
    };

    any a = 5;
    auto b = a;

    require_eq<int>(a, b);
    REQUIRE_FALSE(&a.getValue<int>() == &b.getValue<int>());

    any c;
    c = b;

    require_eq<int>(b, c);
    REQUIRE_FALSE(&b.getValue<int>() == &c.getValue<int>());
    b.clear();
    REQUIRE(b.empty());
    require_eq(c, 5);

    any d = SomeStruct{3};
    require_eq(d, SomeStruct{3});

    d = std::string{"check"};
    require_eq(d, std::string{"check"});

    const auto &str = std::string{"dorou"};
    any e = str;
    const auto &r = e;
    e = r;
    require_eq(e, str);

    a.swap(e);
    require_eq(a, str);
    require_eq(e, 5);
}

TEST_CASE("Any can change type") {
    any a = 5;
    require_eq(a, 5);

    a = 2.3;
    require_eq(a, 2.3);

    a = {};
    REQUIRE(a.empty());

    a = std::vector{1, 2, 3};
    require_eq(a, std::vector{1, 2, 3});
}

TEST_CASE("Any throws") {
    any a = std::string{"just a test"};
    REQUIRE_THROWS_AS(a.getValue<int>(), std::bad_cast);

    any b = std::vector{1, 2, 3};
    require_eq(b, std::vector{1, 2, 3});
    REQUIRE_THROWS_AS(b.getValue<std::string>(), std::bad_cast);
    REQUIRE_THROWS_AS(b.getValue<std::vector<double>>(), std::bad_cast);

    any c;
    REQUIRE_THROWS_AS(c.getValue<double>(), std::bad_cast);
    c = &c;
    require_eq(c, &c);

    any d = 2;
    REQUIRE_THROWS_AS(d.getValue<double>(), std::bad_cast);

    d = static_cast<int *>(nullptr);
    require_eq<int *>(d, nullptr);
    REQUIRE_THROWS_AS(d.getValue<double *>(), std::bad_cast);
    REQUIRE_THROWS_AS(d.getValue<void *>(), std::bad_cast);
    REQUIRE_THROWS_AS(d.getValue<unsigned *>(), std::bad_cast);
}

TEST_CASE("Move") {
    any a = 2.2;
    auto b = std::move(a);
    REQUIRE(a.empty());
    require_eq(b, 2.2);

    any c;
    c = std::move(b);
    REQUIRE(b.empty());
    require_eq(c, 2.2);

    std::vector v = {5, 1, -34};
    auto expected = v;
    a = v;
    REQUIRE_THAT(v, Equals(expected));

    const auto *p = &v[1];
    any d = std::move(v);
    require_eq(d, expected);
    REQUIRE(v.empty());
    REQUIRE(&d.getValue<std::vector<int>>()[1] == p);

    v = {1, 4, 1, 2};
    p = &v[3];
    d.clear();
    REQUIRE(d.empty());
    d = std::move(v);
    REQUIRE(v.empty());
    REQUIRE(&d.getValue<std::vector<int>>()[3] == p);

    any e = 5;
    e.swap(e);
    require_eq(e, 5);
}

TEST_CASE("Fast swap") {
    std::string str1(1'000'000, 'a');
    std::string str2(500'000, 'b');
    const auto &expected1 = str1;
    auto expected2 = str2;
    const auto *p = &str2[0];

    any a = std::as_const(str1);
    any b = std::move(str2);
    for (auto i = 0; i < 1'000'001; ++i) {
        std::swap(a, b);
        a.swap(b);
        b.swap(a);
    }
    require_eq(a, expected2);
    require_eq(b, expected1);
    REQUIRE(&a.getValue<std::string>()[0] == p);
}

TEST_CASE("Vector of Any") {
    constexpr auto kRange = std::views::iota(0, 10'000);

    std::vector<any> a;
    for (auto i: kRange) {
        a.emplace_back(i);
        a.emplace_back(a.back());
        a.emplace_back(3.14);
        a.emplace_back(true);
    }

    auto b = a;
    for (auto i: kRange) {
        require_eq(b[4 * i], i);
        require_eq(b[4 * i + 1], i);
        require_eq(b[4 * i + 2], 3.14);
        require_eq(b[4 * i + 3], true);
    }
}

TEST_CASE("Any inside Any") {
    struct X {
        any x;
    };

    constexpr auto kDepth = 1'000;

    any a = 5;
    for (auto i = 0; i < kDepth; ++i) {
        a = X{std::move(a)};
    }
    auto b = a;

    for (auto i = 0; i < kDepth; ++i) {
        a = a.getValue<X>().x;
    }
    require_eq(a, 5);

    for (auto i = 0; i < kDepth; ++i) {
        b = std::move(b.getValue<X>().x);
    }
    require_eq(b, 5);
}

TEST_CASE("Self-assignment") {
    constexpr auto kSize = 1'000'000;

    any a = std::vector<int>(kSize);
    auto &r = a;
    for (auto i = 0; i < 100'000; ++i) {
        a = r;
    }
    REQUIRE(a.getValue<std::vector<int>>().size() == kSize);

    any b;
    auto &rb = b;
    b = rb;
    REQUIRE(b.empty());
}

TEST_CASE("No default constructor") {
    struct NoDefault {
        NoDefault(int) {
        }
    };

    any a = NoDefault{3};
    REQUIRE_NOTHROW(a.getValue<NoDefault>());
    a = NoDefault{4};
    REQUIRE_FALSE(a.empty());

    a.clear();
    REQUIRE(a.empty());
    REQUIRE_THROWS_AS(a.getValue<NoDefault>(), std::bad_cast);

    any b = NoDefault{6};
    a = b;
    REQUIRE_NOTHROW(a.getValue<NoDefault>());
}
