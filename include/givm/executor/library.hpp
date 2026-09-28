#ifndef GIVM_EXECUTOR_LIBRARY_HPP
#define GIVM_EXECUTOR_LIBRARY_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <expected>
#include <limits>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "instruction.hpp"
#include "compile_error.hpp"
#include "../definition.hpp"
#include "../enums/equipment_type.hpp"
#include "../utils/stack.hpp"

#include "../macro_define.hpp"

namespace givm::detail
{
    struct basic_definition_ids
    {
        definition_id<combat_status_view> dendro_core;
        definition_id<combat_status_view> catalyzing_field;
        definition_id<summon_view> burning_flame;
        definition_id<attachment_view> frozen;
    };

    template<class TView>
    struct definition_handle_vectors
    {
        template<class... TEvents>
        using tuple_for = std::tuple<std::vector<handle_fn_t<TView, TEvents>>...>;

        using type = subscribed_events<TView>::template apply<tuple_for>;
    };

    template<class... TViews>
    using definition_handle_groups = std::tuple<typename definition_handle_vectors<TViews>::type...>;

    template<class... TQueries>
    using definition_query_vectors = std::tuple<std::vector<std::conditional_t<
        std::is_empty_v<TQueries>, typename TQueries::result_t, query_fn_t<TQueries>>>...>;

    template<class TCategory>
    struct definition_bucket
    {
        std::vector<std::string_view> names;
        std::vector<definition_data> data;
        std::vector<tag_mask> tags;
        views_of_definition<TCategory>::template apply<definition_handle_groups> handle_fns;
#ifdef _MSC_VER
        [[msvc::no_unique_address]]
#else
        [[no_unique_address]]
#endif
        typename supported_queries<TCategory>::template apply<definition_query_vectors> queries;
    };

    template<class... TCategories>
    using definition_bucket_tuple = std::tuple<definition_bucket<TCategories>...>;

    template<class TEvent>
    struct history_handler
    {
        std::size_t index;
        history_handle_fn_t<TEvent> function;
    };

    template<class... TEvents>
    using history_handler_lists = std::tuple<std::vector<history_handler<TEvents>>...>;

    using definition_history_handlers = subscribed_events<history_summary_definition>::apply<history_handler_lists>;

    template<class... TQueries>
    using static_query_functions = std::tuple<std::conditional_t<
        std::is_empty_v<TQueries>, query_fn_t<TQueries>, std::monostate>...>;

    template<class TCategory>
    struct compile_definition
    {
        const definition_source_view<TCategory>* source;
        definition_id<TCategory> id;
        const definition_source_declarations* declarations;
        const definition_bucket<TCategory>* bucket;
        supported_queries<TCategory>::template apply<static_query_functions> static_queries{};
        std::conditional_t<std::same_as<TCategory, history_summary_definition>,
            const definition_history_handlers*, std::monostate> history{};
    };

    template<class... TCategories>
    using compile_definition_vectors = std::tuple<std::vector<compile_definition<TCategories>>...>;

    using compile_definitions = definition_types::apply<compile_definition_vectors>;

    struct compiled_history_field
    {
        std::string name;
        dynamic_history_field field;
    };

    struct compiled_history_summary
    {
        std::size_t offset{};
        std::size_t size{};
        std::vector<compiled_history_field> fields;

        const dynamic_history_field* find(std::string_view name) const noexcept
        {
            const auto found = std::ranges::find(fields, name, &compiled_history_field::name);
            return found == fields.end() ? nullptr : &found->field;
        }
    };

    struct response_return
    {
        player_id previous_player;
        execution_position position;
    };

    template<class TExecutionContext>
    inline execution_state execute_return(
        const definition_library&, unrestricted_table& table, TExecutionContext& context, random_fn&)
    {
        const auto record = get<0>(context.stack().template top<response_return>());
        table.state().self_player = record.previous_player;
        return context.jump(record.position);
    }
    // Internal compilation inputs; definition sources do not expose these commands.
    struct round_program_begin {};

    struct round_program_repeat
    {
        execution_position round_start;
        execution_position round_entry;
    };

    template<class TSequence, class TContext>
    inline std::size_t append_commands(program_writer& writer, TSequence&& commands, compile_mode mode,
        const TContext& context, program_kind kind, std::vector<compile_error>& errors, compile_location location
#ifndef NDEBUG
        , std::vector<std::size_t>* input_markers = nullptr
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
                if(input_markers) input_markers->push_back(marker);
            }
#endif
            compile(writer, command, mode);
        };
        const auto append = [&]<class TCommand>(TCommand&& command)
        {
            if constexpr(requires { std::variant_size<std::remove_cvref_t<TCommand>>::value; })
            {
                std::visit(append_command, command);
            }
            else
            {
                append_command(command);
            }
        };

        if constexpr(std::ranges::range<TSequence>)
        {
            for(auto&& command : commands)
            {
                append(std::forward<decltype(command)>(command));
            }
        }
        else
        {
            [&]<std::size_t... I>(std::index_sequence<I...>)
            {
                using std::get;
                (append(get<I>(std::forward<TSequence>(commands))), ...);
            }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<TSequence>>>{});
        }
        return inputs_count;
    }
}

namespace givm
{
    class definition_compile_context
    {
    public:
        template<class TCategory>
        class definition_view
        {
        public:
            definition_id<TCategory> id() const noexcept
            {
                return definition_ ? definition_->id : definition_id<TCategory>{};
            }

            std::string_view name() const noexcept
            {
                return definition_ ? definition_->bucket->names[definition_->id.value()] : std::string_view{};
            }

            std::span<const std::string_view> tags() const noexcept
            {
                return definition_ ? std::span<const std::string_view>{ definition_->declarations->tags } : std::span<const std::string_view>{};
            }

            bool has_tag(std::string_view name) const noexcept
            {
                return std::ranges::find(tags(), name) != tags().end();
            }

            template<class TDependencyCategory>
            std::span<const std::string_view> dependencies() const noexcept
            {
                if(not definition_) return {};
                return definition_->declarations->dependencies[definition_types::index_of<TDependencyCategory>()];
            }

            template<class TEvent, class TView = TCategory>
            bool can_handle() const noexcept
            {
                if(not definition_) return false;
                if constexpr(std::same_as<TCategory, history_summary_definition>
                    && std::same_as<TView, history_summary_definition>)
                {
                    if constexpr(requires { subscribed_events<TCategory>::template index_of<TEvent>(); })
                        return std::ranges::binary_search(
                            std::get<subscribed_events<TCategory>::template index_of<TEvent>()>(*definition_->history),
                            definition_->id.value(), {}, &detail::history_handler<TEvent>::index);
                    else return false;
                }
                else if constexpr(requires { views_of_definition<TCategory>::template index_of<TView>(); })
                {
                    if constexpr(requires { subscribed_events<TView>::template index_of<TEvent>(); })
                        return std::get<subscribed_events<TView>::template index_of<TEvent>()>(
                            std::get<views_of_definition<TCategory>::template index_of<TView>()>(definition_->bucket->handle_fns))
                            [definition_->id.value()] != nullptr;
                    else return false;
                }
                else return false;
            }

            template<class TQuery>
            bool has_query() const noexcept
            {
                if(not definition_) return false;
                if constexpr(requires { supported_queries<TCategory>::template index_of<TQuery>(); })
                {
                    constexpr auto index = supported_queries<TCategory>::template index_of<TQuery>();
                    if constexpr(std::is_empty_v<TQuery>)
                        return std::get<index>(definition_->static_queries) != nullptr;
                    else
                        return std::get<index>(definition_->bucket->queries)[definition_->id.value()] != nullptr;
                }
                else return false;
            }

        private:
            definition_view() noexcept = default;

            definition_view(const detail::compile_definition<TCategory>& definition)
            : definition_{ &definition }
            {}

            const detail::compile_definition<TCategory>* definition_ = nullptr;
            friend class definition_compile_context;
        };

        template<class TCategory>
        auto definitions() const noexcept
        {
            return std::span{ std::get<definition_types::index_of<TCategory>()>(definitions_) }
                | std::views::transform([](const auto& definition) { return definition_view<TCategory>{ definition }; });
        }

        template<class TCategory>
        definition_view<TCategory> operator[](definition_id<TCategory> id) const
        {
            const auto& definitions = std::get<definition_types::index_of<TCategory>()>(definitions_);
            if(id.value() >= definitions.size())
            {
                report(definition_metadata_error{ definition_types::index_of<TCategory>(), id.value(), definitions.size() });
                return {};
            }
            return { definitions[id.value()] };
        }

        template<class TCategory>
        std::optional<definition_view<TCategory>> find_definition(std::string_view name) const
        {
            if(not id_map_.has<TCategory>(name)) return std::nullopt;
            return (*this)[id_map_.get_id<TCategory>(name)];
        }

        definition_id<combat_status_view> dendro_core_id() const noexcept
        {
            return basic_ids_.dendro_core;
        }

        definition_id<combat_status_view> catalyzing_field_id() const noexcept
        {
            return basic_ids_.catalyzing_field;
        }

        definition_id<summon_view> burning_flame_id() const noexcept
        {
            return basic_ids_.burning_flame;
        }

        definition_id<attachment_view> frozen_id() const noexcept
        {
            return basic_ids_.frozen;
        }

        template<class TCategory>
        std::size_t definition_count() const noexcept
        {
            return id_map_.definition_count<TCategory>();
        }

        dynamic_history_field history_field(std::string_view name) const
        {
            const auto* field = find_history_field(name);
            return field ? *field : dynamic_history_field{ history_field_key<bool>{ 0 } };
        }

        template<class T>
        history_field_key<T> history_field(std::string_view name) const
        {
            const auto* field = find_history_field(name);
            if(field)
            {
                if(const auto* key = std::get_if<history_field_key<T>>(field)) return *key;
                report(history_field_type_mismatch{ current_summary_name(), std::string{ name }, detail::history_type_name<T>(),
                    std::visit([](auto key) { return detail::history_type_name<typename decltype(key)::value_type>(); }, *field) });
            }
            return history_field_key<T>{ 0, std::is_unbounded_array_v<T> ? 0 : 1 };
        }

        dynamic_history_value resolve_history_field(std::string_view summary, std::string_view name) const
        {
            const auto result = find_history_value(summary, name);
            if(not result.first) return history_value_key<bool>{ 0 };
            return std::visit([&](auto key) -> dynamic_history_value
            {
                return history_value_key<typename decltype(key)::value_type>{ key.offset() + result.second, key.count() };
            }, *result.first);
        }

        template<class T>
        history_value_key<T> resolve_history_field(std::string_view summary, std::string_view name) const
        {
            const auto result = find_history_value(summary, name);
            if(result.first)
            {
                if(const auto* key = std::get_if<history_field_key<T>>(result.first))
                    return history_value_key<T>{ key->offset() + result.second, key->count() };
                report(history_field_type_mismatch{ std::string{ summary }, std::string{ name }, detail::history_type_name<T>(),
                    std::visit([](auto key) { return detail::history_type_name<typename decltype(key)::value_type>(); }, *result.first) });
            }
            return history_value_key<T>{ 0, std::is_unbounded_array_v<T> ? 0 : 1 };
        }

        template<class TCategory>
        definition_id<TCategory> resolve_id(std::string_view name) const
        {
            return resolve_definition<TCategory>(name).value_or(definition_id<TCategory>{});
        }

        std::optional<tag_id> find_tag(std::string_view name) const
        {
            if(not id_map_.has_tag(name)) return std::nullopt;
            return id_map_.get_tag_id(name);
        }

        template<class TCategory>
        std::vector<definition_id<TCategory>> find_ids_by_tag(std::string_view filter) const
        {
            return id_map_.query_by_tag<TCategory>(filter);
        }

        template<class TCommands>
        program_entry add_program(TCommands&& commands)
        {
            program_entry result{ program_.size() };
            detail::program_writer writer{ program_ };
#ifndef NDEBUG
            result.inputs_begin_ = input_markers_.size();
#endif
            [[maybe_unused]] const auto inputs_count =
                detail::append_commands(writer, std::forward<TCommands>(commands), mode_, *this, program_kind::response, errors_,
                    { compile_stage::program, source_, program_kind::response, program_count_++, {} }
#ifndef NDEBUG
                    , &input_markers_
#endif
                );
#ifndef NDEBUG
            result.inputs_count_ = inputs_count;
#endif
            writer.write(detail::execute_fn{ detail::execute_return });
            return result;
        }

    private:
        void report(compile_error_reason reason) const
        {
            errors_.push_back({ { stage_, source_, {}, {}, {} }, std::move(reason) });
        }

        std::string current_summary_name() const
        {
            return source_ ? source_->name : std::string{};
        }

        template<class TCategory>
        std::optional<definition_id<TCategory>> resolve_definition(std::string_view name) const
        {
            const auto& declared = declarations_.dependencies[definition_types::index_of<TCategory>()];
            if(not contains(declared, name))
            {
                report(definition_resolution_error{ { definition_types::index_of<TCategory>(), std::string{ name } },
                    definition_resolution_error::reason::undeclared_dependency });
                return std::nullopt;
            }
            if(not id_map_.has<TCategory>(name))
            {
                report(definition_resolution_error{ { definition_types::index_of<TCategory>(), std::string{ name } },
                    definition_resolution_error::reason::not_found });
                return std::nullopt;
            }
            return id_map_.get_id<TCategory>(name);
        }

        const dynamic_history_field* find_history_field(std::string_view name) const
        {
            if(not history_layouts_ready_ || not own_history_)
            {
                report(history_field_access_error{ current_summary_name(), std::string{ name }, not history_layouts_ready_
                    ? history_field_access_error::reason::layouts_unavailable : history_field_access_error::reason::no_current_summary });
                return nullptr;
            }
            const auto* field = history_layouts_[own_history_.value()].find(name);
            if(not field) report(history_field_not_found{ current_summary_name(), std::string{ name } });
            return field;
        }

        std::pair<const dynamic_history_field*, std::size_t> find_history_value(std::string_view summary, std::string_view name) const
        {
            if(not history_layouts_ready_)
            {
                report(history_field_access_error{ std::string{ summary }, std::string{ name }, history_field_access_error::reason::layouts_unavailable });
                return {};
            }
            const auto id = resolve_definition<history_summary_definition>(summary);
            if(not id) return {};
            const auto& layout = history_layouts_[id->value()];
            const auto* field = layout.find(name);
            if(not field) report(history_field_not_found{ std::string{ summary }, std::string{ name } });
            return { field, layout.offset };
        }

        definition_compile_context(
            const issued_id_map& id_map,
            const detail::compile_definitions& definitions,
            const detail::basic_definition_ids& basic_ids,
            detail::program_bytes& program,
            const detail::definition_source_declarations& declarations,
            compile_mode mode,
            std::span<const detail::compiled_history_summary> history_layouts,
            definition_id<history_summary_definition> own_history,
            bool history_layouts_ready,
            std::vector<compile_error>& errors,
            compile_stage stage,
            std::optional<definition_name> source
#ifndef NDEBUG
            , std::vector<std::size_t>& input_markers
#endif
        )
        : id_map_{ id_map }, definitions_{ definitions }, basic_ids_{ basic_ids }, program_{ program }, declarations_{ declarations }, mode_{ mode },
          history_layouts_{ history_layouts }, own_history_{ own_history }, history_layouts_ready_{ history_layouts_ready }, errors_{ errors }, stage_{ stage }, source_{ std::move(source) }
#ifndef NDEBUG
        , input_markers_{ input_markers }
#endif
        {}

        static bool contains(const std::vector<std::string_view>& values, std::string_view value)
        {
            return std::ranges::find(values, value) != values.end();
        }

        const issued_id_map& id_map_;
        const detail::compile_definitions& definitions_;
        const detail::basic_definition_ids& basic_ids_;
        detail::program_bytes& program_;
        const detail::definition_source_declarations& declarations_;

        compile_mode mode_;
        std::span<const detail::compiled_history_summary> history_layouts_;
        definition_id<history_summary_definition> own_history_;
        bool history_layouts_ready_;
        std::vector<compile_error>& errors_;
        compile_stage stage_;
        std::optional<definition_name> source_;
        std::size_t program_count_ = 0;
#ifndef NDEBUG
        std::vector<std::size_t>& input_markers_;
#endif

        friend class definition_library;
    };

    class definition_library
    {
        friend class executor;
        friend class detail::execution_context;

    public:
        definition_library(const definition_library& other)
        : program_{ other.program_ },
#ifndef NDEBUG
          input_markers_{ other.input_markers_ },
#endif
          tag_names_{ other.tag_names_ },
          equipment_tags_{ other.equipment_tags_ }, skill_tags_{ other.skill_tags_ }, control_tag_{ other.control_tag_ },
          control_immunity_tag_{ other.control_immunity_tag_ },
          remove_at_zero_usages_tag_{ other.remove_at_zero_usages_tag_ }, basic_ids_{ other.basic_ids_ },
          buckets_{ other.buckets_ },
          history_layouts_{ other.history_layouts_ }, history_size_{ other.history_size_ },
          history_handlers_{ other.history_handlers_ }
        {
            detail::finalize_program(program_);
        }

        definition_library(definition_library&&) noexcept = default;

        definition_library& operator=(const definition_library& other)
        {
            if(this != &other)
            {
                auto replacement = other;
                *this = std::move(replacement);
            }
            return *this;
        }

        definition_library& operator=(definition_library&&) noexcept = default;
        ~definition_library() = default;

        static constexpr size_t definition_count = definition_types::size();

        definition_id<combat_status_view> dendro_core_id() const noexcept
        {
            return basic_ids_.dendro_core;
        }

        definition_id<combat_status_view> catalyzing_field_id() const noexcept
        {
            return basic_ids_.catalyzing_field;
        }

        definition_id<summon_view> burning_flame_id() const noexcept
        {
            return basic_ids_.burning_flame;
        }

        definition_id<attachment_view> frozen_id() const noexcept
        {
            return basic_ids_.frozen;
        }

        template<class TDefinitionType>
        class definition_view
        {
        public:
            definition_id<TDefinitionType> id() const
            {
                return id_;
            }

            std::string_view name() const
            {
                return library_->name(id_);
            }

            givm::equipment_type equipment_type() const noexcept requires std::same_as<TDefinitionType, attachment_view>
            {
                return library_->equipment_type(id_);
            }

            bool has_tag(tag_id tag) const
            {
                return library_->has_tag(id_, tag);
            }

            bool has_all_tags(std::span<const tag_id> tags) const
            {
                return library_->has_all_tags(id_, tags);
            }

            bool has_any_tag(std::span<const tag_id> tags) const
            {
                return library_->has_any_tag(id_, tags);
            }

            bool matches_tags(
                std::span<const tag_id> required_tags,
                std::span<const tag_id> excluded_tags = {}
            ) const
            {
                return library_->matches_tags(id_, required_tags, excluded_tags);
            }

            template<class TEvent, class TView>
            bool can_handle() const noexcept
            {
                return library_->template can_handle<TEvent, TView>(id_);
            }

            template<class TQuery>
            TQuery::result_t query(const TQuery& parameters) const
            {
                return library_->query(id_, parameters);
            }

            template<class TEvent, class TView>
            program_entry handle(
                const TView& entity,
                TEvent& event,
                handle_context& context
            ) const
            {
                return library_->template handle<TEvent>(
                    id_, entity, event, context
                );
            }

        private:
            friend class definition_library;

            definition_view(const definition_library& library, definition_id<TDefinitionType> id)
            : library_{ &library }, id_{ id }
            {}

            const definition_library* library_;
            definition_id<TDefinitionType> id_;
        };

        template<class TDefinitionType>
        definition_view<TDefinitionType> operator[](definition_id<TDefinitionType> id) const
        {
            return { *this, id };
        }

        friend void load_deck(
            table& card_table, const definition_library& library,
            const linked_deck& first_deck, const linked_deck& second_deck
        )
        {
            library.load_deck(card_table, player_id{ 0 }, first_deck);
            library.load_deck(card_table, player_id{ 1 }, second_deck);
        }

    private:
        void load_deck(table& card_table, player_id player, const linked_deck& deck) const
        {
            auto& writable_table = static_cast<detail::unrestricted_table&>(card_table);
            writable_table.load_deck(player, deck);
            for(const auto card : writable_table[player].deck_cards())
            {
                card.state() = (*this)[card.definition_id()].query(card_initial_state{});
            }
            for(const auto character : writable_table[player].characters())
            {
                const auto definition = (*this)[character.definition_id()];
                character.state() = definition.query(character_initial_state{});
                for(std::size_t skill_index = 0; ; ++skill_index)
                {
                    const auto skill = definition.query(character_initial_skill{ skill_index });
                    if(not skill)
                    {
                        break;
                    }
                    character.add(skill, {});
                }
            }
        }

        static constexpr detail::execution_position entry() noexcept
        {
            return detail::entry_position;
        }

    public:
        dynamic_history_value history_field(definition_id<history_summary_definition> id, std::string_view name) const
        {
#ifndef NDEBUG
            if(id.value() >= history_layouts_.size()) throw std::invalid_argument{ "history summary ID out of range" };
#endif
            const auto& layout = history_layouts_[id.value()];
            const auto* field = layout.find(name);
#ifndef NDEBUG
            if(not field) throw std::invalid_argument{ "unknown history field: " + std::string{ name } };
#endif
            return std::visit([&](auto key) -> dynamic_history_value
            {
                return history_value_key<typename decltype(key)::value_type>{ key.offset() + layout.offset, key.count() };
            }, *field);
        }

        template<class T>
        history_value_key<T> history_field(definition_id<history_summary_definition> id, std::string_view name) const
        {
            const auto field = history_field(id, name);
            const auto* key = std::get_if<history_value_key<T>>(&field);
#ifndef NDEBUG
            if(not key) throw std::invalid_argument{ "history field type mismatch: " + std::string{ name } };
#endif
            return *key;
        }

        template<class TDefinitionType>
        std::string_view name(definition_id<TDefinitionType> id) const
        {
            return bucket_for<TDefinitionType>().names[id.value()];
        }

        std::string_view tag_name(tag_id id) const
        {
            return tag_names_[id.value()];
        }

        givm::equipment_type equipment_type(definition_id<attachment_view> id) const noexcept
        {
            for(size_t index = 0; index != equipment_tags_.size(); ++index)
            {
                if(equipment_tags_[index] && has_tag(id, equipment_tags_[index]))
                {
                    return static_cast<givm::equipment_type>(index);
                }
            }
            return givm::equipment_type::none;
        }

        givm::skill_flags skill_flags(definition_id<skill_view> id) const noexcept
        {
            givm::skill_flags result;
            constexpr std::array bits{ skill_flag_bits::normal_attack, skill_flag_bits::elemental_skill,
                                       skill_flag_bits::elemental_burst };
            for(size_t index = 0; index != skill_tags_.size(); ++index)
                if(skill_tags_[index] && has_tag(id, skill_tags_[index])) result.set(bits[index]);
            return result;
        }

        bool is_control(definition_id<attachment_view> id) const noexcept
        {
            return control_tag_ && has_tag(id, control_tag_);
        }

        bool remove_at_zero_usages(definition_id<summon_view> id) const noexcept
        {
            return remove_at_zero_usages_tag_ && has_tag(id, remove_at_zero_usages_tag_);
        }

        bool is_controlled(character_view character) const noexcept
        {
            if(not control_tag_) return false;
            for(const auto attachment : character.attachments())
                if(has_tag(attachment.definition_id(), control_tag_)) return true;
            return false;
        }

        bool is_control_immune(character_view character) const noexcept
        {
            if(not control_immunity_tag_) return false;
            for(const auto attachment : character.attachments())
                if(has_tag(attachment.definition_id(), control_immunity_tag_)) return true;
            return false;
        }

        template<class TDefinitionType>
        bool has_tag(definition_id<TDefinitionType> id, tag_id tag) const
        {
            return bucket_for<TDefinitionType>().tags[id.value()].has(tag);
        }

        template<class TDefinitionType>
        bool has_all_tags(definition_id<TDefinitionType> id, std::span<const tag_id> tags) const
        {
            return bucket_for<TDefinitionType>().tags[id.value()].has_all(tags);
        }

        template<class TDefinitionType>
        bool has_any_tag(definition_id<TDefinitionType> id, std::span<const tag_id> tags) const
        {
            return bucket_for<TDefinitionType>().tags[id.value()].has_any(tags);
        }

        template<class TDefinitionType>
        bool matches_tags(
            definition_id<TDefinitionType> id,
            std::span<const tag_id> required_tags,
            std::span<const tag_id> excluded_tags = {}
        ) const
        {
            return bucket_for<TDefinitionType>().tags[id.value()].matches(required_tags, excluded_tags);
        }

        template<class TEvent, class TView, class TDefinitionType>
        bool can_handle(definition_id<TDefinitionType> id) const noexcept
        {
            return get_handle_fn<TEvent, TView>(id) != nullptr;
        }

        template<class TDefinitionType, class TQuery>
        TQuery::result_t query(definition_id<TDefinitionType> id, const TQuery& parameters) const
        {
            const auto& bucket = bucket_for<TDefinitionType>();
            const auto& queries =
                std::get<supported_queries<TDefinitionType>::template index_of<TQuery>()>(bucket.queries);
            if constexpr(std::is_empty_v<TQuery>)
            {
                return queries[id.value()];
            }
            else
            {
                return queries[id.value()](bucket.data[id.value()], parameters);
            }
        }

        template<class TEvent, class TDefinitionType, class TView>
        program_entry handle(
            definition_id<TDefinitionType> id,
            const TView& entity,
            TEvent& event,
            handle_context& context
        ) const
        {
            const auto& bucket = bucket_for<TDefinitionType>();
            const size_t index = id.value();
            const auto handle_fn = get_handle_fn<TEvent, TView>(id);
            return handle_fn(bucket.data[index], entity, event, context);
        }

    private:
        using definition_type_list = definition_types;

        definition_library(const issued_id_map& id_map, const detail::basic_definition_names& basics)
        : program_(sizeof(detail::execute_fn), 0), tag_names_(id_map.tag_names().begin(), id_map.tag_names().end()),
          basic_ids_{
              id_map.get_id<combat_status_view>(basics.dendro_core),
              id_map.get_id<combat_status_view>(basics.catalyzing_field),
              id_map.get_id<summon_view>(basics.burning_flame),
              id_map.get_id<attachment_view>(basics.frozen) }
        {
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
        static consteval size_t index_of()
        {
            return definition_type_list::template index_of<TDefinitionType>();
        }

        template<class TDefinitionType>
        decltype(auto) bucket_for(this auto& self)
        {
            return (std::get<index_of<TDefinitionType>()>(self.buckets_));
        }

        void initialize_history(table& card_table) const
        {
            auto& writable = static_cast<detail::unrestricted_table&>(card_table);
            writable.reset_history(history_size_);
            record_history(history_summary_initialization{}, writable);
        }

    public:
        template<class TEvent>
        void record_history(const TEvent& event, detail::unrestricted_table& card_table) const
        {
            if constexpr(requires { subscribed_events<history_summary_definition>::template index_of<TEvent>(); })
            {
                const auto& handlers = std::get<subscribed_events<history_summary_definition>::template index_of<TEvent>()>(
                    history_handlers_);
                const auto& definitions = bucket_for<history_summary_definition>().data;
                for(const auto& handler : handlers)
                {
                    const auto& layout = history_layouts_[handler.index];
                    handler.function(definitions[handler.index], card_table.history_summary(layout.offset, layout.size),
                        event, static_cast<const table&>(card_table), *this);
                }
            }
        }

    private:
        template<class TEvent, class TView, class TDefinitionType>
        handle_fn_t<TView, TEvent> get_handle_fn(definition_id<TDefinitionType> id) const noexcept
        {
            const auto& bucket = bucket_for<TDefinitionType>();
            return std::get<subscribed_events<TView>::template index_of<TEvent>()>(
                std::get<views_of_definition<TDefinitionType>::template index_of<TView>()>(bucket.handle_fns)
            )[id.value()];
        }

        template<class TDefinitionType>
        void compile_source(
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
            definition_compile_context context{ id_map, definitions, basic_ids_, program_, *definition.declarations,
                mode, history_layouts_, own_history, true, errors,
                compile_stage::definition, definition_name{ definition_types::index_of<TDefinitionType>(), std::string{ bucket.names[definition.id.value()] } }
#ifndef NDEBUG
                , input_markers_
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

        void prepare_history_layouts(const detail::compile_definitions& definitions, const issued_id_map& ids, compile_mode mode,
            std::vector<compile_error>& errors)
        {
            history_layouts_.resize(ids.definition_count<history_summary_definition>());
            const auto& summaries = std::get<definition_types::index_of<history_summary_definition>()>(definitions);
            for(const auto& definition : summaries)
            {
                const auto& source = *definition.source;
                const auto id = definition.id;
                definition_compile_context context{ ids, definitions, basic_ids_, program_, *definition.declarations, mode, history_layouts_, id, false,
                    errors, compile_stage::history_layout, definition_name{ definition_types::index_of<history_summary_definition>(),
                        std::string{ definition.bucket->names[id.value()] } }
#ifndef NDEBUG
                    , input_markers_
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

        static tag_mask make_tag_mask(
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

        detail::compile_definitions prepare_definitions(const definition_source_library& sources, const issued_id_map& ids)
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

        void complete_dynamic_queries()
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

    public:
        template<class TInitializationSequence, class TRoundSequence>
        static auto compile(const definition_source_library& sources, const basic_definition_sources& basics,
            TInitializationSequence&& initialization_program, TRoundSequence&& round_program, compile_mode mode)
        {
            std::vector<source_preparation_error> preparation_errors;
            const auto [selected_sources, basic_names] = sources.with_basic_definitions(basics, preparation_errors);
            return compile_prepared(selected_sources, basic_names,
                selected_sources.make_issued_id_map(selected_sources.make_full_selection()),
                std::forward<TInitializationSequence>(initialization_program),
                std::forward<TRoundSequence>(round_program), mode, std::move(preparation_errors));
        }

        template<class TInitializationSequence, class TRoundSequence>
        static auto compile(const definition_source_library& sources, const basic_definition_sources& basics,
            const definition_selection& selection, TInitializationSequence&& initialization_program,
            TRoundSequence&& round_program, compile_mode mode)
        {
            std::vector<source_preparation_error> preparation_errors;
            const auto [selected_sources, basic_names] = sources.with_basic_definitions(basics, preparation_errors);
            auto ids = selected_sources.make_issued_id_map(selected_sources.resolve_selection(selection, basic_names, preparation_errors));
            return compile_prepared(selected_sources, basic_names, std::move(ids),
                std::forward<TInitializationSequence>(initialization_program),
                std::forward<TRoundSequence>(round_program), mode, std::move(preparation_errors));
        }

    private:
        template<class TInitializationSequence, class TRoundSequence>
        static auto compile_prepared(const definition_source_library& sources, const detail::basic_definition_names& basics,
            issued_id_map id_map, TInitializationSequence&& initialization_program, TRoundSequence&& round_program,
            compile_mode mode, std::vector<source_preparation_error> preparation_errors)
        {
            struct compile_result
            {
                definition_library library;
                issued_id_map id_map;
            };
            using result_type = std::expected<compile_result, std::vector<compile_error>>;
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
                , library.input_markers_
#endif
            };
            detail::program_writer writer{ library.program_ };
            detail::append_commands(writer, std::forward<TInitializationSequence>(initialization_program), mode,
                context, program_kind::initialization, errors, { compile_stage::program, {}, program_kind::initialization, 0, {} });
            const detail::execution_position round_start = writer.position();
            detail::append_commands(writer, std::tuple{ detail::round_program_begin{} }, mode,
                context, program_kind::round, errors, { compile_stage::program, {}, program_kind::round, 0, {} });
            const detail::execution_position round_entry = writer.position();
            detail::append_commands(writer, std::forward<TRoundSequence>(round_program), mode,
                context, program_kind::round, errors, { compile_stage::program, {}, program_kind::round, 0, {} });
            detail::append_commands(writer, std::tuple{ detail::round_program_repeat{ round_start, round_entry } }, mode,
                context, program_kind::round, errors, { compile_stage::program, {}, program_kind::round, 0, {} });
            definition_types::each([&]<class TCategory>
            {
                for(const auto& definition : std::get<definition_types::index_of<TCategory>()>(definitions))
                    library.compile_source(definition, id_map, definitions, mode, errors);
            });
            if(not errors.empty()) return result_type{ std::unexpected{ std::move(errors) } };
            library.complete_dynamic_queries();
            detail::finalize_program(library.program_);
            return result_type{ compile_result{ .library = std::move(library), .id_map = std::move(id_map) } };
        }

    private:
        detail::program_bytes program_;
#ifndef NDEBUG
        std::vector<std::size_t> input_markers_;
#endif
        std::vector<std::string_view> tag_names_;
        std::array<tag_id, static_cast<size_t>(givm::equipment_type::none)> equipment_tags_{};
        std::array<tag_id, 3> skill_tags_{};
        tag_id control_tag_{};
        tag_id control_immunity_tag_{};
        tag_id remove_at_zero_usages_tag_{};
        detail::basic_definition_ids basic_ids_;
        definition_types::apply<detail::definition_bucket_tuple> buckets_;
        std::vector<detail::compiled_history_summary> history_layouts_;
        std::size_t history_size_{};
        detail::definition_history_handlers history_handlers_;
    };

    template<class TInitializationSequence, class TRoundSequence>
    inline auto compile(
        const definition_source_library& sources,
        const basic_definition_sources& basics,
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

    template<class TInitializationSequence, class TRoundSequence>
    inline auto compile(
        const definition_source_library& sources,
        const basic_definition_sources& basics,
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

#include "../macro_undef.hpp"
#endif
