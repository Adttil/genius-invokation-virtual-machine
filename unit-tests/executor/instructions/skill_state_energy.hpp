#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <givm/givm.hpp>

#include "../../table/test_definition_library.hpp"

namespace givm_test::executor_instructions::skill_state_energy
{
namespace
{
    constexpr givm::character_id active{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id standby{ givm::player_id{ 0 }, 1 };
    constexpr givm::character_id opponent{ givm::player_id{ 1 }, 0 };

    enum class mutation_kind { skill, assign, modify, sequence, use_skill, batch_sequence };

    struct mutation_log
    {
        mutation_kind kind = mutation_kind::skill;
        bool dynamic = false;
        bool explicit_energy = false;
        bool missing_target = false;
        givm::relative_character_target relative{ givm::relative_player::self, 1 };
        givm::character_id target = standby;
        std::uint32_t value = 0;
        std::int64_t delta = 0;
        std::uint32_t initial_energy = 1;
        std::uint32_t max_energy = 3;
        std::uint32_t standby_health = 10;
        std::uint32_t energy_notifications = 0;
        std::vector<std::uint32_t> energy_during_effect;
        std::vector<std::uint32_t> energy_after_skill;
        std::vector<givm::character_id> targets;
        std::vector<givm::character_id> second_targets;
        bool runtime_inputs = false;
    };

    struct mutable_skill_source
    {
        using definition_category = givm::skill_view;
        struct definition_type { mutation_log* log; givm::program_entry effect; };
        mutation_log* log;

        std::string_view name() const { return "MutableSkill"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.add_program(std::tuple{
                givm::modify_energy{ .target = { givm::relative_player::self, 0 }, .delta = 1 } }) };
        }
        static givm::program_entry handle(const definition_type& data,
            givm::skill_effect&, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            const auto self = context.entity();
            data.log->energy_during_effect.push_back(self.character().state().energy);
            return data.log->explicit_energy ? context.invoke(data.effect) : givm::program_entry{};
        }
    };

    struct mutation_observer_source
    {
        using definition_category = givm::skill_view;
        struct definition_type
        {
            mutation_log* log;
            givm::definition_id<givm::skill_view> skill;
            givm::program_entry effect;
        };
        mutation_log* log;

        std::string_view name() const { return "MutationObserver"; }
        auto skill_dependencies() const { return std::array{ std::string_view{ "MutableSkill" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto skill = context.resolve_id<givm::skill_view>("MutableSkill");
            givm::program_entry effect;
            switch(log->kind)
            {
            case mutation_kind::skill:
                effect = context.add_program(std::tuple{ log->dynamic ? givm::set_skill_state{}
                    : givm::set_skill_state{ .character = log->relative, .definition = skill,
                        .state = { log->value } } });
                break;
            case mutation_kind::assign:
                effect = context.add_program(std::tuple{ log->dynamic ? givm::set_energy{}
                    : givm::set_energy{ .target = log->relative, .value = log->value } });
                break;
            case mutation_kind::modify:
                effect = context.add_program(std::tuple{ log->dynamic ? givm::modify_energy{}
                    : givm::modify_energy{ .target = log->relative, .delta = log->delta } });
                break;
            case mutation_kind::sequence:
                if(log->dynamic)
                    effect = context.add_program(std::tuple{ givm::set_energy{}, givm::modify_energy{},
                        givm::modify_energy{}, givm::modify_energy{}, givm::modify_energy{} });
                else
                    effect = context.add_program(std::tuple{
                        givm::set_energy{ .target = log->relative, .value = 2 },
                        givm::modify_energy{ .target = log->relative, .delta = std::numeric_limits<std::int64_t>::max() },
                        givm::modify_energy{ .target = log->relative, .delta = -1 },
                        givm::modify_energy{ .target = log->relative, .delta = std::numeric_limits<std::int64_t>::min() },
                        givm::modify_energy{ .target = log->relative, .delta = 2 } });
                break;
            case mutation_kind::use_skill:
                effect = context.add_program(std::tuple{ givm::use_skill{ .definition = skill } });
                break;
            case mutation_kind::batch_sequence:
                effect = context.add_program(std::tuple{ givm::modify_energy{}, givm::modify_energy{},
                    givm::modify_energy{}, givm::modify_energy{}, givm::modify_energy{} });
                break;
            }
            return { log, skill, effect };
        }
        static givm::program_entry handle(const definition_type& data,
            givm::round_started&, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            const auto& log = *data.log;
            if(not log.dynamic) return context.invoke(data.effect);
            const auto target = std::span{ &log.target, 1 };
            switch(log.kind)
            {
            case mutation_kind::skill:
                for(const auto skill : context.table()[log.target].skills())
                    if(skill.definition_id() == data.skill)
                        return context.invoke(data.effect, givm::set_skill_state_input{ skill.id(), { log.value } });
                FAIL("the target must own the mutable skill");
                return {};
            case mutation_kind::assign:
                return context.invoke(data.effect, givm::set_energy_input{ log.target, log.value });
            case mutation_kind::modify:
                return context.invoke(data.effect, givm::modify_energy_input{ target, log.delta });
            case mutation_kind::sequence:
                return context.invoke(data.effect, givm::set_energy_input{ log.target, 2 },
                    givm::modify_energy_input{ target, std::numeric_limits<std::int64_t>::max() },
                    givm::modify_energy_input{ target, -1 },
                    givm::modify_energy_input{ target, std::numeric_limits<std::int64_t>::min() },
                    givm::modify_energy_input{ target, 2 });
            case mutation_kind::use_skill:
                return context.invoke(data.effect);
            case mutation_kind::batch_sequence:
            {
                const std::array inputs{
                    givm::modify_energy_input{ log.targets, std::numeric_limits<std::int64_t>::max() },
                    givm::modify_energy_input{ {}, std::numeric_limits<std::int64_t>::min() },
                    givm::modify_energy_input{ log.second_targets, -1 },
                    givm::modify_energy_input{ log.targets, std::numeric_limits<std::int64_t>::min() },
                    givm::modify_energy_input{ log.second_targets, 2 } };
                if(log.runtime_inputs)
                {
                    std::vector<givm::program_inputs> sequence;
                    for(const auto& input : inputs) sequence.push_back(givm::pack_inputs(input));
                    return context.invoke(data.effect, givm::concat_inputs(sequence));
                }
                return context.invoke(data.effect, inputs[0], inputs[1], inputs[2], inputs[3], inputs[4]);
            }
            }
            return {};
        }
        static givm::program_entry handle(const definition_type& data,
            givm::changing_energy&, givm::handle_context<givm::skill_view>&, std::uint32_t = 0)
        {
            ++data.log->energy_notifications;
            return {};
        }
        static givm::program_entry handle(const definition_type& data,
            givm::energy_changed&, givm::handle_context<givm::skill_view>&, std::uint32_t = 0)
        {
            ++data.log->energy_notifications;
            return {};
        }
        static givm::program_entry handle(const definition_type& data,
            givm::skill_used& event, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            data.log->energy_after_skill.push_back(context.table()[event.skill.character_id].state().energy);
            return {};
        }
    };

    struct mutable_character_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            givm::character_state state;
            givm::definition_id<givm::skill_view> skill;
            givm::definition_id<givm::skill_view> observer;
        };
        mutation_log* log;
        std::string_view source_name;
        bool observe = false;
        std::uint32_t health = 10;

        std::string_view name() const { return source_name; }
        auto tags() const { return std::array{ std::string_view{ "special_energy" } }; }
        auto skill_dependencies() const
        {
            return std::array{ std::string_view{ "MutableSkill" }, std::string_view{ "MutationObserver" } };
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { { .max_health = 10, .max_energy = log->max_energy, .health = health,
                .energy = log->initial_energy, .energy_tag = *context.find_tag("special_energy") },
                context.resolve_id<givm::skill_view>("MutableSkill"),
                observe ? context.resolve_id<givm::skill_view>("MutationObserver")
                    : givm::definition_id<givm::skill_view>{} };
        }
        static givm::character_state query(const definition_type& data, const givm::character_initial_state&)
        {
            return data.state;
        }
        static givm::definition_id<givm::skill_view> query(const definition_type& data, const givm::character_initial_skill& query)
        {
            if(query.skill_index == 0) return data.skill;
            if(query.skill_index == 1) return data.observer;
            return {};
        }
    };

    template<class Check>
    inline void check_mutation(mutation_log& log, givm::compile_mode mode, Check check)
    {
        const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
            std::tuple{ givm::start_round{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{},
            mutable_skill_source{ &log }, mutation_observer_source{ &log },
            mutable_character_source{ &log, "Actor", true },
            mutable_character_source{ &log, "Standby", false, log.standby_health },
            mutable_character_source{ &log, "Opponent" });
        givm::table table{ { .self_player = givm::player_id{ 0 } }, { .active_character = active },
            { .active_character = log.missing_target ? std::optional<givm::character_id>{} : opponent } };
        load_deck(table, library,
            { .characters = { ids.get_id<givm::character_view>("Actor"), ids.get_id<givm::character_view>("Standby") } },
            { .characters = { ids.get_id<givm::character_view>("Opponent") } });
        const auto energy_tag = table[active].state().energy_tag;
        REQUIRE(energy_tag.is_valid());
        givm_test::executor_driver executor;
        executor.start(library, table);
        auto random = [] { return std::uint32_t{ 0 }; };
        REQUIRE(executor.advance(library, table, random) == givm::execution_state::finished);
        CHECK(log.energy_notifications == 0);
        for(const auto character : { active, standby, opponent })
        {
            CHECK(table[character].state().energy_tag == energy_tag);
            CHECK(table[character].state().max_energy == log.max_energy);
        }
        check(table, ids.get_id<givm::skill_view>("MutableSkill"));
    }

    template<class Check>
    inline void check_group_mutation(mutation_log& log, givm::compile_mode mode, bool empty_opponent, Check check)
    {
        const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
            std::tuple{ givm::start_round{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{},
            mutable_skill_source{ &log }, mutation_observer_source{ &log },
            mutable_character_source{ &log, "Actor", true },
            mutable_character_source{ &log, "Living" },
            mutable_character_source{ &log, "Defeated", false, 0 });
        const auto actor = ids.get_id<givm::character_view>("Actor");
        const auto living = ids.get_id<givm::character_view>("Living");
        const auto defeated = ids.get_id<givm::character_view>("Defeated");
        givm::linked_deck opponent_deck{ .characters = { living, living, defeated, living, living } };
        if(empty_opponent) opponent_deck.characters.clear();
        givm::table table{ { .self_player = givm::player_id{ 0 } }, { .active_character = active },
            { .active_character = log.missing_target or empty_opponent ? std::optional<givm::character_id>{}
                : givm::character_id{ givm::player_id{ 1 }, 2 } } };
        load_deck(table, library, { .characters = { actor, living, living, defeated, living } }, opponent_deck);
        const auto energy_tag = table[active].state().energy_tag;
        givm_test::executor_driver executor;
        executor.start(library, table);
        auto random = [] { return std::uint32_t{ 0 }; };
        REQUIRE(executor.advance(library, table, random) == givm::execution_state::finished);
        CHECK(log.energy_notifications == 0);
        for(const auto player : { givm::player_id{ 0 }, givm::player_id{ 1 } })
            for(const auto character : table[player].characters())
            {
                CHECK(character.state().energy_tag == energy_tag);
                CHECK(character.state().max_energy == log.max_energy);
            }
        check(table);
    }
}

TEST_CASE("set_skill_state changes an existing skill without removing it at zero", "[set_skill_state]")
{
    mutation_log log{ .dynamic = GENERATE(false, true), .value = GENERATE(0u, 7u, UINT32_MAX),
        .standby_health = GENERATE(0u, 10u) };
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    check_mutation(log, mode, [&](const givm::table& table, auto definition)
    {
        for(const auto character : { active, standby, opponent })
        {
            std::size_t matching_skills = 0;
            for(const auto skill : table[character].skills())
                if(skill.definition_id() == definition)
                {
                    ++matching_skills;
                    CHECK(skill.is_valid());
                    CHECK(skill.state().count == (character == standby ? log.value : 0));
                }
            CHECK(matching_skills == 1);
        }
        CHECK(table[standby].state().health == log.standby_health);
    });
}

TEST_CASE("set_energy assigns and clamps energy without broadcasts", "[set_energy]")
{
    mutation_log log{ .kind = mutation_kind::assign, .dynamic = GENERATE(false, true),
        .value = GENERATE(0u, 2u, UINT32_MAX), .max_energy = GENERATE(0u, 3u, UINT32_MAX),
        .standby_health = GENERATE(0u, 10u) };
    log.initial_energy = log.max_energy == 0 ? 0 : 1;
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    check_mutation(log, mode, [&](const givm::table& table, auto)
    {
        CHECK(table[standby].state().energy == std::min(log.value, log.max_energy));
        CHECK(table[active].state().energy == log.initial_energy);
        CHECK(table[opponent].state().energy == log.initial_energy);
    });
}

TEST_CASE("modify_energy saturates signed deltas including integer extremes", "[modify_energy]")
{
    mutation_log log{ .kind = mutation_kind::modify, .dynamic = GENERATE(false, true),
        .relative = { givm::relative_player::opponent, 0 }, .target = opponent,
        .delta = GENERATE(INT64_MIN, -2ll, -1ll, 0ll, 1ll, 2ll, INT64_MAX),
        .max_energy = GENERATE(0u, 3u, UINT32_MAX) };
    log.initial_energy = log.max_energy == 0 ? 0 : log.max_energy - 1;
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    check_mutation(log, mode, [&](const givm::table& table, auto)
    {
        const auto expected = log.delta == INT64_MAX ? log.max_energy
            : static_cast<std::uint32_t>(std::clamp(static_cast<std::int64_t>(log.initial_energy) + log.delta,
                std::int64_t{ 0 }, static_cast<std::int64_t>(log.max_energy)));
        CHECK(table[opponent].state().energy == expected);
        CHECK(table[active].state().energy == log.initial_energy);
        CHECK(table[standby].state().energy == log.initial_energy);
    });
}

TEST_CASE("consecutive energy commands use the energy left by earlier commands", "[set_energy][modify_energy]")
{
    mutation_log log{ .kind = mutation_kind::sequence, .dynamic = GENERATE(false, true) };
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    check_mutation(log, mode, [&](const givm::table& table, auto)
    {
        CHECK(table[standby].state().energy == 2);
        CHECK(table[active].state().energy == 1);
    });
}

TEST_CASE("fixed energy commands skip an absent active target", "[set_energy][modify_energy]")
{
    mutation_log log{ .kind = GENERATE(mutation_kind::assign, mutation_kind::modify), .missing_target = true,
        .relative = { givm::relative_player::opponent, 0 }, .target = opponent, .value = 0, .delta = -1 };
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    check_mutation(log, mode, [&](const givm::table& table, auto)
    {
        CHECK(table[opponent].state().energy == 1);
        CHECK(table[active].state().energy == 1);
    });
}

TEST_CASE("skills gain energy only through their effect before the skill used notification", "[modify_energy][use_skill]")
{
    mutation_log log{ .kind = mutation_kind::use_skill, .explicit_energy = GENERATE(false, true) };
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    check_mutation(log, mode, [&](const givm::table& table, auto)
    {
        const auto expected = log.explicit_energy ? 2u : 1u;
        CHECK(table[active].state().energy == expected);
        CHECK(log.energy_during_effect == std::vector{ 1u });
        CHECK(log.energy_after_skill == std::vector{ expected });
    });
}

TEST_CASE("fixed modify_energy changes living range targets without moving the anchor", "[modify_energy]")
{
    const auto [player, offset] = GENERATE(std::pair{ givm::relative_player::self, -2 },
        std::pair{ givm::relative_player::opponent, 0 }, std::pair{ givm::relative_player::opponent, 7 });
    const auto selection = GENERATE(givm::character_selection::others, givm::character_selection::all);
    mutation_log log{ .kind = mutation_kind::modify, .relative = { player, offset, selection }, .delta = 1 };
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    check_group_mutation(log, mode, false, [&](const givm::table& table)
    {
        const auto selected_player = player == givm::relative_player::self ? givm::player_id{ 0 } : givm::player_id{ 1 };
        const auto active_index = player == givm::relative_player::self ? 0 : 2;
        const auto anchor_index = static_cast<std::size_t>(((active_index + offset) % 5 + 5) % 5);
        for(const auto owner : { givm::player_id{ 0 }, givm::player_id{ 1 } })
            for(std::size_t index = 0; index != 5; ++index)
            {
                const auto character = table[givm::character_id{ owner, index }];
                const auto selected = owner == selected_player and character.state().health != 0
                    and (selection == givm::character_selection::all or index != anchor_index);
                CHECK(character.state().energy == (selected ? 2 : 1));
            }
    });
}

TEST_CASE("fixed modify_energy ranges skip missing active characters and empty character lists", "[modify_energy]")
{
    mutation_log log{ .kind = mutation_kind::modify, .missing_target = true,
        .relative = { givm::relative_player::opponent, -1,
            GENERATE(givm::character_selection::others, givm::character_selection::all) }, .delta = 1 };
    const auto empty_opponent = GENERATE(false, true);
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    check_group_mutation(log, mode, empty_opponent, [&](const givm::table& table)
    {
        for(const auto player : { givm::player_id{ 0 }, givm::player_id{ 1 } })
            for(const auto character : table[player].characters())
                CHECK(character.state().energy == 1);
    });
}

TEST_CASE("dynamic modify_energy batches consume distinct and empty inputs in program order", "[modify_energy]")
{
    mutation_log log{ .kind = mutation_kind::batch_sequence, .dynamic = true };
    log.runtime_inputs = GENERATE(false, true);
    log.targets = { { givm::player_id{ 0 }, 1 }, { givm::player_id{ 0 }, 4 }, { givm::player_id{ 1 }, 2 } };
    log.second_targets = { { givm::player_id{ 0 }, 4 }, { givm::player_id{ 1 }, 0 } };
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    check_group_mutation(log, mode, false, [&](const givm::table& table)
    {
        for(const auto player : { givm::player_id{ 0 }, givm::player_id{ 1 } })
            for(std::size_t index = 0; index != 5; ++index)
            {
                const auto character = givm::character_id{ player, index };
                const auto in_first = std::ranges::any_of(log.targets, [&](auto id) { return id == character; });
                const auto in_second = std::ranges::any_of(log.second_targets, [&](auto id) { return id == character; });
                CHECK(table[character].state().energy == (in_second ? 2 : in_first ? 0 : 1));
            }
    });
}
}
