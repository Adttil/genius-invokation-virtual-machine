#ifndef GIVM_EXECUTOR_COMMAND_INPUT_ERROR_HPP
#define GIVM_EXECUTOR_COMMAND_INPUT_ERROR_HPP

#include <cstddef>
#include <cstdint>
#include <exception>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

#include "../definition.hpp"
#include "../table.hpp"
#include "../enums/equipment_type.hpp"

namespace givm
{
    using command_entity_id = std::variant<player_id, character_id, skill_id, attachment_id,
        hand_card_id, deck_card_id, hand_card_status_id, deck_card_status_id,
        support_id, summon_id, combat_status_id>;

    struct invalid_entity_argument
    {
        enum class reason { out_of_range, removed };
        std::string field;
        command_entity_id entity;
        reason cause;
    };

    struct invalid_definition_argument
    {
        std::string field;
        std::size_t category_index;
        std::size_t value;
        std::size_t count;
    };

    struct invalid_enum_argument
    {
        std::string field;
        std::size_t value;
    };

    struct duplicate_entity_argument
    {
        std::string field;
        std::size_t first_index;
        std::size_t index;
    };

    struct missing_entity_argument
    {
        std::string field;
        std::optional<command_entity_id> owner{};
        std::optional<std::size_t> definition{};
        std::optional<equipment_type> equipment{};
    };

    struct invalid_numeric_argument
    {
        enum class constraint_kind { at_most, less_than, nonzero };
        std::string field;
        std::uint64_t value;
        std::uint64_t limit;
        constraint_kind constraint = constraint_kind::at_most;
    };

    struct invalid_entity_relation
    {
        enum class reason { inactive_character, defeated_character, same_character };
        std::string field;
        reason cause;
    };

    struct insufficient_dice_argument
    {
        player_id player;
        dice_counts requested;
        dice_counts available;
    };

    using command_input_error_reason = std::variant<invalid_entity_argument, invalid_definition_argument,
        invalid_enum_argument, duplicate_entity_argument, missing_entity_argument, invalid_numeric_argument,
        invalid_entity_relation, insufficient_dice_argument>;
}

namespace givm::detail
{
    inline std::string command_entity_string(const command_entity_id& entity)
    {
        return std::visit([](const auto& id)
        {
            using id_type = std::remove_cvref_t<decltype(id)>;
            std::string name;
            if constexpr(std::is_same_v<id_type, player_id>) name = "player";
            else if constexpr(std::is_same_v<id_type, character_id>) name = "character";
            else if constexpr(std::is_same_v<id_type, skill_id>) name = "skill";
            else if constexpr(std::is_same_v<id_type, attachment_id>) name = "attachment";
            else if constexpr(std::is_same_v<id_type, hand_card_id>) name = "hand card";
            else if constexpr(std::is_same_v<id_type, deck_card_id>) name = "deck card";
            else if constexpr(std::is_same_v<id_type, hand_card_status_id>) name = "hand card status";
            else if constexpr(std::is_same_v<id_type, deck_card_status_id>) name = "deck card status";
            else if constexpr(std::is_same_v<id_type, support_id>) name = "support";
            else if constexpr(std::is_same_v<id_type, summon_id>) name = "summon";
            else name = "combat status";
            name += "{";
            if constexpr(requires { id.character_id; })
                name += "player=" + std::to_string(id.character_id.player_id.index)
                    + ", character=" + std::to_string(id.character_id.index) + ", ";
            else if constexpr(requires { id.card_id; })
                name += "player=" + std::to_string(id.card_id.player_id.index)
                    + ", card=" + std::to_string(id.card_id.index) + ", ";
            else if constexpr(requires { id.player_id; })
                name += "player=" + std::to_string(id.player_id.index) + ", ";
            return name + "index=" + std::to_string(id.index) + "}";
        }, entity);
    }
}

namespace givm
{
    inline std::string error_string(const command_input_error_reason& error)
    {
        return std::visit([](const auto& reason) -> std::string
        {
            using reason_type = std::remove_cvref_t<decltype(reason)>;
            if constexpr(std::is_same_v<reason_type, invalid_entity_argument>)
                return reason.field + ": " + detail::command_entity_string(reason.entity)
                    + (reason.cause == reason_type::reason::out_of_range ? " is out of range" : " has been removed");
            else if constexpr(std::is_same_v<reason_type, invalid_definition_argument>)
                return reason.field + ": " + detail::source_definition_name_text({ reason.category_index, std::to_string(reason.value) })
                    + " is outside [0, " + std::to_string(reason.count) + ")";
            else if constexpr(std::is_same_v<reason_type, invalid_enum_argument>)
                return reason.field + ": invalid enum value " + std::to_string(reason.value);
            else if constexpr(std::is_same_v<reason_type, duplicate_entity_argument>)
                return reason.field + "[" + std::to_string(reason.index) + "] duplicates element " + std::to_string(reason.first_index);
            else if constexpr(std::is_same_v<reason_type, missing_entity_argument>)
            {
                auto message = reason.field + ": required entity is missing";
                if(reason.owner) message += "; owner=" + detail::command_entity_string(*reason.owner);
                if(reason.definition) message += "; definition=" + std::to_string(*reason.definition);
                if(reason.equipment) message += "; equipment=" + std::to_string(static_cast<std::size_t>(*reason.equipment));
                return message;
            }
            else if constexpr(std::is_same_v<reason_type, invalid_numeric_argument>)
                return reason.field + (reason.constraint == reason_type::constraint_kind::nonzero
                    ? ": must be nonzero; got " + std::to_string(reason.value)
                    : (reason.constraint == reason_type::constraint_kind::less_than ? ": must be less than " : ": must be at most ")
                        + std::to_string(reason.limit) + "; got " + std::to_string(reason.value));
            else if constexpr(std::is_same_v<reason_type, invalid_entity_relation>)
            {
                switch(reason.cause)
                {
                case reason_type::reason::inactive_character: return reason.field + ": character is not active";
                case reason_type::reason::defeated_character: return reason.field + ": character is defeated";
                case reason_type::reason::same_character: return reason.field + ": source and target character are the same";
                }
                return reason.field;
            }
            else
            {
                std::string message = "player[" + std::to_string(reason.player.index) + "]: insufficient dice; requested/available {";
                constexpr std::string_view names[]{ "cryo", "hydro", "pyro", "electro", "geo", "dendro", "anemo", "omni" };
                for(std::size_t index = 0; index != std::size(names); ++index)
                {
                    if(index != 0) message += ", ";
                    const auto dice = static_cast<elemental_dice>(index);
                    message += std::string{ names[index] } + "=" + std::to_string(reason.requested[dice])
                        + "/" + std::to_string(reason.available[dice]);
                }
                return message + "}";
            }
        }, error);
    }

    class command_input_error : public std::exception
    {
    public:
        const std::string command;
        const command_input_error_reason reason;

        command_input_error(std::string_view command, command_input_error_reason reason)
        : command{ command }, reason{ std::move(reason) }, message_{ this->command + ": " + error_string(this->reason) }
        {}

        const char* what() const noexcept override { return message_.c_str(); }

    private:
        std::string message_;
    };

    inline std::string error_string(const command_input_error& error) { return error.what(); }
}

#endif
