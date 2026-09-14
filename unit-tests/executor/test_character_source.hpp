#ifndef GIVM_TEST_CHARACTER_SOURCE_HPP
#define GIVM_TEST_CHARACTER_SOURCE_HPP

#include <string_view>
#include <givm/givm.hpp>

namespace givm::test
{
    struct initialized_character_source
    {
        using definition_category = givm::character_view;
        struct definition_type { givm::character_state initial_state; };

        std::string_view source_name = "Character";
        givm::character_state initial_state{ .max_health = 10, .max_energy = 3, .health = 10 };

        std::string_view name() const noexcept { return source_name; }
        definition_type compile(givm::definition_compile_context&) const { return { initial_state }; }
        static givm::handler_program_entry_t<givm::character_initialization> handle(
            const definition_type& data, const givm::character_view&, givm::character_initialization& event,
            const givm::table&, givm::random_fn&)
        {
            event.state = data.initial_state;
            return givm::handler_program_entry_t<givm::character_initialization>::null();
        }
    };
}

#endif
