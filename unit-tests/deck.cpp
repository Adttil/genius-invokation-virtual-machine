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

using namespace givm;

namespace
{
    struct stop_execution
    {
        using context_type = void;

        bool execute(card_table&, execution_context& context, random_fn&) const noexcept
        {
            return context.yield();
        }
    };

    struct sequence_random
    {
        std::vector<std::uint32_t> values;
        size_t position = 0;

        std::uint32_t operator()()
        {
            return values[position++];
        }
    };

    struct initializing_character_source
    {
        using definition_category = character_view;

        struct definition_type
        {
            std::string_view name;
            std::uint32_t base_health;
            std::vector<std::string_view>* initialization_order;
        };

        std::string_view source_name;
        std::uint32_t base_health;
        std::vector<std::string_view>* initialization_order;

        std::string_view name() const noexcept
        {
            return source_name;
        }

        definition_type compile(definition_compile_context&) const noexcept
        {
            return {
                .name = source_name,
                .base_health = base_health,
                .initialization_order = initialization_order
            };
        }

        static handler_program_entry_t<character_initialization> handle(
            const definition_type& definition,
            const character_view&,
            character_initialization& event,
            const card_table&,
            random_fn& random
        )
        {
            definition.initialization_order->push_back(definition.name);
            const std::uint32_t health = definition.base_health + random();
            event.state = {
                .max_health = health,
                .max_energy = 3,
                .health = health,
                .energy = 0
            };
            return handler_program_entry_t<character_initialization>::null();
        }
    };

    template<class TPlayer>
    std::vector<size_t> deck_definition_values(const TPlayer& player)
    {
        std::vector<size_t> result;
        for(size_t index = 0; index < player.deck_card_count(); ++index)
        {
            result.push_back(player.deck_card_definition(index).value());
        }
        return result;
    }
}

TEST_CASE("deck linking resolves names and table loading preserves input order", "[deck]")
{
    const test::named_definition_source<card_definition> alpha{ "Alpha" };
    const test::named_definition_source<card_definition> beta{ "Beta" };
    const test::named_definition_source<character_view> first{ "First" };
    const test::named_definition_source<character_view> second{ "Second" };

    definition_source_library sources;
    REQUIRE(sources.add(alpha, beta, first, second));
    const auto program = std::tuple{ stop_execution{} };
    const auto [library, id_map] = sources.compile(program, program);
    const auto deck = link_deck(
        id_map,
        std::array<std::string_view, 3>{ "Beta", "Alpha", "Beta" },
        std::array<std::string_view, 2>{ "Second", "First" }
    );

    card_table table{ library };
    table.load_deck(player_id{ 0 }, deck);
    const auto player = table[player_id{ 0 }];

    CHECK(deck_definition_values(player) == std::vector<size_t>{
        id_map.get_id<card_definition>("Beta").value(),
        id_map.get_id<card_definition>("Alpha").value(),
        id_map.get_id<card_definition>("Beta").value()
    });

    std::vector<size_t> character_definitions;
    for(const auto character : player.characters())
    {
        character_definitions.push_back(character.definition().id().value());
        CHECK(character.state().max_health == 0);
    }
    CHECK(character_definitions == std::vector<size_t>{
        id_map.get_id<character_view>("Second").value(),
        id_map.get_id<character_view>("First").value()
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
    const test::named_definition_source<card_definition> alpha{ "Alpha" };
    const test::named_definition_source<card_definition> beta{ "Beta" };
    const test::named_definition_source<card_definition> gamma{ "Gamma" };
    const test::named_definition_source<card_definition> delta{ "Delta" };

    definition_source_library sources;
    REQUIRE(sources.add(alpha, beta, gamma, delta));
    const auto [library, id_map] = sources.compile(
        std::tuple{ shuffle_deck{ .player = player_id{ 0 } } },
        std::tuple{ stop_execution{} }
    );
    const linked_deck deck{
        .cards = {
            id_map.get_id<card_definition>("Alpha"),
            id_map.get_id<card_definition>("Beta"),
            id_map.get_id<card_definition>("Gamma"),
            id_map.get_id<card_definition>("Delta")
        }
    };

    card_table table{ library };
    table.load_deck(player_id{ 0 }, deck);
    const std::array original_ids{
        deck_card_id{ .player_id = player_id{ 0 }, .index = 0 },
        deck_card_id{ .player_id = player_id{ 0 }, .index = 1 },
        deck_card_id{ .player_id = player_id{ 0 }, .index = 2 },
        deck_card_id{ .player_id = player_id{ 0 }, .index = 3 }
    };

    executor target;
    target.enter_entry(library);
    sequence_random random{
        .values = { std::numeric_limits<std::uint32_t>::max(), 0, std::uint32_t{ 0x80000000u } }
    };
    REQUIRE(target.execute_next(table, random));
    CHECK_FALSE(target.execute_next(table, random));
    CHECK(random.position == 3);

    CHECK(deck_definition_values(table[player_id{ 0 }]) == std::vector<size_t>{
        id_map.get_id<card_definition>("Gamma").value(),
        id_map.get_id<card_definition>("Beta").value(),
        id_map.get_id<card_definition>("Alpha").value(),
        id_map.get_id<card_definition>("Delta").value()
    });
    CHECK(
        table[original_ids[0]].definition().id().value()
        == id_map.get_id<card_definition>("Alpha").value()
    );
    CHECK(
        table[original_ids[1]].definition().id().value()
        == id_map.get_id<card_definition>("Beta").value()
    );
    CHECK(
        table[original_ids[2]].definition().id().value()
        == id_map.get_id<card_definition>("Gamma").value()
    );
    CHECK(
        table[original_ids[3]].definition().id().value()
        == id_map.get_id<card_definition>("Delta").value()
    );
}

TEST_CASE("initialize_characters initializes loaded characters in slot order", "[deck][instruction]")
{
    std::vector<std::string_view> initialization_order;
    const initializing_character_source alpha{ "Alpha", 10, &initialization_order };
    const initializing_character_source beta{ "Beta", 20, &initialization_order };

    definition_source_library sources;
    REQUIRE(sources.add(alpha, beta));
    const auto [library, id_map] = sources.compile(
        std::tuple{ initialize_characters{ .player = player_id{ 0 } } },
        std::tuple{ stop_execution{} }
    );
    const linked_deck deck{
        .characters = {
            id_map.get_id<character_view>("Beta"),
            id_map.get_id<character_view>("Alpha")
        }
    };

    card_table table{ library };
    table.load_deck(player_id{ 0 }, deck);
    executor target;
    target.enter_entry(library);
    sequence_random random{ .values = { 2, 3 } };

    REQUIRE(target.execute_next(table, random));
    REQUIRE(initialization_order.size() == 2);
    CHECK(bool(initialization_order[0] == "Beta"));
    CHECK(bool(initialization_order[1] == "Alpha"));
    CHECK(random.position == 2);

    const auto player = table[player_id{ 0 }];
    auto characters = player.characters();
    auto iterator = characters.begin();
    CHECK((*iterator).state().health == 22);
    ++iterator;
    CHECK((*iterator).state().health == 13);
}
