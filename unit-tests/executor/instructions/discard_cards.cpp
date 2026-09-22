#include <concepts>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/executor.hpp>

#include "../../table/test_definition_library.hpp"

namespace
{
    struct discard_log
    {
        bool dynamic;
        bool self_effects;
        std::vector<char> order;
        std::vector<std::size_t> hand_counts;
        std::vector<std::size_t> deck_counts;
    };

    struct discard_source
    {
        using definition_category = givm::card_definition;
        struct definition_type
        {
            discard_log* log;
            givm::program_entry discard;
            givm::program_entry effect;
        };
        discard_log* log;
        constexpr std::string_view name() const { return "DiscardCard"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto discard = log->dynamic
                ? context.add_program(std::tuple{ givm::discard_hand_card{}, givm::discard_deck_card{} })
                : context.add_program(std::tuple{
                    givm::discard_hand_card{ { givm::player_id{ 0 }, 0 } },
                    givm::discard_deck_card{ { givm::player_id{ 0 }, 1 } } });
            return { log, discard, context.add_program(std::tuple{
                givm::replace_cards{ givm::player_id{ 1 } }, givm::draw_cards{ .count = 1 } }) };
        }
        static givm::card_state query(const definition_type&, const givm::card_initial_state&)
        {
            return { .cost = { .energy = 3 } };
        }
        static givm::program_entry handle(const definition_type& data, const givm::deck_card_view& self,
            givm::test_event&, givm::handle_context& context)
        {
            if(self.player().id() != givm::player_id{ 1 }) return {};
            if(not data.log->dynamic) return context.invoke(data.discard);
            return context.invoke(data.discard,
                givm::hand_card_discarded{ { givm::player_id{ 0 }, 0 } },
                givm::deck_card_discarded{ { givm::player_id{ 0 }, 1 } });
        }
        template<class TView, class TEvent>
            requires((std::same_as<TView, givm::hand_card_view> || std::same_as<TView, givm::deck_card_view>)
                && (std::same_as<TEvent, givm::hand_card_discarded> || std::same_as<TEvent, givm::deck_card_discarded>))
        static givm::program_entry handle(const definition_type& data, const TView& self,
            TEvent& event, givm::handle_context& context)
        {
            const auto discarded = context.table()[event.card];
            CHECK_FALSE(discarded.is_valid());
            CHECK(discarded.state().cost.energy == 3);
            CHECK(discarded.definition_id() == self.definition_id());
            constexpr bool hand = std::same_as<TEvent, givm::hand_card_discarded>;
            if constexpr(std::same_as<decltype(self.id()), std::remove_const_t<decltype(event.card)>>)
            {
                if(self.id() == event.card)
                {
                    data.log->order.push_back(hand ? 'h' : 'd');
                    return data.log->self_effects ? context.invoke(data.effect) : givm::program_entry{};
                }
            }
            CHECK(self.is_valid());
            if(self.player().id() == givm::player_id{ 1 })
            {
                data.log->order.push_back(hand ? 'H' : 'D');
                const auto player = context.table()[givm::player_id{ 0 }];
                data.log->hand_counts.push_back(player.hand_card_count());
                data.log->deck_counts.push_back(player.deck_card_count());
            }
            return {};
        }
    };

    givm::execution_state advance(givm::executor& executor, const givm::definition_library& library, givm::table& table)
    {
        auto random = [] { return std::uint32_t{ 0 }; };
        for(;;)
        {
            const auto state = executor.step(library, table, random);
            if(state == givm::execution_state::card_selection || state == givm::execution_state::finished) return state;
        }
    }
}

TEST_CASE("discarding cards runs their own effects before separate zone notifications", "[discard]")
{
    discard_log log{ GENERATE(false, true), GENERATE(false, true) };
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    const discard_source source{ &log };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{ givm::draw_cards{ .count = 2 }, givm::test_command{}, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, source);
    const auto card = ids.get_id<givm::card_definition>(source.name());
    givm::table table;
    load_deck(table, library, { .cards = { card, card, card, card, card } }, { .cards = { card } });
    givm::executor executor;
    executor.enter_entry(library);
    std::size_t pauses = 0;
    while(advance(executor, library, table) == givm::execution_state::card_selection)
    {
        ++pauses;
        CHECK(executor.view_in<givm::execution_state::card_selection>().selection_validate(table, {}));
        // Restore both public objects while suspended inside the discarded card's program.
        auto table_copy = table;
        auto executor_copy = executor;
        table = std::move(table_copy);
        executor = std::move(executor_copy);
        executor.view_in<givm::execution_state::card_selection>().select({});
    }
    CHECK(pauses == (log.self_effects ? 2 : 0));
    CHECK(log.order == std::vector<char>{ 'h', 'H', 'd', 'D' });
    CHECK(log.hand_counts == (log.self_effects ? std::vector<std::size_t>{ 2, 3 } : std::vector<std::size_t>{ 1, 1 }));
    CHECK(log.deck_counts == (log.self_effects ? std::vector<std::size_t>{ 2, 0 } : std::vector<std::size_t>{ 3, 2 }));
    CHECK_FALSE(table[givm::hand_card_id{ givm::player_id{ 0 }, 0 }].is_valid());
    CHECK_FALSE(table[givm::deck_card_id{ givm::player_id{ 0 }, 1 }].is_valid());
    table.clean_up();
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == log.hand_counts.back());
    CHECK(table[givm::player_id{ 0 }].deck_card_count() == log.deck_counts.back());
}

TEST_CASE("overflow removes cards without invoking discard effects or notifications", "[discard]")
{
    discard_log log{};
    const discard_source source{ &log };
    const auto [library, ids] = givm::test::compile_definitions_with_program(givm::compile_mode::normal,
        std::tuple{ givm::draw_cards{ .count = 3 }, givm::end_game{ givm::game_result::both_loss } },
        std::tuple{}, source);
    const auto card = ids.get_id<givm::card_definition>(source.name());
    givm::table table{ {}, { .hand_limit = 1 } };
    load_deck(table, library, { .cards = { card, card, card } }, { .cards = { card } });
    givm::executor executor;
    executor.enter_entry(library);
    REQUIRE(advance(executor, library, table) == givm::execution_state::finished);
    CHECK(log.order.empty());
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 1);
    CHECK(table[givm::player_id{ 0 }].deck_card_count() == 0);
}
