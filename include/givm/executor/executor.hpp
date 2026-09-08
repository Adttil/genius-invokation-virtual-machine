#ifndef GIVM_EXECUTOR_EXECUTOR_HPP
#define GIVM_EXECUTOR_EXECUTOR_HPP

#include <cstdint>

#include "random_fn.hpp"
#include "../definition.hpp"
#include "../table.hpp"
#include "../utils/debug.hpp"
#include "../utils/stack.hpp"

#include "../macro_define.hpp"

namespace givm
{
    struct assume_enabled_t
    {
        explicit constexpr assume_enabled_t() noexcept = default;
    };

    inline constexpr assume_enabled_t assume_enabled;

    using stage_t = std::uint8_t;

    class execution_context
    {
    public:
        struct return_info
        {
            execution_position return_position;
        };

        constexpr execution_context(const execution_context&) = default;
        constexpr execution_context(execution_context&&) noexcept = default;
        constexpr execution_context& operator=(const execution_context&) = default;
        constexpr execution_context& operator=(execution_context&&) noexcept = default;
        constexpr ~execution_context() = default;

        constexpr auto& stack(this auto& self) noexcept
        {
            return self.stack_;
        }

        constexpr bool enter_next() noexcept
        {
            reset_current_stage();
            ++position_;
            return true;
        }

        constexpr bool yield_next() noexcept
        {
            reset_current_stage();
            ++position_;
            return false;
        }

        constexpr bool yield() noexcept
        {
            return false;
        }

        template<class TContext>
        constexpr bool enter(program_entry<TContext> entry)
        {
            GIVM_ASSERT(not entry.is_null());
            push_activation({ .return_position = position_ });
            position_ = entry.position_;
            return true;
        }

        constexpr bool end_game(game_result result) noexcept
        {
            GIVM_ASSERT(result != game_result::no_result);
            position_ = static_cast<execution_position>(result);
            return false;
        }

        constexpr stage_t& current_stage() noexcept
        {
            auto&& [stage] = stack_.top<stage_t>();
            return stage;
        }

    private:
        friend class executor;

        constexpr execution_context() noexcept = default;

        constexpr void push_activation(return_info info)
        {
            stack_.push(info, stage_t{});
        }

        constexpr void reset_current_stage() noexcept
        {
            current_stage() = stage_t{};
        }

        constexpr void return_from_subroutine()
        {
            auto&& [info_ref, stage] = stack_.top<return_info, stage_t>();
            GIVM_ASSERT(stage == stage_t{});
            const auto info = info_ref;
            stack_.pop<return_info, stage_t>();
            position_ = info.return_position;
        }

        execution_position position_ = detail::null_program_position;
        frame_stack stack_;
    };

    class executor
    {
    public:
        constexpr executor() noexcept = default;
        constexpr executor(const executor&) = default;
        constexpr executor(executor&&) noexcept = default;
        constexpr executor& operator=(const executor&) = default;
        constexpr executor& operator=(executor&&) noexcept = default;
        constexpr ~executor() = default;

        constexpr void clear() noexcept
        {
            context_.stack_.clear();
        }

        constexpr void enter_entry(const definition_library& library)
        {
            context_.stack_.clear();
            context_.stack_.push(stage_t{});
            context_.position_ = library.entry();
        }

        constexpr execution_position position() const noexcept
        {
            return context_.position_;
        }

        constexpr game_result status() const noexcept
        {
            switch(position())
            {
            case detail::player_0_win_position:
                return game_result::player_0_win;
            case detail::player_1_win_position:
                return game_result::player_1_win;
            case detail::both_loss_position:
                return game_result::both_loss;
            default:
                return game_result::no_result;
            }
        }

        template<class TRandom>
        constexpr bool execute_next(card_table& table, TRandom& random_source)
        {
            const auto& library = table.definition_library();
            random_fn random{ random_source };
            const auto& instruction = library.instruction(position());
            const bool result = instruction.execute(table, context_, random);
            settle_control_instructions(library);
            return result;
        }

        constexpr auto& stack(this auto& self) noexcept
        {
            return self.context_.stack_;
        }

    private:
        constexpr void settle_control_instructions(const definition_library& library)
        {
            while(true)
            {
                const auto& instruction = library.instruction(context_.position_);
                if(instruction.template is<detail::return_instruction>())
                {
                    context_.return_from_subroutine();
                    continue;
                }
                if(instruction.template is<detail::jump_instruction>())
                {
                    context_.position_ = instruction.template as<detail::jump_instruction>().target;
                    continue;
                }
                return;
            }
        }

        execution_context context_;
    };
}

#include "../macro_undef.hpp"
#endif
