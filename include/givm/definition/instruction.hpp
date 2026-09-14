#ifndef GIVM_DEFINITION_INSTRUCTION_HPP
#define GIVM_DEFINITION_INSTRUCTION_HPP

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

#include "../utils/debug.hpp"

#include "../macro_define.hpp"

namespace givm
{
    enum class execution_state : std::uint8_t;

    class card_table;
    class definition_library;
    class random_fn;

    template<class TCostEvent>
    struct onpay_context{};

    template<class TContext>
    class any_instruction_for;

    namespace detail
    {
        class execution_context;
        struct executor_access;

        template<class TInstruction>
        struct instruction_implementation
        {
            template<bool Observed>
            static execution_state execute(
                const TInstruction& instruction, const definition_library& library,
                card_table& table, execution_context& context, random_fn& random
            )
            {
                return instruction.execute(library, table, context, random);
            }
        };

        inline constexpr std::size_t instruction_storage_size = 64;

        using instruction_execute_fn = execution_state (*)(
            const void*, const definition_library&, card_table&, execution_context&, random_fn&
        );

        struct instruction_rtti
        {
            instruction_execute_fn execute;
            instruction_execute_fn execute_observed;
        };

        template<class TInstruction>
        inline constexpr instruction_rtti instruction_rtti_of{
            +[](const void* storage, const definition_library& library,
                card_table& table, execution_context& context, random_fn& random)
            {
                const auto& instruction = *reinterpret_cast<const TInstruction*>(storage);
                return instruction_implementation<TInstruction>::template execute<false>(instruction, library, table, context, random);
            },
            +[](const void* storage, const definition_library& library,
                card_table& table, execution_context& context, random_fn& random)
            {
                const auto& instruction = *reinterpret_cast<const TInstruction*>(storage);
                return instruction_implementation<TInstruction>::template execute<true>(instruction, library, table, context, random);
            }
        };

        class any_instruction_view
        {
        public:
            constexpr explicit any_instruction_view(const instruction_rtti* rtti, const unsigned char* storage) noexcept
            : rtti_{ rtti }, storage_{ storage }
            {}

            template<class TInstruction>
            constexpr bool is() const noexcept
            {
                return rtti_ == &instruction_rtti_of<TInstruction>;
            }

            template<class TInstruction>
            constexpr const TInstruction& as() const noexcept
            {
                GIVM_ASSERT(is<TInstruction>());
                return *reinterpret_cast<const TInstruction*>(storage_);
            }

            template<bool Observed>
            constexpr execution_state execute(
                const definition_library& library, card_table& table, execution_context& context, random_fn& random
            ) const
            {
                if constexpr(Observed)
                    return rtti_->execute_observed(storage_, library, table, context, random);
                else
                    return rtti_->execute(storage_, library, table, context, random);
            }

            constexpr const void* type_index() const noexcept
            {
                return rtti_;
            }

        private:
            const instruction_rtti* rtti_;
            const unsigned char* storage_;
        };

        class any_instruction
        {
        public:
            constexpr any_instruction() noexcept = default;

            template<class TContext>
            constexpr explicit any_instruction(const any_instruction_for<TContext>& instruction) noexcept
            : any_instruction{ static_cast<const any_instruction&>(instruction) }
            {}

            template<class TInstruction>
            constexpr explicit any_instruction(const TInstruction& instruction) noexcept
            {
                std::memcpy(storage_, &instruction, sizeof(TInstruction));
                rtti_ = &instruction_rtti_of<TInstruction>;
            }

            constexpr operator any_instruction_view() const noexcept
            {
                return any_instruction_view{ rtti_, storage_ };
            }

            template<bool Observed>
            constexpr execution_state execute(
                const definition_library& library, card_table& table, execution_context& context, random_fn& random
            ) const
            {
                if constexpr(Observed)
                    return rtti_->execute_observed(storage_, library, table, context, random);
                else
                    return rtti_->execute(storage_, library, table, context, random);
            }

        private:
            alignas(std::max_align_t) unsigned char storage_[instruction_storage_size];
            const instruction_rtti* rtti_ = nullptr;
        };

        static_assert(std::is_trivially_copyable_v<any_instruction>);
    }

    namespace detail
    {
        template<class TInstruction>
        inline constexpr auto instruction_type_index = &instruction_rtti_of<TInstruction>;
    }

    template<class TInstruction, class TContext>
    concept instruction_compatible_with =
        std::same_as<
            std::remove_cvref_t<typename std::remove_cvref_t<TInstruction>::context_type>,
            void
        >
        || std::same_as<
            std::remove_cvref_t<typename std::remove_cvref_t<TInstruction>::context_type>,
            std::remove_cvref_t<TContext>
        >;

    template<class TContext>
    class any_instruction_for
    {
    public:
        using context_type = TContext;

        template<instruction_compatible_with<TContext> TInstruction>
        constexpr explicit any_instruction_for(const TInstruction& instruction) noexcept
        : instruction_{ instruction }
        {
            static_assert(sizeof(TInstruction) <= detail::instruction_storage_size,
                          "instruction is too large for erased storage");
            static_assert(alignof(TInstruction) <= alignof(std::max_align_t),
                          "instruction alignment is too large for erased storage");
#if defined(__cpp_lib_is_implicit_lifetime)
            static_assert(std::is_implicit_lifetime_v<TInstruction>,
                          "instruction must be an implicit-lifetime type");
#endif
            static_assert(std::is_trivially_copyable_v<TInstruction>,
                          "instruction must be trivially copyable");
            static_assert(std::is_trivially_destructible_v<TInstruction>,
                          "instruction must be trivially destructible");
        }

    private:
        friend class detail::any_instruction;

        constexpr explicit operator const detail::any_instruction&() const noexcept
        {
            return instruction_;
        }

        detail::any_instruction instruction_;
    };
}

#include "../macro_undef.hpp"

#endif
