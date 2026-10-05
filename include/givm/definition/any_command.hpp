#ifndef GIVM_DEFINITION_ANY_COMMAND_HPP
#define GIVM_DEFINITION_ANY_COMMAND_HPP

#include <cstddef>
#include <variant>

#include "../utils/type_list.hpp"
#include "commands.hpp"

namespace givm::detail
{
    using command_types = type_list<
        insert_deck_card,
        enter_character,
        shuffle_deck,
        set_active_character,
        select_active_character_both,
        draw_cards,
        create_hand_card,
        discard_hand_card,
        discard_deck_cards,
        add_support,
        set_support_state,
        modify_support_state,
        remove_support,
        summon,
        add_summon,
        set_summon_state,
        modify_summon_state,
        remove_summon,
        generate_combat_status,
        add_combat_status,
        set_combat_status_state,
        modify_combat_status_state,
        remove_combat_status,
        attach,
        set_attachment_state,
        modify_attachment_state,
        add_attachment,
        transfer_attachment,
        remove_attachment,
        replace_cards,
        replace_cards_both,
        start_round,
        begin_action,
        use_skill,
        set_skill_state,
        set_energy,
        modify_energy,
        end_round,
        end_game,
        start_dice_roll_phase,
        reroll_dice,
        add_dice,
        remove_dice,
        start_battle,
        deal_damage,
        apply_element,
        heal,
        increase_max_health,
        return_response,
        defer_program,
        end_segment,
        settle>;

    using command_input_types = decltype([]<class... T>(type_list<T...>)
    {
        return type_list_cat<decltype([]
        {
            if constexpr(requires { typename T::input_type; })
                return type_list<typename T::input_type>{};
            else
                return type_list<>{};
        }())...>{};
    }(command_types{}));
}

namespace givm
{
    using any_command = detail::command_types::apply<std::variant>;
}

#endif
