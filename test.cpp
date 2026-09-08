#include <array>
#include <bitset>
#include <concepts>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <typeinfo>

#include <givm/givm.hpp>
#include <givm/definition.hpp>
#include <givm/utils/debug.hpp>

#include <givm/macro_define.hpp>

using namespace givm;

namespace
{
#define MANUAL_CHECK(expression) \
    do \
    { \
        if(not (expression)) \
        { \
            debug_log("manual test check failed at line {}: {}", __LINE__, #expression); \
            return 1; \
        } \
    } while(false)

template<class TEntity>
struct TestDefinitionSource
{
    using definition_category = TEntity;
    using entity_type = TEntity;

    struct definition_type
    {
        std::string_view name;
        definition_id<support_view> support;
    };

    std::string_view name_;

    constexpr std::string_view name() const
    {
        return name_;
    }

    constexpr auto tags() const
    {
        return std::array{ std::string_view{ "test" } };
    }

    constexpr auto support_dependencies() const
    {
        return std::array{ std::string_view{ "TestSupport" } };
    }

    definition_type compile(definition_compile_context& context) const
    {
        return {
            .name = name_,
            .support = context.resolve_id<support_view>("TestSupport")
        };
    }

    template<class TEvent>
    static handler_program_entry_t<TEvent> handle(
        const definition_type& data,
        const entity_type&,
        TEvent& event,
        const card_table&,
        random_fn& random
    )
    {
        if constexpr(std::same_as<TEntity, character_view> && std::same_as<TEvent, character_initialization>)
        {
            event.state = {
                .max_health = 10,
                .max_energy = 3,
                .health = 10,
                .energy = 0
            };
        }
        if constexpr(std::same_as<TEvent, dice_roll_preparation>)
        {
            for(auto& fixed_dice : event.fixed_dice)
            {
                if(fixed_dice[elemental_dice::pyro] < 2)
                {
                    fixed_dice[elemental_dice::pyro] = 2;
                }
            }
        }
        random();
        debug_log("\"{}\" handled by {}", typeid(TEvent).name(), data.name);
        return handler_program_entry_t<TEvent>::null();
    }
};

struct TestCardDefinitionSource
{
    using definition_category = card_definition;

    struct definition_type
    {
        std::string_view name;
    };

    std::string_view name_;

    constexpr std::string_view name() const
    {
        return name_;
    }

    constexpr auto tags() const
    {
        return std::array{ std::string_view{ "test" } };
    }

    constexpr definition_type compile(definition_compile_context&) const
    {
        return { .name = name_ };
    }

    template<class TView, class TEvent>
    static handler_program_entry_t<TEvent> handle(
        const definition_type& data,
        const TView&,
        TEvent& event,
        const card_table&,
        random_fn& random
    )
    {
        if constexpr(std::same_as<TEvent, dice_roll_preparation>)
        {
            for(auto& fixed_dice : event.fixed_dice)
            {
                if(fixed_dice[elemental_dice::hydro] < 2)
                {
                    fixed_dice[elemental_dice::hydro] = 2;
                }
            }
        }
        random();
        debug_log("\"{}\" handled by {}", typeid(TEvent).name(), data.name);
        return handler_program_entry_t<TEvent>::null();
    }
};

struct TestStatusDefinitionSource
{
    using definition_category = status_definition;

    struct definition_type
    {
        std::string_view name;
    };

    std::string_view name_;

    constexpr std::string_view name() const
    {
        return name_;
    }

    constexpr auto tags() const
    {
        return std::array{ std::string_view{ "test" } };
    }

    constexpr definition_type compile(definition_compile_context&) const
    {
        return { .name = name_ };
    }

    template<class TView, class TEvent>
    static handler_program_entry_t<TEvent> handle(
        const definition_type& data,
        const TView&,
        TEvent&,
        const card_table&,
        random_fn& random
    )
    {
        random();
        debug_log("\"{}\" handled by {}", typeid(TEvent).name(), data.name);
        return handler_program_entry_t<TEvent>::null();
    }
};

    struct increasing_random
    {
        std::uint32_t value = 0;

        std::uint32_t operator()() noexcept
        {
            return value++;
        }
    };

    template<class TRandom>
    void run_until_blocked(
        executor& target,
        card_table& table,
        TRandom& random
    )
    {
        while(
            target.status() == game_result::no_result
            && target.execute_next(table, random)
        )
        {
        }
    }

    bool submit_selector(
        executor& target,
        player_id player,
        std::bitset<selection_capacity> selected
    )
    {
        if(target.stack().empty())
        {
            return false;
        }

        auto&& [input, stage] = target.stack().top<selector, stage_t>();
        (void)stage;
        input.player = player;
        input.selected = selected;
        return true;
    }

    bool submit_active_character_selection(executor& target, character_id selected)
    {
        if(target.stack().empty())
        {
            return false;
        }

        auto&& [input, stage] = target.stack().top<character_id, stage_t>();
        (void)stage;
        input = selected;
        return true;
    }

    dice_counts select_one_available_dice(const card_table& table, player_id player)
    {
        dice_counts result;
        const auto& dice = table[player].state().dice;
        for(size_t dice_index = 0; dice_index < 8; ++dice_index)
        {
            const auto dice_kind = static_cast<elemental_dice>(dice_index);
            if(dice[dice_kind] != 0)
            {
                ++result[dice_kind];
                break;
            }
        }
        return result;
    }

    bool waiting_for_action(const executor& target, const card_table& table)
    {
        return target.status() == game_result::no_result
            && table.definition_library().instruction(target.position()).is<begin_action>();
    }

    bool submit_action(
        executor& target,
        const card_table& table,
        action_request request,
        action_argument argument = {}
    )
    {
        if(not waiting_for_action(target, table))
        {
            return false;
        }

        auto&& [stored_argument, stored_request, stage] =
            target.stack().top<action_argument, action_request, stage_t>();
        (void)stage;
        stored_argument = argument;
        stored_request = request;
        return true;
    }

    template<class TRandom>
    bool perform_first_available_switch(executor& target, card_table& table, TRandom& random)
    {
        if(not waiting_for_action(target, table))
        {
            return false;
        }

        const auto acting_player = table.state().active_player;
        const auto active_before = table[acting_player].state().active_character;
        if(not active_before.has_value())
        {
            return false;
        }

        action_argument argument{
            .paid_dice = select_one_available_dice(table, acting_player)
        };
        if(argument.paid_dice.total() != 1)
        {
            return false;
        }

        const auto dice_before = table[acting_player].state().dice.total();
        if(not submit_action(
            target,
            table,
            action_request{
                .request_kind = action_request_kind::do_action,
                .action_kind = action_kind::switch_active,
                .action_index = 0
            },
            argument
        ))
        {
            return false;
        }

        run_until_blocked(target, table, random);

        const auto active_after = table[acting_player].state().active_character;
        return active_after.has_value()
            && *active_after != *active_before
            && table[acting_player].state().dice.total() == dice_before - 1
            && table.state().active_player == other_player(acting_player)
            && waiting_for_action(target, table);
    }

    template<class TRandom>
    bool submit_round_end(executor& target, card_table& table, TRandom& random)
    {
        if(not submit_action(
            target,
            table,
            action_request{
                .request_kind = action_request_kind::do_action,
                .action_kind = action_kind::declare_round_end
            }
        ))
        {
            return false;
        }

        run_until_blocked(target, table, random);
        return true;
    }

    bool game_ended(const executor& target)
    {
        return target.status() != game_result::no_result;
    }
}

int main()
{
    TestCardDefinitionSource card_source{ "TestCard" };
    TestCardDefinitionSource other_card_source{ "OtherCard" };
    TestStatusDefinitionSource status_source{ "TestStatus" };
    TestDefinitionSource<support_view> support_source{ "TestSupport" };
    TestDefinitionSource<summon_view> summon_source{ "TestSummon" };
    TestDefinitionSource<combat_status_view> combat_status_source{ "TestCombatStatus" };
    TestDefinitionSource<character_view> character_source{ "TestCharacter" };
    TestDefinitionSource<skill_view> skill_source{ "TestSkill" };
    TestDefinitionSource<attachment_view> attachment_source{ "TestAttachment" };

    definition_source_library source_library;
    MANUAL_CHECK(source_library.add(
        card_source,
        other_card_source,
        status_source,
        support_source,
        summon_source,
        combat_status_source,
        character_source,
        skill_source,
        attachment_source
    ));

    constexpr std::uint32_t max_rounds = 2;
    const auto initialization = std::tuple{
        shuffle_deck{ .player = player_id{ 0 } },
        shuffle_deck{ .player = player_id{ 1 } },
        initialize_characters{ .player = player_id{ 0 } },
        initialize_characters{ .player = player_id{ 1 } },
        draw_cards{ .count = 5, .player = relative_player::current },
        draw_cards{ .count = 5, .player = relative_player::other },
        replace_cards_both{},
        select_active_character_both{}
    };
    const auto round = std::tuple{
        start_round{ .max_rounds = max_rounds },
        start_dice_roll_phase{ .count = 8, .reroll_count = { 1, 1 } },
        start_battle{},
        begin_action{},
        end_round{},
        draw_cards{ .count = 2, .player = relative_player::current },
        draw_cards{ .count = 2, .player = relative_player::other }
    };
    const auto [library, id_map] = source_library.compile(initialization, round);
    std::array<std::string_view, 10> card_names;
    card_names.fill(card_source.name());
    std::array<std::string_view, 3> character_names;
    character_names.fill(character_source.name());
    const auto deck = link_deck(id_map, card_names, character_names);

    card_table table{
        library,
        game_parameters{
            .hand_limit = 10
        }
    };
    table.load_deck(player_id{ 0 }, deck);
    table.load_deck(player_id{ 1 }, deck);
    executor target;
    target.enter_entry(table.definition_library());
    increasing_random random;

    table.state().active_player = player_id{ 0 };

    run_until_blocked(target, table, random);
    MANUAL_CHECK(table[player_id{ 0 }].hand_card_count() == 5);
    MANUAL_CHECK(table[player_id{ 1 }].hand_card_count() == 5);

    MANUAL_CHECK(submit_selector(target, player_id{ 0 }, std::bitset<selection_capacity>{ 0b11 }));
    run_until_blocked(target, table, random);
    MANUAL_CHECK(submit_selector(target, player_id{ 1 }, {}));
    run_until_blocked(target, table, random);

    MANUAL_CHECK(table[player_id{ 0 }].hand_card_count() == 5);
    MANUAL_CHECK(table[player_id{ 0 }].deck_card_count() == 5);
    MANUAL_CHECK(table[player_id{ 1 }].hand_card_count() == 5);
    MANUAL_CHECK(table[player_id{ 1 }].deck_card_count() == 5);

    MANUAL_CHECK(table.definition_library().instruction(target.position()).is<select_active_character_both>());
    MANUAL_CHECK(submit_active_character_selection(
        target,
        character_id{ .player_id = player_id{ 0 }, .index = 0 }
    ));
    run_until_blocked(target, table, random);

    MANUAL_CHECK(table.definition_library().instruction(target.position()).is<select_active_character_both>());
    MANUAL_CHECK(submit_active_character_selection(
        target,
        character_id{ .player_id = player_id{ 1 }, .index = 0 }
    ));
    run_until_blocked(target, table, random);

    MANUAL_CHECK(table[player_id{ 0 }].state().active_character.has_value());
    MANUAL_CHECK(table[player_id{ 1 }].state().active_character.has_value());

    for(std::uint32_t round = 1; round <= max_rounds; ++round)
    {
        MANUAL_CHECK(table.definition_library().instruction(target.position()).is<start_dice_roll_phase>());
        MANUAL_CHECK(table.state().round_number == round);
        MANUAL_CHECK(table[player_id{ 0 }].state().dice.total() == 8);
        MANUAL_CHECK(table[player_id{ 1 }].state().dice.total() == 8);

        MANUAL_CHECK(submit_selector(target, player_id{ 0 }, {}));
        run_until_blocked(target, table, random);
        MANUAL_CHECK(table.definition_library().instruction(target.position()).is<start_dice_roll_phase>());

        MANUAL_CHECK(submit_selector(target, player_id{ 1 }, {}));
        run_until_blocked(target, table, random);
        MANUAL_CHECK(waiting_for_action(target, table));
        MANUAL_CHECK(table.state().active_player == player_id{ 0 });

        MANUAL_CHECK(perform_first_available_switch(target, table, random));
        MANUAL_CHECK(perform_first_available_switch(target, table, random));
        MANUAL_CHECK(table.state().active_player == player_id{ 0 });

        MANUAL_CHECK(submit_round_end(target, table, random));
        MANUAL_CHECK(not game_ended(target));
        MANUAL_CHECK(table.state().first_ended);
        MANUAL_CHECK(table.state().active_player == player_id{ 1 });

        MANUAL_CHECK(submit_round_end(target, table, random));
        MANUAL_CHECK(table[player_id{ 0 }].hand_card_count() == 5 + round * 2);
        MANUAL_CHECK(table[player_id{ 1 }].hand_card_count() == 5 + round * 2);
        if(round < max_rounds)
        {
            MANUAL_CHECK(not game_ended(target));
        }
    }

    MANUAL_CHECK(game_ended(target));
    MANUAL_CHECK(table.state().round_number == max_rounds);
    MANUAL_CHECK(target.status() == game_result::both_loss);
    return 0;
}

#undef MANUAL_CHECK
