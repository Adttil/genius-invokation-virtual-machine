#ifndef GIVM_UNIT_TESTS_TABLE_TEST_DEFINITION_LIBRARY_HPP
#define GIVM_UNIT_TESTS_TABLE_TEST_DEFINITION_LIBRARY_HPP

#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>

#include <givm/definition.hpp>
#include <givm/executor.hpp>

namespace givm::test
{
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

        constexpr definition_type compile(givm::definition_compile_context&) const noexcept
        {
            return {};
        }
    };

    template<class... TSources>
    auto compile_definitions(const TSources&... sources)
    {
        givm::definition_source_library source_library;
        if(not source_library.add(sources...))
        {
            throw std::logic_error{ "invalid unit-test definition sources" };
        }

        const auto program = std::tuple{ givm::end_game{ givm::game_result::both_loss } };
        return compile(source_library, program, program, givm::compile_mode::normal);
    }

    template<class TInitialization, class TRound, class... TSources>
    auto compile_definitions_with_program(
        givm::compile_mode mode,
        TInitialization&& initialization,
        TRound&& round,
        const TSources&... sources
    )
    {
        givm::definition_source_library source_library;
        if(not source_library.add(sources...))
        {
            throw std::logic_error{ "invalid unit-test definition sources" };
        }

        return compile(source_library,
            std::forward<TInitialization>(initialization),
            std::forward<TRound>(round), mode
        );
    }
}

#endif
