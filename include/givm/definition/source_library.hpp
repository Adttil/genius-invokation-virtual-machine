#ifndef GIVM_DEFINITION_SOURCE_LIBRARY_HPP
#define GIVM_DEFINITION_SOURCE_LIBRARY_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "library.hpp"
#include "source_view.hpp"
#include "issued_id_map.hpp"
#include "types.hpp"
#include "../utils/type_list.hpp"

namespace givm
{
    struct definition_compile_result
    {
        definition_library library;
        issued_id_map id_map;
    };

    using definition_selection = std::array<std::span<const std::string_view>, definition_types::size()>;

    class definition_source_library
    {
    public:
        static constexpr size_t definition_count = definition_types::size();

        bool add()
        {
            return true;
        }

        bool add(const definition_source_library& library)
        {
            if(has_name_conflict(library))
            {
                return false;
            }

            append_library(library);
            return true;
        }

        template<class TSource>
        bool add(const TSource& source)
        {
            using source_type = std::remove_cvref_t<TSource>;
            using definition_category = typename source_type::definition_category;
            constexpr size_t definition_index = index_of<definition_category>();

            definition_source_view<definition_category> view{ source };
            const std::string_view name = view.name();
            auto& bucket = bucket_for<definition_category>();
            if(bucket.name_to_index.contains(name))
            {
                return false;
            }

            auto declarations = view.declarations();
            if(not check_single_dependencies<definition_index>(declarations.dependencies, name))
            {
                return false;
            }

            bucket.name_to_index.emplace(name, bucket.entries.size());
            bucket.entries.push_back({
                .source = view,
                .name = name,
                .declarations = std::move(declarations)
            });
            return true;
        }

        template<class TFirstSource, class TSecondSource, class...TOtherSource>
        bool add(const TFirstSource& first, const TSecondSource& second, const TOtherSource&...others)
        {
            pending_tuple pending;
            pending_names_type pending_names;
            bool valid = true;

            prepare_add(pending, pending_names, valid, first);
            prepare_add(pending, pending_names, valid, second);
            (prepare_add(pending, pending_names, valid, others), ...);
            if(not valid)
            {
                return false;
            }

            if(not check_pending_dependencies(pending, pending_names))
            {
                return false;
            }

            commit_pending(pending);
            return true;
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

        issued_id_map make_issued_id_map() const
        {
            return make_issued_id_map(make_full_selection());
        }

        issued_id_map make_issued_id_map(const definition_selection& selection) const
        {
            return make_issued_id_map(resolve_selection(selection));
        }

        template<class TInitializationSequence, class TRoundSequence>
        definition_compile_result compile(
            TInitializationSequence&& initialization_program,
            TRoundSequence&& round_program
        ) const
        {
            return compile(
                make_full_selection(),
                std::forward<TInitializationSequence>(initialization_program),
                std::forward<TRoundSequence>(round_program)
            );
        }

        template<class TInitializationSequence, class TRoundSequence>
        definition_compile_result compile(
            const definition_selection& selection,
            TInitializationSequence&& initialization_program,
            TRoundSequence&& round_program
        ) const
        {
            return compile(
                resolve_selection(selection),
                std::forward<TInitializationSequence>(initialization_program),
                std::forward<TRoundSequence>(round_program)
            );
        }

    private:
        using definition_type_list = definition_types;
        using dependency_array = detail::definition_dependency_lists;
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

        struct parsed_tag_filter
        {
            std::vector<std::string_view> required;
            std::vector<std::string_view> excluded;
        };

        template<class...TDefinition>
        using bucket_tuple_for = std::tuple<bucket<TDefinition>...>;

        template<class...TDefinition>
        using pending_tuple_for = std::tuple<std::vector<entry<TDefinition>>...>;

        using bucket_tuple = definition_type_list::apply<bucket_tuple_for>;
        using pending_tuple = definition_type_list::apply<pending_tuple_for>;

        static constexpr std::string_view whitespace = " \t\n\r\f\v";
        static constexpr std::string_view negation_or_whitespace = "! \t\n\r\f\v";

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
        bool has_bucket_name_conflict(const definition_source_library& library) const
        {
            const auto& bucket = std::get<I>(buckets_);
            for(const auto& entry : std::get<I>(library.buckets_).entries)
            {
                if(bucket.name_to_index.contains(entry.name))
                {
                    return true;
                }
            }
            return false;
        }

        bool has_name_conflict(const definition_source_library& library) const
        {
            bool conflict = false;
            [&]<size_t...I>(std::index_sequence<I...>)
            {
                ((conflict = conflict || has_bucket_name_conflict<I>(library)), ...);
            }(std::make_index_sequence<definition_count>{});
            return conflict;
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

        template<class TSource>
        void prepare_add(
            pending_tuple& pending,
            pending_names_type& pending_names,
            bool& valid,
            const TSource& source
        ) const
        {
            if(not valid)
            {
                return;
            }

            using source_type = std::remove_cvref_t<TSource>;
            using definition_category = typename source_type::definition_category;
            constexpr size_t definition_index = index_of<definition_category>();

            definition_source_view<definition_category> view{ source };
            const std::string_view name = view.name();
            if(bucket_for<definition_category>().name_to_index.contains(name))
            {
                valid = false;
                return;
            }

            auto [_, inserted] = pending_names[definition_index].emplace(name);
            if(not inserted)
            {
                valid = false;
                return;
            }

            std::get<definition_index>(pending).push_back({
                .source = view,
                .name = name,
                .declarations = view.declarations()
            });
        }

        template<size_t I>
        bool dependency_exists(std::string_view name, const pending_names_type& pending_names) const
        {
            return std::get<I>(buckets_).name_to_index.contains(name) || pending_names[I].contains(name);
        }

        template<size_t SourceIndex, size_t I>
        bool single_dependency_exists(std::string_view dependency_name, std::string_view source_name) const
        {
            if constexpr (SourceIndex == I)
            {
                return dependency_name == source_name || std::get<I>(buckets_).name_to_index.contains(dependency_name);
            }
            else
            {
                return std::get<I>(buckets_).name_to_index.contains(dependency_name);
            }
        }

        template<size_t SourceIndex, size_t...I>
        bool check_single_dependencies(
            const dependency_array& dependencies,
            std::string_view source_name,
            std::index_sequence<I...>
        ) const
        {
            bool valid = true;
                ((valid = valid && std::ranges::all_of(dependencies[I], [&](std::string_view dependency_name)
            {
                return single_dependency_exists<SourceIndex, I>(dependency_name, source_name);
            })), ...);
            return valid;
        }

        template<size_t SourceIndex>
        bool check_single_dependencies(
            const dependency_array& dependencies,
            std::string_view source_name
        ) const
        {
            return check_single_dependencies<SourceIndex>(
                dependencies,
                source_name,
                std::make_index_sequence<definition_count>{}
            );
        }

        template<class TEntry, size_t...I>
        bool check_entry_dependencies(
            const TEntry& entry,
            const pending_names_type& pending_names,
            std::index_sequence<I...>
        ) const
        {
            bool valid = true;
                ((valid = valid && std::ranges::all_of(entry.declarations.dependencies[I], [&](std::string_view name)
            {
                return dependency_exists<I>(name, pending_names);
            })), ...);
            return valid;
        }

        template<size_t I>
        bool check_pending_bucket_dependencies(
            const pending_tuple& pending,
            const pending_names_type& pending_names
        ) const
        {
            for(const auto& entry : std::get<I>(pending))
            {
                if(not check_entry_dependencies(entry, pending_names, std::make_index_sequence<definition_count>{}))
                {
                    return false;
                }
            }
            return true;
        }

        bool check_pending_dependencies(
            const pending_tuple& pending,
            const pending_names_type& pending_names
        ) const
        {
            bool valid = true;
            [&]<size_t...I>(std::index_sequence<I...>)
            {
                ((valid = valid && check_pending_bucket_dependencies<I>(pending, pending_names)), ...);
            }(std::make_index_sequence<definition_count>{});
            return valid;
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
                bucket.entries.push_back(std::move(entry));
            }
        }

        void commit_pending(pending_tuple& pending)
        {
            [&]<size_t...I>(std::index_sequence<I...>)
            {
                (commit_pending_bucket<I>(pending), ...);
            }(std::make_index_sequence<definition_count>{});
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

        selection_mask resolve_selection(const definition_selection& selection) const
        {
            auto selected = make_empty_selection();
            std::vector<queue_item> queue;

            [&]<size_t...I>(std::index_sequence<I...>)
            {
                (enqueue_selected_names<I>(selection[I], selected, queue), ...);
            }(std::make_index_sequence<definition_count>{});

            for(size_t index = 0; index < queue.size(); ++index)
            {
                process_queue_item(queue[index], selected, queue);
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
            std::vector<queue_item>& queue
        ) const
        {
            const auto& bucket = std::get<I>(buckets_);
            const auto iter = bucket.name_to_index.find(name);
            if(iter == bucket.name_to_index.end())
            {
                throw std::invalid_argument{ "unknown definition source name" };
            }
            enqueue<I>(iter->second, selected, queue);
        }

        template<size_t I>
        void enqueue_selected_names(
            std::span<const std::string_view> names,
            selection_mask& selected,
            std::vector<queue_item>& queue
        ) const
        {
            for(std::string_view name : names)
            {
                enqueue_name<I>(name, selected, queue);
            }
        }

        void process_queue_item(
            queue_item item,
            selection_mask& selected,
            std::vector<queue_item>& queue
        ) const
        {
            [&]<size_t...I>(std::index_sequence<I...>)
            {
                ((item.entity_index == I ? (process_selected_source<I>(item.source_index, selected, queue), 0) : 0), ...);
            }(std::make_index_sequence<definition_count>{});
        }

        template<size_t I>
        void process_selected_source(
            size_t source_index,
            selection_mask& selected,
            std::vector<queue_item>& queue
        ) const
        {
            const auto& entry = std::get<I>(buckets_).entries[source_index];
            [&]<size_t...J>(std::index_sequence<J...>)
            {
                (enqueue_dependencies<J>(entry.declarations.dependencies[J], selected, queue), ...);
                (enqueue_dependencies_by_tag<J>(entry.declarations.dependencies_by_tag[J], selected, queue), ...);
            }(std::make_index_sequence<definition_count>{});
        }

        template<size_t I>
        void enqueue_dependencies(
            const std::vector<std::string_view>& dependencies,
            selection_mask& selected,
            std::vector<queue_item>& queue
        ) const
        {
            for(std::string_view name : dependencies)
            {
                enqueue_name<I>(name, selected, queue);
            }
        }

        template<size_t I>
        void enqueue_dependencies_by_tag(
            const std::vector<std::string_view>& filters,
            selection_mask& selected,
            std::vector<queue_item>& queue
        ) const
        {
            for(std::string_view filter : filters)
            {
                const parsed_tag_filter parsed = parse_tag_filter(filter);
                enqueue_matching_tags<I>(parsed, selected, queue);
            }
        }

        template<size_t I>
        void enqueue_matching_tags(
            const parsed_tag_filter& filter,
            selection_mask& selected,
            std::vector<queue_item>& queue
        ) const
        {
            const auto& entries = std::get<I>(buckets_).entries;
            for(size_t index = 0; index < entries.size(); ++index)
            {
                if(match_tags(entries[index].declarations.tags, filter))
                {
                    enqueue<I>(index, selected, queue);
                }
            }
        }

        static parsed_tag_filter parse_tag_filter(std::string_view expression)
        {
            const auto terms = expression
                | std::views::split('&')
                | std::ranges::to<std::vector>();

            parsed_tag_filter result;
            result.required.reserve(terms.size());
            result.excluded.reserve(terms.size());

            for(const auto& range : terms)
            {
                const std::string_view term{ range };
                const size_t tag_end = term.find_last_not_of(whitespace) + 1;
                const size_t split = term.find_last_of(negation_or_whitespace, tag_end - 1);
                if(split == std::string_view::npos)
                {
                    result.required.emplace_back(term.substr(0, tag_end));
                    continue;
                }

                const std::string_view tag = term.substr(split + 1, tag_end - split - 1);
                if(term[split] == '!' || term.find_last_not_of(whitespace, split) != std::string_view::npos)
                {
                    result.excluded.emplace_back(tag);
                }
                else
                {
                    result.required.emplace_back(tag);
                }
            }

            return result;
        }

        static bool contains_tag(
            const std::vector<std::string_view>& tags,
            std::string_view tag
        )
        {
            return std::ranges::find(tags, tag) != tags.end();
        }

        static bool match_tags(
            const std::vector<std::string_view>& tags,
            const parsed_tag_filter& filter
        )
        {
            for(std::string_view tag : filter.required)
            {
                if(not contains_tag(tags, tag))
                {
                    return false;
                }
            }

            for(std::string_view tag : filter.excluded)
            {
                if(contains_tag(tags, tag))
                {
                    return false;
                }
            }

            return true;
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

        template<class TInitializationSequence, class TRoundSequence>
        definition_compile_result compile(
            const selection_mask& selected,
            TInitializationSequence&& initialization_program,
            TRoundSequence&& round_program
        ) const
        {
            auto id_map = make_issued_id_map(selected);
            definition_library result{ id_map.tag_names() };
            detail::append_instructions<void>(
                result.program_,
                std::forward<TInitializationSequence>(initialization_program)
            );
            const execution_position round_entry_position = result.program_.size();
            detail::append_instructions<void>(
                result.program_,
                std::forward<TRoundSequence>(round_program)
            );
            result.program_.push_back(detail::any_instruction{
                detail::jump_instruction{ .target = round_entry_position }
            });

            [&]<size_t...I>(std::index_sequence<I...>)
            {
                (append_selected_sources<I>(selected[I], id_map, result), ...);
            }(std::make_index_sequence<definition_count>{});

            return {
                .library = std::move(result),
                .id_map = std::move(id_map)
            };
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
                tags.insert(
                    tags.end(),
                    declarations.tag_dependencies.begin(),
                    declarations.tag_dependencies.end()
                );

                [&]<size_t...J>(std::index_sequence<J...>)
                {
                    (collect_filter_tags(declarations.dependencies_by_tag[J], tags), ...);
                }(std::make_index_sequence<definition_count>{});
            }
        }

        static void collect_filter_tags(
            const std::vector<std::string_view>& filters,
            std::vector<std::string_view>& tags
        )
        {
            for(std::string_view filter : filters)
            {
                const parsed_tag_filter parsed = parse_tag_filter(filter);
                tags.insert(tags.end(), parsed.required.begin(), parsed.required.end());
                tags.insert(tags.end(), parsed.excluded.begin(), parsed.excluded.end());
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
        void append_selected_sources(
            const std::vector<bool>& selected,
            const issued_id_map& id_map,
            definition_library& library
        ) const
        {
            using definition_category = typename definition_type_list::template type_at<I>;
            const auto& entries = std::get<I>(buckets_).entries;
            for(size_t index : ordered_selected_indices<I>(selected))
            {
                library.template append<definition_category>(
                    entries[index].source,
                    entries[index].name,
                    id_map,
                    entries[index].declarations
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
    };
}

#endif
