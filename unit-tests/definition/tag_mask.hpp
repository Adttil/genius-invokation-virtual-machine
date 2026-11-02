#include <algorithm>
#include <array>
#include <cstddef>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <givm/compile.hpp>
#include <givm/runtime.hpp>

#include "../table/test_definition_library.hpp"

namespace givm_test::definition::tag_mask
{
    static_assert(sizeof(std::size_t) != 8 || sizeof(givm::tag_mask) == 32);
    static_assert(std::ranges::random_access_range<givm::tag_mask_view>);

    TEST_CASE("tag masks preserve matching and prefixes across the inline limit", "[tags][tag_mask]")
    {
        const auto count = GENERATE(std::size_t{ 191 }, 192, 193, 257);
        const givm::tag_id first{ 0 }, boundary{ 64 }, last{ count - 1 }, absent{ 1 };
        givm::tag_mask tags{ count };
        tags.set(first);
        tags.set(boundary);
        tags.set(last);
        const std::array required{ first, last };
        const std::array excluded{ absent };
        CHECK(tags.has_all(required));
        CHECK_FALSE(tags.has_any(excluded));
        CHECK(tags.matches(required, excluded));
        const auto prefix = tags.take(65);
        CHECK(prefix.size() == 65);
        CHECK(prefix.has(first));
        CHECK(prefix.has(boundary));
        CHECK_FALSE(prefix.has(last));
        CHECK(std::ranges::count(prefix, true) == 2);
        CHECK(std::ranges::count(tags.take(count), true) == 3);
        auto copy = tags;
        copy.set(first, false);
        CHECK(tags.has(first));
        CHECK_FALSE(copy.has(first));
        CHECK_FALSE(copy.matches(required, excluded));
        const givm::tag_mask empty;
        CHECK(empty.take(0).begin() == empty.take(0).end());
    }

    struct tagged_card : givm::test::named_definition_source<givm::definition_category::card>
    {
        std::array<std::string_view, 4> labels;
        constexpr auto tags() const { return labels; }
    };

    TEST_CASE("compiled tag lookup and filters work around 192 public tags", "[tags][library][issued_id_map]")
    {
        const auto count = GENERATE(std::size_t{ 191 }, 192, 193);
        const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
        std::vector<std::string> names;
        for(std::size_t index = 0; index < count; ++index) names.push_back("tag_" + std::to_string(index));
        std::vector<std::string_view> visible;
        for(const auto& name : names) visible.push_back(name);
        const tagged_card card{ { "WideTagsCard" }, { visible.front(), visible[count / 2], visible.back(), "private" } };
        auto sources = givm_test::make_source_library();
        REQUIRE(sources.add(card));
        const auto [library, ids] = givm_test::require_success(givm::compile(
            sources, givm_test::basic_sources, std::tuple{}, std::tuple{}, mode, visible));
        const auto id = ids.get_id<givm::definition_category::card>(card.name());
        REQUIRE(library.public_tag_count() == count);
        std::vector<bool> expected(count);
        for(std::size_t index = 0; index < 3; ++index)
        {
            const auto tag = ids.get_tag_id(card.labels[index]);
            expected[tag.value()] = true;
            CHECK(library.has_tag(id, tag));
        }
        const auto private_tag = ids.get_tag_id("private");
        CHECK(library.has_tag(id, private_tag));
        CHECK_FALSE(library.public_tags(id).has(private_tag));
        CHECK(std::ranges::equal(library.public_tags(id), expected));
        const auto expression = std::string{ visible.back() } + " & private & !" + std::string{ visible[1] };
        CHECK(ids.query_by_tag<givm::definition_category::card>(expression) == std::vector{ id });
        CHECK(ids.query_by_tag<givm::definition_category::card>("private & !" + std::string{ visible.back() }).empty());
        const auto copied = library;
        CHECK(std::ranges::equal(copied.public_tags(id), expected));
    }
}
