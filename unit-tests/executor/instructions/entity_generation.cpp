#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <ranges>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/executor.hpp>

#include "../../table/test_definition_library.hpp"

namespace
{
    using state_values = std::array<std::uint32_t, 2>;

    struct summon_traits
    {
        using view = givm::summon_view;
        using id = givm::summon_id;
        using state = givm::summon_state;
        using query = givm::summon_state_limit;
        using generate = givm::summon;
        using generation = givm::summoning;
        using regeneration = givm::resummoning;
        using add = givm::add_summon;
        using addition = givm::summon_addition;
        using set = givm::set_summon_state;
        using modify = givm::modify_summon_state;
        using modification = givm::summon_state_modification;
        using change = givm::summon_state_change;
        using changed = givm::summon_state_changed;
        using remove = givm::remove_summon;
        using removal = givm::summon_removal;
        using removed = givm::summon_removed;

        static id removed_id(const removed& event) { return event.summon; }

        static auto owner(givm::player_id player, std::size_t = 0) { return player; }
        static auto owner_of(const view& self) { return self.player().id(); }
        static auto entities(const givm::table& table, givm::player_id player, std::size_t = 0) { return table[player].summons(); }
        static state_values values(state value) { return { value.value, value.usages }; }
    };

    struct combat_status_traits
    {
        using view = givm::combat_status_view;
        using id = givm::combat_status_id;
        using state = givm::combat_status_state;
        using query = givm::combat_status_state_limit;
        using generate = givm::generate_combat_status;
        using generation = givm::combat_status_generation;
        using regeneration = givm::combat_status_regeneration;
        using add = givm::add_combat_status;
        using addition = givm::combat_status_addition;
        using set = givm::set_combat_status_state;
        using modify = givm::modify_combat_status_state;
        using modification = givm::combat_status_state_modification;
        using change = givm::combat_status_state_change;
        using changed = givm::combat_status_state_changed;
        using remove = givm::remove_combat_status;
        using removal = givm::combat_status_removal;
        using removed = givm::combat_status_removed;

        static id removed_id(const removed& event) { return event.status; }

        static auto owner(givm::player_id player, std::size_t = 0) { return player; }
        static auto owner_of(const view& self) { return self.player().id(); }
        static auto entities(const givm::table& table, givm::player_id player, std::size_t = 0) { return table[player].combat_statuses(); }
        static state_values values(state value) { return { value.count, value.round_usages }; }
    };

    struct attachment_traits
    {
        using view = givm::attachment_view;
        using id = givm::attachment_id;
        using state = givm::attachment_state;
        using query = givm::attachment_state_limit;
        using generate = givm::attach;
        using generation = givm::attachment_application;
        using regeneration = givm::attachment_reapplication;
        using add = givm::add_attachment;
        using addition = givm::attachment_addition;
        using set = givm::set_attachment_state;
        using modify = givm::modify_attachment_state;
        using modification = givm::attachment_state_modification;
        using change = givm::attachment_state_change;
        using changed = givm::attachment_state_changed;
        using remove = givm::remove_attachment;
        using removal = givm::attachment_removal;
        using removed = givm::attachment_removed;

        static id removed_id(const removed& event) { return event.attachment; }

        static auto owner(givm::player_id player, std::size_t index = 0) { return givm::character_id{ player, index }; }
        static auto owner_of(const view& self) { return self.character().id(); }
        static auto entities(const givm::table& table, givm::player_id player, std::size_t index = 0)
        {
            return table[owner(player, index)].attachments();
        }
        static state_values values(state value) { return { value.count, value.round_usages }; }
    };

    constexpr auto maximum = std::numeric_limits<std::uint32_t>::max();
    using state_deltas = std::array<std::int64_t, 2>;

    enum class operation { generate, add, set, remove, modify, modify_twice };
    enum class regeneration_behavior { ignore, accumulate, refresh, independent };

    struct action
    {
        operation operation;
        state_values state{ maximum, maximum };
        givm::player_id player{ 1 };
        std::size_t target_index = 0;
        std::size_t character_index = 0;
        state_deltas delta{};
        state_deltas next_delta{};
        bool other_summon = false;
    };

    template<class T>
    struct lifecycle_log
    {
        std::vector<action> actions;
        regeneration_behavior behavior = regeneration_behavior::ignore;
        bool dynamic = false;
        bool erase_empty = false;
        bool restore_empty = false;
        bool regeneration_handler_enabled = true;
        bool state_handler_enabled = true;
        state_values limit{ 20, 30 };
        std::uint32_t limit_queries = 0;
        std::size_t next_action = 0;
        std::vector<typename T::id> created;
        std::vector<typename T::id> regenerated;
        std::vector<typename T::id> changed;
        std::vector<state_values> regenerated_states;
        std::vector<state_values> previous_states;
        std::vector<state_values> current_states;
        std::vector<typename T::id> left;
        std::vector<state_values> departing_states;
    };

    template<class T>
    void observe_created(lifecycle_log<T>& log, const givm::table& table)
    {
        for(const auto player : { givm::player_id{ 0 }, givm::player_id{ 1 } })
        {
            const auto character_count = player == givm::player_id{ 0 } ? 1uz : 2uz;
            const auto owners = std::is_same_v<T, attachment_traits> ? character_count : 1uz;
            for(std::size_t index = 0; index < owners; ++index)
                for(const auto entity : T::entities(table, player, index))
                    if(std::ranges::find_if(log.created, [&](const auto id) { return id == entity.id(); }) == log.created.end())
                        log.created.push_back(entity.id());
        }
    }

    template<class T>
    struct lifecycle_source
    {
        using definition_category = typename T::view;
        static constexpr bool is_dynamic = true;

        struct definition_type
        {
            lifecycle_log<T>* log;
            givm::program_entry change;
            givm::program_entry add;
            givm::program_entry remove;
        };
        lifecycle_log<T>* log;

        std::string_view name() const { return "LifecycleEntity"; }
        template<class TView, class TEvent>
        bool can_handle() const
        {
            return (std::is_same_v<TEvent, typename T::regeneration> && log->regeneration_handler_enabled)
                || (std::is_same_v<TEvent, typename T::changed> && log->state_handler_enabled);
        }
        template<class TQuery>
        bool can_query() const { return std::is_same_v<TQuery, typename T::query>; }

        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log,
                context.add_program(std::tuple{ typename T::set{} }),
                context.add_program(std::tuple{ typename T::add{} }),
                context.add_program(std::tuple{ typename T::remove{} }) };
        }
        static typename T::state query(const definition_type& data, const typename T::query&)
        {
            ++data.log->limit_queries;
            return { data.log->limit[0], data.log->limit[1] };
        }
        static givm::program_entry handle(const definition_type& data, const typename T::view& self,
            typename T::regeneration& event, givm::handle_context& context)
        {
            data.log->regenerated.push_back(self.id());
            data.log->regenerated_states.push_back(T::values(event.state));
            if(data.log->behavior == regeneration_behavior::ignore) return {};
            if(data.log->behavior == regeneration_behavior::independent)
                return context.invoke(data.add, typename T::addition{
                    T::owner_of(self), self.definition_id(), event.state });

            const auto previous = T::values(self.state());
            const auto incoming = T::values(event.state);
            const auto combine = [&](std::size_t index)
            {
                return data.log->behavior == regeneration_behavior::accumulate
                    ? previous[index] + incoming[index] : std::max(previous[index], incoming[index]);
            };
            return context.invoke(data.change, typename T::change{ self.id(), { combine(0), combine(1) } });
        }
        static givm::program_entry handle(const definition_type& data, const typename T::view& self,
            typename T::changed& event, givm::handle_context& context)
        {
            data.log->changed.push_back(self.id());
            data.log->previous_states.push_back(T::values(event.previous));
            data.log->current_states.push_back(T::values(event.current));
            CHECK(self.is_valid());
            CHECK(T::values(self.state()) == T::values(event.current));
            if constexpr(std::is_same_v<T, summon_traits>)
            {
                if(self.state().usages == 0)
                {
                    if(data.log->erase_empty)
                        return context.invoke(data.remove, typename T::removal{ self.id() });
                    if(data.log->restore_empty)
                        return context.invoke(data.change, typename T::change{ self.id(), { self.state().value, 1 } });
                }
            }
            else
                if(data.log->erase_empty && self.state().count == 0)
                    return context.invoke(data.remove, typename T::removal{ self.id() });
            return {};
        }
    };

    template<class T>
    struct lifecycle_driver
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            lifecycle_log<T>* log;
            givm::definition_id<typename T::view> entity;
            std::vector<givm::program_entry> entries;
            givm::definition_id<givm::summon_view> other_summon;

        };
        lifecycle_log<T>* log;

        std::string_view name() const { return "LifecycleDriver"; }
        auto summon_dependencies() const
        {
            if constexpr(std::is_same_v<T, summon_traits>)
                return std::array<std::string_view, 2>{ "LifecycleEntity", "OtherSummon" };
            else return std::array<std::string_view, 0>{};
        }
        auto combat_status_dependencies() const
        {
            if constexpr(std::is_same_v<T, combat_status_traits>) return std::array{ std::string_view{ "LifecycleEntity" } };
            else return std::array<std::string_view, 0>{};
        }
        auto attachment_dependencies() const
        {
            if constexpr(std::is_same_v<T, attachment_traits>) return std::array{ std::string_view{ "LifecycleEntity" } };
            else return std::array<std::string_view, 0>{};
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto entity = context.resolve_id<typename T::view>("LifecycleEntity");
            definition_type result{ log, entity, {}, {} };
            if constexpr(std::is_same_v<T, summon_traits>)
                result.other_summon = context.resolve_id<givm::summon_view>("OtherSummon");
            for(const auto& action : log->actions)
            {
                const auto player = action.player == givm::player_id{ 0 }
                    ? givm::relative_player::self : givm::relative_player::opponent;
                auto definition = log->dynamic ? givm::definition_id<typename T::view>{} : entity;
                if constexpr(std::is_same_v<T, summon_traits>)
                    if(action.other_summon && not log->dynamic) definition = result.other_summon;
                const typename T::state state{ action.state[0], action.state[1] };
                switch(action.operation)
                {
                case operation::generate:
                    result.entries.push_back(context.add_program(std::tuple{ typename T::generate{ player, definition, state } }));
                    break;
                case operation::add:
                    result.entries.push_back(context.add_program(std::tuple{ typename T::add{ player, definition, state } }));
                    break;
                case operation::set:
                    result.entries.push_back(context.add_program(std::tuple{ typename T::set{ player, definition, state } }));
                    break;
                case operation::remove:
                    result.entries.push_back(context.add_program(std::tuple{ typename T::remove{ player, definition } }));
                    break;
                case operation::modify:
                    result.entries.push_back(context.add_program(std::tuple{
                        typename T::modify{ player, definition, action.delta[0], action.delta[1] } }));
                    break;
                case operation::modify_twice:
                    result.entries.push_back(context.add_program(std::tuple{
                        typename T::modify{ player, definition, action.delta[0], action.delta[1] },
                        typename T::modify{ player, definition, action.next_delta[0], action.next_delta[1] } }));
                    break;
                }
            }
            return result;
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::test_event&, givm::handle_context& context)
        {
            observe_created(*data.log, context.table());
            const auto index = data.log->next_action++;
            REQUIRE(index < data.log->actions.size());
            const auto& action = data.log->actions[index];
            const auto entry = data.entries[index];
            if(not data.log->dynamic) return context.invoke(entry);
            const typename T::state state{ action.state[0], action.state[1] };
            auto definition = data.entity;
            if constexpr(std::is_same_v<T, summon_traits>)
                if(action.other_summon) definition = data.other_summon;
            if(action.operation == operation::generate)
                return context.invoke(entry, typename T::generation{ T::owner(action.player, action.character_index), definition, state });
            if(action.operation == operation::add)
                return context.invoke(entry, typename T::addition{ T::owner(action.player, action.character_index), definition, state });
            auto entities = T::entities(context.table(), action.player, action.character_index);
            REQUIRE(std::ranges::distance(entities) > static_cast<std::ptrdiff_t>(action.target_index));
            const auto target = (*std::ranges::next(entities.begin(), action.target_index)).id();
            if(action.operation == operation::set)
                return context.invoke(entry, typename T::change{ target, state });
            if(action.operation == operation::remove)
                return context.invoke(entry, typename T::removal{ target });
            if(action.operation == operation::modify_twice)
                return context.invoke(entry,
                    typename T::modification{ target, action.delta[0], action.delta[1] },
                    typename T::modification{ target, action.next_delta[0], action.next_delta[1] });
            return context.invoke(entry, typename T::modification{ target, action.delta[0], action.delta[1] });
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            typename T::removed& event, givm::handle_context& context)
        {
            const auto id = T::removed_id(event);
            const auto departed = context.table()[id];
            CHECK_FALSE(departed.is_valid());
            CHECK(departed.definition_id() == data.entity);
            data.log->departing_states.push_back(T::values(departed.state()));
            data.log->left.push_back(id);
            return {};
        }
    };

    template<class T>
    givm::table run_lifecycle(lifecycle_log<T>& log, givm::compile_mode mode)
    {
        const lifecycle_source<T> entity{ &log };
        const auto driver = givm::test::with_passive_skill(lifecycle_driver<T>{ &log });
        struct target_source : givm::test::named_definition_source<givm::character_view>
        {
            static givm::character_state query(const definition_type&, const givm::character_initial_state&)
            {
                return { .max_health = 10, .health = 10 };
            }
        };
        const target_source character{ { "LifecycleTarget" } };
        const givm::test::named_definition_source<givm::summon_view> other_summon{ "OtherSummon" };
        std::vector<givm::any_command> program;
        for(std::size_t index = 0; index < log.actions.size(); ++index) program.emplace_back(givm::test_command{});
        program.emplace_back(givm::end_game{ givm::game_result::both_loss });
        const auto [library, ids] = givm::test::compile_definitions_with_program(
            mode, program, std::tuple{}, entity, driver, character, other_summon);
        givm::table table{ { .self_player = givm::player_id{ 0 } }, { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
            { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
        load_deck(table, library,
            { .characters = { ids.template get_id<givm::character_view>(driver.name()) } },
            { .characters = { ids.template get_id<givm::character_view>(character.name()),
                ids.template get_id<givm::character_view>(character.name()) } });
        givm::executor executor;
        executor.enter_entry(library);
        auto random = [] { return std::uint32_t{ 0 }; };
        auto state = executor.step(library, table, random);
        while(state == givm::execution_state::active_character_changed)
            state = executor.step(library, table, random);
        REQUIRE(state == givm::execution_state::finished);
        CHECK(log.next_action == log.actions.size());
        CHECK(log.limit_queries == 1);
        observe_created(log, table);
        return table;
    }

    template<class T>
    auto ids_of(const givm::table& table, givm::player_id player = givm::player_id{ 1 })
    {
        std::vector<typename T::id> result;
        for(const auto entity : T::entities(table, player)) result.push_back(entity.id());
        return result;
    }
}


TEMPLATE_TEST_CASE("generation defaults to limits and repeated generation selects one existing entity",
    "[entity-generation]", summon_traits, combat_status_traits, attachment_traits)
{
    using T = TestType;
    lifecycle_log<T> log;
    log.dynamic = GENERATE(false, true);
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    log.actions = { { operation::generate }, { operation::add, { 7, 8 } }, { operation::generate } };
    const auto table = run_lifecycle(log, mode);
    REQUIRE(log.created.size() == 2);
    CHECK(ids_of<T>(table) == log.created);
    CHECK(log.regenerated == std::vector{ log.created[0] });
    CHECK(log.regenerated_states == std::vector{ log.limit });
    CHECK(log.changed.empty());
    CHECK(T::values(table[log.created[0]].state()) == log.limit);
    CHECK(T::values(table[log.created[1]].state()) == state_values{ 7, 8 });
    CHECK(ids_of<T>(table, givm::player_id{ 0 }).empty());
}

TEMPLATE_TEST_CASE("generation addition and assignment clamp each supplied field to its definition limit",
    "[entity-generation][entity-state]", summon_traits, combat_status_traits, attachment_traits)
{
    using T = TestType;
    lifecycle_log<T> log;
    log.dynamic = GENERATE(false, true);
    const auto op = GENERATE(operation::generate, operation::add, operation::set);
    const auto supplied = GENERATE(state_values{ maximum, 7 }, state_values{ 9, maximum }, state_values{ maximum, maximum });
    if(op == operation::set) log.actions.push_back({ operation::add, { 2, 3 } });
    log.actions.push_back({ op, supplied });
    const auto table = run_lifecycle(log, givm::compile_mode::normal);
    REQUIRE(log.created.size() == 1);
    const state_values expected{ std::min(supplied[0], log.limit[0]), std::min(supplied[1], log.limit[1]) };
    CHECK(T::values(table[log.created[0]].state()) == expected);
    CHECK(log.regenerated.empty());
    if(op == operation::set)
    {
        CHECK(log.changed == log.created);
        CHECK(log.previous_states == std::vector<state_values>{ { 2, 3 } });
        CHECK(log.current_states == std::vector{ expected });
    }
    else CHECK(log.changed.empty());
}

TEMPLATE_TEST_CASE("missing entity handlers preserve repeated generation and permit state writes",
    "[entity-generation][entity-state]", summon_traits, combat_status_traits, attachment_traits)
{
    using T = TestType;
    lifecycle_log<T> log;
    log.dynamic = GENERATE(false, true);
    log.regeneration_handler_enabled = false;
    log.state_handler_enabled = false;
    log.actions = { { operation::generate }, { operation::generate, { 9, 2 } },
        { operation::set, { 7, 8 } }, { .operation = operation::modify, .delta = { -2, 3 } } };
    const auto table = run_lifecycle(log, givm::compile_mode::normal);
    REQUIRE(log.created.size() == 1);
    CHECK(T::values(table[log.created[0]].state()) == state_values{ 5, 11 });
    CHECK(log.regenerated.empty());
    CHECK(log.changed.empty());
    CHECK(log.left.empty());
}

TEMPLATE_TEST_CASE("repeated generation delegates accumulation refresh and independent addition to one entity",
    "[entity-generation]", summon_traits, combat_status_traits, attachment_traits)
{
    using T = TestType;
    lifecycle_log<T> log;
    log.dynamic = GENERATE(false, true);
    log.behavior = GENERATE(regeneration_behavior::accumulate, regeneration_behavior::refresh, regeneration_behavior::independent);
    log.actions = { { operation::add, { 7, 1 } }, { operation::add, { 9, 9 } }, { operation::generate, { 2, 3 } } };
    const auto table = run_lifecycle(log, givm::compile_mode::normal);
    REQUIRE(log.created.size() == (log.behavior == regeneration_behavior::independent ? 3u : 2u));
    CHECK(ids_of<T>(table) == log.created);
    CHECK(log.regenerated == std::vector{ log.created[0] });
    CHECK(log.regenerated_states == std::vector<state_values>{ { 2, 3 } });
    CHECK(T::values(table[log.created[1]].state()) == state_values{ 9, 9 });
    if(log.behavior == regeneration_behavior::independent)
    {
        CHECK(T::values(table[log.created[0]].state()) == state_values{ 7, 1 });
        CHECK(T::values(table[log.created[2]].state()) == state_values{ 2, 3 });
        CHECK(log.changed.empty());
    }
    else
    {
        const state_values expected = log.behavior == regeneration_behavior::accumulate ? state_values{ 9, 4 } : state_values{ 7, 3 };
        CHECK(T::values(table[log.created[0]].state()) == expected);
        CHECK(log.changed == std::vector{ log.created[0] });
        CHECK(log.previous_states == std::vector<state_values>{ { 7, 1 } });
        CHECK(log.current_states == std::vector{ expected });
    }
}

TEMPLATE_TEST_CASE("repeated generation clamps supplied state before notifying the existing entity",
    "[entity-generation]", summon_traits, combat_status_traits, attachment_traits)
{
    using T = TestType;
    lifecycle_log<T> log;
    log.dynamic = GENERATE(false, true);
    log.actions = { { operation::add, { 7, 8 } }, { operation::generate, { maximum, 4 } } };
    const auto table = run_lifecycle(log, givm::compile_mode::normal);
    REQUIRE(log.created.size() == 1);
    CHECK(log.regenerated_states == std::vector<state_values>{ { 20, 4 } });
    CHECK(T::values(table[log.created[0]].state()) == state_values{ 7, 8 });
}

TEMPLATE_TEST_CASE("generation after removal creates a new identity and departure state remains readable",
    "[entity-generation]", summon_traits, combat_status_traits, attachment_traits)
{
    using T = TestType;
    lifecycle_log<T> log;
    log.dynamic = GENERATE(false, true);
    log.actions = { { operation::generate }, { operation::remove }, { operation::generate } };
    const auto table = run_lifecycle(log, givm::compile_mode::normal);
    REQUIRE(log.created.size() == 2);
    CHECK(log.created[0] != log.created[1]);
    CHECK(ids_of<T>(table) == std::vector{ log.created[1] });
    CHECK(log.regenerated.empty());
    CHECK(log.left == std::vector{ log.created[0] });
    CHECK(log.departing_states == std::vector{ log.limit });
    CHECK_FALSE(table[log.created[0]].is_valid());
}

TEMPLATE_TEST_CASE("generation resolves existing entities within the requested player",
    "[entity-generation]", summon_traits, combat_status_traits, attachment_traits)
{
    using T = TestType;
    lifecycle_log<T> log;
    log.dynamic = GENERATE(false, true);
    log.actions = { { .operation = operation::generate, .player = givm::player_id{ 0 } },
        { operation::generate }, { operation::generate } };
    const auto table = run_lifecycle(log, givm::compile_mode::normal);
    REQUIRE(log.created.size() == 2);
    CHECK(ids_of<T>(table, givm::player_id{ 0 }) == std::vector{ log.created[0] });
    CHECK(ids_of<T>(table) == std::vector{ log.created[1] });
    CHECK(log.regenerated == std::vector{ log.created[1] });
}

TEST_CASE("dynamic attachment application and modification can select a standby character", "[entity-generation][attachment]")
{
    lifecycle_log<attachment_traits> log;
    log.dynamic = true;
    log.actions = { { operation::generate },
        { .operation = operation::generate, .character_index = 1 },
        { .operation = operation::generate, .character_index = 1 },
        { .operation = operation::modify, .character_index = 1, .delta = { -11, -29 } } };
    const auto table = run_lifecycle(log, givm::compile_mode::normal);
    REQUIRE(log.created.size() == 2);
    CHECK(ids_of<attachment_traits>(table) == std::vector{ log.created[0] });
    CHECK(log.regenerated == std::vector{ log.created[1] });
    CHECK(log.changed == std::vector{ log.created[1] });
    CHECK(table[log.created[1]].character().id() == givm::character_id{ givm::player_id{ 1 }, 1 });
    CHECK(table[log.created[1]].state().count == 9);
    CHECK(table[log.created[1]].state().round_usages == 1);
    CHECK(table[givm::player_id{ 1 }].state().active_character == givm::character_id{ givm::player_id{ 1 }, 0 });
}

TEMPLATE_TEST_CASE("assignment and modification notify only the selected entity with previous and current state",
    "[entity-state]", summon_traits, combat_status_traits, attachment_traits)
{
    using T = TestType;
    lifecycle_log<T> log;
    log.dynamic = GENERATE(false, true);
    const auto target_index = log.dynamic ? 1uz : 0uz;
    log.actions = { { operation::add, { 7, 8 } }, { operation::add, { 7, 8 } },
        { operation::set, { 9, 4 }, givm::player_id{ 1 }, target_index },
        { .operation = operation::modify, .target_index = target_index, .delta = { -2, 0 } } };
    const auto table = run_lifecycle(log, givm::compile_mode::normal);
    REQUIRE(log.created.size() == 2);
    CHECK(ids_of<T>(table) == log.created);
    CHECK(log.changed == std::vector{ log.created[target_index], log.created[target_index] });
    CHECK(log.previous_states == std::vector<state_values>{ { 7, 8 }, { 9, 4 } });
    CHECK(log.current_states == std::vector<state_values>{ { 9, 4 }, { 7, 4 } });
    CHECK(T::values(table[log.created[target_index]].state()) == state_values{ 7, 4 });
    CHECK(T::values(table[log.created[1 - target_index]].state()) == state_values{ 7, 8 });
}

TEMPLATE_TEST_CASE("multiple delta commands in one response read state when each command executes",
    "[entity-state][program-input]", summon_traits, combat_status_traits, attachment_traits)
{
    using T = TestType;
    lifecycle_log<T> log;
    log.dynamic = GENERATE(false, true);
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    log.actions = { { operation::add, { 7, 8 } },
        { .operation = operation::modify_twice, .delta = { 2, 0 }, .next_delta = { 0, -3 } },
        { .operation = operation::modify_twice, .delta = { 3, 1 }, .next_delta = { 2, 2 } } };
    const auto table = run_lifecycle(log, mode);
    REQUIRE(log.created.size() == 1);
    CHECK(log.previous_states == std::vector<state_values>{ { 7, 8 }, { 9, 8 }, { 9, 5 }, { 12, 6 } });
    CHECK(log.current_states == std::vector<state_values>{ { 9, 8 }, { 9, 5 }, { 12, 6 }, { 14, 8 } });
    CHECK(T::values(table[log.created[0]].state()) == state_values{ 14, 8 });
}

TEMPLATE_TEST_CASE("signed deltas saturate without overflow at zero and definition limits",
    "[entity-state]", summon_traits, combat_status_traits, attachment_traits)
{
    using T = TestType;
    lifecycle_log<T> log;
    log.dynamic = GENERATE(false, true);
    const auto upper = GENERATE(std::int64_t{ 100 }, std::numeric_limits<std::int64_t>::max());
    const auto lower = GENERATE(std::int64_t{ -100 }, std::numeric_limits<std::int64_t>::min());
    log.actions = { { operation::add, { 7, 8 } },
        { .operation = operation::modify, .delta = { upper, upper } },
        { .operation = operation::modify, .delta = { lower, 0 } } };
    const auto table = run_lifecycle(log, givm::compile_mode::normal);
    REQUIRE(log.created.size() == 1);
    CHECK(log.previous_states == std::vector<state_values>{ { 7, 8 }, { 20, 30 } });
    CHECK(log.current_states == std::vector<state_values>{ { 20, 30 }, { 0, 30 } });
    CHECK(T::values(table[log.created[0]].state()) == state_values{ 0, 30 });
}

TEMPLATE_TEST_CASE("signed deltas handle the full unsigned state range",
    "[entity-state]", summon_traits, combat_status_traits, attachment_traits)
{
    using T = TestType;
    lifecycle_log<T> log;
    log.dynamic = GENERATE(false, true);
    log.limit = { maximum, maximum };
    log.actions = { { operation::generate },
        { .operation = operation::modify, .delta = { -1, -1 } },
        { .operation = operation::modify, .delta = { std::numeric_limits<std::int64_t>::max(), std::numeric_limits<std::int64_t>::max() } } };
    const auto table = run_lifecycle(log, givm::compile_mode::normal);
    REQUIRE(log.created.size() == 1);
    CHECK(log.current_states == std::vector<state_values>{ { maximum - 1, maximum - 1 }, { maximum, maximum } });
    CHECK(T::values(table[log.created[0]].state()) == log.limit);
}

TEMPLATE_TEST_CASE("zero state remains unless the entity response removes it",
    "[entity-state]", summon_traits, combat_status_traits, attachment_traits)
{
    using T = TestType;
    lifecycle_log<T> log;
    log.dynamic = GENERATE(false, true);
    log.erase_empty = GENERATE(false, true);
    const auto op = GENERATE(operation::set, operation::modify);
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    log.actions = { { operation::generate },
        { .operation = op, .state = { 0, 0 }, .delta = { std::numeric_limits<std::int64_t>::min(), std::numeric_limits<std::int64_t>::min() } } };
    const auto table = run_lifecycle(log, mode);
    REQUIRE(log.created.size() == 1);
    CHECK(log.changed == log.created);
    CHECK(log.previous_states == std::vector{ log.limit });
    CHECK(log.current_states == std::vector<state_values>{ { 0, 0 } });
    CHECK(ids_of<T>(table).size() == (log.erase_empty ? 0u : 1u));
    if(log.erase_empty)
    {
        CHECK(log.left == log.created);
        CHECK(log.departing_states == std::vector<state_values>{ { 0, 0 } });
    }
    else
    {
        CHECK(log.left.empty());
        CHECK(T::values(table[log.created[0]].state()) == state_values{ 0, 0 });
    }
}

TEST_CASE("summons without a state handler remain available for resummoning at zero usages", "[entity-state][summon]")
{
    lifecycle_log<summon_traits> log;
    log.dynamic = GENERATE(false, true);
    log.state_handler_enabled = false;
    const auto op = GENERATE(operation::set, operation::modify);
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    log.actions = { { operation::generate },
        { .operation = op, .state = { 7, 0 }, .delta = { -13, std::numeric_limits<std::int64_t>::min() } },
        { operation::generate } };
    const auto table = run_lifecycle(log, mode);
    REQUIRE(log.created.size() == 1);
    CHECK(ids_of<summon_traits>(table) == log.created);
    CHECK(summon_traits::values(table[log.created[0]].state()) == state_values{ 7, 0 });
    CHECK(log.changed.empty());
    CHECK(log.regenerated == log.created);
    CHECK(log.left.empty());
    CHECK(log.departing_states.empty());
}

TEST_CASE("a summon can restore usages in its state response before the next command executes", "[entity-state][summon]")
{
    lifecycle_log<summon_traits> log;
    log.dynamic = GENERATE(false, true);
    log.restore_empty = true;
    const auto op = GENERATE(operation::set, operation::modify);
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    log.actions = { { operation::generate },
        { .operation = op, .state = { 7, 0 }, .delta = { -13, -30 } },
        { .operation = operation::modify_twice, .delta = { 0, -1 }, .next_delta = { 2, 3 } } };
    const auto table = run_lifecycle(log, mode);
    REQUIRE(log.created.size() == 1);
    CHECK(ids_of<summon_traits>(table) == log.created);
    CHECK(log.changed == std::vector(5, log.created[0]));
    CHECK(log.previous_states == std::vector<state_values>{ log.limit, { 7, 0 }, { 7, 1 }, { 7, 0 }, { 7, 1 } });
    CHECK(log.current_states == std::vector<state_values>{ { 7, 0 }, { 7, 1 }, { 7, 0 }, { 7, 1 }, { 9, 4 } });
    CHECK(summon_traits::values(table[log.created[0]].state()) == state_values{ 9, 4 });
    CHECK(log.left.empty());
}

TEST_CASE("summon creation accepts zero usages including a zero definition limit", "[entity-generation][summon]")
{
    lifecycle_log<summon_traits> log;
    log.dynamic = GENERATE(false, true);
    const auto op = GENERATE(operation::generate, operation::add);
    const auto zero_limit = GENERATE(false, true);
    if(zero_limit) log.limit[1] = 0;
    log.actions = { { op, { 7, zero_limit ? maximum : 0 } } };
    const auto table = run_lifecycle(log, givm::compile_mode::normal);
    REQUIRE(log.created.size() == 1);
    CHECK(ids_of<summon_traits>(table) == log.created);
    CHECK(summon_traits::values(table[log.created[0]].state()) == state_values{ 7, 0 });
    CHECK(log.changed.empty());
    CHECK(log.left.empty());
}

TEMPLATE_TEST_CASE("entity limit queries cache source results and default to the unsigned maximum",
    "[definition][query][entity-generation]", summon_traits, combat_status_traits, attachment_traits)
{
    using T = TestType;
    STATIC_REQUIRE(std::is_empty_v<typename T::query>);
    STATIC_REQUIRE(std::is_same_v<typename T::query::result_t, typename T::state>);
    lifecycle_log<T> log;
    const lifecycle_source<T> source{ &log };
    const givm::test::named_definition_source<typename T::view> defaults{ "DefaultEntity" };
    const auto [library, ids] = givm::test::compile_definitions(source, defaults);
    const auto id = ids.template get_id<typename T::view>(source.name());
    const auto default_id = ids.template get_id<typename T::view>(defaults.name());
    REQUIRE(log.limit_queries == 1);
    CHECK(T::values(library.query(id, typename T::query{})) == log.limit);
    auto copied = library;
    auto moved = std::move(copied);
    CHECK(T::values(moved[id].query(typename T::query{})) == log.limit);
    CHECK(T::values(library[default_id].query(typename T::query{})) == state_values{ maximum, maximum });
    CHECK(log.limit_queries == 1);
    CHECK(T::values(typename T::generate{}.state) == state_values{ maximum, maximum });
    CHECK(T::values(typename T::generation{}.state) == state_values{ maximum, maximum });
    CHECK(T::values(typename T::add{}.state) == state_values{ maximum, maximum });
    CHECK(T::values(typename T::addition{}.state) == state_values{ maximum, maximum });
    CHECK(T::values(typename T::state{}) == state_values{ 0, 0 });
    if constexpr(std::is_same_v<T, summon_traits>)
    {
        STATIC_REQUIRE(std::is_same_v<decltype(T::modify::value), std::int64_t>);
        STATIC_REQUIRE(std::is_same_v<decltype(T::modification::usages), std::int64_t>);
        CHECK(typename T::modify{}.value == 0);
        CHECK(typename T::modify{}.usages == 0);
        CHECK(typename T::modification{}.value == 0);
        CHECK(typename T::modification{}.usages == 0);
    }
    else
    {
        STATIC_REQUIRE(std::is_same_v<decltype(T::modify::count), std::int64_t>);
        STATIC_REQUIRE(std::is_same_v<decltype(T::modification::round_usages), std::int64_t>);
        CHECK(typename T::modify{}.count == 0);
        CHECK(typename T::modify{}.round_usages == 0);
        CHECK(typename T::modification{}.count == 0);
        CHECK(typename T::modification{}.round_usages == 0);
    }
}


TEST_CASE("summon capacity defaults to four and blocks new entities independently for each player", "[summon][entity-generation]")
{
    lifecycle_log<summon_traits> log;
    log.dynamic = GENERATE(false, true);
    for(std::size_t index = 0; index < 4; ++index)
        log.actions.push_back({ operation::add, { 2, 3 } });
    log.actions.push_back({ operation::add, { 7, 8 } });
    log.actions.push_back({ .operation = operation::generate, .other_summon = true });
    log.actions.push_back({ .operation = operation::add, .state = { 7, 8 }, .other_summon = true });
    log.actions.push_back({ .operation = operation::generate, .player = givm::player_id{ 0 } });
    for(std::size_t index = 0; index < 4; ++index)
        log.actions.push_back({ operation::add, { 2, 3 }, givm::player_id{ 0 } });

    const auto table = run_lifecycle(log, givm::compile_mode::normal);
    CHECK(givm::player_state{}.summon_limit == 4);
    REQUIRE(log.created.size() == 8);
    for(const auto player : { givm::player_id{ 0 }, givm::player_id{ 1 } })
    {
        CHECK(table[player].state().summon_limit == 4);
        CHECK(ids_of<summon_traits>(table, player).size() == 4);
    }
    for(const auto id : ids_of<summon_traits>(table))
        CHECK(summon_traits::values(table[id].state()) == state_values{ 2, 3 });
    CHECK(log.regenerated.empty());
    CHECK(log.changed.empty());
    CHECK(log.left.empty());
}

TEST_CASE("a full summon area still dispatches resummoning but its response cannot add beyond capacity", "[summon][entity-generation]")
{
    lifecycle_log<summon_traits> log;
    log.dynamic = GENERATE(false, true);
    log.behavior = GENERATE(regeneration_behavior::refresh, regeneration_behavior::independent);
    for(std::size_t index = 0; index < 4; ++index)
        log.actions.push_back({ operation::add, { 7, 1 } });
    log.actions.push_back({ operation::generate, { 9, 8 } });

    const auto table = run_lifecycle(log, givm::compile_mode::normal);
    REQUIRE(log.created.size() == 4);
    CHECK(ids_of<summon_traits>(table) == log.created);
    CHECK(log.regenerated == std::vector{ log.created[0] });
    CHECK(log.regenerated_states == std::vector<state_values>{ { 9, 8 } });
    const state_values expected = log.behavior == regeneration_behavior::refresh
        ? state_values{ 9, 8 } : state_values{ 7, 1 };
    CHECK(summon_traits::values(table[log.created[0]].state()) == expected);
    for(std::size_t index = 1; index < log.created.size(); ++index)
        CHECK(summon_traits::values(table[log.created[index]].state()) == state_values{ 7, 1 });
    if(log.behavior == regeneration_behavior::refresh)
        CHECK(log.changed == std::vector{ log.created[0] });
    else
        CHECK(log.changed.empty());
    CHECK(log.left.empty());
}

TEST_CASE("removing a summon releases capacity while its departure information remains readable", "[summon][entity-generation]")
{
    lifecycle_log<summon_traits> log;
    log.dynamic = GENERATE(false, true);
    for(std::size_t index = 0; index < 4; ++index)
        log.actions.push_back({ operation::add, { 2, 3 } });
    log.actions.push_back({ operation::remove });
    log.actions.push_back({ .operation = operation::generate, .state = { 7, 8 }, .other_summon = true });
    log.actions.push_back({ .operation = operation::add, .state = { 9, 10 }, .other_summon = true });

    const auto table = run_lifecycle(log, givm::compile_mode::normal);
    REQUIRE(log.created.size() == 5);
    CHECK(log.left == std::vector{ log.created[0] });
    CHECK_FALSE(table[log.created[0]].is_valid());
    CHECK(summon_traits::values(table[log.created[0]].state()) == state_values{ 2, 3 });
    CHECK(ids_of<summon_traits>(table) == std::vector{ log.created[1], log.created[2], log.created[3], log.created[4] });
    CHECK(table[log.created[4]].definition_id() != table[log.created[0]].definition_id());
    CHECK(summon_traits::values(table[log.created[4]].state()) == state_values{ 7, 8 });
    CHECK(log.regenerated.empty());
    CHECK(log.changed.empty());
}
