#include <givm/definition/issued_id_map.hpp>

namespace givm
{
    template<definition_category TDefinition>
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

    template std::vector<definition_id<definition_category::card>> issued_id_map::query_by_tag<definition_category::card>(std::string_view) const;
    template std::vector<definition_id<definition_category::card_status>> issued_id_map::query_by_tag<definition_category::card_status>(std::string_view) const;
    template std::vector<definition_id<definition_category::support>> issued_id_map::query_by_tag<definition_category::support>(std::string_view) const;
    template std::vector<definition_id<definition_category::summon>> issued_id_map::query_by_tag<definition_category::summon>(std::string_view) const;
    template std::vector<definition_id<definition_category::combat_status>> issued_id_map::query_by_tag<definition_category::combat_status>(std::string_view) const;
    template std::vector<definition_id<definition_category::character>> issued_id_map::query_by_tag<definition_category::character>(std::string_view) const;
    template std::vector<definition_id<definition_category::skill>> issued_id_map::query_by_tag<definition_category::skill>(std::string_view) const;
    template std::vector<definition_id<definition_category::attachment>> issued_id_map::query_by_tag<definition_category::attachment>(std::string_view) const;
    template std::vector<definition_id<definition_category::history_summary>> issued_id_map::query_by_tag<definition_category::history_summary>(std::string_view) const;
    template std::vector<definition_id<definition_category::reaction>> issued_id_map::query_by_tag<definition_category::reaction>(std::string_view) const;
}
