#include "test_source_library.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <givm/givm.hpp>

#include "table/test_definition_library.hpp"

namespace
{
    struct sequence_random
    {
        std::vector<std::uint32_t> values;
        std::size_t position = 0;

        std::uint32_t operator()()
        {
            return values[position++];
        }
    };

    struct initializing_character_source
    {
        using definition_category = givm::character_view;

        struct definition_type
        {
            std::string_view name;
            std::uint32_t base_health;
            std::uint32_t* initial_state_queries;
        };

        std::string_view source_name;
        std::uint32_t base_health;
        std::uint32_t* initial_state_queries;

        std::string_view name() const noexcept
        {
            return source_name;
        }

        definition_type compile(givm::definition_compile_context&) const noexcept
        {
            return {
                .name = source_name,
                .base_health = base_health,
                .initial_state_queries = initial_state_queries
            };
        }

        static givm::character_state query(const definition_type& definition, const givm::character_initial_state&)
        {
            ++*definition.initial_state_queries;
            return {
                .max_health = definition.base_health,
                .max_energy = 3,
                .health = definition.base_health,
                .energy = 0
            };
        }
    };

    template<class TPlayer>
    std::vector<std::size_t> deck_definition_values(const TPlayer& player)
    {
        std::vector<std::size_t> result;
        for(std::size_t index = 0; index < player.deck_card_count(); ++index)
        {
            result.push_back(player.deck_card_definition(index).value());
        }
        return result;
    }
}

TEST_CASE("deck linking resolves names and table loading preserves input order", "[deck]")
{
    const givm::test::named_definition_source<givm::card_definition> alpha{ "Alpha" };
    const givm::test::named_definition_source<givm::card_definition> beta{ "Beta" };
    const givm::test::named_definition_source<givm::character_view> first{ "First" };
    const givm::test::named_definition_source<givm::character_view> second{ "Second" };

    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(alpha, beta, first, second));
    const auto program = std::tuple{ givm::end_game{ .result = givm::game_result::both_loss } };
    const auto [library, id_map] = compile(sources, program, program, givm::compile_mode::normal);
    const auto deck = link_deck(
        id_map,
        std::array<std::string_view, 3>{ "Beta", "Alpha", "Beta" },
        std::array<std::string_view, 2>{ "Second", "First" }
    );

    givm::table table{};
    load_deck(table, library, deck, {});
    const auto player = table[givm::player_id{ 0 }];

    CHECK(deck_definition_values(player) == std::vector<std::size_t>{
        id_map.get_id<givm::card_definition>("Beta").value(),
        id_map.get_id<givm::card_definition>("Alpha").value(),
        id_map.get_id<givm::card_definition>("Beta").value()
    });

    std::vector<std::size_t> character_definitions;
    for(const auto character : player.characters())
    {
        character_definitions.push_back(character.definition_id().value());
        CHECK(character.state().max_health == 0);
    }
    CHECK(character_definitions == std::vector<std::size_t>{
        id_map.get_id<givm::character_view>("Second").value(),
        id_map.get_id<givm::character_view>("First").value()
    });

    CHECK_THROWS_AS(
        link_deck(
            id_map,
            std::array<std::string_view, 1>{ "Missing" },
            std::array<std::string_view, 0>{}
        ),
        std::invalid_argument
    );
}

TEST_CASE("shuffle_deck changes only logical order", "[deck][instruction]")
{
    const givm::test::named_definition_source<givm::card_definition> alpha{ "Alpha" };
    const givm::test::named_definition_source<givm::card_definition> beta{ "Beta" };
    const givm::test::named_definition_source<givm::card_definition> gamma{ "Gamma" };
    const givm::test::named_definition_source<givm::card_definition> delta{ "Delta" };

    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(alpha, beta, gamma, delta));
    const auto [library, id_map] = compile(sources,
        std::tuple{ givm::shuffle_deck{ .player = givm::player_id{ 0 } } },
        std::tuple{ givm::end_game{ .result = givm::game_result::both_loss } }, givm::compile_mode::normal
    );
    const givm::linked_deck deck{
        .cards = {
            id_map.get_id<givm::card_definition>("Alpha"),
            id_map.get_id<givm::card_definition>("Beta"),
            id_map.get_id<givm::card_definition>("Gamma"),
            id_map.get_id<givm::card_definition>("Delta")
        }
    };

    givm::table table{};
    load_deck(table, library, deck, {});
    std::vector<givm::deck_card_id> original_ids;
    for(const auto card : table[givm::player_id{ 0 }].deck_cards())
    {
        original_ids.push_back(card.id());
    }
    REQUIRE(original_ids.size() == 4);

    givm::executor target;
    target.enter_entry(library);
    sequence_random random{
        .values = { std::numeric_limits<std::uint32_t>::max(), 0, std::uint32_t{ 0x80000000u } }
    };
    REQUIRE(target.step(library, table, random) == givm::execution_state::finished);
    CHECK(random.position == 3);

    CHECK(deck_definition_values(table[givm::player_id{ 0 }]) == std::vector<std::size_t>{
        id_map.get_id<givm::card_definition>("Gamma").value(),
        id_map.get_id<givm::card_definition>("Beta").value(),
        id_map.get_id<givm::card_definition>("Alpha").value(),
        id_map.get_id<givm::card_definition>("Delta").value()
    });
    CHECK(
        table[original_ids[0]].definition_id().value()
        == id_map.get_id<givm::card_definition>("Alpha").value()
    );
    CHECK(
        table[original_ids[1]].definition_id().value()
        == id_map.get_id<givm::card_definition>("Beta").value()
    );
    CHECK(
        table[original_ids[2]].definition_id().value()
        == id_map.get_id<givm::card_definition>("Gamma").value()
    );
    CHECK(
        table[original_ids[3]].definition_id().value()
        == id_map.get_id<givm::card_definition>("Delta").value()
    );
}

TEST_CASE("loading decks immediately initializes characters from cached states independently for each player", "[deck][query]")
{
    std::uint32_t initial_state_queries = 0;
    const initializing_character_source alpha{ "Alpha", 10, &initial_state_queries };
    const initializing_character_source beta{ "Beta", 20, &initial_state_queries };

    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(alpha, beta));
    const auto [library, id_map] = compile(sources,
        std::tuple{},
        std::tuple{ givm::end_game{ .result = givm::game_result::both_loss } }, givm::compile_mode::normal
    );
    CHECK(initial_state_queries == 2);
    const givm::linked_deck deck{
        .characters = {
            id_map.get_id<givm::character_view>("Beta"),
            id_map.get_id<givm::character_view>("Alpha")
        }
    };

    givm::table table{};
    load_deck(table, library, deck, {
        .characters = { id_map.get_id<givm::character_view>("Alpha") }
    });
    CHECK(initial_state_queries == 2);

    const auto player = table[givm::player_id{ 0 }];
    auto characters = player.characters();
    auto iterator = characters.begin();
    CHECK((*iterator).state().health == 20);
    CHECK((*iterator).state().max_health == 20);
    CHECK((*iterator).state().max_energy == 3);
    CHECK((*iterator).state().energy == 0);
    ++iterator;
    CHECK((*iterator).state().health == 10);
    CHECK_FALSE(player.state().active_character.has_value());

    CHECK(table[givm::character_id{ givm::player_id{ 1 }, 0 }].state().health == 10);
    CHECK(table[givm::character_id{ givm::player_id{ 0 }, 0 }].state().health == 20);
    CHECK(table[givm::character_id{ givm::player_id{ 0 }, 1 }].state().health == 10);
    CHECK_FALSE(table[givm::player_id{ 1 }].state().active_character.has_value());
}
