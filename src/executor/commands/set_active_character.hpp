#ifndef GIVM_EXECUTOR_COMMANDS_SET_ACTIVE_CHARACTER_HPP
#define GIVM_EXECUTOR_COMMANDS_SET_ACTIVE_CHARACTER_HPP


#include "../program_writer.hpp"

#include <optional>
#include <utility>
#include <vector>

#include <givm/executor/executor.hpp>
#include "../character_target.hpp"
#include "../broadcast.hpp"
#include <givm/definition.hpp>
#include <givm/executor/instruction.hpp>
#include <givm/utils/debug.hpp>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <givm/macro_define.hpp>

namespace givm::detail
{
    inline void apply_active_character_switch(const definition_library& library,
        unrestricted_table& table, execution_context& context, active_character_changed event)
    {
        auto& state = table[event.current.player_id].state();
        if(state.active_character)
        {
            for(const auto attachment : table[*state.active_character].attachments())
            {
                if(library[attachment.definition_id()].can_handle<this_prepared_skill_use, attachment_view>())
                {
                    attachment.erase();
                    append_removal_record<this_attachment_remove>(context, attachment.id(),
                        attachment_removed{ attachment.id() });
                }
            }
        }
        state.can_plunge = true;
        state.active_character = event.current;
        append_event_record(context, event);
    }

    inline execution_state apply_active_character_change(const definition_library& library,
        unrestricted_table& table, execution_context& context, random_fn&)
    {
        const auto event = get<0>(context.stack().top<active_character_changed>());
        context.stack().pop<active_character_changed>();
        apply_active_character_switch(library, table, context, event);
        return context.enter_next();
    }

    template<bool Fixed, bool Observed>
    inline execution_state prepare_active_character_change(const definition_library& library,
        unrestricted_table& table, execution_context& context, random_fn&)
    {
        std::optional<character_id> target;
        if constexpr(Fixed)
        {
            const auto command = context.instruction_data<1, givm::set_active_character>(library);
            target = resolve_character_target<true>(table, command.target);
            context.advance(instruction_extent<1, givm::set_active_character>);
        }
        else
        {
            target = get<0>(context.stack().top<set_active_character_input>()).current;
#ifndef NDEBUG
            debug_validate_entity(table, *target, "set_active_character", "current");
            if(not table[*target].state().alive || table[*target].state().health == 0)
                throw command_input_error{ "set_active_character", invalid_entity_relation{
                    "current", invalid_entity_relation::reason::defeated_character } };
#endif
            context.stack().pop<set_active_character_input>();
            context.enter_next();
        }
        if(not target) return context.advance(Observed * sizeof(execute_fn));
        const auto& state = table[target->player_id].state();
        if(state.active_character == *target || (state.active_character
            && library.is_control_immune(std::as_const(table)[*state.active_character])))
            return context.advance(Observed * sizeof(execute_fn));
        const active_character_changed event{ .current = *target };
        if constexpr(Observed)
        {
            context.stack().push(event);
            return context.yield(execution_state::active_character_changed);
        }
        apply_active_character_switch(library, table, context, event);
        return continue_execution;
    }

    inline void compile(program_writer& writer, const givm::set_active_character& command, compile_mode mode)
    {
        if(command.target.offset == std::numeric_limits<std::int32_t>::max())
            writer.write(mode == compile_mode::observed
                ? execute_fn{ prepare_active_character_change<false, true> }
                : execute_fn{ prepare_active_character_change<false, false> });
        else
        {
            writer.write(mode == compile_mode::observed
                ? execute_fn{ prepare_active_character_change<true, true> }
                : execute_fn{ prepare_active_character_change<true, false> });
            writer.write(command);
        }
        if(mode == compile_mode::observed) writer.write(execute_fn{ apply_active_character_change });
    }

}

namespace givm::detail
{
    inline std::vector<set_active_character::error_type> check(const set_active_character& command,
        const definition_compile_context&, program_kind kind)
    {
        using reason = set_active_character::error_type::reason;
        std::vector<set_active_character::error_type> errors;
        if(command.target.offset == std::numeric_limits<std::int32_t>::max())
        {
            if(kind != program_kind::response)
                errors.push_back({ .cause = reason::dynamic_input_in_root });
            return errors;
        }
        if(command.target.player != relative_player::self && command.target.player != relative_player::opponent)
            errors.push_back({ .cause = reason::invalid_target_player, .value = static_cast<std::size_t>(command.target.player) });
        if(command.target.selection != character_selection::character)
            errors.push_back({ .cause = reason::invalid_target_selection, .value = static_cast<std::size_t>(command.target.selection) });
        return errors;
    }
}

#include <givm/macro_undef.hpp>

namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const set_active_character& command) noexcept
    {
        return command.target.offset == std::numeric_limits<std::int32_t>::max() ? TInputTypes::template index_of<set_active_character::input_type>() : std::size_t(-1);
    }
}

#endif
