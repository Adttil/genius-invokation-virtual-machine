#ifndef GIVM_DEFINITION_SOURCE_LIBRARY_HPP
#define GIVM_DEFINITION_SOURCE_LIBRARY_HPP

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
#include "source_error.hpp"
#include "definition_categories.hpp"
#include "../utils/type_list.hpp"

namespace givm
{
    class issued_id_map;

    class reaction_definition_names
    {
    public:
        constexpr reaction_definition_names() noexcept = default;

        constexpr explicit reaction_definition_names(std::string_view name) noexcept
        {
            names_.fill(name);
        }

        constexpr std::string_view& operator[](elemental_reaction slot) noexcept
        {
            return names_[static_cast<std::size_t>(slot) - 1];
        }

        constexpr const std::string_view& operator[](elemental_reaction slot) const noexcept
        {
            return names_[static_cast<std::size_t>(slot) - 1];
        }

    private:
        std::array<std::string_view, elemental_reaction_count> names_{};
    };

    class definition_source_library
    {
    public:
        static constexpr size_t definition_count = definition_types::size();

        definition_source_library() = default;

        bool empty() const noexcept
        {
            return std::apply([](const auto&... buckets)
            {
                return (buckets.entries.empty() && ...);
            }, buckets_);
        }

        std::expected<void, std::vector<source_add_error>> add()
        {
            return {};
        }

        std::expected<void, std::vector<source_conflict>> add(const definition_source_library& library);

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
            const definition_source_library& library, std::vector<source_conflict>& errors) const;

        template<size_t I>
        void append_library_bucket(const definition_source_library& library);

        void append_library(const definition_source_library& library);

        template<class... TSources>
        std::expected<void, std::vector<source_add_error>> add_sources(const TSources&... sources)
        {
            pending_tuple pending;
            pending_sources_by_name names;
            std::vector<source_add_error> errors;
            std::size_t input_index = 0;
            const std::array indices{
                prepare_add(pending, names, errors, input_index++,
                    definition_source_view<typename TSources::definition_category>{ sources })...
            };
            input_index = 0;
            (collect_dependency_errors<typename TSources::definition_category>(
                pending, names, indices[input_index++], errors), ...);
            if(not errors.empty()) return std::unexpected{ std::move(errors) };
            commit_pending(pending);
            return {};
        }

        template<class TCategory>
        std::optional<std::size_t> prepare_add(pending_tuple& pending, pending_sources_by_name& names,
            std::vector<source_add_error>& errors, std::size_t input_index, const definition_source_view<TCategory>& view) const;

        template<size_t I, class TPendingNames>
        bool dependency_exists(std::string_view name, const TPendingNames& pending_names) const;

        template<class TCategory, class TPendingNames, class TError>
        void collect_dependency_errors(const pending_tuple& pending, const TPendingNames& names,
            std::optional<std::size_t> index, std::vector<TError>& errors) const;

        template<size_t I>
        void commit_pending_bucket(pending_tuple& pending);

        void commit_pending(pending_tuple& pending);

        selection_mask make_empty_selection() const;

        selection_mask make_full_selection() const;

        selection_mask resolve_selection(const std::array<std::span<const std::string_view>, definition_types::size()>& selection,
            const reaction_definition_names& basics, std::vector<source_preparation_error>& errors) const;

        template<size_t I>
        void enqueue(size_t source_index, selection_mask& selected, std::vector<queue_item>& queue) const;

        template<size_t I>
        void enqueue_name(
            std::string_view name,
            selection_mask& selected,
            std::vector<queue_item>& queue,
            std::vector<source_preparation_error>& errors,
            const definition_name* required_by = nullptr
        ) const;

        template<size_t I>
        void enqueue_selected_names(
            std::span<const std::string_view> names,
            selection_mask& selected,
            std::vector<queue_item>& queue,
            std::vector<source_preparation_error>& errors
        ) const;

        void process_queue_item(
            queue_item item,
            selection_mask& selected,
            std::vector<queue_item>& queue,
            std::vector<source_preparation_error>& errors
        ) const;

        template<size_t I>
        void process_selected_source(
            size_t source_index,
            selection_mask& selected,
            std::vector<queue_item>& queue,
            std::vector<source_preparation_error>& errors
        ) const;

        template<size_t I>
        void enqueue_dependencies(
            const std::vector<std::string_view>& dependencies,
            selection_mask& selected,
            std::vector<queue_item>& queue,
            std::vector<source_preparation_error>& errors,
            const definition_name& required_by
        ) const;

        issued_id_map make_issued_id_map(const selection_mask& selected) const;

        template<size_t I>
        void collect_selected_tags(
            const std::vector<bool>& selected,
            std::vector<std::string_view>& tags
        ) const;

        template<size_t I>
        void add_selected_sources(const std::vector<bool>& selected, issued_id_map& id_map) const;

        template<size_t I>
        std::vector<size_t> ordered_selected_indices(const std::vector<bool>& selected) const;

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
