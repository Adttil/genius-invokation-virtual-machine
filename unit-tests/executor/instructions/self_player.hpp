#include <cstddef>
#include <array>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/executor.hpp>

#include "../../table/test_definition_library.hpp"
#include "../test_character_source.hpp"

namespace givm_test::executor_instructions::self_player
{
constexpr std::array<std::size_t, 1> draw_positions_1{ 0 };

namespace
{
    constexpr givm::player_id first{ 0 };
    constexpr givm::player_id second{ 1 };
    constexpr givm::optional_player_id no_self{};
    constexpr givm::character_id first_character{ first, 0 };
    constexpr givm::character_id second_character{ second, 0 };

    struct self_log
    {
        bool nested = false;
        std::vector<givm::player_id> draws;
    };

    struct self_source
    {
        static constexpr auto category = givm::definition_category::character;
        struct definition_type
        {
            self_log* log;
            givm::normal_effect outer;
            givm::normal_effect nested;
            std::uint32_t health;
        };
        self_log* log;
        std::string_view source_name;
        std::uint32_t health;

        std::string_view name() const { return source_name; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const std::array outer_damage{ givm::deal_damage{
                .source = { givm::relative_player::self },
                .target = { givm::relative_player::opponent },
                .value = 4, .type = givm::damage_type::physical } };
            const std::array nested_damage{ givm::deal_damage{
                .source = { givm::relative_player::self },
                .target = { givm::relative_player::opponent },
                .value = 2, .type = givm::damage_type::physical } };
            return { log, context.add_normal_effect(std::tuple{
                givm::draw_cards{ .position = 0, .count = 1 },
                givm::heal{ .source = { givm::relative_player::self },
                    .target = { givm::relative_player::self }, .value = 3 },
                outer_damage[0],
                givm::draw_cards{ .position = 0, .count = 1 }
            }), context.add_normal_effect(std::tuple{
                nested_damage[0],
                givm::deal_damage{}, givm::deal_damage{},
                givm::draw_cards{ .player = givm::relative_player::opponent, .position = 0, .count = 1 }
            }), health };
        }
        static givm::character_state query(const definition_type& data, const givm::character_initial_state&)
        {
            return { .max_health = 20, .health = data.health };
        }
        static givm::normal_effect handle(const definition_type& data,
            givm::round_started&, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            const auto self = context.entity().character();
            CHECK(context.table().state().self_player == no_self);
            CHECK(context.table().state().active_player == first);
            return self.id().player_id() == second ? context.invoke(data.outer) : givm::normal_effect{};
        }
        static givm::normal_effect handle(const definition_type& data,
            givm::card_drawn& event, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            const auto self = context.entity().character();
            if(self.id().player_id() != first) return {};
            CHECK(event.card.player_id() == second);
            data.log->draws.push_back(context.table().state().self_player.get());
            if(data.log->nested) return {};
            data.log->nested = true;
            CHECK(context.table().state().self_player == second);
            // The relative target uses this responding player's side. The exact ID does not.
            return context.invoke(data.nested,
                givm::deal_damage_input{ std::array{ givm::damage{ .source = first_character,
                    .target = givm::relative_character_target{ givm::relative_player::self },
                    .value = 1, .type = givm::damage_type::physical } } },
                givm::deal_damage_input{ std::array{ givm::damage{ .source = first_character, .target = second_character,
                    .value = 1, .type = givm::damage_type::physical } } });
        }
        static givm::immediate_effect handle(const definition_type&,
            givm::damage_preparation& event, givm::handle_context<givm::skill_view, givm::event_category::immediate>& context, std::uint32_t = 0)
        {
            const auto self = context.entity().character();
            if(self.id().player_id() != first) return {};
            const auto owner = event.value == 4 ? second : first;
            CHECK(context.table().state().self_player == owner);
            CHECK(event.source.template get<givm::entity_category::character>().player_id() == owner);
            if(event.value != 1) CHECK(event.target.player_id() != owner);
            return {};
        }
        static givm::normal_effect handle(const definition_type&,
            givm::healed& event, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            const auto self = context.entity().character();
            if(self.id().player_id() != first) return {};
            CHECK(context.table().state().self_player == second);
            CHECK(event.source.template get<givm::entity_category::character>() == second_character);
            CHECK(event.target == second_character);
            CHECK(event.value == 3);
            return {};
        }
    };

    void finish_responses(givm_test::executor_driver& executor, const givm::definition_library& library,
        givm::table& table, bool observed)
    {
        auto random = [] { return std::uint32_t{ 0 }; };
        for(;;)
        {
            const auto state = executor.advance(library, table, random);
            if(state == givm::execution_state::finished) break;
            REQUIRE(observed);
            REQUIRE(state == givm::execution_state::health_reduced);
            const auto view = executor.view_in<givm::execution_state::health_reduced>();
            CHECK(table.state().self_player == (view.value() == 4 ? second : first));
        }
        CHECK(table.state().self_player == no_self);
        CHECK(table.state().active_player == first);
        CHECK(table[first_character].state().health == 15);
        CHECK(table[second_character].state().health == 10);
        CHECK(table[first].hand_card_count() == 0);
        CHECK(table[second].hand_card_count() == 3);
    }

    struct payment_log
    {
        std::uint32_t previews = 0;
        std::uint32_t draws = 0;
    };

    struct payment_source
    {
        static constexpr auto category = givm::definition_category::character;
        struct definition_type
        {
            payment_log* log;
            givm::preview_effect payment;
        };
        payment_log* log;
        std::string_view name() const { return "OtherPlayerPayment"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.add_preview_effect(std::tuple{ givm::draw_cards{ .position = 0, .count = 1 } }) };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::preview_effect handle(const definition_type& data,
            givm::cost_of_switch& event, givm::handle_context<givm::skill_view, givm::event_category::preview>& context)
        {
            const auto self = context.entity().character();
            ++data.log->previews;
            CHECK(self.id().player_id() == second);
            CHECK(context.table().state().active_player == first);
            CHECK(context.table().state().self_player == no_self);
            event.requirement.dice_requirement.any = 0;
            event.requirement.speed = givm::action_speed::fast;
            return context.invoke(data.payment);
        }
        static givm::normal_effect handle(const definition_type& data,
            givm::card_drawn& event, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
        {
            ++data.log->draws;
            CHECK(context.table().state().active_player == first);
            CHECK(context.table().state().self_player == second);
            CHECK(event.card.player_id() == second);
            return {};
        }
    };
}

TEST_CASE("response programs resolve relative players and restore their caller across nested and copied execution", "[self-player][resume]")
{
    const bool observed = GENERATE(false, true);
    self_log log;
    const auto first_source = givm::test::with_passive_skill(self_source{ &log, "FirstResponder", 20 });
    const auto second_source = givm::test::with_passive_skill(self_source{ &log, "SecondResponder", 10 });
    const givm::test::named_definition_source<givm::definition_category::card> card{ "ResponseDraw" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple{ givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, first_source, second_source, card);
    givm::table table{ {}, { .active_character = first_character }, { .active_character = second_character } };
    const auto card_id = ids.get_id<givm::definition_category::card>(card.name());
    load_deck(table, library,
        { .cards = { card_id, card_id, card_id },
            .characters = { ids.get_id<givm::definition_category::character>(first_source.name()) } },
        { .cards = { card_id, card_id, card_id },
            .characters = { ids.get_id<givm::definition_category::character>(second_source.name()) } });
    CHECK(table.state().self_player == no_self);
    givm_test::executor_driver executor;
    executor.start(library, table);
    if(observed)
    {
        auto random = [] { return std::uint32_t{ 0 }; };
        REQUIRE(executor.advance(library, table, random) == givm::execution_state::health_reduced);
        CHECK(table.state().self_player == second);
        CHECK(table[first_character].state().health == 16);
        auto copied_executor = executor;
        auto copied_table = table;
        finish_responses(executor, library, table, true);
        CHECK(log.draws == std::vector{ second, first, second });
        log.draws.clear();
        log.nested = false;
        finish_responses(copied_executor, library, copied_table, true);
        CHECK(log.draws == std::vector{ second, first, second });
    }
    else
    {
        finish_responses(executor, library, table, false);
        CHECK(log.draws == std::vector{ second, first, second });
    }
}

TEST_CASE("cost preview preserves the caller while cached payment executes on its responder's side", "[self-player][onpay][resume]")
{
    const bool observed = GENERATE(false, true);
    payment_log log;
    const auto responder = givm::test::with_passive_skill(payment_source{ &log });
    const givm::test::initialized_character_source character;
    const givm::test::named_definition_source<givm::definition_category::card> card{ "PaymentDraw" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple{ givm::begin_action{} }, std::tuple{}, responder, character, card);
    givm::table table{ {}, { .active_character = first_character }, { .active_character = second_character } };
    const auto character_id = ids.get_id<givm::definition_category::character>(character.name());
    const auto card_id = ids.get_id<givm::definition_category::card>(card.name());
    load_deck(table, library, { .cards = { card_id }, .characters = { character_id, character_id } },
        { .cards = { card_id }, .characters = { ids.get_id<givm::definition_category::character>(responder.name()) } });
    givm_test::executor_driver executor;
    executor.start(library, table);
    auto random = [] { return std::uint32_t{ 0 }; };
    if(observed) REQUIRE(executor.advance(library, table, random) == givm::execution_state::action_started);
    REQUIRE(executor.advance(library, table, random) == givm::execution_state::action_selection);
    CHECK(table.state().self_player == no_self);
    const auto action = executor.view_in<givm::execution_state::action_selection>();
    const auto quote_1 = action.calculate_switch_cost(library, table, 0);
    CHECK(action.switch_cost(quote_1).requirement.dice_requirement.any == 0);
    CHECK(action.switch_payment_validate(table, quote_1, {}) == givm::switch_payment_validation::valid);
    CHECK(table.state().self_player == no_self);
    CHECK(table[first].hand_card_count() == 0);
    CHECK(table[second].hand_card_count() == 0);
    CHECK(log.previews == 1);
    CHECK(log.draws == 0);

    auto copied_executor = executor;
    auto copied_table = table;
    for(const auto [running, current] : { std::pair{ &executor, &table }, std::pair{ &copied_executor, &copied_table } })
    {
        running->submitted(running->view_in<givm::execution_state::action_selection>().switch_active_character_with_cached_cost(library, *current, random, quote_1, {}));
        auto state = running->advance(library, *current, random);
        if(observed)
        {
            REQUIRE(state == givm::execution_state::active_character_changed);
            state = running->advance(library, *current, random);
        }
        REQUIRE(state == givm::execution_state::action_selection);
        CHECK(current->state().self_player == no_self);
        CHECK(current->state().active_player == first);
        CHECK((*current)[first].state().active_character == givm::character_id{ first, 1 });
        CHECK((*current)[first].hand_card_count() == 0);
        CHECK((*current)[second].hand_card_count() == 1);
    }
    CHECK(log.previews == 1);
    CHECK(log.draws == 2);
}
}
