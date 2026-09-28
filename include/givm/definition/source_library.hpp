#ifndef GIVM_DEFINITION_SOURCE_LIBRARY_HPP
#define GIVM_DEFINITION_SOURCE_LIBRARY_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <expected>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include "source_view.hpp"
#include "issued_id_map.hpp"
#include "definition_categories.hpp"
#include "../utils/type_list.hpp"

namespace givm
{
    using definition_selection = std::array<std::span<const std::string_view>, definition_types::size()>;

    struct definition_name
    {
        std::size_t category_index;
        std::string name;
    };

    struct source_conflict
    {
        enum class reason { different_object, different_type };

        definition_name definition;
        reason cause;
        std::optional<std::size_t> first_input_index;
        std::optional<std::size_t> second_input_index;
    };

    struct source_missing_dependency
    {
        definition_name source;
        std::size_t input_index;
        definition_name dependency;
    };

    using source_add_error = std::variant<source_conflict, source_missing_dependency>;

    struct source_selection_error
    {
        definition_name definition;
        std::optional<definition_name> required_by;
    };

    using source_preparation_error = std::variant<source_conflict, source_missing_dependency, source_selection_error>;

    namespace detail
    {
        struct basic_definition_names
        {
            std::string_view dendro_core;
            std::string_view catalyzing_field;
            std::string_view burning_flame;
            std::string_view frozen;
        };

        inline std::string source_definition_name_text(const definition_name& definition)
        {
            constexpr auto category_names = []
            {
                std::array<std::string_view, definition_types::size()> names{};
                names[definition_types::index_of<card_definition>()] = "card_definition";
                names[definition_types::index_of<status_definition>()] = "status_definition";
                names[definition_types::index_of<support_view>()] = "support_view";
                names[definition_types::index_of<summon_view>()] = "summon_view";
                names[definition_types::index_of<combat_status_view>()] = "combat_status_view";
                names[definition_types::index_of<character_view>()] = "character_view";
                names[definition_types::index_of<skill_view>()] = "skill_view";
                names[definition_types::index_of<attachment_view>()] = "attachment_view";
                names[definition_types::index_of<history_summary_definition>()] = "history_summary_definition";
                return names;
            }();
            const auto category_index = definition.category_index;
            std::string result = category_index < category_names.size() && not category_names[category_index].empty()
                ? std::string{ category_names[category_index] }
                : "category[" + std::to_string(category_index) + "]";
            result += " \"";
            for(const auto character : definition.name)
            {
                switch(character)
                {
                case '\\': result += "\\\\"; break;
                case '\"': result += "\\\""; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += character; break;
                }
            }
            result += '\"';
            return result;
        }

        inline void append_source_error(std::string& text, const source_conflict& error)
        {
            if(not text.empty()) text += '\n';
            text += "source conflict (";
            text += error.cause == source_conflict::reason::different_type ? "different_type" : "different_object";
            text += "): ";
            text += source_definition_name_text(error.definition);
            text += "; first: ";
            text += error.first_input_index ? "input[" + std::to_string(*error.first_input_index) + "]" : "receiver library";
            text += "; second: ";
            text += error.second_input_index ? "input[" + std::to_string(*error.second_input_index) + "]" : "incoming library";
        }

        inline void append_source_error(std::string& text, const source_missing_dependency& error)
        {
            if(not text.empty()) text += '\n';
            text += "missing dependency: ";
            text += source_definition_name_text(error.source);
            text += " (input[" + std::to_string(error.input_index) + "]) requires ";
            text += source_definition_name_text(error.dependency);
        }

        inline void append_source_error(std::string& text, const source_selection_error& error)
        {
            if(not text.empty()) text += '\n';
            text += "missing selected definition: ";
            text += source_definition_name_text(error.definition);
            if(error.required_by)
            {
                text += "; required by ";
                text += source_definition_name_text(*error.required_by);
            }
        }
    }

    inline std::string error_string(const source_conflict& error)
    {
        std::string result;
        detail::append_source_error(result, error);
        return result;
    }

    inline std::string error_string(const source_missing_dependency& error)
    {
        std::string result;
        detail::append_source_error(result, error);
        return result;
    }

    inline std::string error_string(const source_selection_error& error)
    {
        std::string result;
        detail::append_source_error(result, error);
        return result;
    }

    inline std::string error_string(const std::vector<source_add_error>& errors)
    {
        std::string result;
        for(const auto& error : errors)
            std::visit([&](const auto& item) { detail::append_source_error(result, item); }, error);
        return result;
    }

    inline std::string error_string(const std::vector<source_conflict>& errors)
    {
        std::string result;
        for(const auto& error : errors) detail::append_source_error(result, error);
        return result;
    }

    inline std::string error_string(const std::vector<source_preparation_error>& errors)
    {
        std::string result;
        for(const auto& error : errors)
            std::visit([&](const auto& item) { detail::append_source_error(result, item); }, error);
        return result;
    }

    struct basic_definition_sources
    {
        definition_source_view<combat_status_view> dendro_core;
        definition_source_view<combat_status_view> catalyzing_field;
        definition_source_view<summon_view> burning_flame;
        definition_source_view<attachment_view> frozen;
    };

    class definition_source_library
    {
    public:
        static constexpr size_t definition_count = definition_types::size();

        definition_source_library() = default;

        std::expected<void, std::vector<source_add_error>> add()
        {
            return {};
        }

        std::expected<void, std::vector<source_conflict>> add(const definition_source_library& library)
        {
            if(this == &library) return {};
            std::vector<source_conflict> errors;
            definition_types::each([&]<class TCategory>
            {
                collect_bucket_conflicts<definition_types::index_of<TCategory>()>(library, errors);
            });
            if(not errors.empty()) return std::unexpected{ std::move(errors) };

            append_library(library);
            return {};
        }

        template<class TSource>
        std::expected<void, std::vector<source_add_error>> add(const TSource& source)
        {
            return add_sources(source);
        }

        template<class TFirstSource, class TSecondSource, class...TOtherSource>
        std::expected<void, std::vector<source_add_error>> add(
            const TFirstSource& first, const TSecondSource& second, const TOtherSource&...others)
        {
            return add_sources(first, second, others...);
        }

        template<class TDefinitionType>
        bool has(std::string_view name) const
        {
            return bucket_for<TDefinitionType>().name_to_index.contains(name);
        }

        template<class TDefinitionType>
        definition_source_view<TDefinitionType> get(std::string_view name) const
        {
            const auto& bucket = bucket_for<TDefinitionType>();
            return bucket.entries[bucket.name_to_index.at(name)].source;
        }

        template<class TDefinitionType>
        auto source_views() const
        {
            return bucket_for<TDefinitionType>().entries
                | std::views::transform([](const auto& entry)
                {
                    return entry.source;
                });
        }

        std::expected<issued_id_map, std::vector<source_preparation_error>> make_issued_id_map(const basic_definition_sources& basics) const
        {
            std::vector<source_preparation_error> errors;
            const auto [sources, names] = with_basic_definitions(basics, errors);
            if(not errors.empty()) return std::unexpected{ std::move(errors) };
            return sources.make_issued_id_map(sources.make_full_selection());
        }

        std::expected<issued_id_map, std::vector<source_preparation_error>> make_issued_id_map(
            const basic_definition_sources& basics, const definition_selection& selection) const
        {
            std::vector<source_preparation_error> errors;
            const auto [sources, names] = with_basic_definitions(basics, errors);
            const auto selected = sources.resolve_selection(selection, names, errors);
            if(not errors.empty()) return std::unexpected{ std::move(errors) };
            return sources.make_issued_id_map(selected);
        }

    private:
        using definition_type_list = definition_types;
        using pending_names_type = std::array<std::unordered_set<std::string_view>, definition_count>;
        using selection_mask = std::array<std::vector<bool>, definition_count>;

        template<class TDefinitionType>
        struct entry
        {
            definition_source_view<TDefinitionType> source;
            std::string_view name;
            detail::definition_source_declarations declarations;
        };

        template<class TDefinitionType>
        struct pending_entry
        {
            definition_source_view<TDefinitionType> source;
            std::string_view name;
            detail::definition_source_declarations declarations;
            std::size_t input_index = 0;
        };

        template<class TDefinitionType>
        struct bucket
        {
            std::vector<entry<TDefinitionType>> entries;
            std::unordered_map<std::string_view, size_t> name_to_index;
        };

        struct queue_item
        {
            size_t entity_index;
            size_t source_index;
        };

        template<class...TDefinition>
        using bucket_tuple_for = std::tuple<bucket<TDefinition>...>;

        template<class...TDefinition>
        using pending_tuple_for = std::tuple<std::vector<pending_entry<TDefinition>>...>;

        using bucket_tuple = definition_type_list::apply<bucket_tuple_for>;
        using pending_tuple = definition_type_list::apply<pending_tuple_for>;
        using pending_sources_by_name = std::array<
            std::unordered_map<std::string_view, std::vector<std::size_t>>, definition_count>;

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

        template<size_t I>
        void collect_bucket_conflicts(
            const definition_source_library& library, std::vector<source_conflict>& errors) const
        {
            const auto& bucket = std::get<I>(buckets_);
            for(const auto& entry : std::get<I>(library.buckets_).entries)
            {
                if(const auto found = bucket.name_to_index.find(entry.name); found != bucket.name_to_index.end())
                {
                    const auto& existing = bucket.entries[found->second];
                    if(existing.source.source_ == entry.source.source_
                        && existing.source.rtti_ == entry.source.rtti_)
                    {
                        continue;
                    }
                    errors.push_back({
                        .definition = { I, std::string{ entry.name } },
                        .cause = existing.source.rtti_ != entry.source.rtti_
                            ? source_conflict::reason::different_type
                            : source_conflict::reason::different_object
                    });
                }
            }
        }

        template<size_t I>
        void append_library_bucket(const definition_source_library& library)
        {
            auto& bucket = std::get<I>(buckets_);
            const auto& other_entries = std::get<I>(library.buckets_).entries;
            bucket.entries.reserve(bucket.entries.size() + other_entries.size());
            bucket.name_to_index.reserve(bucket.name_to_index.size() + other_entries.size());

            for(const auto& entry : other_entries)
            {
                if(bucket.name_to_index.contains(entry.name)) continue;
                bucket.name_to_index.emplace(entry.name, bucket.entries.size());
                bucket.entries.push_back(entry);
            }
        }

        void append_library(const definition_source_library& library)
        {
            [&]<size_t...I>(std::index_sequence<I...>)
            {
                (append_library_bucket<I>(library), ...);
            }(std::make_index_sequence<definition_count>{});
        }

        template<class... TSources>
        std::expected<void, std::vector<source_add_error>> add_sources(const TSources&... sources)
        {
            pending_tuple pending;
            pending_sources_by_name names;
            std::vector<source_add_error> errors;
            std::size_t input_index = 0;
            const std::array indices{
                prepare_add(pending, names, errors, input_index++, sources)...
            };
            input_index = 0;
            (collect_dependency_errors<typename TSources::definition_category>(
                pending, names, indices[input_index++], errors), ...);
            if(not errors.empty()) return std::unexpected{ std::move(errors) };
            commit_pending(pending);
            return {};
        }

        template<class TSource>
        std::optional<std::size_t> prepare_add(pending_tuple& pending, pending_sources_by_name& names,
            std::vector<source_add_error>& errors, std::size_t input_index, const TSource& source) const
        {
            using category = typename TSource::definition_category;
            constexpr auto category_index = index_of<category>();

            const definition_source_view<category> view{ source };
            const auto name = view.name();
            const auto& bucket = bucket_for<category>();
            const auto existing = bucket.name_to_index.find(name);
            const auto same_source = [&](const auto& other)
            {
                return other.source_ == view.source_ && other.rtti_ == view.rtti_;
            };
            if(existing != bucket.name_to_index.end() && same_source(bucket.entries[existing->second].source))
                return std::nullopt;

            auto& entries = std::get<category_index>(pending);
            auto& same_named = names[category_index][name];
            for(const auto index : same_named)
            {
                if(same_source(entries[index].source)) return std::nullopt;
            }

            if(existing != bucket.name_to_index.end() || not same_named.empty())
            {
                const auto previous = existing != bucket.name_to_index.end()
                    ? bucket.entries[existing->second].source : entries[same_named.front()].source;
                errors.emplace_back(source_conflict{
                    .definition = { category_index, std::string{ name } },
                    .cause = previous.rtti_ != view.rtti_
                        ? source_conflict::reason::different_type : source_conflict::reason::different_object,
                    .first_input_index = existing != bucket.name_to_index.end()
                        ? std::nullopt : std::optional{ entries[same_named.front()].input_index },
                    .second_input_index = input_index
                });
            }

            const auto index = entries.size();
            same_named.push_back(index);
            entries.push_back({
                .source = view,
                .name = name,
                .declarations = view.declarations(),
                .input_index = input_index
            });
            return index;
        }

        template<size_t I, class TPendingNames>
        bool dependency_exists(std::string_view name, const TPendingNames& pending_names) const
        {
            return std::get<I>(buckets_).name_to_index.contains(name) || pending_names[I].contains(name);
        }

        template<class TCategory, class TPendingNames, class TError>
        void collect_dependency_errors(const pending_tuple& pending, const TPendingNames& names,
            std::optional<std::size_t> index, std::vector<TError>& errors) const
        {
            if(not index) return;
            constexpr auto category_index = index_of<TCategory>();
            const auto& entry = std::get<category_index>(pending)[*index];
            definition_types::each([&]<class TDependency>
            {
                constexpr auto dependency_index = index_of<TDependency>();
                std::unordered_set<std::string_view> reported;
                for(const auto name : entry.declarations.dependencies[dependency_index])
                {
                    if(not dependency_exists<dependency_index>(name, names) && reported.insert(name).second)
                    {
                        errors.emplace_back(source_missing_dependency{
                            .source = { category_index, std::string{ entry.name } },
                            .input_index = entry.input_index,
                            .dependency = { dependency_index, std::string{ name } }
                        });
                    }
                }
            });
        }

        template<size_t I>
        void commit_pending_bucket(pending_tuple& pending)
        {
            auto& bucket = std::get<I>(buckets_);
            auto& pending_entries = std::get<I>(pending);
            bucket.entries.reserve(bucket.entries.size() + pending_entries.size());
            bucket.name_to_index.reserve(bucket.name_to_index.size() + pending_entries.size());

            for(auto& entry : pending_entries)
            {
                bucket.name_to_index.emplace(entry.name, bucket.entries.size());
                bucket.entries.push_back({ entry.source, entry.name, std::move(entry.declarations) });
            }
        }

        void commit_pending(pending_tuple& pending)
        {
            [&]<size_t...I>(std::index_sequence<I...>)
            {
                (commit_pending_bucket<I>(pending), ...);
            }(std::make_index_sequence<definition_count>{});
        }

        std::pair<definition_source_library, detail::basic_definition_names> with_basic_definitions(
            const basic_definition_sources& basics, std::vector<source_preparation_error>& errors) const
        {
            auto sources = *this;
            pending_tuple pending;
            pending_names_type pending_names;
            const auto prepare = [&]<class TCategory>(
                const definition_source_view<TCategory>& source, std::size_t input_index) -> std::string_view
            {
                constexpr auto index = index_of<TCategory>();
                const auto name = source.name();
                const auto& bucket = sources.bucket_for<TCategory>();
                if(const auto found = bucket.name_to_index.find(name); found != bucket.name_to_index.end())
                {
                    const auto existing = bucket.entries[found->second].source;
                    if(existing.source_ != source.source_ || existing.rtti_ != source.rtti_)
                    {
                        errors.emplace_back(source_conflict{
                            .definition = { index, std::string{ name } },
                            .cause = existing.rtti_ != source.rtti_
                                ? source_conflict::reason::different_type : source_conflict::reason::different_object,
                            .first_input_index = std::nullopt,
                            .second_input_index = input_index
                        });
                    }
                    return name;
                }
                auto& entries = std::get<index>(pending);
                const auto found = std::ranges::find(entries, name, &pending_entry<TCategory>::name);
                if(found != entries.end())
                {
                    if(found->source.source_ != source.source_ || found->source.rtti_ != source.rtti_)
                    {
                        errors.emplace_back(source_conflict{
                            .definition = { index, std::string{ name } },
                            .cause = found->source.rtti_ != source.rtti_
                                ? source_conflict::reason::different_type : source_conflict::reason::different_object,
                            .first_input_index = found->input_index,
                            .second_input_index = input_index
                        });
                    }
                    return name;
                }
                pending_names[index].insert(name);
                entries.push_back({ .source = source, .name = name,
                    .declarations = source.declarations(), .input_index = input_index });
                return name;
            };
            const detail::basic_definition_names names{
                .dendro_core = prepare(basics.dendro_core, 0),
                .catalyzing_field = prepare(basics.catalyzing_field, 1),
                .burning_flame = prepare(basics.burning_flame, 2),
                .frozen = prepare(basics.frozen, 3)
            };
            definition_types::each([&]<class TCategory>
            {
                const auto& entries = std::get<index_of<TCategory>()>(pending);
                for(std::size_t index = 0; index < entries.size(); ++index)
                    sources.collect_dependency_errors<TCategory>(pending, pending_names, index, errors);
            });
            sources.commit_pending(pending);
            return { std::move(sources), names };
        }

        selection_mask make_empty_selection() const
        {
            selection_mask selected;
            [&]<size_t...I>(std::index_sequence<I...>)
            {
                ((selected[I].assign(std::get<I>(buckets_).entries.size(), false)), ...);
            }(std::make_index_sequence<definition_count>{});
            return selected;
        }

        selection_mask make_full_selection() const
        {
            auto selected = make_empty_selection();
            [&]<size_t...I>(std::index_sequence<I...>)
            {
                ((selected[I].assign(std::get<I>(buckets_).entries.size(), true)), ...);
            }(std::make_index_sequence<definition_count>{});
            return selected;
        }

        selection_mask resolve_selection(const definition_selection& selection,
            const detail::basic_definition_names& basics, std::vector<source_preparation_error>& errors) const
        {
            auto selected = make_empty_selection();
            std::vector<queue_item> queue;

            enqueue_name<index_of<combat_status_view>()>(basics.dendro_core, selected, queue, errors);
            enqueue_name<index_of<combat_status_view>()>(basics.catalyzing_field, selected, queue, errors);
            enqueue_name<index_of<summon_view>()>(basics.burning_flame, selected, queue, errors);
            enqueue_name<index_of<attachment_view>()>(basics.frozen, selected, queue, errors);

            [&]<size_t...I>(std::index_sequence<I...>)
            {
                (enqueue_selected_names<I>(selection[I], selected, queue, errors), ...);
            }(std::make_index_sequence<definition_count>{});

            for(size_t index = 0; index < queue.size(); ++index)
            {
                process_queue_item(queue[index], selected, queue, errors);
            }

            return selected;
        }

        template<size_t I>
        void enqueue(size_t source_index, selection_mask& selected, std::vector<queue_item>& queue) const
        {
            if(not selected[I][source_index])
            {
                selected[I][source_index] = true;
                queue.push_back({ .entity_index = I, .source_index = source_index });
            }
        }

        template<size_t I>
        void enqueue_name(
            std::string_view name,
            selection_mask& selected,
            std::vector<queue_item>& queue,
            std::vector<source_preparation_error>& errors,
            const definition_name* required_by = nullptr
        ) const
        {
            const auto& bucket = std::get<I>(buckets_);
            const auto iter = bucket.name_to_index.find(name);
            if(iter == bucket.name_to_index.end())
            {
                const auto already_reported = std::ranges::any_of(errors, [&](const auto& error)
                {
                    const auto same_owner = [&](const definition_name& source)
                    {
                        return required_by && source.category_index == required_by->category_index
                            && source.name == required_by->name;
                    };
                    if(const auto* missing = std::get_if<source_missing_dependency>(&error))
                        return same_owner(missing->source) && missing->dependency.category_index == I
                            && missing->dependency.name == name;
                    if(const auto* missing = std::get_if<source_selection_error>(&error))
                        return missing->definition.category_index == I && missing->definition.name == name
                            && (required_by ? missing->required_by && same_owner(*missing->required_by)
                                : not missing->required_by);
                    return false;
                });
                if(not already_reported)
                    errors.emplace_back(source_selection_error{ { I, std::string{ name } },
                        required_by ? std::optional{ *required_by } : std::nullopt });
                return;
            }
            enqueue<I>(iter->second, selected, queue);
        }

        template<size_t I>
        void enqueue_selected_names(
            std::span<const std::string_view> names,
            selection_mask& selected,
            std::vector<queue_item>& queue,
            std::vector<source_preparation_error>& errors
        ) const
        {
            for(std::string_view name : names)
            {
                enqueue_name<I>(name, selected, queue, errors);
            }
        }

        void process_queue_item(
            queue_item item,
            selection_mask& selected,
            std::vector<queue_item>& queue,
            std::vector<source_preparation_error>& errors
        ) const
        {
            [&]<size_t...I>(std::index_sequence<I...>)
            {
                ((item.entity_index == I ? (process_selected_source<I>(item.source_index, selected, queue, errors), 0) : 0), ...);
            }(std::make_index_sequence<definition_count>{});
        }

        template<size_t I>
        void process_selected_source(
            size_t source_index,
            selection_mask& selected,
            std::vector<queue_item>& queue,
            std::vector<source_preparation_error>& errors
        ) const
        {
            const auto& entry = std::get<I>(buckets_).entries[source_index];
            const definition_name required_by{ I, std::string{ entry.name } };
            [&]<size_t...J>(std::index_sequence<J...>)
            {
                (enqueue_dependencies<J>(entry.declarations.dependencies[J], selected, queue, errors, required_by), ...);
            }(std::make_index_sequence<definition_count>{});
        }

        template<size_t I>
        void enqueue_dependencies(
            const std::vector<std::string_view>& dependencies,
            selection_mask& selected,
            std::vector<queue_item>& queue,
            std::vector<source_preparation_error>& errors,
            const definition_name& required_by
        ) const
        {
            for(std::string_view name : dependencies)
            {
                enqueue_name<I>(name, selected, queue, errors, &required_by);
            }
        }

        issued_id_map make_issued_id_map(const selection_mask& selected) const
        {
            std::vector<std::string_view> tags;
            [&]<size_t...I>(std::index_sequence<I...>)
            {
                (collect_selected_tags<I>(selected[I], tags), ...);
            }(std::make_index_sequence<definition_count>{});

            std::ranges::sort(tags);
            const auto unique_end = std::ranges::unique(tags).begin();
            tags.erase(unique_end, tags.end());

            issued_id_map id_map{ tags };
            [&]<size_t...I>(std::index_sequence<I...>)
            {
                (add_selected_sources<I>(selected[I], id_map), ...);
            }(std::make_index_sequence<definition_count>{});

            return id_map;
        }

        template<size_t I>
        void collect_selected_tags(
            const std::vector<bool>& selected,
            std::vector<std::string_view>& tags
        ) const
        {
            const auto& entries = std::get<I>(buckets_).entries;
            for(size_t index : ordered_selected_indices<I>(selected))
            {
                const auto& declarations = entries[index].declarations;
                tags.insert(tags.end(), declarations.tags.begin(), declarations.tags.end());
            }
        }

        template<size_t I>
        void add_selected_sources(const std::vector<bool>& selected, issued_id_map& id_map) const
        {
            using definition_category = typename definition_type_list::template type_at<I>;
            const auto& entries = std::get<I>(buckets_).entries;
            for(size_t index : ordered_selected_indices<I>(selected))
            {
                id_map.template add<definition_category>(
                    entries[index].name,
                    entries[index].declarations.tags
                );
            }
        }

        template<size_t I>
        std::vector<size_t> ordered_selected_indices(const std::vector<bool>& selected) const
        {
            const auto& entries = std::get<I>(buckets_).entries;
            std::vector<size_t> indices;
            indices.reserve(entries.size());
            for(size_t index = 0; index < entries.size(); ++index)
            {
                if(selected[index])
                {
                    indices.push_back(index);
                }
            }

            std::ranges::sort(indices, [&](size_t left, size_t right)
            {
                return entries[left].name < entries[right].name;
            });
            return indices;
        }

        bucket_tuple buckets_;

        friend class definition_library;
    };

    template<class... TSources>
    inline std::expected<definition_source_library, std::vector<source_add_error>> make_definition_source_library(const TSources&... sources)
    {
        definition_source_library library;
        auto result = library.add(sources...);
        if(not result) return std::unexpected{ std::move(result.error()) };
        return library;
    }
}

#endif
