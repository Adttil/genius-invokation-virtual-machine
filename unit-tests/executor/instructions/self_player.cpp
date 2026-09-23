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

namespace
{
    constexpr givm::player_id first{ 0 };
    constexpr givm::player_id second{ 1 };
    constexpr givm::player_id no_self{ 2 };
    constexpr givm::character_id first_character{ first, 0 };
    constexpr givm::character_id second_character{ second, 0 };

    struct self_log
    {
        bool nested = false;
        std::vector<givm::player_id> draws;
    };

    struct self_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            self_log* log;
            givm::program_entry outer;
            givm::program_entry nested;
            std::uint32_t health;
        };
        self_log* log;
        std::string_view source_name;
        std::uint32_t health;

        std::string_view name() const { return source_name; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const std::array outer_damage{ givm::fixed_damage{
                .source = { givm::relative_player::self },
                .target = { givm::relative_player::opponent },
                .value = 4, .type = givm::damage_type::physical } };
            const std::array nested_damage{ givm::fixed_damage{
                .source = { givm::relative_player::self },
                .target = { givm::relative_player::opponent },
                .value = 2, .type = givm::damage_type::physical } };
            return { log, context.add_program(std::tuple{
                givm::draw_cards{ .count = 1 },
                givm::heal{ .source = { givm::relative_player::self },
                    .target = { givm::relative_player::self }, .value = 3 },
                givm::deal_damage{ .damages = outer_damage },
                givm::draw_cards{ .count = 1 }
            }), context.add_program(std::tuple{
                givm::deal_damage{ .damages = nested_damage },
                givm::deal_damage{ .input_count = 2 },
                givm::draw_cards{ .count = 1, .player = givm::relative_player::opponent }
            }), health };
        }
        static givm::character_state query(const definition_type& data, const givm::character_initial_state&)
        {
            return { .max_health = 20, .health = data.health };
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view& self,
            givm::test_event&, givm::handle_context& context)
        {
            CHECK(context.table().state().self_player == no_self);
            CHECK(context.table().state().active_player == first);
            return self.id().player_id == second ? context.invoke(data.outer) : givm::program_entry{};
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view& self,
            givm::card_drawn& event, givm::handle_context& context)
        {
            if(self.id().player_id != first) return {};
            CHECK(event.card.player_id == second);
            data.log->draws.push_back(context.table().state().self_player);
            if(data.log->nested) return {};
            data.log->nested = true;
            CHECK(context.table().state().self_player == second);
            // The relative target uses this responding player's side. The exact ID does not.
            return context.invoke(data.nested,
                givm::damage{ .source = first_character,
                    .target = givm::relative_character_target{ givm::relative_player::self },
                    .value = 1, .type = givm::damage_type::physical },
                givm::damage{ .source = first_character, .target = second_character,
                    .value = 1, .type = givm::damage_type::physical });
        }
        static givm::program_entry handle(const definition_type&, const givm::character_view& self,
            givm::damage_preparation& event, givm::handle_context& context)
        {
            if(self.id().player_id != first) return {};
            const auto owner = event.value == 4 ? second : first;
            CHECK(context.table().state().self_player == owner);
            CHECK(std::get<givm::character_id>(event.source).player_id == owner);
            if(event.value != 1) CHECK(event.target.player_id != owner);
            return {};
        }
        static givm::program_entry handle(const definition_type&, const givm::character_view& self,
            givm::healed& event, givm::handle_context& context)
        {
            if(self.id().player_id != first) return {};
            CHECK(context.table().state().self_player == second);
            CHECK(std::get<givm::character_id>(event.source) == second_character);
            CHECK(event.target == second_character);
            CHECK(event.value == 3);
            return {};
        }
    };

    void finish_responses(givm::executor& executor, const givm::definition_library& library,
        givm::table& table, bool observed)
    {
        auto random = [] { return std::uint32_t{ 0 }; };
        for(;;)
        {
            const auto state = executor.step(library, table, random);
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
        using definition_category = givm::character_view;
        struct definition_type
        {
            payment_log* log;
            givm::program_entry payment;
        };
        payment_log* log;
        std::string_view name() const { return "OtherPlayerPayment"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { log, context.add_program(std::tuple{ givm::draw_cards{ .count = 1 } }) };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view& self,
            givm::cost_of_switch& event, givm::handle_context& context)
        {
            ++data.log->previews;
            CHECK(self.id().player_id == second);
            CHECK(context.table().state().active_player == first);
            CHECK(context.table().state().self_player == no_self);
            event.requirement.dice_requirement.any = 0;
            event.requirement.speed = givm::action_speed::fast;
            return context.invoke(givm::substack_t{}, data.payment);
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&,
            givm::card_drawn& event, givm::handle_context& context)
        {
            ++data.log->draws;
            CHECK(context.table().state().active_player == first);
            CHECK(context.table().state().self_player == second);
            CHECK(event.card.player_id == second);
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
    const givm::test::named_definition_source<givm::card_definition> card{ "ResponseDraw" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple{ givm::test_command{}, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, first_source, second_source, card);
    givm::table table{ {}, { .active_character = first_character }, { .active_character = second_character } };
    const auto card_id = ids.get_id<givm::card_definition>(card.name());
    load_deck(table, library,
        { .cards = { card_id, card_id, card_id },
            .characters = { ids.get_id<givm::character_view>(first_source.name()) } },
        { .cards = { card_id, card_id, card_id },
            .characters = { ids.get_id<givm::character_view>(second_source.name()) } });
    CHECK(table.state().self_player == no_self);
    givm::executor executor;
    executor.enter_entry(library);
    if(observed)
    {
        auto random = [] { return std::uint32_t{ 0 }; };
        REQUIRE(executor.step(library, table, random) == givm::execution_state::health_reduced);
        CHECK(table.state().self_player == first);
        CHECK(table[second_character].state().health == 8);
        auto copied_executor = executor;
        auto copied_table = table;
        finish_responses(executor, library, table, true);
        CHECK(log.draws == std::vector{ second, first, second });
        log.draws.clear();
        finish_responses(copied_executor, library, copied_table, true);
        CHECK(log.draws == std::vector{ first, second });
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
    const givm::test::named_definition_source<givm::card_definition> card{ "PaymentDraw" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        observed ? givm::compile_mode::observed : givm::compile_mode::normal,
        std::tuple{ givm::begin_action{} }, std::tuple{}, responder, character, card);
    givm::table table{ {}, { .active_character = first_character }, { .active_character = second_character } };
    const auto character_id = ids.get_id<givm::character_view>(character.name());
    const auto card_id = ids.get_id<givm::card_definition>(card.name());
    load_deck(table, library, { .cards = { card_id }, .characters = { character_id, character_id } },
        { .cards = { card_id }, .characters = { ids.get_id<givm::character_view>(responder.name()) } });
    givm::executor executor;
    executor.enter_entry(library);
    auto random = [] { return std::uint32_t{ 0 }; };
    if(observed) REQUIRE(executor.step(library, table, random) == givm::execution_state::action_started);
    REQUIRE(executor.step(library, table, random) == givm::execution_state::action_selection);
    CHECK(table.state().self_player == no_self);
    const auto action = executor.view_in<givm::execution_state::action_selection>();
    CHECK(action.calculate_switch_cost(library, table, 0).requirement.dice_requirement.any == 0);
    CHECK(action.switch_payment_validate(table, 0, {}) == givm::switch_payment_validation::valid);
    CHECK(table.state().self_player == no_self);
    CHECK(table[first].hand_card_count() == 0);
    CHECK(table[second].hand_card_count() == 0);
    CHECK(log.previews == 1);
    CHECK(log.draws == 0);

    auto copied_executor = executor;
    auto copied_table = table;
    for(const auto [running, current] : { std::pair{ &executor, &table }, std::pair{ &copied_executor, &copied_table } })
    {
        running->view_in<givm::execution_state::action_selection>().switch_active_character(0, {});
        auto state = running->step(library, *current, random);
        if(observed)
        {
            REQUIRE(state == givm::execution_state::active_character_changed);
            state = running->step(library, *current, random);
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
