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

namespace
{
    struct alignas(givm::max_alignment) aligned_record
    {
        std::uint64_t first;
        std::uint32_t second;

        constexpr bool operator==(const aligned_record&) const = default;
    };

    static_assert(std::is_trivially_copyable_v<aligned_record>);

    template<class... T>
    concept stack_pushable = requires(givm::frame_stack& stack, const T&... values)
    {
        stack.push(values...);
    };

    template<class T>
    concept can_push_stack_frame = requires(T& stack)
    {
        stack.push(std::uint32_t{ 1 });
    };

    template<class T>
    concept can_pop_stack_frame = requires(T& stack)
    {
        stack.template pop<std::uint32_t>();
    };

    template<class T, class... Values>
    concept accepts_stack_values = requires(T& stack, const Values&... values)
    {
        stack.push(values...);
    };

    template<class T, class... Fields>
    concept accepts_blank_stack_frame = requires(T& stack)
    {
        stack.template push<Fields...>();
    };
}

TEST_CASE("frame_stack accepts dynamic arrays only as a frame prefix", "[stack]")
{
    using dynamic_values = decltype(givm::dynamic_array<std::uint16_t>(givm::stack_count_t{ 1 }));

    STATIC_REQUIRE(stack_pushable<dynamic_values, std::uint8_t>);
    STATIC_REQUIRE_FALSE(stack_pushable<std::uint8_t, dynamic_values>);
}

TEST_CASE("frame_stack manages empty and cleared storage", "[stack]")
{
    givm::frame_stack stack;
    CHECK(stack.empty());
    CHECK(stack.size() == 0);

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
    givm::frame_stack stack{ 1 };

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
    givm::frame_stack stack{ 1 };

    stack.push(givm::dynamic_array<std::uint32_t>(initial), std::uint16_t{ 13 });
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
    givm::frame_stack stack{ 1 };

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
        givm::frame_stack stack;
        auto pushed = stack.push(givm::dynamic_array<std::uint32_t>(4u), std::uint16_t{ 19 });
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
        givm::frame_stack stack{ 1 };

        stack.push(
            givm::dynamic_array<std::uint16_t>(linked_values),
            givm::dynamic_array<aligned_record>(aligned_values),
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
        givm::frame_stack stack;
        stack.push(givm::dynamic_array<std::uint8_t>(empty_values), std::uint16_t{ 23 });

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
    givm::frame_stack stack;
    stack.push(
        givm::dynamic_array<std::uint16_t>(initial),
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
    givm::frame_stack original;
    original.push(std::uint16_t{ 2 });
    original.push(givm::dynamic_array<std::uint32_t>(initial), aligned_record{ 13, 21 });

    givm::frame_stack copy = original;
    auto&& [copy_values, copy_record] = copy.top<std::uint32_t[], aligned_record>();
    copy_values[0] = 34;
    copy_record.second = 55;

    auto&& [original_values, original_record] = original.top<std::uint32_t[], aligned_record>();
    CHECK(std::ranges::equal(original_values, initial));
    CHECK(original_record == aligned_record{ 13, 21 });
    CHECK(copy_values[0] == 34);
    CHECK(copy_record == aligned_record{ 13, 55 });

    givm::frame_stack assigned;
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
    givm::frame_stack source;
    source.push(std::uint32_t{ 17 });

    givm::frame_stack moved = std::move(source);
    auto&& [moved_value] = moved.top<std::uint32_t>();
    CHECK(moved_value == 17);

    source.push(std::uint16_t{ 23 });
    auto&& [reused_source_value] = source.top<std::uint16_t>();
    CHECK(reused_source_value == 23);

    givm::frame_stack other;
    other.push(std::uint32_t{ 29 });
    swap(moved, other);
    auto&& [left_value] = moved.top<std::uint32_t>();
    auto&& [right_value] = other.top<std::uint32_t>();
    CHECK(left_value == 29);
    CHECK(right_value == 17);

    givm::frame_stack move_assigned;
    move_assigned = std::move(other);
    auto&& [assigned_value] = move_assigned.top<std::uint32_t>();
    CHECK(assigned_value == 17);

    other.push(std::uint8_t{ 31 });
    auto&& [reused_other_value] = other.top<std::uint8_t>();
    CHECK(reused_other_value == 31);
}

TEST_CASE("frame_stack addresses and removes multiple frames together", "[stack]")
{
    givm::frame_stack stack;
    stack.push(std::uint32_t{ 1 });
    stack.push(std::uint16_t{ 2 }, std::uint8_t{ 3 });

    auto [first, second] = stack.top<
        givm::frame_t<std::uint32_t>{},
        givm::frame_t<std::uint16_t, std::uint8_t>{}
    >();
    auto&& [first_value] = first;
    auto&& [second_value, third_value] = second;
    CHECK(first_value == 1);
    CHECK(second_value == 2);
    CHECK(third_value == 3);

    givm::frame_stack copy = stack;
    stack.pop(first, second);
    CHECK(stack.empty());

    copy.pop<
        givm::frame_t<std::uint32_t>{},
        givm::frame_t<std::uint16_t, std::uint8_t>{}
    >();
    CHECK(copy.empty());

    givm::frame_stack single_pop;
    single_pop.push(std::uint32_t{ 5 });
    single_pop.push(std::uint16_t{ 8 }, std::uint8_t{ 13 });
    single_pop.pop<std::uint16_t, std::uint8_t>();
    auto&& [remaining] = single_pop.top<std::uint32_t>();
    CHECK(remaining == 5);
}

TEST_CASE("frame_stack accepts a substack only at the end of a frame", "[stack][substack]")
{
    using child_initializer = decltype(givm::substack());
    using dynamic_values = decltype(givm::dynamic_array<std::uint16_t>(givm::stack_count_t{ 1 }));
    using marker_array = decltype(givm::dynamic_array<givm::substack_t>(1u));

    STATIC_REQUIRE(stack_pushable<child_initializer>);
    STATIC_REQUIRE(stack_pushable<dynamic_values, std::uint32_t, child_initializer>);
    STATIC_REQUIRE_FALSE(stack_pushable<child_initializer, std::uint32_t>);
    STATIC_REQUIRE_FALSE(stack_pushable<child_initializer, child_initializer>);
    STATIC_REQUIRE_FALSE(stack_pushable<std::uint32_t, dynamic_values, child_initializer>);
    STATIC_REQUIRE_FALSE(stack_pushable<marker_array>);
}

TEST_CASE("an emptied substack preserves its containing frame", "[stack][substack]")
{
    givm::frame_stack stack;
    auto containing = stack.push(std::uint16_t{ 7 }, givm::substack());
    auto child = get<1>(containing);
    CHECK(child.empty());
    CHECK(child.size() == 0);
    CHECK_FALSE(stack.empty());
    const auto empty_parent_size = stack.size();

    child.push(std::uint32_t{ 11 });
    CHECK_FALSE(child.empty());
    CHECK(child.size() > 0);
    CHECK(get<0>(child.top<std::uint32_t>()) == 11);
    child.pop<std::uint32_t>();

    CHECK(child.empty());
    CHECK(child.size() == 0);
    CHECK(stack.size() == empty_parent_size);
    CHECK(get<0>(containing) == 7);
    CHECK(get<1>(containing).empty());

    auto next = child.push(std::uint8_t{ 13 }, std::uint32_t{ 17 });
    CHECK(get<0>(next) == 13);
    CHECK(get<1>(next) == 17);
    child.pop(next);
    CHECK(child.empty());
    CHECK(get<0>(containing) == 7);

    stack.pop<std::uint16_t, givm::substack_t>();
    CHECK(stack.empty());
}

TEST_CASE("blank frames initialize their trailing substacks", "[stack][substack]")
{
    givm::frame_stack stack;
    auto containing = stack.push<std::uint32_t, givm::substack_t>();
    get<0>(containing) = 7;
    auto child = get<1>(containing);
    CHECK(child.empty());
    auto blank = child.push<std::uint16_t, std::uint32_t>();
    get<0>(blank) = 11;
    get<1>(blank) = 13;
    CHECK(get<0>(containing) == 7);
    CHECK(get<0>(blank) == 11);
    CHECK(get<1>(blank) == 13);
    child.pop<std::uint16_t, std::uint32_t>();
    CHECK(child.empty());

    using child_initializer = decltype(givm::substack());
    STATIC_REQUIRE_FALSE(accepts_stack_values<decltype(child), child_initializer>);
    STATIC_REQUIRE_FALSE(accepts_stack_values<decltype(child), std::uint32_t, child_initializer>);
    STATIC_REQUIRE_FALSE(accepts_blank_stack_frame<decltype(child), givm::substack_t>);
    STATIC_REQUIRE_FALSE(accepts_blank_stack_frame<decltype(child), std::uint32_t, givm::substack_t>);
    stack.pop(containing);
    CHECK(stack.empty());
}

TEST_CASE("substack growth preserves its frame arrays and fixed suffix", "[stack][substack]")
{
    const std::array<std::uint16_t, 3> numbers{ 3, 5, 8 };
    const std::array<aligned_record, 2> records{
        aligned_record{ 13, 21 }, aligned_record{ 34, 55 }
    };
    givm::frame_stack stack{ 1 };
    auto containing = stack.push(
        givm::dynamic_array<std::uint16_t>(numbers),
        givm::dynamic_array<aligned_record>(records),
        std::uint8_t{ 89 },
        givm::substack()
    );
    auto suffix = stack.top<std::uint8_t, givm::substack_t>();
    auto child = get<1>(suffix);
    const auto old_capacity = stack.capacity();
    const std::array<std::uint32_t, 1024> large_values{};

    child.push(givm::dynamic_array<std::uint32_t>(large_values), std::uint32_t{ 144 });
    REQUIRE(stack.capacity() > old_capacity);
    CHECK(std::ranges::equal(get<0>(containing), numbers));
    CHECK(std::ranges::equal(get<1>(containing), records));
    CHECK(get<2>(containing) == 89);
    CHECK(get<0>(suffix) == 89);

    get<0>(containing)[1] = 233;
    get<1>(containing)[0].second = 377;
    get<0>(suffix) = 144;
    auto child_frame = get<3>(containing).top<std::uint32_t[], std::uint32_t>();
    REQUIRE(get<0>(child_frame).size() == large_values.size());
    get<0>(child_frame)[999] = 610;
    CHECK(get<1>(child_frame) == 144);
    CHECK(get<0>(containing)[1] == 233);
    CHECK(get<1>(containing)[0] == aligned_record{ 13, 377 });
    CHECK(get<2>(containing) == 144);
    CHECK(get<0>(child.top<std::uint32_t[], std::uint32_t>())[999] == 610);

    child.pop<std::uint32_t[], std::uint32_t>();
    CHECK(child.empty());
    CHECK(get<2>(containing) == 144);
    stack.pop(containing);
    CHECK(stack.empty());
}

TEST_CASE("multiple frame views follow growth of the last substack", "[stack][substack]")
{
    givm::frame_stack stack{ 1 };
    auto first_pushed = stack.push(std::uint16_t{ 2 }, givm::substack());
    get<1>(first_pushed).push(std::uint32_t{ 3 });
    stack.push(std::uint8_t{ 5 }, givm::substack());

    auto [first, last] = stack.top<
        givm::frame<std::uint16_t, givm::substack_t>,
        givm::frame<std::uint8_t, givm::substack_t>
    >();
    auto first_child = get<1>(first);
    auto last_child = get<1>(last);
    STATIC_REQUIRE_FALSE(can_push_stack_frame<decltype(first_child)>);
    STATIC_REQUIRE_FALSE(can_pop_stack_frame<decltype(first_child)>);
    STATIC_REQUIRE(can_push_stack_frame<decltype(last_child)>);
    STATIC_REQUIRE(can_pop_stack_frame<decltype(last_child)>);
    get<0>(first_child.top<std::uint32_t>()) = 8;

    const auto old_capacity = stack.capacity();
    const std::array<std::uint64_t, 1024> large_values{};
    last_child.push(givm::dynamic_array<std::uint64_t>(large_values), std::uint32_t{ 13 });
    REQUIRE(stack.capacity() > old_capacity);
    CHECK(get<0>(first) == 2);
    CHECK(get<0>(last) == 5);
    CHECK(get<0>(get<1>(first).top<std::uint32_t>()) == 8);
    CHECK(get<1>(get<1>(last).top<std::uint64_t[], std::uint32_t>()) == 13);

    givm::frame_stack copy = stack;
    stack.pop(first, last);
    CHECK(stack.empty());
    copy.pop<
        givm::frame<std::uint16_t, givm::substack_t>,
        givm::frame<std::uint8_t, givm::substack_t>
    >();
    CHECK(copy.empty());
}

TEST_CASE("a covered substack retains its contents and can grow after uncovering", "[stack][substack]")
{
    givm::frame_stack stack;
    auto containing = stack.push(std::uint16_t{ 7 }, givm::substack());
    get<1>(containing).push(std::uint32_t{ 11 });
    stack.push(std::uint8_t{ 13 });

    auto [covered, covering] = stack.top<
        givm::frame<std::uint16_t, givm::substack_t>,
        givm::frame<std::uint8_t>
    >();
    auto child = get<1>(covered);
    STATIC_REQUIRE_FALSE(can_push_stack_frame<decltype(child)>);
    STATIC_REQUIRE_FALSE(can_pop_stack_frame<decltype(child)>);
    CHECK(get<0>(covered) == 7);
    CHECK(get<0>(child.top<std::uint32_t>()) == 11);
    CHECK(get<0>(covering) == 13);
    get<0>(child.top<std::uint32_t>()) = 17;

    stack.pop(covering);
    auto uncovered = stack.top<std::uint16_t, givm::substack_t>();
    auto mutable_child = get<1>(uncovered);
    mutable_child.push(std::uint8_t{ 19 });
    CHECK(get<0>(mutable_child.top<std::uint8_t>()) == 19);
    mutable_child.pop<std::uint8_t>();
    CHECK(get<0>(mutable_child.top<std::uint32_t>()) == 17);
    mutable_child.pop<std::uint32_t>();
    CHECK(mutable_child.empty());
    CHECK(get<0>(uncovered) == 7);
    stack.pop(uncovered);
    CHECK(stack.empty());
}

TEST_CASE("top substack growth preserves earlier parent frame views", "[stack][substack]")
{
    const std::array<std::uint16_t, 3> values{ 2, 3, 5 };
    givm::frame_stack stack{ 1 };
    stack.push(givm::dynamic_array<std::uint16_t>(values), std::uint32_t{ 7 });
    auto middle_pushed = stack.push(std::uint16_t{ 11 }, givm::substack());
    get<1>(middle_pushed).push(std::uint8_t{ 13 }, std::uint32_t{ 17 });
    stack.push(givm::substack());

    auto [first, middle, last] = stack.top<
        givm::frame<std::uint16_t[], std::uint32_t>,
        givm::frame<std::uint16_t, givm::substack_t>,
        givm::frame<givm::substack_t>
    >();
    auto covered_child = get<1>(middle);
    STATIC_REQUIRE_FALSE(can_push_stack_frame<decltype(covered_child)>);
    STATIC_REQUIRE_FALSE(can_pop_stack_frame<decltype(covered_child)>);
    get<1>(covered_child.top<std::uint8_t, std::uint32_t>()) = 19;

    const auto old_capacity = stack.capacity();
    const std::array<std::uint64_t, 1024> large_values{};
    get<0>(last).push(givm::dynamic_array<std::uint64_t>(large_values));
    REQUIRE(stack.capacity() > old_capacity);
    CHECK(std::ranges::equal(get<0>(first), values));
    CHECK(get<1>(first) == 7);
    CHECK(get<0>(middle) == 11);
    CHECK(get<0>(covered_child.top<std::uint8_t, std::uint32_t>()) == 13);
    CHECK(get<1>(covered_child.top<std::uint8_t, std::uint32_t>()) == 19);
    get<0>(first)[1] = 23;
    get<1>(first) = 29;
    get<1>(covered_child.top<std::uint8_t, std::uint32_t>()) = 31;

    auto [updated_first, updated_middle, updated_last] = stack.top<
        givm::frame<std::uint16_t[], std::uint32_t>,
        givm::frame<std::uint16_t, givm::substack_t>,
        givm::frame<givm::substack_t>
    >();
    CHECK(get<0>(updated_first)[1] == 23);
    CHECK(get<1>(updated_first) == 29);
    CHECK(get<1>(get<1>(updated_middle).top<std::uint8_t, std::uint32_t>()) == 31);
    CHECK_FALSE(get<0>(updated_last).empty());
    stack.pop(first, middle, last);
    CHECK(stack.empty());
}

TEST_CASE("substacks address and remove multiple ordinary frames", "[stack][substack]")
{
    const std::array<std::uint16_t, 3> values{ 2, 3, 5 };
    givm::frame_stack stack{ 1 };
    auto containing = stack.push(std::uint32_t{ 7 }, givm::substack());
    auto child = get<1>(containing);
    child.push(std::uint8_t{ 11 });
    child.push(givm::dynamic_array<std::uint16_t>(values), std::uint32_t{ 13 });
    const auto initial_child_size = child.size();
    const auto old_capacity = stack.capacity();
    const std::array<aligned_record, 512> large_values{};
    child.push(givm::dynamic_array<aligned_record>(large_values), std::uint16_t{ 17 });
    REQUIRE(stack.capacity() > old_capacity);
    CHECK(get<0>(containing) == 7);
    CHECK(child.size() > initial_child_size);
    CHECK(get<1>(child.top<aligned_record[], std::uint16_t>()) == 17);
    child.pop<aligned_record[], std::uint16_t>();
    CHECK(child.size() == initial_child_size);

    auto [first, second] = child.top<
        givm::frame<std::uint8_t>,
        givm::frame<std::uint16_t[], std::uint32_t>
    >();
    CHECK(get<0>(first) == 11);
    CHECK(std::ranges::equal(get<0>(second), values));
    CHECK(get<1>(second) == 13);
    child.pop(first, second);
    CHECK(child.empty());
    CHECK(child.size() == 0);
    CHECK(get<0>(containing) == 7);
    CHECK(get<1>(containing).empty());

    child.push(std::uint32_t{ 23 });
    child.push(std::uint8_t{ 29 }, std::uint16_t{ 31 });
    child.pop<
        givm::frame<std::uint32_t>,
        givm::frame<std::uint8_t, std::uint16_t>
    >();
    CHECK(child.empty());
    CHECK(get<0>(containing) == 7);
    stack.pop<std::uint32_t, givm::substack_t>();
    CHECK(stack.empty());
}

TEST_CASE("const substacks expose only const contents", "[stack][substack]")
{
    const std::array<std::uint16_t, 3> values{ 3, 5, 8 };
    givm::frame_stack stack;
    auto containing = stack.push(std::uint8_t{ 13 }, givm::substack());
    get<1>(containing).push(givm::dynamic_array<std::uint16_t>(values), std::uint32_t{ 21 });

    auto const_containing = std::as_const(stack).top<std::uint8_t, givm::substack_t>();
    auto const_child = get<1>(const_containing);
    STATIC_REQUIRE_FALSE(can_push_stack_frame<decltype(const_child)>);
    STATIC_REQUIRE_FALSE(can_pop_stack_frame<decltype(const_child)>);
    auto&& [stored_values, marker] = const_child.top<std::uint16_t[], std::uint32_t>();
    STATIC_REQUIRE(std::is_same_v<decltype(stored_values), std::span<const std::uint16_t>>);
    STATIC_REQUIRE(std::is_same_v<decltype(marker), const std::uint32_t&>);
    CHECK(std::ranges::equal(stored_values, values));
    CHECK(marker == 21);
    CHECK(get<0>(const_containing) == 13);
}

TEST_CASE("copied and moved substacks own independent contents", "[stack][substack]")
{
    const std::array<std::uint16_t, 3> values{ 3, 5, 8 };
    givm::frame_stack original;
    auto original_frame = original.push(std::uint32_t{ 13 }, givm::substack());
    auto original_child = get<1>(original_frame);
    original_child.push(std::uint8_t{ 21 });
    original_child.push(givm::dynamic_array<std::uint16_t>(values), std::uint32_t{ 34 });

    givm::frame_stack copy = original;
    auto copy_frame = copy.top<std::uint32_t, givm::substack_t>();
    get<0>(copy_frame) = 55;
    auto copy_child = get<1>(copy_frame);
    get<0>(copy_child.top<std::uint16_t[], std::uint32_t>())[0] = 144;
    copy_child.push(std::uint64_t{ 233 });

    auto unchanged = original.top<std::uint32_t, givm::substack_t>();
    CHECK(get<0>(unchanged) == 13);
    CHECK(std::ranges::equal(
        get<0>(get<1>(unchanged).top<std::uint16_t[], std::uint32_t>()), values
    ));

    givm::frame_stack assigned;
    assigned.push(std::uint8_t{ 1 });
    assigned = copy;
    copy.clear();
    givm::frame_stack moved = std::move(assigned);
    assigned.push(std::uint32_t{ 377 });
    CHECK(get<0>(assigned.top<std::uint32_t>()) == 377);

    auto moved_frame = moved.top<std::uint32_t, givm::substack_t>();
    CHECK(get<0>(moved_frame) == 55);
    auto moved_child = get<1>(moved_frame);
    CHECK(get<0>(moved_child.top<std::uint64_t>()) == 233);
    moved_child.pop<std::uint64_t>();
    CHECK(get<0>(moved_child.top<std::uint16_t[], std::uint32_t>())[0] == 144);
    moved_child.pop<std::uint16_t[], std::uint32_t>();
    CHECK(get<0>(moved_child.top<std::uint8_t>()) == 21);
    moved_child.pop<std::uint8_t>();
    CHECK(moved_child.empty());

    givm::frame_stack move_assigned;
    move_assigned = std::move(moved);
    CHECK(get<0>(move_assigned.top<std::uint32_t, givm::substack_t>()) == 55);
    CHECK(get<1>(move_assigned.top<std::uint32_t, givm::substack_t>()).empty());
    move_assigned.pop<std::uint32_t, givm::substack_t>();
    CHECK(move_assigned.empty());
}
