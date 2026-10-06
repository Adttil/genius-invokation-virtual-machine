#include <givm/compile.hpp>
#include <givm/basic_definitions.hpp>

#include "interface_layer_sources.hpp"

#include <array>
#include <span>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace givm_test::interface_layers
{
    // Do not include the runtime-facing factory header here: this translation
    // unit verifies compilation through compile.hpp without the runtime interface.
    givm::definition_library make_layered_library(givm::linked_deck& deck, bool observed)
    {
        auto sources = make_layered_sources();
        sources.add(givm::genshin_impact::reaction_sources_3_3_0());
        const auto basics = givm::genshin_impact::reaction_names_3_3_0;
        const std::array<givm::any_command, 3> round{
            givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::player_0_win } };
        auto compiled = givm::compile(sources, basics, std::span<const givm::any_command>{}, round,
            observed ? givm::compile_mode::observed : givm::compile_mode::normal);
        if(not compiled) throw std::logic_error{ givm::error_string(compiled.error()) };
        auto linked = givm::link_deck(compiled->id_map, std::array<std::string_view, 0>{},
            std::array<std::string_view, 1>{ "LayeredCharacter" });
        if(not linked) throw std::logic_error{ givm::error_string(linked.error()) };
        deck = std::move(*linked);
        return std::move(compiled->library);
    }
}
