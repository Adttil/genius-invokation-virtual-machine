#include <cstdint>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <givm/executor.hpp>

#include "test_definition_library.hpp"

namespace
{
    void check_removed_card(
        const givm::table& table,
        givm::deck_card_id card_id,
        givm::definition_id<givm::card_definition> definition
    )
    {
        const auto card = table[card_id];
        CHECK_FALSE(card.is_valid());
        CHECK_FALSE(static_cast<bool>(card));
        CHECK(card.id() == card_id);
        CHECK(card.definition_id() == definition);
        CHECK(card.player().id() == card_id.player_id);
        CHECK(card.statuses().begin() == card.statuses().end());
        // card_state currently has no fields, but reading it must remain supported.
        (void)card.state();
    }

    struct overflow_log
    {
        std::vector<givm::deck_card_id> handlers;
        givm::deck_card_id removed;
        givm::definition_id<givm::card_definition> removed_definition;
    };

    struct overflow_response_source
    {
        using definition_category = givm::card_definition;
        struct definition_type
        {
            overflow_log* log;
            givm::program_entry<givm::test_event> draw_entry;
        };

        std::string_view source_name;
        overflow_log* log;
        bool draws;

        std::string_view name() const noexcept { return source_name; }

        definition_type compile(givm::definition_compile_context& context) const
        {
            return {
                log,
                draws
                    ? context.add_program<givm::test_event>(std::tuple{ givm::draw_cards{ .count = 1 } })
                    : givm::program_entry<givm::test_event>::null()
            };
        }

        static givm::program_entry<givm::test_event> handle(
            const definition_type& data, const givm::deck_card_view& self,
            givm::test_event&, const givm::table& table, givm::random_fn&
        )
        {
            const bool first_response = data.log->handlers.empty();
            data.log->handlers.push_back(self.id());
            if(first_response)
                return data.draw_entry;

            check_removed_card(table, data.log->removed, data.log->removed_definition);
            return givm::program_entry<givm::test_event>::null();
        }
    };

    std::vector<givm::definition_id<givm::card_definition>> deck_definitions(givm::player_view player)
    {
        std::vector<givm::definition_id<givm::card_definition>> result;
        for(const auto card : player.deck_cards())
            result.push_back(card.definition_id());
        return result;
    }
}

TEST_CASE("overflow discards retain readable card information and leave broadcast participation", "[table][broadcast][draw_cards][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    overflow_log log;
    const overflow_response_source first{ "OverflowFirst", &log, true };
    const overflow_response_source second{ "OverflowSecond", &log, false };
    const overflow_response_source discarded{ "OverflowDiscarded", &log, false };
    const givm::test::named_definition_source<givm::card_definition> drawn{ "OverflowDrawn" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(
        mode,
        std::tuple{
            givm::draw_cards{ .count = 1 },
            givm::test_command{},
            givm::test_command{},
            givm::end_game{ givm::game_result::both_loss }
        }, std::tuple{}, first, second, discarded, drawn
    );
    const auto first_definition = ids.get_id<givm::card_definition>(first.name());
    const auto second_definition = ids.get_id<givm::card_definition>(second.name());
    const auto discarded_definition = ids.get_id<givm::card_definition>(discarded.name());
    const auto drawn_definition = ids.get_id<givm::card_definition>(drawn.name());
    const givm::player_id player{ 0 };
    givm::table table{ givm::game_parameters{ .hand_limit = 1 } };
    load_deck(table, library, {
        .cards = { first_definition, second_definition, discarded_definition, drawn_definition }
    }, {});
    std::vector<givm::deck_card_id> original_cards;
    for(const auto card : table[player].deck_cards())
        original_cards.push_back(card.id());
    REQUIRE(original_cards.size() == 4);
    log.removed = original_cards[2];
    log.removed_definition = discarded_definition;

    std::uint32_t random_calls = 0;
    auto random = [&]() -> std::uint32_t { ++random_calls; return 0; };
    givm::executor execution;
    execution.enter_entry(library);
    REQUIRE(execution.step(library, table, random) == givm::execution_state::finished);

    // The first response burns a later recipient; both this broadcast and the next skip it.
    CHECK(log.handlers == std::vector{
        original_cards[0], original_cards[1], original_cards[0], original_cards[1]
    });
    CHECK(random_calls == 0);
    check_removed_card(table, log.removed, discarded_definition);
    const auto check_live_cards = [&](const givm::table& current)
    {
        const auto current_player = current[player];
        CHECK(current_player.hand_card_count() == 1);
        CHECK(current_player.deck_card_count() == 2);
        CHECK(deck_definitions(current_player) == std::vector{ first_definition, second_definition });
        for(const auto card : current_player.hand_cards())
            CHECK(card.definition_id() == drawn_definition);
    };
    check_live_cards(table);

    auto copy = table;
    check_removed_card(copy, log.removed, discarded_definition);
    table.clean_up();
    check_live_cards(table);
    check_removed_card(copy, log.removed, discarded_definition);
    check_live_cards(copy);
    copy.clean_up();
    check_live_cards(copy);
}
