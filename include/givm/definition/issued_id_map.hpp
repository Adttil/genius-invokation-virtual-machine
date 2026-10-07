#ifndef GIVM_DEFINITION_ISSUED_ID_MAP_HPP
#define GIVM_DEFINITION_ISSUED_ID_MAP_HPP

#include <array>
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <ranges>
#include <span>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "../table.hpp"
#include "definition_categories.hpp"

namespace givm
{
    class issued_id_map
    {
    public:
        template<definition_category TDefinition>
        using id_type = definition_id<TDefinition>;

        explicit issued_id_map(std::initializer_list<std::string_view> tags)
        {
            for(std::string_view tag : tags)
            {
                add_tag(tag);
            }
        }

        template<std::ranges::input_range TTags>
            requires std::convertible_to<std::ranges::range_reference_t<TTags>, std::string_view>
        explicit issued_id_map(TTags&& tags)
        {
            for(auto&& tag : tags)
            {
                add_tag(std::string_view{ tag });
            }
        }

        template<definition_category TDefinition>
        definition_id<TDefinition> add(std::string_view name, std::initializer_list<std::string_view> tags)
        {
            return add<TDefinition>(name, std::span<const std::string_view>{ tags.begin(), tags.size() });
        }

        template<definition_category TDefinition, std::ranges::input_range TTags>
            requires std::convertible_to<std::ranges::range_reference_t<TTags>, std::string_view>
        definition_id<TDefinition> add(std::string_view name, TTags&& tags)
        {
            constexpr size_t index = static_cast<std::size_t>(TDefinition);
            auto& tags_for_entity = tag_masks_[index];
            const size_t value = tags_for_entity.size();
            auto& mask = tags_for_entity.emplace_back(tag_to_id_.size());
            for(auto&& tag : tags)
            {
                mask[get_tag_id(std::string_view{ tag }).value()] = true;
            }
            name_to_ids_[index].emplace(name, value);
            return definition_id<TDefinition>{ value };
        }

        template<definition_category TDefinition>
        bool has(std::string_view name) const
        {
            return name_to_ids_[static_cast<std::size_t>(TDefinition)].contains(name);
        }

        template<definition_category TDefinition>
        size_t definition_count() const noexcept
        {
            return tag_masks_[static_cast<std::size_t>(TDefinition)].size();
        }

        bool has_tag(std::string_view name) const
        {
            return tag_to_id_.contains(name);
        }

        template<definition_category TDefinition>
        definition_id<TDefinition> get_id(std::string_view name) const
        {
            return definition_id<TDefinition>{ name_to_ids_[static_cast<std::size_t>(TDefinition)].find(name)->second };
        }

        tag_id get_tag_id(std::string_view name) const
        {
            return tag_id{ tag_to_id_.find(name)->second };
        }

        std::string_view tag_name(tag_id id) const
        {
            return tag_names_[id.value()];
        }

        std::span<const std::string_view> tag_names() const
        {
            return tag_names_;
        }

        template<definition_category TDefinition>
        std::vector<definition_id<TDefinition>> query_by_tag(std::string_view expression) const;

    private:
        using name_map_type = std::unordered_map<std::string_view, size_t>;
        using tag_mask = std::vector<bool>;
        static constexpr std::string_view whitespace = " \t\n\r\f\v";
        static constexpr std::string_view negation_or_whitespace = "! \t\n\r\f\v";

        void add_tag(std::string_view tag)
        {
            const auto [_, inserted] = tag_to_id_.try_emplace(tag, tag_names_.size());
            if(inserted)
            {
                tag_names_.push_back(tag);
            }
        }

        static bool match_tags(
            const tag_mask& tags,
            std::span<const tag_id> required_tags,
            std::span<const tag_id> excluded_tags
        )
        {
            for(tag_id tag : required_tags)
            {
                if(not tags[tag.value()])
                {
                    return false;
                }
            }

            for(tag_id tag : excluded_tags)
            {
                if(tags[tag.value()])
                {
                    return false;
                }
            }

            return true;
        }

        template<definition_category TDefinition>
        std::vector<definition_id<TDefinition>> query_by_tags(
            std::span<const tag_id> required_tags,
            std::span<const tag_id> excluded_tags = {}
        ) const
        {
            constexpr size_t index = static_cast<std::size_t>(TDefinition);
            const auto& tags = tag_masks_[index];
            std::vector<definition_id<TDefinition>> result;
            for(size_t value = 0; value < tags.size(); ++value)
            {
                if(match_tags(tags[value], required_tags, excluded_tags))
                {
                    result.push_back(definition_id<TDefinition>{ value });
                }
            }
            return result;
        }

        std::array<std::vector<tag_mask>, detail::definition_categories.size()> tag_masks_;
        std::array<name_map_type, detail::definition_categories.size()> name_to_ids_;
        name_map_type tag_to_id_;
        std::vector<std::string_view> tag_names_;
    };
}

#endif
