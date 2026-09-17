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
        static givm::character_state query(const definition_type& data, const givm::character_initial_state&)
        {
            return data.initial_state;
        }
    };
}

#endif
