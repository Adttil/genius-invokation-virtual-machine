#include <algorithm>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <givm/utils/small_bitset.hpp>

namespace givm_test::utils::small_bitset
{
namespace
{
    constexpr auto inline_bits = []
    {
        givm::small_bitset<65> bits{ 128 };
        bits[0] = true;
        bits.set(63);
        bits[64] = bits[63];
        bits[127] = true;
        return bits;
    }();

    static_assert(inline_bits.size() == 128);
    static_assert(inline_bits[0] && inline_bits[63] && inline_bits[64] && inline_bits[127]);
    static_assert(!inline_bits[1] && !inline_bits[65]);
    static_assert(givm::small_bitset<0>::inline_capacity == 0);
    static_assert(givm::small_bitset<1>::inline_capacity == 64);
    static_assert(givm::small_bitset<64>::inline_capacity == 64);
    static_assert(givm::small_bitset<65>::inline_capacity == 128);
    static_assert(std::is_same_v<decltype(inline_bits[0]), bool>);
    static_assert(std::is_same_v<decltype(std::declval<givm::small_bitset<65>&>()[0]),
        givm::small_bitset<65>::reference>);
    static_assert(std::is_same_v<decltype(inline_bits.view()), givm::bitset_view>);
    static_assert(std::is_nothrow_move_constructible_v<givm::small_bitset<64>>);
    static_assert(std::is_nothrow_move_assignable_v<givm::small_bitset<64>>);
    static_assert(std::ranges::random_access_range<givm::bitset_view>);
    static_assert(std::ranges::sized_range<givm::bitset_view>);
    static_assert(!std::indirectly_writable<givm::bitset_view::iterator, bool>);

    static_assert([]
    {
        givm::small_bitset<0> allocated{ 65 };
        allocated.set(64);
        auto copied = allocated;
        auto moved = std::move(allocated);
        givm::small_bitset<0> assigned;
        assigned = std::move(moved);
        copied = assigned;
        return assigned[64] && copied[64] && allocated.empty() && moved.empty();
    }());

    static_assert([]
    {
        givm::small_bitset<64> local{ 64 }, heap{ 65 };
        heap[64] = true;
        local.swap(heap);
        if(local.size() != 65 || !local[64] || heap.size() != 64) return false;
        swap(local, heap);
        return heap[64] && local.size() == 64;
    }());
}

TEST_CASE("small_bitset initializes and resets across word and storage boundaries", "[utils][small_bitset]")
{
    const auto size = GENERATE(std::size_t{ 0 }, 1, 63, 64, 65, 127, 128, 129, 257);
    givm::small_bitset<65> bits{ size };
    REQUIRE(bits.size() == size);
    CHECK(bits.empty() == (size == 0));
    for(std::size_t index = 0; index < size; ++index)
    {
        CHECK_FALSE(bits[index]);
        bits.set(index, index % 3 == 0);
    }
    for(std::size_t index = 0; index < size; ++index) CHECK(bits[index] == (index % 3 == 0));
    if(size != 0)
    {
        bits.set(size - 1);
        CHECK(bits[size - 1]);
        bits.set(size - 1, false);
        CHECK_FALSE(bits[size - 1]);
    }
    bits.reset();
    CHECK(bits.size() == size);
    for(std::size_t index = 0; index < size; ++index) CHECK_FALSE(bits[index]);
}

TEST_CASE("bitset_view borrows a prefix independently of inline capacity", "[utils][small_bitset]")
{
    const bool allocate = GENERATE(false, true);
    givm::small_bitset<128> bits{ allocate ? 193u : 128u };
    const auto view = bits.view();
    const auto prefix = view.take(65);
    CHECK(view.size() == bits.size());
    CHECK(prefix.size() == 65);
    CHECK_FALSE(prefix.empty());
    CHECK(prefix.take(0).empty());
    CHECK(bits.take(65).size() == prefix.size());
    bits.set(63);
    bits.set(64);
    bits.set(65);
    CHECK(prefix[63]);
    CHECK(prefix[64]);
    CHECK(view[65]);
    bits.set(64, false);
    CHECK_FALSE(prefix[64]);
    bits.reset();
    CHECK_FALSE(prefix[63]);
    CHECK(view.take(view.size()).size() == view.size());

    const givm::bitset_view empty;
    CHECK(empty.empty());
    CHECK(empty.size() == 0);
    CHECK(empty.take(0).empty());
}

TEST_CASE("small_bitset bit references assign values without rebinding", "[utils][small_bitset]")
{
    const bool allocate = GENERATE(false, true);
    givm::small_bitset<128> bits{ allocate ? 193u : 128u };
    bits[0] = true;
    bits[63] = bits[0];
    bits[64] = false;
    auto bit = bits[63];
    const bool saved = bit;
    bit = bits[64];
    CHECK_FALSE(bits[63]);
    CHECK(saved);
    bit = true;
    CHECK(bits[63]);
    CHECK_FALSE(bits[64]);
    bits[64] = bit;
    CHECK(bits[64]);
    bits[64] = false;
    CHECK(bits[63]);
    const auto& read_only = bits;
    STATIC_REQUIRE(std::is_same_v<decltype(read_only[0]), bool>);
    CHECK(read_only[0]);
}

TEST_CASE("bitset_view iterators traverse and index words without retaining the view", "[utils][small_bitset]")
{
    const auto size = GENERATE(std::size_t{ 0 }, 63, 64, 65, 127, 128, 129, 193);
    givm::small_bitset<128> bits{ size };
    for(std::size_t index = 0; index < size; ++index) bits[index] = index % 3 == 0;
    const auto view = bits.view();
    CHECK(std::ranges::distance(view) == static_cast<std::ptrdiff_t>(size));
    CHECK(view.end() - view.begin() == static_cast<std::ptrdiff_t>(size));
    CHECK(view.begin() - view.end() == -static_cast<std::ptrdiff_t>(size));
    CHECK(view.begin() + static_cast<std::ptrdiff_t>(size) == view.end());
    std::size_t index = 0;
    for(bool value : view) CHECK(value == (index++ % 3 == 0));
    CHECK(index == size);
    const auto from_temporary = bits.take(size).begin();
    for(std::size_t offset = 0; offset < size; ++offset)
        CHECK(from_temporary[static_cast<std::ptrdiff_t>(offset)] == (offset % 3 == 0));
    if(size != 0)
    {
        auto last = view.end();
        CHECK(last-- == view.end());
        CHECK(*last == ((size - 1) % 3 == 0));
        CHECK(*std::prev(view.end()) == *last);
        CHECK(*(static_cast<std::ptrdiff_t>(size - 1) + view.begin()) == *last);
        last -= -1;
        CHECK(last == view.end());
        last += -1;
        CHECK(last == view.end() - 1);
        auto first = view.begin();
        CHECK(first++ == view.begin());
        CHECK(--first == view.begin());
        CHECK(view.begin() < view.end());
    }
    const auto prefix = bits.take(size / 2);
    CHECK(std::ranges::count(prefix, true) == static_cast<std::ptrdiff_t>((prefix.size() + 2) / 3));
    const givm::bitset_view empty;
    CHECK(empty.begin() == empty.end());
    CHECK(empty.end() - empty.begin() == 0);
}

TEST_CASE("small_bitset swap exchanges inline and allocated values without changing bits", "[utils][small_bitset]")
{
    const auto first_size = GENERATE(std::size_t{ 0 }, 64, 65, 193);
    const auto second_size = GENERATE(std::size_t{ 0 }, 64, 65, 193);
    givm::small_bitset<64> first{ first_size }, second{ second_size };
    if(first_size != 0) first[0] = true;
    if(second_size != 0) second[second_size - 1] = true;
    first.swap(second);
    CHECK(first.size() == second_size);
    CHECK(second.size() == first_size);
    for(std::size_t index = 0; index < second_size; ++index)
        CHECK(first[index] == (index == second_size - 1));
    for(std::size_t index = 0; index < first_size; ++index) CHECK(second[index] == (index == 0));
    using std::swap;
    swap(first, second);
    CHECK(first.size() == first_size);
    CHECK(second.size() == second_size);
    first.swap(first);
    CHECK(first.size() == first_size);
    if(first_size != 0) CHECK(first[0]);
}

TEST_CASE("small_bitset copies own their storage across assignment transitions", "[utils][small_bitset]")
{
    const auto source_size = GENERATE(std::size_t{ 0 }, 64, 65, 193);
    const auto target_size = GENERATE(std::size_t{ 0 }, 64, 65, 193);
    givm::small_bitset<64> retained;
    {
        givm::small_bitset<64> source{ source_size };
        if(source_size != 0) source.set(source_size - 1);
        auto copied = source;
        CHECK(copied.size() == source_size);
        if(source_size != 0)
        {
            copied.set(source_size - 1, false);
            CHECK(source[source_size - 1]);
        }
        givm::small_bitset<64> target{ target_size };
        if(target_size != 0) target.set(0);
        target = source;
        CHECK(target.size() == source_size);
        for(std::size_t index = 0; index < source_size; ++index)
            CHECK(target[index] == (index == source_size - 1));
        const auto& alias = target;
        target = alias;
        CHECK(target.size() == source_size);
        retained = target;
    }
    CHECK(retained.size() == source_size);
    if(source_size != 0) CHECK(retained[source_size - 1]);
}

TEST_CASE("small_bitset moves transfer storage and leave reusable empty sources", "[utils][small_bitset]")
{
    const auto source_size = GENERATE(std::size_t{ 0 }, 64, 65, 193);
    const auto target_size = GENERATE(std::size_t{ 0 }, 64, 65, 193);
    givm::small_bitset<64> source{ source_size };
    if(source_size != 0) source.set(source_size - 1);
    auto moved = std::move(source);
    CHECK(source.empty());
    CHECK(source.view().empty());
    source.reset();
    givm::small_bitset<64> target{ target_size };
    if(target_size != 0) target.set(0);
    target = std::move(moved);
    CHECK(moved.empty());
    CHECK(moved.take(0).empty());
    CHECK(target.size() == source_size);
    for(std::size_t index = 0; index < source_size; ++index)
        CHECK(target[index] == (index == source_size - 1));
    auto& alias = target;
    target = std::move(alias);
    CHECK(target.size() == source_size);
    if(source_size != 0) CHECK(target[source_size - 1]);
    source = givm::small_bitset<64>{ 129 };
    source.set(128);
    CHECK(source[128]);
    moved = givm::small_bitset<64>{ 64 };
    moved.set(63);
    CHECK(moved[63]);
}

TEST_CASE("small_bitset permits zero inline capacity", "[utils][small_bitset]")
{
    givm::small_bitset<0> bits;
    CHECK(bits.empty());
    CHECK(bits.view().empty());
    bits.reset();
    bits = givm::small_bitset<0>{ 1 };
    CHECK_FALSE(bits[0]);
    bits.set(0);
    CHECK(bits[0]);
    auto copy = bits;
    bits.reset();
    CHECK(copy[0]);
    CHECK_FALSE(bits[0]);
}
}
