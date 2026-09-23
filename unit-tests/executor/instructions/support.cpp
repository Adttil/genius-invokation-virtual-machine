#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <ranges>
#include <span>
#include <string_view>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/executor.hpp>

#include "../../table/test_definition_library.hpp"

namespace
{
    using values = std::array<std::uint32_t, 2>;
    enum class operation { add, set, modify, remove };
    struct action
    {
        operation kind;
        givm::support_state state{};
        givm::player_id player{ 0 };
        std::size_t target_index = 0;
        std::int64_t count = 0;
        std::int64_t round_usages = 0;
    };
    struct support_log
    {
        std::vector<action> actions;
        bool dynamic = false;
        bool refill_on_removal = false;
        bool remove_empty = false;
        std::size_t next_action = 0;
        std::size_t limit_queries = 0;
        std::vector<givm::support_id> changed;
        std::vector<values> previous;
        std::vector<values> current;
        std::vector<givm::support_id> removed;
        std::vector<givm::player_id> removal_observers;
        std::vector<std::size_t> remaining_on_removal;
    };

    struct support_source
    {
        using definition_category = givm::support_view;
        struct definition_type { support_log* log; givm::program_entry remove; };
        std::string_view source_name;
        support_log* log;
        std::string_view name() const { return source_name; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, log->remove_empty ? context.add_program(std::tuple{
                givm::replace_cards{ givm::player_id{ 0 } }, givm::remove_support{} }) : givm::program_entry{} };
        }
        static givm::support_state query(const definition_type& data, const givm::support_state_limit&)
        {
            ++data.log->limit_queries;
            return { 0xff, 3 };
        }
        static givm::program_entry handle(const definition_type& data, const givm::support_view& self,
            givm::support_state_changed& event, givm::handle_context& context)
        {
            data.log->changed.push_back(self.id());
            data.log->previous.push_back({ event.previous.count, event.previous.round_usages });
            data.log->current.push_back({ event.current.count, event.current.round_usages });
            CHECK(self.state().count == event.current.count);
            CHECK(self.state().round_usages == event.current.round_usages);
            if(data.remove && event.current.count == 0)
                return context.invoke(data.remove, givm::support_removal{ self.id() });
            return {};
        }
    };

    struct support_driver
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            support_log* log;
            givm::definition_id<givm::support_view> support;
            std::vector<givm::program_entry> actions;
            givm::program_entry refill;
        };
        support_log* log;
        std::string_view name() const { return "SupportDriver"; }
        auto support_dependencies() const { return std::array<std::string_view, 2>{ "SupportA", "SupportB" }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto support = context.resolve_id<givm::support_view>("SupportA");
            const auto replacement = context.resolve_id<givm::support_view>("SupportB");
            definition_type result{ log, support, {}, {} };
            for(const auto& action : log->actions)
            {
                const auto player = action.player == givm::player_id{ 0 }
                    ? givm::relative_player::self : givm::relative_player::opponent;
                const auto definition = log->dynamic ? givm::definition_id<givm::support_view>{} : support;
                switch(action.kind)
                {
                case operation::add:
                    result.actions.push_back(context.add_program(std::tuple{ givm::add_support{ player, definition, action.state } }));
                    break;
                case operation::set:
                    result.actions.push_back(context.add_program(std::tuple{ givm::set_support_state{ player, definition, action.state } }));
                    break;
                case operation::modify:
                    result.actions.push_back(context.add_program(std::tuple{
                        givm::modify_support_state{ player, definition, action.count, action.round_usages } }));
                    break;
                case operation::remove:
                    result.actions.push_back(context.add_program(std::tuple{ givm::remove_support{ player, definition } }));
                    break;
                }
            }
            if(log->refill_on_removal)
                result.refill = context.add_program(std::tuple{
                    givm::replace_cards{ givm::player_id{ 0 } },
                    givm::add_support{ .definition = replacement, .state = { 99, 2 } } });
            return result;
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view& self,
            givm::test_event&, givm::handle_context& context)
        {
            if(self.player().id() != givm::player_id{ 0 }) return {};
            const auto index = data.log->next_action++;
            REQUIRE(index < data.log->actions.size());
            const auto& action = data.log->actions[index];
            if(not data.log->dynamic) return context.invoke(data.actions[index]);
            if(action.kind == operation::add)
                return context.invoke(data.actions[index], givm::support_addition{ action.player, data.support, action.state });
            auto supports = context.table()[action.player].supports();
            REQUIRE(std::ranges::distance(supports) > static_cast<std::ptrdiff_t>(action.target_index));
            const auto target = (*std::ranges::next(supports.begin(), action.target_index)).id();
            if(action.kind == operation::set)
                return context.invoke(data.actions[index], givm::support_state_change{ target, action.state });
            if(action.kind == operation::modify)
                return context.invoke(data.actions[index], givm::support_state_modification{ target, action.count, action.round_usages });
            return context.invoke(data.actions[index], givm::support_removal{ target });
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view& self,
            givm::support_removed& event, givm::handle_context& context)
        {
            const auto removed = context.table()[event.support];
            CHECK_FALSE(removed.is_valid());
            CHECK(removed.definition_id() == data.support);
            data.log->removed.push_back(event.support);
            data.log->removal_observers.push_back(self.player().id());
            data.log->remaining_on_removal.push_back(std::ranges::distance(context.table()[event.support.player_id].supports()));
            if(data.refill && self.player().id() == givm::player_id{ 0 })
                return context.invoke(data.refill);
            return {};
        }
    };

    struct support_card
    {
        using definition_category = givm::card_definition;
        struct definition_type { givm::program_entry add; givm::program_entry replace; };
        std::string_view name() const { return "SupportCard"; }
        auto support_dependencies() const { return std::array{ std::string_view{ "SupportB" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const givm::add_support add{ .definition = context.resolve_id<givm::support_view>("SupportB"), .state = { 17, 1 } };
            return { context.add_program(std::tuple{ add }), context.add_program(std::tuple{ givm::remove_support{}, add }) };
        }
        static givm::card_state query(const definition_type&, const givm::card_initial_state&)
        {
            return { .cost = { .speed = givm::action_speed::fast } };
        }
        static givm::target_validation query(const definition_type&, const givm::card_target_validation& query)
        {
            const auto player = query.card.player().id();
            const bool full = std::ranges::distance(query.table[player].supports()) >= query.table[player].state().support_limit;
            if(query.target_count == 0)
                return full ? givm::target_validation::valid_incomplete : givm::target_validation::valid_complete;
            const auto* support = std::get_if<givm::support_id>(&query.targets[0]);
            return full && query.target_count == 1 && support && support->player_id == player && query.table[*support].is_valid()
                ? givm::target_validation::valid_complete : givm::target_validation::invalid;
        }
        static givm::program_entry handle(const definition_type& data, const givm::hand_card_view& self,
            givm::card_effect& event, givm::handle_context& context)
        {
            const auto player = self.player().id();
            if(std::ranges::distance(context.table()[player].supports()) >= context.table()[player].state().support_limit)
                return context.invoke(data.replace, givm::support_removal{ std::get<givm::support_id>(event.targets[0]) });
            return context.invoke(data.add);
        }
    };

    auto compile_scenario(support_log& log, givm::compile_mode mode, bool play_card = false)
    {
        const auto driver = givm::test::with_passive_skill(support_driver{ &log });
        const support_source a{ "SupportA", &log }, b{ "SupportB", &log };
        std::vector<givm::any_command> commands;
        for(std::size_t index = 0; index < log.actions.size(); ++index) commands.emplace_back(givm::test_command{});
        if(play_card)
        {
            commands.emplace_back(givm::draw_cards{ .count = 1 });
            commands.emplace_back(givm::begin_action{});
        }
        commands.emplace_back(givm::end_game{ givm::game_result::both_loss });
        return givm::test::compile_definitions_with_program(mode, commands, std::tuple{}, driver, a, b, support_card{});
    }

    void load_scenario(givm::table& table, const givm::definition_library& library, const givm::issued_id_map& ids)
    {
        const auto character = ids.get_id<givm::character_view>("SupportDriver");
        load_deck(table, library,
            { .cards = { ids.get_id<givm::card_definition>("SupportCard") }, .characters = { character } },
            { .characters = { character } });
    }
    givm::execution_state advance(givm::executor& executor, const givm::definition_library& library, givm::table& table)
    {
        auto random = [] { return std::uint32_t{ 0 }; };
        for(;;)
        {
            const auto state = executor.step(library, table, random);
            if(state != givm::execution_state::active_character_changed && state != givm::execution_state::action_started)
                return state;
        }
    }
    std::vector<givm::support_id> support_ids(const givm::table& table, givm::player_id player = givm::player_id{ 0 })
    {
        std::vector<givm::support_id> result;
        for(const auto support : table[player].supports()) result.push_back(support.id());
        return result;
    }
}

TEST_CASE("support capacity counts duplicate definitions and removal releases a place", "[support][entity-generation]")
{
    support_log log;
    log.dynamic = GENERATE(false, true);
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    for(std::size_t index = 0; index < 5; ++index) log.actions.push_back({ operation::add, { 7, 1 } });
    log.actions.push_back({ .kind = operation::remove });
    log.actions.push_back({ operation::add, { 9, 2 } });
    log.actions.push_back({ operation::add, { 12, 3 } });
    log.actions.push_back({ operation::add, { std::numeric_limits<std::uint32_t>::max(),
        std::numeric_limits<std::uint32_t>::max() }, givm::player_id{ 1 } });
    const auto [library, ids] = compile_scenario(log, mode);
    givm::table table{ { .self_player = givm::player_id{ 0 } }, { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_scenario(table, library, ids);
    givm::executor executor;
    executor.enter_entry(library);
    REQUIRE(advance(executor, library, table) == givm::execution_state::finished);
    CHECK(givm::player_state{}.support_limit == 4);
    const auto remaining = support_ids(table);
    REQUIRE(remaining.size() == 4);
    CHECK(table[remaining.back()].state().count == 9);
    for(std::size_t index = 0; index < 3; ++index) CHECK(table[remaining[index]].state().count == 7);
    REQUIRE(log.removed.size() == 2);
    CHECK(log.removed[0] == log.removed[1]);
    CHECK_FALSE(table[log.removed[0]].is_valid());
    CHECK(table[log.removed[0]].state().count == 7);
    CHECK(log.removal_observers == std::vector{ givm::player_id{ 0 }, givm::player_id{ 1 } });
    CHECK(log.remaining_on_removal == std::vector<std::size_t>{ 3, 3 });
    REQUIRE(support_ids(table, givm::player_id{ 1 }).size() == 1);
    const auto opponent_support = table[support_ids(table, givm::player_id{ 1 })[0]];
    CHECK(opponent_support.state().count == 255);
    CHECK(opponent_support.state().round_usages == 3);
    CHECK(log.changed.empty());
}

TEST_CASE("support state changes preserve packed dice and saturate without deleting zero", "[support][entity-state]")
{
    support_log log;
    log.dynamic = GENERATE(false, true);
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const auto target_index = log.dynamic ? 1uz : 0uz;
    constexpr std::uint32_t packed = 2 | (3 << 2) | (7 << 5);
    log.actions = { { operation::add, { 7, 2 } }, { operation::add, { 7, 2 } },
        { .kind = operation::set, .state = { packed, 1 }, .target_index = target_index },
        { .kind = operation::modify, .target_index = target_index,
            .count = std::numeric_limits<std::int64_t>::max(), .round_usages = std::numeric_limits<std::int64_t>::max() },
        { .kind = operation::modify, .target_index = target_index,
            .count = std::numeric_limits<std::int64_t>::min(), .round_usages = std::numeric_limits<std::int64_t>::min() } };
    const auto [library, ids] = compile_scenario(log, mode);
    REQUIRE(log.limit_queries == 2);
    const auto limit = library.query(ids.get_id<givm::support_view>("SupportA"), givm::support_state_limit{});
    CHECK(limit.count == 255);
    CHECK(limit.round_usages == 3);
    givm::table table{ { .self_player = givm::player_id{ 0 } }, { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_scenario(table, library, ids);
    givm::executor executor;
    executor.enter_entry(library);
    REQUIRE(advance(executor, library, table) == givm::execution_state::finished);
    const auto supports = support_ids(table);
    REQUIRE(supports.size() == 2);
    CHECK(log.changed == std::vector{ supports[target_index], supports[target_index], supports[target_index] });
    CHECK(log.previous == std::vector<values>{ { 7, 2 }, { packed, 1 }, { 255, 3 } });
    CHECK(log.current == std::vector<values>{ { packed, 1 }, { 255, 3 }, { 0, 0 } });
    CHECK((log.current[0][0] & 3) == 2);
    CHECK(((log.current[0][0] >> 2) & 7) == 3);
    CHECK(((log.current[0][0] >> 5) & 7) == 7);
    CHECK(table[supports[target_index]].state().count == 0);
    CHECK(table[supports[1 - target_index]].state().count == 7);
    CHECK(log.removed.empty());
    CHECK(log.limit_queries == 2);
}

TEST_CASE("support cards select a replacement only when full and wait for its removal programs", "[support][play_card]")
{
    const bool full = GENERATE(false, true);
    const bool refill = GENERATE(false, true);
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    support_log log{ .refill_on_removal = refill };
    for(std::size_t index = 0; index < (full ? 4uz : 3uz); ++index)
        log.actions.push_back({ operation::add, { 7, 1 } });
    log.actions.push_back({ operation::add, { 8, 1 }, givm::player_id{ 1 } });
    const auto [library, ids] = compile_scenario(log, mode, true);
    givm::table table{ { .self_player = givm::player_id{ 0 } }, { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_scenario(table, library, ids);
    givm::executor executor;
    executor.enter_entry(library);
    REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
    const auto before = support_ids(table);
    const auto action = executor.view_in<givm::execution_state::action_selection>();
    REQUIRE(action.card_count() == 1);
    CHECK(action.card_targets_validate(library, table, 0) == (full
        ? givm::target_validation::valid_incomplete : givm::target_validation::valid_complete));
    const std::array<givm::card_target_id, 1> selected{ before[1] };
    const std::array<givm::card_target_id, 1> opponent{ support_ids(table, givm::player_id{ 1 })[0] };
    CHECK(action.card_targets_validate(library, table, 0, opponent) == givm::target_validation::invalid);
    CHECK(action.card_targets_validate(library, table, 0, selected) == (full
        ? givm::target_validation::valid_complete : givm::target_validation::invalid));
    action.calculate_card_cost(library, table, 0);
    REQUIRE(action.card_payment_validate(table, 0, {}) == givm::card_payment_validation::valid);
    if(full) action.play_card(0, {}, selected);
    else action.play_card(0, {});
    const auto check_result = [&](const givm::table& current)
    {
        const auto after = support_ids(current);
        REQUIRE(after.size() == 4);
        CHECK(current[after.back()].definition_id() == ids.get_id<givm::support_view>("SupportB"));
        CHECK(current[after.back()].state().count == (full && refill ? 99 : 17));
        CHECK(current[before[1]].is_valid() == not full);
        CHECK(current[givm::player_id{ 0 }].hand_card_count() == 0);
    };
    if(full && refill)
    {
        REQUIRE(advance(executor, library, table) == givm::execution_state::card_selection);
        CHECK(support_ids(table).size() == 3);
        CHECK_FALSE(table[before[1]].is_valid());
        CHECK(log.removed == std::vector{ before[1] });
        auto copied_executor = executor;
        auto copied_table = table;
        for(auto [running, current] : { std::pair{ &executor, &table }, std::pair{ &copied_executor, &copied_table } })
        {
            running->view_in<givm::execution_state::card_selection>().select({});
            REQUIRE(advance(*running, library, *current) == givm::execution_state::action_selection);
            check_result(*current);
        }
        CHECK(log.remaining_on_removal == std::vector<std::size_t>{ 3, 4, 4 });
    }
    else
    {
        REQUIRE(advance(executor, library, table) == givm::execution_state::action_selection);
        check_result(table);
        CHECK(log.removed.size() == (full ? 2 : 0));
        if(full) CHECK(log.remaining_on_removal == std::vector<std::size_t>{ 3, 3 });
    }
}

TEST_CASE("support definitions can remove themselves through a resumable state change response", "[support][entity-state][resume]")
{
    support_log log{ .remove_empty = true };
    log.dynamic = GENERATE(false, true);
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    log.actions = { { operation::add, { 7, 1 } }, { operation::set, { 0, 2 } }, { operation::add, { 9, 3 } } };
    const auto [library, ids] = compile_scenario(log, mode);
    givm::table table{ { .self_player = givm::player_id{ 0 } }, { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_scenario(table, library, ids);
    givm::executor executor;
    executor.enter_entry(library);
    REQUIRE(advance(executor, library, table) == givm::execution_state::card_selection);
    const auto waiting = support_ids(table);
    REQUIRE(waiting.size() == 1);
    CHECK(table[waiting[0]].is_valid());
    CHECK(table[waiting[0]].state().count == 0);
    CHECK(table[waiting[0]].state().round_usages == 2);
    CHECK(log.changed == waiting);
    CHECK(log.previous == std::vector<values>{ { 7, 1 } });
    CHECK(log.current == std::vector<values>{ { 0, 2 } });
    CHECK(log.removed.empty());
    CHECK(log.next_action == 2);
    executor.view_in<givm::execution_state::card_selection>().select({});
    REQUIRE(advance(executor, library, table) == givm::execution_state::finished);
    CHECK(log.next_action == 3);
    CHECK(log.removed == std::vector{ waiting[0], waiting[0] });
    CHECK(log.removal_observers == std::vector{ givm::player_id{ 0 }, givm::player_id{ 1 } });
    CHECK(log.remaining_on_removal == std::vector<std::size_t>{ 0, 0 });
    CHECK_FALSE(table[waiting[0]].is_valid());
    const auto after = support_ids(table);
    REQUIRE(after.size() == 1);
    CHECK(after[0] != waiting[0]);
    CHECK(table[after[0]].state().count == 9);
    CHECK(table[after[0]].state().round_usages == 3);
}
