#ifndef GIVM_UNIT_TESTS_TABLE_TEST_DEFINITION_LIBRARY_HPP
#define GIVM_UNIT_TESTS_TABLE_TEST_DEFINITION_LIBRARY_HPP

#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>

#include <givm/definition/source_library.hpp>

namespace givm::test
{
    struct test_program_instruction
    {
        using context_type = void;

        bool execute(card_table&, execution_context&, random_fn&) const noexcept
        {
            return false;
        }
    };

    template<class TDefinition>
    struct named_definition_source
    {
        using definition_category = TDefinition;

        struct definition_type{};

        std::string_view source_name;

        constexpr std::string_view name() const noexcept
        {
            return source_name;
        }

        constexpr definition_type compile(definition_compile_context&) const noexcept
        {
            return {};
        }
    };

    template<class... TSources>
    auto compile_definitions(const TSources&... sources)
    {
        definition_source_library source_library;
        if(not source_library.add(sources...))
        {
            throw std::logic_error{ "invalid unit-test definition sources" };
        }

        const auto program = std::tuple{ test_program_instruction{} };
        return source_library.compile(program, program);
    }

    template<class TInitialization, class TRound, class... TSources>
    auto compile_definitions_with_program(
        TInitialization&& initialization,
        TRound&& round,
        const TSources&... sources
    )
    {
        definition_source_library source_library;
        if(not source_library.add(sources...))
        {
            throw std::logic_error{ "invalid unit-test definition sources" };
        }

        return source_library.compile(
            std::forward<TInitialization>(initialization),
            std::forward<TRound>(round)
        );
    }
}

#endif
