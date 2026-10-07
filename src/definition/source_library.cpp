#include <givm/definition/source_library.hpp>
#include <givm/definition/issued_id_map.hpp>

#include <algorithm>

namespace givm::detail
{
    std::string source_definition_name_text(const definition_name& definition)
    {
        constexpr auto category_names = []
        {
            std::array<std::string_view, detail::definition_categories.size()> names{};
            names[static_cast<std::size_t>(definition_category::card)] = "card";
            names[static_cast<std::size_t>(definition_category::card_status)] = "card_status";
            names[static_cast<std::size_t>(definition_category::support)] = "support";
            names[static_cast<std::size_t>(definition_category::summon)] = "summon";
            names[static_cast<std::size_t>(definition_category::combat_status)] = "combat_status";
            names[static_cast<std::size_t>(definition_category::character)] = "character";
            names[static_cast<std::size_t>(definition_category::skill)] = "skill";
            names[static_cast<std::size_t>(definition_category::attachment)] = "attachment";
            names[static_cast<std::size_t>(definition_category::history_summary)] = "history_summary";
            names[static_cast<std::size_t>(definition_category::reaction)] = "reaction";
            return names;
        }();
        const auto category_index = static_cast<std::size_t>(definition.category);
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

    void append_source_error(std::string& text, const source_conflict& error)
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

    void append_source_error(std::string& text, const source_missing_dependency& error)
    {
        if(not text.empty()) text += '\n';
        text += "missing dependency: ";
        text += source_definition_name_text(error.source);
        text += " (input[" + std::to_string(error.input_index) + "]) requires ";
        text += source_definition_name_text(error.dependency);
    }

    void append_source_error(std::string& text, const source_selection_error& error)
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

namespace givm
{
    std::string error_string(const source_conflict& error)
    {
        std::string result;
        detail::append_source_error(result, error);
        return result;
    }

    std::string error_string(const source_missing_dependency& error)
    {
        std::string result;
        detail::append_source_error(result, error);
        return result;
    }

    std::string error_string(const source_selection_error& error)
    {
        std::string result;
        detail::append_source_error(result, error);
        return result;
    }

    std::string error_string(const std::vector<source_add_error>& errors)
    {
        std::string result;
        for(const auto& error : errors)
            std::visit([&](const auto& item) { detail::append_source_error(result, item); }, error);
        return result;
    }

    std::string error_string(const std::vector<source_conflict>& errors)
    {
        std::string result;
        for(const auto& error : errors) detail::append_source_error(result, error);
        return result;
    }

    std::string error_string(const std::vector<source_preparation_error>& errors)
    {
        std::string result;
        for(const auto& error : errors)
            std::visit([&](const auto& item) { detail::append_source_error(result, item); }, error);
        return result;
    }

    std::expected<void, std::vector<source_conflict>> definition_source_library::add(const definition_source_library& library)
    {
        if(this == &library) return {};
        std::vector<source_conflict> errors;
        []<std::size_t... I>(std::index_sequence<I...>, auto&& fn)
        {
            (fn.template operator()<detail::definition_categories[I]>(), ...);
        }(std::make_index_sequence<detail::definition_categories.size()>{}, [&]<definition_category TCategory>
        {
            collect_bucket_conflicts<static_cast<std::size_t>(TCategory)>(library, errors);
        });
        if(not errors.empty()) return std::unexpected{ std::move(errors) };

        append_library(library);
        return {};
    }

    template<size_t I>
    void definition_source_library::collect_bucket_conflicts(
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
                    .definition = { static_cast<definition_category>(I), std::string{ entry.name } },
                    .cause = existing.source.rtti_ != entry.source.rtti_
                        ? source_conflict::reason::different_type
                        : source_conflict::reason::different_object
                });
            }
        }
    }

    template<size_t I>
    void definition_source_library::append_library_bucket(const definition_source_library& library)
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

    void definition_source_library::append_library(const definition_source_library& library)
    {
        [&]<size_t...I>(std::index_sequence<I...>)
        {
            (append_library_bucket<I>(library), ...);
        }(std::make_index_sequence<definition_count>{});
    }

    template<definition_category TCategory>
    std::optional<std::size_t> definition_source_library::prepare_add(pending_tuple& pending, pending_sources_by_name& names,
        std::vector<source_add_error>& errors, std::size_t input_index, const definition_source_view<TCategory>& view) const
    {
        constexpr auto category_index = index_of<TCategory>();
        const auto name = view.name();
        const auto& bucket = bucket_for<TCategory>();
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
                .definition = { TCategory, std::string{ name } },
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
    bool definition_source_library::dependency_exists(std::string_view name, const TPendingNames& pending_names) const
    {
        return std::get<I>(buckets_).name_to_index.contains(name) || pending_names[I].contains(name);
    }

    template<definition_category TCategory, class TPendingNames, class TError>
    void definition_source_library::collect_dependency_errors(const pending_tuple& pending, const TPendingNames& names,
        std::optional<std::size_t> index, std::vector<TError>& errors) const
    {
        if(not index) return;
        constexpr auto category_index = index_of<TCategory>();
        const auto& entry = std::get<category_index>(pending)[*index];
        []<std::size_t... I>(std::index_sequence<I...>, auto&& fn)
        {
            (fn.template operator()<detail::definition_categories[I]>(), ...);
        }(std::make_index_sequence<detail::definition_categories.size()>{}, [&]<definition_category TDependency>
        {
            constexpr auto dependency_index = index_of<TDependency>();
            std::unordered_set<std::string_view> reported;
            for(const auto name : entry.declarations.dependencies[dependency_index])
            {
                if(not dependency_exists<dependency_index>(name, names) && reported.insert(name).second)
                {
                    errors.emplace_back(source_missing_dependency{
                        .source = { TCategory, std::string{ entry.name } },
                        .input_index = entry.input_index,
                        .dependency = { TDependency, std::string{ name } }
                    });
                }
            }
        });
    }

    template<size_t I>
    void definition_source_library::commit_pending_bucket(pending_tuple& pending)
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

    void definition_source_library::commit_pending(pending_tuple& pending)
    {
        [&]<size_t...I>(std::index_sequence<I...>)
        {
            (commit_pending_bucket<I>(pending), ...);
        }(std::make_index_sequence<definition_count>{});
    }

    definition_source_library::selection_mask definition_source_library::make_empty_selection() const
    {
        selection_mask selected;
        [&]<size_t...I>(std::index_sequence<I...>)
        {
            ((selected[I].assign(std::get<I>(buckets_).entries.size(), false)), ...);
        }(std::make_index_sequence<definition_count>{});
        return selected;
    }

    definition_source_library::selection_mask definition_source_library::make_full_selection() const
    {
        auto selected = make_empty_selection();
        [&]<size_t...I>(std::index_sequence<I...>)
        {
            ((selected[I].assign(std::get<I>(buckets_).entries.size(), true)), ...);
        }(std::make_index_sequence<definition_count>{});
        return selected;
    }

    definition_source_library::selection_mask definition_source_library::resolve_selection(
        const std::array<std::span<const std::string_view>, detail::definition_categories.size()>& selection,
        const reaction_definition_names& basics, std::vector<source_preparation_error>& errors) const
    {
        auto selected = make_empty_selection();
        std::vector<queue_item> queue;

        for(std::size_t index = 0; index != elemental_reaction_count; ++index)
            enqueue_name<index_of<definition_category::reaction>()>(basics[static_cast<elemental_reaction>(index + 1)],
                selected, queue, errors);

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
    void definition_source_library::enqueue(size_t source_index, selection_mask& selected, std::vector<queue_item>& queue) const
    {
        if(not selected[I][source_index])
        {
            selected[I][source_index] = true;
            queue.push_back({ .entity_index = I, .source_index = source_index });
        }
    }

    template<size_t I>
    void definition_source_library::enqueue_name(
        std::string_view name,
        selection_mask& selected,
        std::vector<queue_item>& queue,
        std::vector<source_preparation_error>& errors,
        const definition_name* required_by
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
                    return required_by && source.category == required_by->category
                        && source.name == required_by->name;
                };
                if(const auto* missing = std::get_if<source_missing_dependency>(&error))
                    return same_owner(missing->source) && missing->dependency.category == static_cast<definition_category>(I)
                        && missing->dependency.name == name;
                if(const auto* missing = std::get_if<source_selection_error>(&error))
                    return missing->definition.category == static_cast<definition_category>(I) && missing->definition.name == name
                        && (required_by ? missing->required_by && same_owner(*missing->required_by)
                            : not missing->required_by);
                return false;
            });
            if(not already_reported)
                errors.emplace_back(source_selection_error{ { static_cast<definition_category>(I), std::string{ name } },
                    required_by ? std::optional{ *required_by } : std::nullopt });
            return;
        }
        enqueue<I>(iter->second, selected, queue);
    }

    template<size_t I>
    void definition_source_library::enqueue_selected_names(
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

    void definition_source_library::process_queue_item(
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
    void definition_source_library::process_selected_source(
        size_t source_index,
        selection_mask& selected,
        std::vector<queue_item>& queue,
        std::vector<source_preparation_error>& errors
    ) const
    {
        const auto& entry = std::get<I>(buckets_).entries[source_index];
        const definition_name required_by{ static_cast<definition_category>(I), std::string{ entry.name } };
        [&]<size_t...J>(std::index_sequence<J...>)
        {
            (enqueue_dependencies<J>(entry.declarations.dependencies[J], selected, queue, errors, required_by), ...);
        }(std::make_index_sequence<definition_count>{});
    }

    template<size_t I>
    void definition_source_library::enqueue_dependencies(
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

    issued_id_map definition_source_library::make_issued_id_map(const selection_mask& selected) const
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
    void definition_source_library::collect_selected_tags(
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
    void definition_source_library::add_selected_sources(const std::vector<bool>& selected, issued_id_map& id_map) const
    {
        constexpr auto category = static_cast<definition_category>(I);
        const auto& entries = std::get<I>(buckets_).entries;
        for(size_t index : ordered_selected_indices<I>(selected))
        {
            id_map.template add<category>(
                entries[index].name,
                entries[index].declarations.tags
            );
        }
    }

    template<size_t I>
    std::vector<size_t> definition_source_library::ordered_selected_indices(const std::vector<bool>& selected) const
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

    template std::optional<std::size_t> definition_source_library::prepare_add<definition_category::card>(
        pending_tuple&, pending_sources_by_name&, std::vector<source_add_error>&,
        std::size_t, const definition_source_view<definition_category::card>&) const;
    template void definition_source_library::collect_dependency_errors<definition_category::card>(
        const pending_tuple&, const pending_sources_by_name&,
        std::optional<std::size_t>, std::vector<source_add_error>&) const;

    template std::optional<std::size_t> definition_source_library::prepare_add<definition_category::card_status>(
        pending_tuple&, pending_sources_by_name&, std::vector<source_add_error>&,
        std::size_t, const definition_source_view<definition_category::card_status>&) const;
    template void definition_source_library::collect_dependency_errors<definition_category::card_status>(
        const pending_tuple&, const pending_sources_by_name&,
        std::optional<std::size_t>, std::vector<source_add_error>&) const;

    template std::optional<std::size_t> definition_source_library::prepare_add<definition_category::support>(
        pending_tuple&, pending_sources_by_name&, std::vector<source_add_error>&,
        std::size_t, const definition_source_view<definition_category::support>&) const;
    template void definition_source_library::collect_dependency_errors<definition_category::support>(
        const pending_tuple&, const pending_sources_by_name&,
        std::optional<std::size_t>, std::vector<source_add_error>&) const;

    template std::optional<std::size_t> definition_source_library::prepare_add<definition_category::summon>(
        pending_tuple&, pending_sources_by_name&, std::vector<source_add_error>&,
        std::size_t, const definition_source_view<definition_category::summon>&) const;
    template void definition_source_library::collect_dependency_errors<definition_category::summon>(
        const pending_tuple&, const pending_sources_by_name&,
        std::optional<std::size_t>, std::vector<source_add_error>&) const;

    template std::optional<std::size_t> definition_source_library::prepare_add<definition_category::combat_status>(
        pending_tuple&, pending_sources_by_name&, std::vector<source_add_error>&,
        std::size_t, const definition_source_view<definition_category::combat_status>&) const;
    template void definition_source_library::collect_dependency_errors<definition_category::combat_status>(
        const pending_tuple&, const pending_sources_by_name&,
        std::optional<std::size_t>, std::vector<source_add_error>&) const;

    template std::optional<std::size_t> definition_source_library::prepare_add<definition_category::character>(
        pending_tuple&, pending_sources_by_name&, std::vector<source_add_error>&,
        std::size_t, const definition_source_view<definition_category::character>&) const;
    template void definition_source_library::collect_dependency_errors<definition_category::character>(
        const pending_tuple&, const pending_sources_by_name&,
        std::optional<std::size_t>, std::vector<source_add_error>&) const;

    template std::optional<std::size_t> definition_source_library::prepare_add<definition_category::skill>(
        pending_tuple&, pending_sources_by_name&, std::vector<source_add_error>&,
        std::size_t, const definition_source_view<definition_category::skill>&) const;
    template void definition_source_library::collect_dependency_errors<definition_category::skill>(
        const pending_tuple&, const pending_sources_by_name&,
        std::optional<std::size_t>, std::vector<source_add_error>&) const;

    template std::optional<std::size_t> definition_source_library::prepare_add<definition_category::attachment>(
        pending_tuple&, pending_sources_by_name&, std::vector<source_add_error>&,
        std::size_t, const definition_source_view<definition_category::attachment>&) const;
    template void definition_source_library::collect_dependency_errors<definition_category::attachment>(
        const pending_tuple&, const pending_sources_by_name&,
        std::optional<std::size_t>, std::vector<source_add_error>&) const;

    template std::optional<std::size_t> definition_source_library::prepare_add<definition_category::history_summary>(
        pending_tuple&, pending_sources_by_name&, std::vector<source_add_error>&,
        std::size_t, const definition_source_view<definition_category::history_summary>&) const;
    template void definition_source_library::collect_dependency_errors<definition_category::history_summary>(
        const pending_tuple&, const pending_sources_by_name&,
        std::optional<std::size_t>, std::vector<source_add_error>&) const;

    template std::optional<std::size_t> definition_source_library::prepare_add<definition_category::reaction>(
        pending_tuple&, pending_sources_by_name&, std::vector<source_add_error>&,
        std::size_t, const definition_source_view<definition_category::reaction>&) const;
    template void definition_source_library::collect_dependency_errors<definition_category::reaction>(
        const pending_tuple&, const pending_sources_by_name&,
        std::optional<std::size_t>, std::vector<source_add_error>&) const;

}
