#ifndef GIVM_EXECUTOR_COMMANDS_RETURN_RESPONSE_HPP
#define GIVM_EXECUTOR_COMMANDS_RETURN_RESPONSE_HPP

#include "../program_writer.hpp"
#include "../settlement.hpp"
#include <vector>
#include <givm/executor/executor.hpp>
#include <givm/definition.hpp>

namespace givm::detail
{
    template<bool Fixed, bool Immediate>
    inline execution_state execute_response_return(
        const definition_library& library, unrestricted_table& table, execution_context& context, random_fn&)
    {
        std::uint32_t index;
        if constexpr(Fixed)
            index = context.instruction_data<1, return_response>(library).index;
        else
        {
            index = get<0>(context.stack().top<return_response_input>()).index;
            context.stack().pop<return_response_input>();
        }
        auto& record = get<0>(context.stack().top<response_return>());
        record.result = index;
        if constexpr(Immediate)
        {
            table.state().self_player = record.previous_player;
            return context.jump(record.position);
        }
        else if constexpr(Fixed) return context.advance(instruction_extent<1, return_response>);
        else return context.enter_next();
    }

    inline execution_state begin_effect(const definition_library&, unrestricted_table&,
        execution_context& context, random_fn&)
    {
        begin_response<true>(context);
        return context.enter_next();
    }

    inline execution_state finish_effect(const definition_library&, unrestricted_table& table,
        execution_context& context, random_fn&)
    {
        const auto record = get<0>(context.stack().top<response_return>());
        end_response<true>(context);
        table.state().self_player = record.previous_player;
        return context.jump(record.position);
    }

    template<bool Immediate>
    void compile_effect_return(program_writer& writer, const return_response& command)
    {
        if(command.index == return_response::dynamic)
            writer.write(execute_fn{ execute_response_return<false, Immediate> });
        else
        {
            writer.write(execute_fn{ execute_response_return<true, Immediate> });
            writer.write(command);
        }
        if constexpr(not Immediate)
        {
            compile_settlement(writer, begin_settlement);
            writer.write(execute_fn{ finish_effect });
        }
    }

    inline void compile_effect_return(program_writer& writer, const return_response& command, event_category category)
    {
        if(category == event_category::immediate) compile_effect_return<true>(writer, command);
        else compile_effect_return<false>(writer, command);
    }

    inline std::vector<return_response_error> check(
        const return_response&, const definition_compile_context&, program_kind kind)
    {
        if(kind != program_kind::response)
            return { { return_response_error::reason::return_in_root } };
        return {};
    }

    template<class TInputTypes>
    constexpr std::size_t input_marker(const return_response& command) noexcept
    {
        return command.index == return_response::dynamic
            ? TInputTypes::template index_of<return_response_input>() : std::size_t(-1);
    }
}

#endif
