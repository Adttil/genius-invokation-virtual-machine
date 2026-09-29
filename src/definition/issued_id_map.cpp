#include <givm/definition/issued_id_map.hpp>

namespace givm
{
    template<class TDefinition>
    std::vector<definition_id<TDefinition>> issued_id_map::query_by_tag(std::string_view expression) const
    {
        const auto terms = expression
            | std::views::split('&')
            | std::ranges::to<std::vector>();

        std::vector<tag_id> tags(terms.size());
        size_t required_count = 0;
        size_t excluded_begin = tags.size();

        for(const auto& range : terms)
        {
            const std::string_view term{ range };
            const size_t tag_end = term.find_last_not_of(whitespace) + 1;
            const size_t split = term.find_last_of(negation_or_whitespace, tag_end - 1);
            const bool excluded = split != std::string_view::npos
                && (term[split] == '!' || term.find_last_not_of(whitespace, split) != std::string_view::npos);
            const auto name = split == std::string_view::npos
                ? term.substr(0, tag_end) : term.substr(split + 1, tag_end - split - 1);
            const auto found = tag_to_id_.find(name);
            if(found == tag_to_id_.end())
            {
                if(not excluded) return {};
                continue;
            }

            const tag_id tag{ found->second };
            if(excluded)
            {
                tags[--excluded_begin] = tag;
            }
            else
            {
                tags[required_count++] = tag;
            }
        }

        return query_by_tags<TDefinition>(
            std::span{ tags.data(), required_count },
            std::span{ tags.data() + excluded_begin, tags.size() - excluded_begin }
        );
    }

    template std::vector<definition_id<card_definition>> issued_id_map::query_by_tag<card_definition>(std::string_view) const;
    template std::vector<definition_id<status_definition>> issued_id_map::query_by_tag<status_definition>(std::string_view) const;
    template std::vector<definition_id<support_view>> issued_id_map::query_by_tag<support_view>(std::string_view) const;
    template std::vector<definition_id<summon_view>> issued_id_map::query_by_tag<summon_view>(std::string_view) const;
    template std::vector<definition_id<combat_status_view>> issued_id_map::query_by_tag<combat_status_view>(std::string_view) const;
    template std::vector<definition_id<character_view>> issued_id_map::query_by_tag<character_view>(std::string_view) const;
    template std::vector<definition_id<skill_view>> issued_id_map::query_by_tag<skill_view>(std::string_view) const;
    template std::vector<definition_id<attachment_view>> issued_id_map::query_by_tag<attachment_view>(std::string_view) const;
    template std::vector<definition_id<history_summary_definition>> issued_id_map::query_by_tag<history_summary_definition>(std::string_view) const;
}
