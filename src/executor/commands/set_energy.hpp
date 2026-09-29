#ifndef GIVM_EXECUTOR_COMMANDS_SET_ENERGY_HPP
#define GIVM_EXECUTOR_COMMANDS_SET_ENERGY_HPP

#include "../program_writer.hpp"

#include <vector>

#ifndef NDEBUG
#include "../debug_validation.hpp"
#endif

#include <algorithm>

#include <givm/executor/executor.hpp>
#include "../character_target.hpp"
#include <givm/definition.hpp>
#include <givm/macro_define.hpp>

namespace givm::detail
{
    template<bool Fixed>
    inline execution_state execute_energy_change(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn&)
    {
        set_energy_input input;
        if constexpr(Fixed)
        {
            const auto& command = context.instruction_data<1, set_energy>(library);
            context.advance(instruction_extent<1, set_energy>);
            const auto target = resolve_character_target<false>(table, command.target);
            if(not target) return continue_execution;
            input = { *target, command.value };
        }
        else
        {
            input = get<0>(context.stack().top<set_energy_input>());
            context.stack().pop<set_energy_input>();
            context.enter_next();
        }
#ifndef NDEBUG
        debug_validate_entity(table, input.target, "set_energy", "target");
#endif
        GIVM_ASSERT(table[input.target].is_valid());
        auto& state = table[input.target].state();
        state.energy = std::min(input.value, state.max_energy);
        return continue_execution;
    }

    inline void compile(program_writer& writer, const givm::set_energy& command, compile_mode)
    {
        if(command.target.offset == std::numeric_limits<std::int32_t>::max())
            writer.write(execute_fn{ execute_energy_change<false> });
        else
        {
            GIVM_ASSERT(command.target.selection == character_selection::character);
            [[assume(command.target.selection == character_selection::character)]];
            writer.write(execute_fn{ execute_energy_change<true> });
            writer.write(command);
        }
    }
}

namespace givm::detail
{
    inline std::vector<set_energy::error_type> check(const set_energy& command, const definition_compile_context&, program_kind kind)
    {
        using reason = set_energy::error_type::reason;
        std::vector<set_energy::error_type> errors;
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

#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes>
    constexpr std::size_t input_marker(const set_energy& command) noexcept
    {
        return command.target.offset == std::numeric_limits<std::int32_t>::max() ? TInputTypes::template index_of<set_energy::input_type>() : std::size_t(-1);
    }
}
#endif

#endif
