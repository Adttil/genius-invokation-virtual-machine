#include <array>
#include <concepts>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <givm/definition.hpp>

using namespace givm;

namespace
{
    struct stored_instruction
    {
        using context_type = void;

        int value;

        execution_state execute(const definition_library&, card_table&, detail::execution_context&, random_fn&) const
        {
            return execution_state{};
        }
    };

    struct unknown_context_instruction
    {
        execution_state execute(const definition_library&, card_table&, detail::execution_context&, random_fn&) const
        {
            return execution_state{};
        }
    };

    struct event_context{};
    struct other_context{};
    struct cost_event{};

    struct event_instruction
    {
        using context_type = event_context;

        int value;

        execution_state execute(const definition_library&, card_table&, detail::execution_context&, random_fn&) const
        {
            return execution_state{};
        }
    };

    struct context_free_instruction
    {
        using context_type = void;

        execution_state execute(const definition_library&, card_table&, detail::execution_context&, random_fn&) const
        {
            return execution_state{};
        }
    };

    struct onpay_instruction
    {
        using context_type = onpay_context<cost_event>;

        execution_state execute(const definition_library&, card_table&, detail::execution_context&, random_fn&) const
        {
            return execution_state{};
        }
    };

    template<class T>
    concept instruction_interface = requires(
        const T& instruction,
        const definition_library& library,
        card_table& table,
        detail::execution_context& context,
        random_fn& random
    )
    {
        typename T::context_type;
        { instruction.execute(library, table, context, random) } -> std::same_as<execution_state>;
    };
}

TEST_CASE("typed erased instructions enforce their context when constructed", "[definition][instruction]")
{
    using event_any_instruction = any_instruction_for<event_context>;
    using other_any_instruction = any_instruction_for<other_context>;
    using context_free_any_instruction = any_instruction_for<void>;
    using onpay_any_instruction = any_instruction_for<onpay_context<cost_event>>;

    STATIC_REQUIRE(instruction_compatible_with<stored_instruction, event_context>);
    STATIC_REQUIRE(instruction_compatible_with<stored_instruction, void>);
    STATIC_REQUIRE(not instruction_compatible_with<unknown_context_instruction, event_context>);
    STATIC_REQUIRE(not instruction_compatible_with<unknown_context_instruction, void>);
    STATIC_REQUIRE(instruction_compatible_with<event_instruction, event_context>);
    STATIC_REQUIRE(not instruction_compatible_with<event_instruction, other_context>);
    STATIC_REQUIRE(instruction_compatible_with<context_free_instruction, void>);
    STATIC_REQUIRE(instruction_compatible_with<context_free_instruction, event_context>);
    STATIC_REQUIRE(instruction_compatible_with<onpay_instruction, onpay_context<cost_event>>);
    STATIC_REQUIRE(not instruction_compatible_with<onpay_instruction, cost_event>);
    STATIC_REQUIRE(instruction_compatible_with<event_any_instruction, event_context>);
    STATIC_REQUIRE(not instruction_compatible_with<event_any_instruction, other_context>);
    STATIC_REQUIRE(not instruction_interface<event_any_instruction>);

    STATIC_REQUIRE(std::constructible_from<event_any_instruction, stored_instruction>);
    STATIC_REQUIRE(std::constructible_from<event_any_instruction, event_instruction>);
    STATIC_REQUIRE(not std::constructible_from<other_any_instruction, event_instruction>);
    STATIC_REQUIRE(std::constructible_from<context_free_any_instruction, stored_instruction>);
    STATIC_REQUIRE(std::constructible_from<context_free_any_instruction, context_free_instruction>);
    STATIC_REQUIRE(std::constructible_from<event_any_instruction, context_free_any_instruction>);
    STATIC_REQUIRE(not std::constructible_from<event_any_instruction, unknown_context_instruction>);
    STATIC_REQUIRE(std::constructible_from<onpay_any_instruction, stored_instruction>);
    STATIC_REQUIRE(std::constructible_from<onpay_any_instruction, onpay_instruction>);
    STATIC_REQUIRE(not std::constructible_from<event_any_instruction, other_any_instruction>);
    STATIC_REQUIRE(std::copy_constructible<event_any_instruction>);

    const std::array<event_any_instruction, 2> static_program{
        event_any_instruction{ stored_instruction{ .value = 1 } },
        event_any_instruction{ event_instruction{ .value = 2 } }
    };
    CHECK(static_program.size() == 2);

    std::vector<event_any_instruction> erased_sequence;
    erased_sequence.emplace_back(stored_instruction{ .value = 3 });
    erased_sequence.emplace_back(event_instruction{ .value = 4 });
    CHECK(erased_sequence.size() == 2);
}
