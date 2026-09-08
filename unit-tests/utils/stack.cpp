#include <algorithm>
#include <array>
#include <cstdint>
#include <list>
#include <ranges>
#include <span>
#include <type_traits>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include <givm/utils/stack.hpp>

using namespace givm;

namespace
{
    struct alignas(max_alignment) aligned_record
    {
        std::uint64_t first;
        std::uint32_t second;

        constexpr bool operator==(const aligned_record&) const = default;
    };

    static_assert(std::is_trivially_copyable_v<aligned_record>);

    template<class... T>
    concept stack_pushable = requires(frame_stack& stack, const T&... values)
    {
        stack.push(values...);
    };
}

TEST_CASE("frame_stack accepts dynamic arrays only as a frame prefix", "[stack]")
{
    using dynamic_values = decltype(dynamic_array<std::uint16_t>(stack_count_t{ 1 }));

    STATIC_REQUIRE(stack_pushable<dynamic_values, std::uint8_t>);
    STATIC_REQUIRE_FALSE(stack_pushable<std::uint8_t, dynamic_values>);
}

TEST_CASE("frame_stack manages empty and cleared storage", "[stack]")
{
    frame_stack stack;
    CHECK(stack.empty());
    CHECK(stack.size() == 0);
    CHECK(stack.capacity() == 0);

    stack.reserve(64);
    CHECK(stack.empty());
    CHECK(stack.capacity() >= 64);
    CHECK(stack.data() != nullptr);

    auto blank = stack.push<std::uint8_t, aligned_record>();
    get<0>(blank) = 7;
    get<1>(blank) = aligned_record{ 11, 13 };

    auto&& [byte, record] = stack.top<std::uint8_t, aligned_record>();
    CHECK(byte == 7);
    CHECK(record == aligned_record{ 11, 13 });

    const auto retained_capacity = stack.capacity();
    stack.clear();
    CHECK(stack.empty());
    CHECK(stack.size() == 0);
    CHECK(stack.capacity() == retained_capacity);

    auto&& [replacement] = stack.push(std::uint16_t{ 17 });
    CHECK(replacement == 17);
}

TEST_CASE("frame_stack stores and mutates a fixed frame", "[stack]")
{
    frame_stack stack{ 1 };

    auto&& [integer, real] = stack.push(std::uint32_t{ 7 }, 2.5);
    CHECK(integer == 7);
    CHECK(real == 2.5);
    CHECK_FALSE(stack.empty());
    CHECK(stack.capacity() >= stack.size());

    integer = 11;
    real = 4.5;

    auto&& [stored_integer, stored_real] = stack.top<std::uint32_t, double>();
    CHECK(stored_integer == 11);
    CHECK(stored_real == 4.5);

    const auto& const_stack = stack;
    auto&& [const_integer, const_real] = const_stack.top<std::uint32_t, double>();
    STATIC_REQUIRE(std::is_same_v<decltype(const_integer), const std::uint32_t&>);
    STATIC_REQUIRE(std::is_same_v<decltype(const_real), const double&>);

    stack.pop<std::uint32_t, double>();
    CHECK(stack.empty());
}

TEST_CASE("frame_stack preserves dynamic arrays while growing", "[stack]")
{
    const std::array<std::uint32_t, 3> initial{ 3, 5, 8 };
    frame_stack stack{ 1 };

    stack.push(dynamic_array<std::uint32_t>(initial), std::uint16_t{ 13 });
    const auto size_before_reserve = stack.size();
    stack.reserve(stack.capacity() + 64);

    auto&& [values, marker] = stack.top<std::uint32_t[], std::uint16_t>();
    REQUIRE(values.size() == initial.size());
    CHECK(std::ranges::equal(values, initial));
    CHECK(marker == 13);
    CHECK(stack.size() == size_before_reserve);

    values[1] = 21;
    auto&& [updated_values, updated_marker] = stack.top<std::uint32_t[], std::uint16_t>();
    CHECK(updated_values[1] == 21);
    CHECK(updated_marker == 13);
}

TEST_CASE("frame_stack preserves earlier frames during automatic growth", "[stack]")
{
    frame_stack stack{ 1 };

    for(std::uint32_t i = 0; i < 64; ++i)
    {
        stack.push(
            static_cast<std::uint8_t>(i),
            aligned_record{ i * 2, i * 3 }
        );
    }

    CHECK(stack.capacity() >= stack.size());
    for(std::uint32_t i = 64; i > 0; --i)
    {
        const auto expected = i - 1;
        auto&& [index, record] = stack.top<std::uint8_t, aligned_record>();
        CHECK(index == expected);
        CHECK(record == aligned_record{ expected * 2, expected * 3 });
        stack.pop<std::uint8_t, aligned_record>();
    }
    CHECK(stack.empty());
}

TEST_CASE("frame_stack initializes dynamic arrays from their public source forms", "[stack]")
{
    SECTION("a count creates writable storage")
    {
        frame_stack stack;
        auto pushed = stack.push(dynamic_array<std::uint32_t>(4u), std::uint16_t{ 19 });
        auto values = get<0>(pushed);
        REQUIRE(values.size() == 4);

        const std::array<std::uint32_t, 4> expected{ 2, 3, 5, 7 };
        std::ranges::copy(expected, values.begin());

        auto&& [stored_values, marker] = stack.top<std::uint32_t[], std::uint16_t>();
        CHECK(std::ranges::equal(stored_values, expected));
        CHECK(marker == 19);
    }

    SECTION("multiple array prefixes preserve their element alignment")
    {
        const std::list<std::uint16_t> linked_values{ 3, 5, 8 };
        const std::array<aligned_record, 2> aligned_values{
            aligned_record{ 13, 21 },
            aligned_record{ 34, 55 }
        };
        frame_stack stack{ 1 };

        stack.push(
            dynamic_array<std::uint16_t>(linked_values),
            dynamic_array<aligned_record>(aligned_values),
            std::uint8_t{ 2 },
            std::uint32_t{ 89 }
        );

        const auto& const_stack = stack;
        auto const_view = const_stack.top<
            std::uint16_t[],
            aligned_record[],
            std::uint8_t,
            std::uint32_t
        >();
        auto&& [stored_linked, stored_aligned, tag, marker] = const_view;
        STATIC_REQUIRE(std::is_same_v<decltype(stored_linked), std::span<const std::uint16_t>>);
        STATIC_REQUIRE(std::is_same_v<decltype(stored_aligned), std::span<const aligned_record>>);
        STATIC_REQUIRE(std::is_same_v<decltype(tag), const std::uint8_t&>);
        STATIC_REQUIRE(std::is_same_v<decltype(marker), const std::uint32_t&>);
        CHECK(tag == 2);
        CHECK(std::ranges::equal(stored_linked, linked_values));
        CHECK(marker == 89);
        CHECK(std::ranges::equal(stored_aligned, aligned_values));

        stack.pop<std::uint16_t[], aligned_record[], std::uint8_t, std::uint32_t>();
        CHECK(stack.empty());
    }

    SECTION("an empty source creates an empty array")
    {
        const std::array<std::uint8_t, 0> empty_values{};
        frame_stack stack;
        stack.push(dynamic_array<std::uint8_t>(empty_values), std::uint16_t{ 23 });

        auto&& [stored_values, marker] = stack.top<std::uint8_t[], std::uint16_t>();
        CHECK(marker == 23);
        CHECK(stored_values.empty());

        stack.pop<std::uint8_t[], std::uint16_t>();
        CHECK(stack.empty());
    }
}

TEST_CASE("frame_stack exposes the fixed suffix of its top frame", "[stack]")
{
    const std::array<std::uint16_t, 3> initial{ 3, 5, 8 };
    frame_stack stack;
    stack.push(
        dynamic_array<std::uint16_t>(initial),
        std::uint32_t{ 13 },
        std::uint8_t{ 21 }
    );

    auto&& [input] = stack.top<std::uint8_t>();
    input = 34;

    auto&& [header, same_input] = stack.top<std::uint32_t, std::uint8_t>();
    CHECK(header == 13);
    CHECK(same_input == 34);

    const auto whole_frame = stack.top<std::uint16_t[], std::uint32_t, std::uint8_t>();
    auto&& [values, same_header, updated_input] = whole_frame;
    CHECK(std::ranges::equal(values, initial));
    CHECK(same_header == 13);
    CHECK(updated_input == 34);

    stack.pop(whole_frame);
    CHECK(stack.empty());
}

TEST_CASE("frame_stack copies frames independently", "[stack]")
{
    const std::array<std::uint32_t, 3> initial{ 3, 5, 8 };
    frame_stack original;
    original.push(std::uint16_t{ 2 });
    original.push(dynamic_array<std::uint32_t>(initial), aligned_record{ 13, 21 });

    frame_stack copy = original;
    auto&& [copy_values, copy_record] = copy.top<std::uint32_t[], aligned_record>();
    copy_values[0] = 34;
    copy_record.second = 55;

    auto&& [original_values, original_record] = original.top<std::uint32_t[], aligned_record>();
    CHECK(std::ranges::equal(original_values, initial));
    CHECK(original_record == aligned_record{ 13, 21 });
    CHECK(copy_values[0] == 34);
    CHECK(copy_record == aligned_record{ 13, 55 });

    frame_stack assigned;
    assigned.push(std::uint8_t{ 1 });
    assigned = original;
    original.clear();

    auto&& [assigned_values, assigned_record] = assigned.top<std::uint32_t[], aligned_record>();
    CHECK(std::ranges::equal(assigned_values, initial));
    CHECK(assigned_record == aligned_record{ 13, 21 });
    assigned.pop<std::uint32_t[], aligned_record>();
    auto&& [assigned_base] = assigned.top<std::uint16_t>();
    CHECK(assigned_base == 2);
}

TEST_CASE("frame_stack supports move and swap through observable contents", "[stack]")
{
    frame_stack source;
    source.push(std::uint32_t{ 17 });

    frame_stack moved = std::move(source);
    auto&& [moved_value] = moved.top<std::uint32_t>();
    CHECK(moved_value == 17);

    source.push(std::uint16_t{ 23 });
    auto&& [reused_source_value] = source.top<std::uint16_t>();
    CHECK(reused_source_value == 23);

    frame_stack other;
    other.push(std::uint32_t{ 29 });
    swap(moved, other);
    auto&& [left_value] = moved.top<std::uint32_t>();
    auto&& [right_value] = other.top<std::uint32_t>();
    CHECK(left_value == 29);
    CHECK(right_value == 17);

    frame_stack move_assigned;
    move_assigned = std::move(other);
    auto&& [assigned_value] = move_assigned.top<std::uint32_t>();
    CHECK(assigned_value == 17);

    other.push(std::uint8_t{ 31 });
    auto&& [reused_other_value] = other.top<std::uint8_t>();
    CHECK(reused_other_value == 31);
}

TEST_CASE("frame_stack addresses and removes multiple frames together", "[stack]")
{
    frame_stack stack;
    stack.push(std::uint32_t{ 1 });
    stack.push(std::uint16_t{ 2 }, std::uint8_t{ 3 });

    auto [first, second] = stack.top<
        frame_t<std::uint32_t>{},
        frame_t<std::uint16_t, std::uint8_t>{}
    >();
    auto&& [first_value] = first;
    auto&& [second_value, third_value] = second;
    CHECK(first_value == 1);
    CHECK(second_value == 2);
    CHECK(third_value == 3);

    frame_stack copy = stack;
    stack.pop(first, second);
    CHECK(stack.empty());

    copy.pop<
        frame_t<std::uint32_t>{},
        frame_t<std::uint16_t, std::uint8_t>{}
    >();
    CHECK(copy.empty());

    frame_stack single_pop;
    single_pop.push(std::uint32_t{ 5 });
    single_pop.push(std::uint16_t{ 8 }, std::uint8_t{ 13 });
    single_pop.pop<std::uint16_t, std::uint8_t>();
    auto&& [remaining] = single_pop.top<std::uint32_t>();
    CHECK(remaining == 5);
}
