#ifndef GIVM_EXECUTOR_DEFINITION_COMPILE_CONTEXT_HPP
#define GIVM_EXECUTOR_DEFINITION_COMPILE_CONTEXT_HPP

#include "../definition.hpp"
#include "library.hpp"
#include "compile_error.hpp"

namespace givm::detail
{
    template<class... TQueries>
    using static_query_functions = std::tuple<std::conditional_t<
        std::is_empty_v<TQueries>, query_fn_t<TQueries>, std::monostate>...>;

    template<definition_category TCategory>
    struct compile_definition
    {
        const definition_source_view<TCategory>* source;
        definition_id<TCategory> id;
        const definition_source_declarations* declarations;
        const definition_bucket<TCategory>* bucket;
        supported_queries<TCategory>::template apply<static_query_functions> static_queries{};
        std::conditional_t<(TCategory == definition_category::history_summary),
            const definition_history_handlers*, std::monostate> history{};
    };

}

namespace givm
{
    class definition_compile_context
    {
    public:
        template<definition_category TCategory>
        class definition_view
        {
        public:
            optional_definition_id<TCategory> id() const noexcept
            {
                return definition_ ? optional_definition_id<TCategory>{ definition_->id } : nullptr;
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

            template<definition_category TDependencyCategory>
            std::span<const std::string_view> dependencies() const noexcept
            {
                if(not definition_) return {};
                return definition_->declarations->dependencies[static_cast<std::size_t>(TDependencyCategory)];
            }

            template<class TEvent, entity_category Entity = entity_categories_of<TCategory>.size() == 1
                ? entity_categories_of<TCategory>.front() : entity_category::null>
            bool can_handle() const noexcept
            {
                if(not definition_) return false;
                if constexpr(TCategory == definition_category::history_summary && Entity == entity_category::null)
                {
                    if constexpr(requires { history_subscribed_events::template index_of<TEvent>(); })
                        return std::ranges::binary_search(
                            std::get<history_subscribed_events::template index_of<TEvent>()>(*definition_->history),
                            definition_->id.value(), {}, &detail::history_handler<TEvent>::index);
                    else return false;
                }
                else if constexpr(Entity != entity_category::null && definition_category_of<Entity> == TCategory)
                {
                    using TView = entity_view<Entity>;
                    if constexpr(requires { subscribed_events<Entity>::template index_of<TEvent>(); })
                        return std::get<subscribed_events<Entity>::template index_of<TEvent>()>(
                            std::get<detail::definition_views<TCategory>::template index_of<TView>()>(definition_->bucket->handle_fns))
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

        template<definition_category TCategory>
        auto definitions() const noexcept
        {
            return std::span{ std::get<static_cast<std::size_t>(TCategory)>(definitions_) }
                | std::views::transform([](const auto& definition) { return definition_view<TCategory>{ definition }; });
        }

        template<definition_category TCategory>
        definition_view<TCategory> operator[](definition_id<TCategory> id) const
        {
            const auto& definitions = std::get<static_cast<std::size_t>(TCategory)>(definitions_);
            if(id.value() >= definitions.size())
            {
                report(definition_metadata_error{ TCategory, id.value(), definitions.size() });
                return {};
            }
            return { definitions[id.value()] };
        }

        template<definition_category TCategory>
        std::optional<definition_view<TCategory>> find_definition(std::string_view name) const
        {
            if(not id_map_.has<TCategory>(name)) return std::nullopt;
            return (*this)[id_map_.get_id<TCategory>(name)];
        }

        definition_id<definition_category::reaction> default_reaction_id(elemental_reaction slot) const noexcept
        {
            return default_reactions_[static_cast<std::size_t>(slot) - 1];
        }

        template<definition_category TCategory>
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

        template<definition_category TCategory>
        optional_definition_id<TCategory> resolve_id(std::string_view name) const
        {
            const auto id = resolve_definition<TCategory>(name);
            return id ? optional_definition_id<TCategory>{ *id } : nullptr;
        }

        optional_tag_id find_tag(std::string_view name) const
        {
            if(not id_map_.has_tag(name)) return nullptr;
            return id_map_.get_tag_id(name);
        }

        template<definition_category TCategory>
        std::vector<definition_id<TCategory>> find_ids_by_tag(std::string_view filter) const
        {
            return id_map_.query_by_tag<TCategory>(filter);
        }

        template<event_category Category>
        effect<Category> add_effect(std::span<const any_command> commands)
        {
            return effect<Category>{ compile_effect(commands, Category) };
        }

        template<event_category Category, detail::command_sequence TCommands>
        requires (not std::convertible_to<TCommands, std::span<const any_command>>)
        effect<Category> add_effect(TCommands&& commands)
        {
            const auto sequence = detail::make_command_sequence(std::forward<TCommands>(commands));
            return add_effect<Category>(std::span<const any_command>{ sequence });
        }

        template<event_category Category, class... TCommands>
        requires (std::constructible_from<any_command, TCommands> && ...)
        effect<Category> add_effect(TCommands&&... commands)
        {
            const std::array<any_command, sizeof...(TCommands)> sequence{ any_command{ std::forward<TCommands>(commands) }... };
            return add_effect<Category>(std::span<const any_command>{ sequence });
        }

        normal_effect add_normal_effect(std::span<const any_command> commands)
        {
            return add_effect<event_category::normal>(commands);
        }

        template<class... TCommands>
        requires ((std::constructible_from<any_command, TCommands> && ...)
            || (sizeof...(TCommands) == 1 && (detail::command_sequence<TCommands> && ...)))
        normal_effect add_normal_effect(TCommands&&... commands)
        {
            return add_effect<event_category::normal>(std::forward<TCommands>(commands)...);
        }

        immediate_effect add_immediate_effect(std::span<const any_command> commands)
        {
            return add_effect<event_category::immediate>(commands);
        }

        template<class... TCommands>
        requires ((std::constructible_from<any_command, TCommands> && ...)
            || (sizeof...(TCommands) == 1 && (detail::command_sequence<TCommands> && ...)))
        immediate_effect add_immediate_effect(TCommands&&... commands)
        {
            return add_effect<event_category::immediate>(std::forward<TCommands>(commands)...);
        }

        preview_effect add_preview_effect(std::span<const any_command> commands)
        {
            return add_effect<event_category::preview>(commands);
        }

        template<class... TCommands>
        requires ((std::constructible_from<any_command, TCommands> && ...)
            || (sizeof...(TCommands) == 1 && (detail::command_sequence<TCommands> && ...)))
        preview_effect add_preview_effect(TCommands&&... commands)
        {
            return add_effect<event_category::preview>(std::forward<TCommands>(commands)...);
        }


    private:
        normal_effect compile_effect(std::span<const any_command> commands, event_category category);

        void report(compile_error_reason reason) const
        {
            errors_.push_back({ { stage_, source_, {}, {}, {} }, std::move(reason) });
        }

        std::string current_summary_name() const
        {
            return source_ ? source_->name : std::string{};
        }

        template<definition_category TCategory>
        std::optional<definition_id<TCategory>> resolve_definition(std::string_view name) const
        {
            const auto& declared = declarations_.dependencies[static_cast<std::size_t>(TCategory)];
            if(not contains(declared, name))
            {
                report(definition_resolution_error{ { TCategory, std::string{ name } },
                    definition_resolution_error::reason::undeclared_dependency });
                return std::nullopt;
            }
            if(not id_map_.has<TCategory>(name))
            {
                report(definition_resolution_error{ { TCategory, std::string{ name } },
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
            const auto* field = history_layouts_[own_history_.get<definition_category::history_summary>().value()].find(name);
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
            const auto id = resolve_definition<definition_category::history_summary>(summary);
            if(not id) return {};
            const auto& layout = history_layouts_[id->value()];
            const auto* field = layout.find(name);
            if(not field) report(history_field_not_found{ std::string{ summary }, std::string{ name } });
            return { field, layout.offset };
        }

        definition_compile_context(
            const issued_id_map& id_map,
            const detail::compile_definitions& definitions,
            const detail::default_reaction_ids& default_reactions,
            detail::program_bytes& program,
            const detail::definition_source_declarations& declarations,
            compile_mode mode,
            std::span<const detail::compiled_history_summary> history_layouts,
            optional_definition_id<definition_category::history_summary> own_history,
            bool history_layouts_ready,
            std::vector<compile_error>& errors,
            compile_stage stage,
            std::optional<definition_name> source,
            detail::program_input_records& input_records
        )
        : id_map_{ id_map }, definitions_{ definitions }, default_reactions_{ default_reactions }, program_{ program }, declarations_{ declarations }, mode_{ mode },
          history_layouts_{ history_layouts }, own_history_{ own_history }, history_layouts_ready_{ history_layouts_ready }, errors_{ errors }, stage_{ stage }, source_{ std::move(source) },
          input_records_{ input_records }
        {}

        static bool contains(const std::vector<std::string_view>& values, std::string_view value)
        {
            return std::ranges::find(values, value) != values.end();
        }

        const issued_id_map& id_map_;
        const detail::compile_definitions& definitions_;
        const detail::default_reaction_ids& default_reactions_;
        detail::program_bytes& program_;
        const detail::definition_source_declarations& declarations_;

        compile_mode mode_;
        std::span<const detail::compiled_history_summary> history_layouts_;
        optional_definition_id<definition_category::history_summary> own_history_;
        bool history_layouts_ready_;
        std::vector<compile_error>& errors_;
        compile_stage stage_;
        std::optional<definition_name> source_;
        std::size_t program_count_ = 0;
        detail::program_input_records& input_records_;

        friend class definition_library;
    };

}

#endif
