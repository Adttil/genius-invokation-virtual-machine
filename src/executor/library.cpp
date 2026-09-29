#include <givm/executor.hpp>

namespace givm::detail
{
    execution_state execute_return(
        const definition_library&, unrestricted_table& table, execution_context& context, random_fn&)
    {
        const auto record = get<0>(context.stack().top<response_return>());
        table.state().self_player = record.previous_player;
        return context.jump(record.position);
    }

    std::size_t append_commands(program_writer& writer, std::span<const any_command> commands, compile_mode mode,
        const definition_compile_context& context, program_kind kind, std::vector<compile_error>& errors, compile_location location
#ifndef NDEBUG
        , std::vector<debug_input_requirement>* input_markers = nullptr
#endif
    )
    {
        std::size_t inputs_count = 0;
        std::size_t command_index = 0;
        const auto append_command = [&](const auto& command)
        {
            location.command_index = command_index++;
            if constexpr(requires { typename std::remove_cvref_t<decltype(command)>::error_type; })
            {
                auto command_errors = check(command, context, kind);
                for(auto& error : command_errors) errors.push_back({ location, std::move(error) });
                if(not command_errors.empty()) return;
            }
#ifndef NDEBUG
            const auto marker = input_marker<command_input_types>(command);
            if(marker != size_t(-1))
            {
                ++inputs_count;
                if(input_markers) input_markers->push_back({ marker, *location.command_index, debug_command_name<std::remove_cvref_t<decltype(command)>> });
            }
#endif
            compile(writer, command, mode);
        };
        for(const auto& command : commands) std::visit(append_command, command);
        return inputs_count;
    }
}

namespace givm
{
    program_entry definition_compile_context::add_program(std::span<const any_command> commands)
    {
        program_entry result{ program_.size() };
        detail::program_writer writer{ program_ };
#ifndef NDEBUG
        result.library_identity_ = library_identity_;
        result.debug_index_ = debug_programs_.size();
        debug_programs_.push_back({ source_, program_count_, program_.size(), input_markers_.size(), 0 });
#endif
        [[maybe_unused]] const auto inputs_count =
            detail::append_commands(writer, commands, mode_, *this, program_kind::response, errors_,
                { compile_stage::program, source_, program_kind::response, program_count_++, {} }
#ifndef NDEBUG
                , &input_markers_
#endif
            );
#ifndef NDEBUG
        debug_programs_[result.debug_index_].inputs_count = inputs_count;
#endif
        writer.write(detail::execute_fn{ detail::execute_return });
        return result;
    }

    std::expected<definition_compile_result, std::vector<compile_error>> definition_library::compile(
        const definition_source_library& sources, const basic_definition_sources& basics,
        std::span<const any_command> initialization_program, std::span<const any_command> round_program, compile_mode mode)
    {
        std::vector<source_preparation_error> preparation_errors;
        const auto [selected_sources, basic_names] = sources.with_basic_definitions(basics, preparation_errors);
        return compile_prepared(selected_sources, basic_names,
            selected_sources.make_issued_id_map(selected_sources.make_full_selection()),
            initialization_program, round_program, mode, std::move(preparation_errors));
    }

    std::expected<definition_compile_result, std::vector<compile_error>> definition_library::compile(
        const definition_source_library& sources, const basic_definition_sources& basics,
        const definition_selection& selection, std::span<const any_command> initialization_program,
        std::span<const any_command> round_program, compile_mode mode)
    {
        std::vector<source_preparation_error> preparation_errors;
        const auto [selected_sources, basic_names] = sources.with_basic_definitions(basics, preparation_errors);
        auto ids = selected_sources.make_issued_id_map(selected_sources.resolve_selection(selection, basic_names, preparation_errors));
        return compile_prepared(selected_sources, basic_names, std::move(ids),
            initialization_program, round_program, mode, std::move(preparation_errors));
    }

    std::expected<definition_compile_result, std::vector<compile_error>> definition_library::compile_prepared(
        const definition_source_library& sources, const detail::basic_definition_names& basics,
        issued_id_map id_map, std::span<const any_command> initialization_program, std::span<const any_command> round_program,
        compile_mode mode, std::vector<source_preparation_error> preparation_errors)
    {
        using result_type = std::expected<definition_compile_result, std::vector<compile_error>>;
        std::vector<compile_error> errors;
        for(auto& error : preparation_errors)
            std::visit([&](auto&& reason) { errors.push_back({ { compile_stage::source_selection, {}, {}, {}, {} }, std::move(reason) }); }, error);
        definition_library library{ id_map, basics };
        const auto definitions = library.prepare_definitions(sources, id_map);
        library.prepare_history_layouts(definitions, id_map, mode, errors);
        const detail::definition_source_declarations root_declarations{};
        definition_compile_context context{ id_map, definitions, library.basic_ids_, library.program_, root_declarations,
            mode, library.history_layouts_, {}, true, errors, compile_stage::program, std::nullopt
#ifndef NDEBUG
            , library.input_markers_, library.debug_programs_, library.debug_library_identity_
#endif
        };
        detail::program_writer writer{ library.program_ };
        detail::append_commands(writer, initialization_program, mode,
            context, program_kind::initialization, errors, { compile_stage::program, {}, program_kind::initialization, 0, {} });
        const detail::execution_position round_start = writer.position();
        detail::compile(writer, detail::round_program_begin{}, mode);
        const detail::execution_position round_entry = writer.position();
        detail::append_commands(writer, round_program, mode,
            context, program_kind::round, errors, { compile_stage::program, {}, program_kind::round, 0, {} });
        detail::compile(writer, detail::round_program_repeat{ round_start, round_entry }, mode);
        definition_types::each([&]<class TCategory>
        {
            for(const auto& definition : std::get<definition_types::index_of<TCategory>()>(definitions))
                library.compile_source(definition, id_map, definitions, mode, errors);
        });
        if(not errors.empty()) return result_type{ std::unexpected{ std::move(errors) } };
        library.complete_dynamic_queries();
        detail::finalize_program(library.program_);
        return result_type{ definition_compile_result{ .library = std::move(library), .id_map = std::move(id_map) } };
    }

    std::expected<definition_compile_result, std::vector<compile_error>> compile(
        const definition_source_library& sources, const basic_definition_sources& basics,
        std::span<const any_command> initialization_program, std::span<const any_command> round_program, compile_mode mode)
    {
        return definition_library::compile(sources, basics, initialization_program, round_program, mode);
    }

    std::expected<definition_compile_result, std::vector<compile_error>> compile(
        const definition_source_library& sources, const basic_definition_sources& basics, const definition_selection& selection,
        std::span<const any_command> initialization_program, std::span<const any_command> round_program, compile_mode mode)
    {
        return definition_library::compile(sources, basics, selection, initialization_program, round_program, mode);
    }
}
