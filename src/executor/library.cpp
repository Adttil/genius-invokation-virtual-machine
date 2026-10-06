#include <givm/executor.hpp>

#include "program_writer.hpp"
#include "commands.hpp"
#ifndef NDEBUG
namespace givm::detail
{
    template<class T> inline constexpr std::string_view debug_command_name;
    template<> inline constexpr std::string_view debug_command_name<insert_deck_card> = "insert_deck_card";
    template<> inline constexpr std::string_view debug_command_name<enter_character> = "enter_character";
    template<> inline constexpr std::string_view debug_command_name<shuffle_deck> = "shuffle_deck";
    template<> inline constexpr std::string_view debug_command_name<set_active_character> = "set_active_character";
    template<> inline constexpr std::string_view debug_command_name<select_active_character_both> = "select_active_character_both";
    template<> inline constexpr std::string_view debug_command_name<draw_cards> = "draw_cards";
    template<> inline constexpr std::string_view debug_command_name<create_hand_card> = "create_hand_card";
    template<> inline constexpr std::string_view debug_command_name<discard_hand_card> = "discard_hand_card";
    template<> inline constexpr std::string_view debug_command_name<discard_deck_cards> = "discard_deck_cards";
    template<> inline constexpr std::string_view debug_command_name<add_support> = "add_support";
    template<> inline constexpr std::string_view debug_command_name<set_support_state> = "set_support_state";
    template<> inline constexpr std::string_view debug_command_name<modify_support_state> = "modify_support_state";
    template<> inline constexpr std::string_view debug_command_name<remove_support> = "remove_support";
    template<> inline constexpr std::string_view debug_command_name<summon> = "summon";
    template<> inline constexpr std::string_view debug_command_name<add_summon> = "add_summon";
    template<> inline constexpr std::string_view debug_command_name<set_summon_state> = "set_summon_state";
    template<> inline constexpr std::string_view debug_command_name<modify_summon_state> = "modify_summon_state";
    template<> inline constexpr std::string_view debug_command_name<remove_summon> = "remove_summon";
    template<> inline constexpr std::string_view debug_command_name<generate_combat_status> = "generate_combat_status";
    template<> inline constexpr std::string_view debug_command_name<add_combat_status> = "add_combat_status";
    template<> inline constexpr std::string_view debug_command_name<set_combat_status_state> = "set_combat_status_state";
    template<> inline constexpr std::string_view debug_command_name<modify_combat_status_state> = "modify_combat_status_state";
    template<> inline constexpr std::string_view debug_command_name<remove_combat_status> = "remove_combat_status";
    template<> inline constexpr std::string_view debug_command_name<attach> = "attach";
    template<> inline constexpr std::string_view debug_command_name<set_attachment_state> = "set_attachment_state";
    template<> inline constexpr std::string_view debug_command_name<modify_attachment_state> = "modify_attachment_state";
    template<> inline constexpr std::string_view debug_command_name<add_attachment> = "add_attachment";
    template<> inline constexpr std::string_view debug_command_name<transfer_attachment> = "transfer_attachment";
    template<> inline constexpr std::string_view debug_command_name<remove_attachment> = "remove_attachment";
    template<> inline constexpr std::string_view debug_command_name<givm::replace_cards> = "replace_cards";
    template<> inline constexpr std::string_view debug_command_name<replace_cards_both> = "replace_cards_both";
    template<> inline constexpr std::string_view debug_command_name<start_round> = "start_round";
    template<> inline constexpr std::string_view debug_command_name<begin_action> = "begin_action";
    template<> inline constexpr std::string_view debug_command_name<use_skill> = "use_skill";
    template<> inline constexpr std::string_view debug_command_name<set_skill_state> = "set_skill_state";
    template<> inline constexpr std::string_view debug_command_name<set_energy> = "set_energy";
    template<> inline constexpr std::string_view debug_command_name<modify_energy> = "modify_energy";
    template<> inline constexpr std::string_view debug_command_name<end_round> = "end_round";
    template<> inline constexpr std::string_view debug_command_name<end_game> = "end_game";
    template<> inline constexpr std::string_view debug_command_name<start_dice_roll_phase> = "start_dice_roll_phase";
    template<> inline constexpr std::string_view debug_command_name<reroll_dice> = "reroll_dice";
    template<> inline constexpr std::string_view debug_command_name<add_dice> = "add_dice";
    template<> inline constexpr std::string_view debug_command_name<remove_dice> = "remove_dice";
    template<> inline constexpr std::string_view debug_command_name<start_battle> = "start_battle";
    template<> inline constexpr std::string_view debug_command_name<deal_damage> = "deal_damage";
    template<> inline constexpr std::string_view debug_command_name<apply_element> = "apply_element";
    template<> inline constexpr std::string_view debug_command_name<heal> = "heal";
    template<> inline constexpr std::string_view debug_command_name<increase_max_health> = "increase_max_health";
    template<> inline constexpr std::string_view debug_command_name<return_response> = "return_response";
    template<> inline constexpr std::string_view debug_command_name<defer_program> = "defer_program";
    template<> inline constexpr std::string_view debug_command_name<end_segment> = "end_segment";
    template<> inline constexpr std::string_view debug_command_name<settle> = "settle";

    constinit const std::array<std::string_view, command_input_types::size()> debug_input_command_names = []
    {
        std::array<std::string_view, command_input_types::size()> result{};
        command_types::each([&]<class T>
        {
            if constexpr(requires { typename T::input_type; })
                result[command_input_types::index_of<typename T::input_type>()] = debug_command_name<T>;
        });
        return result;
    }();
}
#endif

#ifndef NDEBUG
namespace givm::detail
{
    template<class TInputTypes, class T>
    requires (not requires { typename T::input_type; })
    constexpr std::size_t input_marker(const T&) noexcept { return std::size_t(-1); }
}
#endif

namespace givm::detail
{
    std::size_t append_commands(program_writer& writer, std::span<const any_command> commands, compile_mode mode,
        const definition_compile_context& context, program_kind kind, std::vector<compile_error>& errors, compile_location location
#ifndef NDEBUG
        , std::size_t library_identity, const std::vector<debug_program_info>& programs
        , std::vector<debug_input_requirement>& input_markers
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
            if constexpr(std::same_as<std::remove_cvref_t<decltype(command)>, defer_program>)
            {
                if(command.input.entry)
                {
                    try
                    {
                        const program_input_validator validator{ { library_identity, programs, input_markers } };
                        validator.validate_value(command.input);
                    }
                    catch(const program_input_error& error)
                    {
                        errors.push_back({ location, fixed_program_input_error{ error.reason } });
                        return;
                    }
                }
            }
            const auto marker = input_marker<command_input_types>(command);
            if(marker != size_t(-1))
            {
                ++inputs_count;
                if(kind == program_kind::response)
                    input_markers.push_back({ marker, *location.command_index,
                        debug_command_name<std::remove_cvref_t<decltype(command)>> });
            }
#endif
            compile(writer, command, mode);
        };
        for(const auto& command : commands)
        {
            std::visit(append_command, command);
            if(std::holds_alternative<return_response>(command)) break;
        }
        return inputs_count;
    }
}

namespace givm
{
    definition_library::definition_library(const issued_id_map& id_map, const reaction_definition_names& basics)
    : program_(sizeof(detail::execute_fn), 0), tag_names_(id_map.tag_names().begin(), id_map.tag_names().end()),
      default_reactions_{}
    {
        for(std::size_t index = 0; index != elemental_reaction_count; ++index)
            default_reactions_[index] = id_map.get_id<reaction_view>(basics[static_cast<elemental_reaction>(index + 1)]);
        constexpr std::array<std::string_view, static_cast<size_t>(givm::equipment_type::none)> equipment_tag_names{
            "weapon", "artifact", "talent", "technique"
        };
        for(size_t index = 0; index != equipment_tags_.size(); ++index)
        {
            if(id_map.has_tag(equipment_tag_names[index]))
            {
                equipment_tags_[index] = id_map.get_tag_id(equipment_tag_names[index]);
            }
        }
        constexpr std::array<std::string_view, 3> skill_tag_names{ "normal_attack", "elemental_skill", "elemental_burst" };
        for(size_t index = 0; index != skill_tags_.size(); ++index)
            if(id_map.has_tag(skill_tag_names[index])) skill_tags_[index] = id_map.get_tag_id(skill_tag_names[index]);
        if(id_map.has_tag("control")) control_tag_ = id_map.get_tag_id("control");
        if(id_map.has_tag("control_immunity")) control_immunity_tag_ = id_map.get_tag_id("control_immunity");
        if(id_map.has_tag("remove_at_zero_usages"))
            remove_at_zero_usages_tag_ = id_map.get_tag_id("remove_at_zero_usages");
    }

    template<class TDefinitionType>
    void definition_library::compile_source(
        const detail::compile_definition<TDefinitionType>& definition,
        const issued_id_map& id_map,
        const detail::compile_definitions& definitions,
        compile_mode mode,
        std::vector<compile_error>& errors
    )
    {
        auto& bucket = bucket_for<TDefinitionType>();
        definition_id<history_summary_definition> own_history;
        if constexpr(std::same_as<TDefinitionType, history_summary_definition>)
            own_history = definition.id;
        definition_compile_context context{ id_map, definitions, default_reactions_, program_, *definition.declarations,
            mode, history_layouts_, own_history, true, errors,
            compile_stage::definition, definition_name{ definition_types::index_of<TDefinitionType>(), std::string{ bucket.names[definition.id.value()] } }
#ifndef NDEBUG
            , input_markers_, debug_programs_, debug_library_identity_
#endif
        };
        auto& data = bucket.data[definition.id.value()];
        const auto errors_before = errors.size();
        data = definition.source->compile(context);
        if(errors.size() != errors_before) return;
        if constexpr(supported_queries<TDefinitionType>::size() != 0)
        {
            supported_queries<TDefinitionType>::each([&]<class TQuery>
            {
                if constexpr(std::is_empty_v<TQuery>)
                {
                    constexpr auto index = supported_queries<TDefinitionType>::template index_of<TQuery>();
                    auto query_fn = std::get<index>(definition.static_queries);
                    if(not query_fn) query_fn = detail::default_query<TQuery>;
                    std::get<index>(bucket.queries).push_back(query_fn(data, TQuery{}));
                }
            });
        }
    }

    void definition_library::prepare_history_layouts(const detail::compile_definitions& definitions, const issued_id_map& ids, compile_mode mode,
        std::vector<compile_error>& errors)
    {
        history_layouts_.resize(ids.definition_count<history_summary_definition>());
        const auto& summaries = std::get<definition_types::index_of<history_summary_definition>()>(definitions);
        for(const auto& definition : summaries)
        {
            const auto& source = *definition.source;
            const auto id = definition.id;
            definition_compile_context context{ ids, definitions, default_reactions_, program_, *definition.declarations, mode, history_layouts_, id, false,
                errors, compile_stage::history_layout, definition_name{ definition_types::index_of<history_summary_definition>(),
                    std::string{ definition.bucket->names[id.value()] } }
#ifndef NDEBUG
                , input_markers_, debug_programs_, debug_library_identity_
#endif
            };
            auto& layout = history_layouts_[id.value()];
            std::vector<std::size_t> field_indices;
            std::size_t field_index = 0;
            for(auto&& member : source.rtti_->history.layout(source.source_, context))
            {
                std::visit([&](auto&& declaration)
                {
                    using T = typename std::remove_cvref_t<decltype(declaration)>::value_type;
                    constexpr auto size = sizeof(std::remove_extent_t<T>);
                    constexpr auto alignment = alignof(std::remove_extent_t<T>);
                    const auto count = [&] -> std::size_t
                    {
                        if constexpr(std::is_unbounded_array_v<T>) return declaration.count;
                        else return 1;
                    }();
                    if(declaration.name.empty()) context.report(history_field_empty_name{ field_index });
                    const auto found = std::ranges::find(layout.fields, declaration.name, &detail::compiled_history_field::name);
                    const bool duplicate = found != layout.fields.end();
                    if(duplicate) context.report(history_field_duplicate_name{ declaration.name,
                        field_indices[static_cast<std::size_t>(found - layout.fields.begin())], field_index });
                    constexpr auto maximum = std::numeric_limits<std::size_t>::max();
                    auto offset = layout.size;
                    bool overflow = offset > maximum - (alignment - 1);
                    if(not overflow)
                    {
                        offset = (offset + alignment - 1) & ~(alignment - 1);
                        overflow = count > (maximum - offset) / size;
                    }
                    if(overflow) context.report(history_field_layout_overflow{ declaration.name, field_index, count, size, alignment, layout.size });
                    if(not duplicate)
                    {
                        field_indices.push_back(field_index);
                        layout.fields.push_back({ std::move(declaration.name), history_field_key<T>{ overflow ? 0 : offset,
                            overflow && std::is_unbounded_array_v<T> ? 0 : count } });
                        if(not overflow) layout.size = offset + size * count;
                    }
                }, member);
                ++field_index;
            }
        }
        for(std::size_t index = 0; index != history_layouts_.size(); ++index)
        {
            auto& layout = history_layouts_[index];
            constexpr auto alignment = alignof(std::max_align_t);
            constexpr auto maximum = std::numeric_limits<std::size_t>::max();
            const bool alignment_overflow = history_size_ > maximum - (alignment - 1);
            const auto offset = alignment_overflow ? 0 : (history_size_ + alignment - 1) & ~(alignment - 1);
            if(alignment_overflow || layout.size > maximum - offset)
            {
                errors.push_back({ { compile_stage::history_layout,
                    definition_name{ definition_types::index_of<history_summary_definition>(), std::string{ summaries[index].bucket->names[index] } }, {}, {}, {} },
                    history_storage_layout_overflow{ history_size_, layout.size, alignment } });
                layout.offset = 0;
            }
            else
            {
                layout.offset = offset;
                history_size_ = offset + layout.size;
            }
        }
    }

    tag_mask definition_library::make_tag_mask(
        const std::vector<std::string_view>& tags,
        const issued_id_map& id_map
    )
    {
        tag_mask result{ id_map.tag_names().size() };
        for(std::string_view tag : tags)
        {
            result.set(id_map.get_tag_id(tag));
        }
        return result;
    }

    detail::compile_definitions definition_library::prepare_definitions(const definition_source_library& sources, const issued_id_map& ids)
    {
        detail::compile_definitions result;
        definition_types::each([&]<class TCategory>
        {
            auto& definitions = std::get<definition_types::index_of<TCategory>()>(result);
            auto& bucket = bucket_for<TCategory>();
            const auto count = ids.definition_count<TCategory>();
            definitions.reserve(count);
            bucket.names.resize(count);
            bucket.data.resize(count);
            bucket.tags.resize(count);
            for(const auto& entry : sources.bucket_for<TCategory>().entries)
            {
                if(not ids.has<TCategory>(entry.name)) continue;
                const auto id = ids.get_id<TCategory>(entry.name);
                definitions.push_back({ &entry.source, id, &entry.declarations, &bucket });
                bucket.names[id.value()] = entry.name;
                bucket.tags[id.value()] = make_tag_mask(entry.declarations.tags, ids);
            }
            std::ranges::sort(definitions, {}, [](const auto& definition) { return definition.id.value(); });
            if constexpr(views_of_definition<TCategory>::size() != 0)
            {
                views_of_definition<TCategory>::each([&]<class TView>
                {
                    if constexpr(subscribed_events<TView>::size() != 0)
                    {
                        auto& handles = std::get<views_of_definition<TCategory>::template index_of<TView>()>(bucket.handle_fns);
                        subscribed_events<TView>::each([&]<class TEvent>
                        {
                            auto& functions = std::get<subscribed_events<TView>::template index_of<TEvent>()>(handles);
                            functions.reserve(count);
                            for(const auto& definition : definitions)
                                functions.push_back(definition.source->template get_handle_fn<TView, TEvent>());
                        });
                    }
                });
            }
            if constexpr(supported_queries<TCategory>::size() != 0)
            {
                supported_queries<TCategory>::each([&]<class TQuery>
                {
                    constexpr auto index = supported_queries<TCategory>::template index_of<TQuery>();
                    auto& queries = std::get<index>(bucket.queries);
                    queries.reserve(count);
                    for(auto& definition : definitions)
                    {
                        const auto query_fn = definition.source->template get_query_fn<TQuery>();
                        if constexpr(std::is_empty_v<TQuery>)
                            std::get<index>(definition.static_queries) = query_fn;
                        else
                            queries.push_back(query_fn);
                    }
                });
            }
            if constexpr(std::same_as<TCategory, history_summary_definition>)
            {
                for(auto& definition : definitions) definition.history = &history_handlers_;
                subscribed_events<TCategory>::each([&]<class TEvent>
                {
                    constexpr auto index = subscribed_events<TCategory>::template index_of<TEvent>();
                    auto& handlers = std::get<index>(history_handlers_);
                    for(const auto& definition : definitions)
                    {
                        const auto& source = *definition.source;
                        if(const auto handler = std::get<index>(source.rtti_->history.handles)(source.source_))
                            handlers.push_back({ definition.id.value(), handler });
                    }
                });
            }
        });
        return result;
    }

    void definition_library::complete_dynamic_queries()
    {
        definition_types::each([&]<class TCategory>
        {
            if constexpr(supported_queries<TCategory>::size() != 0)
            {
                supported_queries<TCategory>::each([&]<class TQuery>
                {
                    if constexpr(not std::is_empty_v<TQuery>)
                    {
                        auto& queries = std::get<supported_queries<TCategory>::template index_of<TQuery>()>(
                            bucket_for<TCategory>().queries);
                        for(auto& query : queries)
                            if(not query) query = detail::default_query<TQuery>;
                    }
                });
            }
        });
    }

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
                , library_identity_, debug_programs_, input_markers_
#endif
            );
#ifndef NDEBUG
        debug_programs_[result.debug_index_].inputs_count = inputs_count;
#endif
        if(std::ranges::none_of(commands, [](const auto& command)
            { return std::holds_alternative<return_response>(command); }))
            detail::compile(writer, return_response{ return_response::null }, mode_);
        return result;
    }

    std::expected<definition_compile_result, std::vector<compile_error>> definition_library::compile(
        const definition_source_library& sources, const reaction_definition_names& basics,
        std::span<const any_command> initialization_program, std::span<const any_command> round_program, compile_mode mode)
    {
        std::vector<source_preparation_error> preparation_errors;
        sources.resolve_selection({}, basics, preparation_errors);
        return compile_prepared(sources, basics,
            sources.make_issued_id_map(sources.make_full_selection()),
            initialization_program, round_program, mode, std::move(preparation_errors));
    }

    std::expected<definition_compile_result, std::vector<compile_error>> definition_library::compile(
        const definition_source_library& sources, const reaction_definition_names& basics,
        const definition_selection& selection, std::span<const any_command> initialization_program,
        std::span<const any_command> round_program, compile_mode mode)
    {
        std::vector<source_preparation_error> preparation_errors;
        auto ids = sources.make_issued_id_map(sources.resolve_selection(selection, basics, preparation_errors));
        return compile_prepared(sources, basics, std::move(ids),
            initialization_program, round_program, mode, std::move(preparation_errors));
    }

    std::expected<definition_compile_result, std::vector<compile_error>> definition_library::compile_prepared(
        const definition_source_library& sources, const reaction_definition_names& basics,
        issued_id_map id_map, std::span<const any_command> initialization_program, std::span<const any_command> round_program,
        compile_mode mode, std::vector<source_preparation_error> preparation_errors)
    {
        using result_type = std::expected<definition_compile_result, std::vector<compile_error>>;
        std::vector<compile_error> errors;
        for(auto& error : preparation_errors)
            std::visit([&](auto&& reason) { errors.push_back({ { compile_stage::source_selection, {}, {}, {}, {} }, std::move(reason) }); }, error);
        if(not errors.empty()) return result_type{ std::unexpected{ std::move(errors) } };
        definition_library library{ id_map, basics };
        const auto definitions = library.prepare_definitions(sources, id_map);
        library.prepare_history_layouts(definitions, id_map, mode, errors);
        const detail::definition_source_declarations root_declarations{};
        definition_compile_context context{ id_map, definitions, library.default_reactions_, library.program_, root_declarations,
            mode, library.history_layouts_, {}, true, errors, compile_stage::program, std::nullopt
#ifndef NDEBUG
            , library.input_markers_, library.debug_programs_, library.debug_library_identity_
#endif
        };
        detail::program_writer writer{ library.program_ };
        detail::append_commands(writer, initialization_program, mode,
            context, program_kind::initialization, errors, { compile_stage::program, {}, program_kind::initialization, 0, {} }
#ifndef NDEBUG
            , library.debug_library_identity_, library.debug_programs_, library.input_markers_
#endif
        );
        const detail::execution_position round_start = writer.position();
        detail::compile(writer, detail::round_program_begin{}, mode);
        const detail::execution_position round_entry = writer.position();
        detail::append_commands(writer, round_program, mode,
            context, program_kind::round, errors, { compile_stage::program, {}, program_kind::round, 0, {} }
#ifndef NDEBUG
            , library.debug_library_identity_, library.debug_programs_, library.input_markers_
#endif
        );
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
        const definition_source_library& sources, const reaction_definition_names& basics,
        std::span<const any_command> initialization_program, std::span<const any_command> round_program, compile_mode mode)
    {
        return definition_library::compile(sources, basics, initialization_program, round_program, mode);
    }

    std::expected<definition_compile_result, std::vector<compile_error>> compile(
        const definition_source_library& sources, const reaction_definition_names& basics, const definition_selection& selection,
        std::span<const any_command> initialization_program, std::span<const any_command> round_program, compile_mode mode)
    {
        return definition_library::compile(sources, basics, selection, initialization_program, round_program, mode);
    }
}
