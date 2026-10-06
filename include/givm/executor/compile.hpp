#ifndef GIVM_EXECUTOR_COMPILE_HPP
#define GIVM_EXECUTOR_COMPILE_HPP

#include "../definition_source_interface.hpp"
#include "library.hpp"
#include "compile_error.hpp"

namespace givm
{
    using definition_selection = std::array<std::span<const std::string_view>, definition_types::size()>;

    struct definition_compile_result
    {
        definition_library library;
        issued_id_map id_map;
    };

    inline std::expected<definition_compile_result, std::vector<compile_error>> compile(
        const definition_source_library& sources, const reaction_definition_names& basics,
        std::span<const any_command> initialization_program, std::span<const any_command> round_program, compile_mode mode)
    {
        return definition_library::compile(sources, basics, initialization_program, round_program, mode);
    }

    inline std::expected<definition_compile_result, std::vector<compile_error>> compile(
        const definition_source_library& sources, const reaction_definition_names& basics, const definition_selection& selection,
        std::span<const any_command> initialization_program, std::span<const any_command> round_program, compile_mode mode)
    {
        return definition_library::compile(sources, basics, selection, initialization_program, round_program, mode);
    }

    template<detail::command_sequence TInitializationSequence, detail::command_sequence TRoundSequence>
    requires (not std::convertible_to<TInitializationSequence, std::span<const any_command>>
        || not std::convertible_to<TRoundSequence, std::span<const any_command>>)
    inline auto compile(
        const definition_source_library& sources,
        const reaction_definition_names& basics,
        TInitializationSequence&& initialization_program,
        TRoundSequence&& round_program,
        compile_mode mode
    )
    {
        return definition_library::compile(
            sources, basics,
            std::forward<TInitializationSequence>(initialization_program),
            std::forward<TRoundSequence>(round_program), mode
        );
    }

    template<detail::command_sequence TInitializationSequence, detail::command_sequence TRoundSequence>
    requires (not std::convertible_to<TInitializationSequence, std::span<const any_command>>
        || not std::convertible_to<TRoundSequence, std::span<const any_command>>)
    inline auto compile(
        const definition_source_library& sources,
        const reaction_definition_names& basics,
        const definition_selection& selection,
        TInitializationSequence&& initialization_program,
        TRoundSequence&& round_program,
        compile_mode mode
    )
    {
        return definition_library::compile(
            sources, basics, selection,
            std::forward<TInitializationSequence>(initialization_program),
            std::forward<TRoundSequence>(round_program), mode
        );
    }
}

#endif
