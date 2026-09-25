#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <optional>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <givm/givm.hpp>

#include "../../table/test_definition_library.hpp"

namespace
{
    constexpr givm::character_id active{ givm::player_id{ 0 }, 0 };
    constexpr givm::character_id standby{ givm::player_id{ 0 }, 1 };
    constexpr givm::character_id opponent{ givm::player_id{ 1 }, 0 };

    enum class mutation_kind { skill, assign, modify, sequence, use_skill };

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
        static givm::program_entry handle(const definition_type& data, const givm::skill_view& self,
            givm::skill_effect&, givm::handle_context& context)
        {
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
            }
            return { log, skill, effect };
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::test_event&, givm::handle_context& context)
        {
            const auto& log = *data.log;
            if(not log.dynamic) return context.invoke(data.effect);
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
                return context.invoke(data.effect, givm::modify_energy_input{ log.target, log.delta });
            case mutation_kind::sequence:
                return context.invoke(data.effect, givm::set_energy_input{ log.target, 2 },
                    givm::modify_energy_input{ log.target, std::numeric_limits<std::int64_t>::max() },
                    givm::modify_energy_input{ log.target, -1 },
                    givm::modify_energy_input{ log.target, std::numeric_limits<std::int64_t>::min() },
                    givm::modify_energy_input{ log.target, 2 });
            case mutation_kind::use_skill:
                return context.invoke(data.effect);
            }
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::changing_energy&, givm::handle_context&)
        {
            ++data.log->energy_notifications;
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::energy_changed&, givm::handle_context&)
        {
            ++data.log->energy_notifications;
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
            givm::skill_used& event, givm::handle_context& context)
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
        auto tag_dependencies() const { return std::array{ std::string_view{ "special_energy" } }; }
        auto skill_dependencies() const
        {
            return std::array{ std::string_view{ "MutableSkill" }, std::string_view{ "MutationObserver" } };
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { { .max_health = 10, .max_energy = log->max_energy, .health = health,
                .energy = log->initial_energy, .energy_tag = context.resolve_tag("special_energy") },
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
            std::tuple{ givm::test_command{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{},
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
        givm::executor executor;
        executor.enter_entry(library);
        auto random = [] { return std::uint32_t{ 0 }; };
        REQUIRE(executor.step(library, table, random) == givm::execution_state::finished);
        CHECK(log.energy_notifications == 0);
        for(const auto character : { active, standby, opponent })
        {
            CHECK(table[character].state().energy_tag == energy_tag);
            CHECK(table[character].state().max_energy == log.max_energy);
        }
        check(table, ids.get_id<givm::skill_view>("MutableSkill"));
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
