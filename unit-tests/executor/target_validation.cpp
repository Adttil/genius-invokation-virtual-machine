#include <array>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <variant>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

#include "../table/test_definition_library.hpp"
#include "test_character_source.hpp"

namespace
{
    struct tagged_target_card_source
    {
        using definition_category = givm::card_definition;
        struct definition_type { givm::tag_id target_tag; };

        std::string_view name() const noexcept { return "TaggedTargetCard"; }
        auto tag_dependencies() const { return std::array{ std::string_view{ "allowed_target" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.resolve_tag("allowed_target") };
        }
        static givm::target_validation query(const definition_type& data, const givm::card_target_validation& query)
        {
            if(query.target_count == 0) return givm::target_validation::valid_incomplete;
            if(query.target_count != 1) return givm::target_validation::invalid;
            const auto* target = std::get_if<givm::character_id>(&query.targets[0]);
            return target != nullptr && query.library[query.table[*target].definition_id()].has_tag(data.target_tag)
                ? givm::target_validation::valid_complete : givm::target_validation::invalid;
        }
        static givm::program_entry handle(
            const definition_type&, const givm::hand_card_view&, givm::card_effect&, givm::handle_context&)
        {
            return {};
        }
    };

    struct tagged_target_skill_source
    {
        using definition_category = givm::skill_view;
        struct definition_type { givm::tag_id target_tag; };

        std::string_view name() const noexcept { return "TaggedTargetSkill"; }
        auto tag_dependencies() const { return std::array{ std::string_view{ "allowed_target" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.resolve_tag("allowed_target") };
        }
        static givm::target_validation query(const definition_type& data, const givm::skill_target_validation& query)
        {
            if(query.target_count == 0) return givm::target_validation::valid_incomplete;
            if(query.target_count != 1) return givm::target_validation::invalid;
            const auto* target = std::get_if<givm::character_id>(&query.targets[0]);
            return target != nullptr && query.library[query.table[*target].definition_id()].has_tag(data.target_tag)
                ? givm::target_validation::valid_complete : givm::target_validation::invalid;
        }
        static givm::program_entry handle(
            const definition_type&, const givm::skill_view&, givm::skill_effect&, givm::handle_context&)
        {
            return {};
        }
    };

    struct tagged_target_character_source
    {
        using definition_category = givm::character_view;
        struct definition_type { givm::definition_id<givm::skill_view> skill; };

        std::string_view name() const noexcept { return "TaggedTargetCharacter"; }
        auto tags() const { return std::array{ std::string_view{ "allowed_target" } }; }
        auto skill_dependencies() const { return std::array{ std::string_view{ "TaggedTargetSkill" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.resolve_id<givm::skill_view>("TaggedTargetSkill") };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::definition_id<givm::skill_view> query(const definition_type& data, const givm::character_initial_skill& query)
        {
            return query.skill_index == 0 ? data.skill : givm::definition_id<givm::skill_view>{};
        }
    };
}

TEST_CASE("action target queries can inspect target definition tags", "[action][query][targets][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const tagged_target_card_source card;
    const tagged_target_skill_source skill;
    const tagged_target_character_source tagged;
    const givm::test::initialized_character_source untagged;
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode, std::tuple{
        givm::set_active_character{ givm::character_id{ givm::player_id{ 0 }, 0 } },
        givm::set_active_character{ givm::character_id{ givm::player_id{ 1 }, 0 } },
        givm::draw_cards{ .count = 1 }, givm::begin_action{}
    }, std::tuple{}, card, skill, tagged, untagged);
    const auto tagged_id = ids.get_id<givm::character_view>(tagged.name());
    const auto untagged_id = ids.get_id<givm::character_view>(untagged.name());
    givm::table table;
    load_deck(table, library,
        { .cards = { ids.get_id<givm::card_definition>(card.name()) }, .characters = { tagged_id, untagged_id } },
        { .characters = { untagged_id } });
    givm::executor executor;
    executor.enter_entry(library);
    auto random = []() -> std::uint32_t { return 0; };
    auto state = executor.step(library, table, random);
    while(state == givm::execution_state::active_character_changed || state == givm::execution_state::action_started)
        state = executor.step(library, table, random);
    REQUIRE(state == givm::execution_state::action_selection);
    const auto action = executor.view_in<givm::execution_state::action_selection>();
    REQUIRE(action.card_count() == 1);
    REQUIRE(action.skill_count() == 1);
    CHECK(action.card_targets_validate(library, table, 0) == givm::target_validation::valid_incomplete);
    CHECK(action.skill_targets_validate(library, table, 0) == givm::target_validation::valid_incomplete);

    const std::array<givm::card_target_id, 1> allowed{ givm::character_id{ givm::player_id{ 0 }, 0 } };
    const std::array<givm::card_target_id, 1> rejected{ givm::character_id{ givm::player_id{ 0 }, 1 } };
    CHECK(action.card_targets_validate(library, table, 0, allowed) == givm::target_validation::valid_complete);
    CHECK(action.card_targets_validate(library, table, 0, rejected) == givm::target_validation::invalid);
    CHECK(action.skill_targets_validate(library, table, 0, allowed) == givm::target_validation::valid_complete);
    CHECK(action.skill_targets_validate(library, table, 0, rejected) == givm::target_validation::invalid);
}
