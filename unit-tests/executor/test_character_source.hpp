#ifndef GIVM_TEST_CHARACTER_SOURCE_HPP
#define GIVM_TEST_CHARACTER_SOURCE_HPP

#include <string_view>
#include <givm/givm.hpp>

namespace givm::test
{
    struct initialized_character_source
    {
        using definition_category = character_view;
        struct definition_type { character_state initial_state; };

        std::string_view source_name = "Character";
        character_state initial_state{ .max_health = 10, .max_energy = 3, .health = 10 };

        std::string_view name() const noexcept { return source_name; }
        definition_type compile(definition_compile_context&) const { return { initial_state }; }
        static handler_program_entry_t<character_initialization> handle(
            const definition_type& data, const character_view&, character_initialization& event,
            const card_table&, random_fn&)
        {
            event.state = data.initial_state;
            return handler_program_entry_t<character_initialization>::null();
        }
    };
}

#endif
