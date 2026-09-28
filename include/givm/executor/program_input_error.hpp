#ifndef GIVM_EXECUTOR_PROGRAM_INPUT_ERROR_HPP
#define GIVM_EXECUTOR_PROGRAM_INPUT_ERROR_HPP

#include <exception>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#ifndef NDEBUG
#include <array>
#include <atomic>
#include <span>
#endif

#include "../definition.hpp"

namespace givm
{
    struct program_input_count_mismatch { std::size_t expected; std::size_t actual; };
    struct program_input_type_mismatch
    {
        std::size_t input_index;
        std::size_t command_index;
        std::string command;
        std::string expected;
        std::string actual;
    };
    enum class invalid_program_entry { null_entry, different_library, unknown_entry };
    struct program_invocation_mode_mismatch { bool expected_substack; bool actual_substack; };
    struct repeated_program_invocation {};
    using program_input_error_reason = std::variant<program_input_count_mismatch, program_input_type_mismatch,
        invalid_program_entry, program_invocation_mode_mismatch, repeated_program_invocation>;

    inline std::string error_string(const program_input_count_mismatch& error)
    {
        return "input count mismatch: expected " + std::to_string(error.expected) + ", actual " + std::to_string(error.actual);
    }
    inline std::string error_string(const program_input_type_mismatch& error)
    {
        return "input[" + std::to_string(error.input_index) + "] for command[" + std::to_string(error.command_index)
            + "] " + error.command + ": expected " + error.expected + ", actual " + error.actual;
    }
    inline std::string error_string(invalid_program_entry error)
    {
        switch(error)
        {
        case invalid_program_entry::null_entry: return "invoke requires a non-null program entry";
        case invalid_program_entry::different_library: return "program entry belongs to a different definition library";
        case invalid_program_entry::unknown_entry: return "program entry has no matching input description";
        }
        return {};
    }
    inline std::string error_string(const program_invocation_mode_mismatch& error)
    {
        return std::string{ "invocation mode mismatch: expected " } + (error.expected_substack ? "invoke(substack_t{}, ...)" : "invoke(...)")
            + ", actual " + (error.actual_substack ? "invoke(substack_t{}, ...)" : "invoke(...)");
    }
    inline std::string error_string(repeated_program_invocation)
    {
        return "a response may invoke a program only once";
    }

    class program_input_error : public std::exception
    {
    public:
        const std::optional<definition_name> source;
        const std::optional<std::size_t> program_index;
        const program_input_error_reason reason;

        program_input_error(program_input_error_reason cause, std::optional<definition_name> definition = {}, std::optional<std::size_t> program = {})
        : source{ std::move(definition) }, program_index{ program }, reason{ std::move(cause) }, message_{ format() }
        {}

        const char* what() const noexcept override { return message_.c_str(); }

    private:
        std::string format() const
        {
            std::string result;
            if(source) result = detail::source_definition_name_text(*source) + "; ";
            if(program_index) result += "response program[" + std::to_string(*program_index) + "]: ";
            result += std::visit([](const auto& error) { return error_string(error); }, reason);
            return result;
        }
        std::string message_;
    };

    inline std::string error_string(const program_input_error& error) { return error.what(); }
}

#ifndef NDEBUG
namespace givm::detail
{
    template<class T> inline constexpr std::string_view debug_command_name;
    template<> inline constexpr std::string_view debug_command_name<insert_deck_card> = "insert_deck_card";
    template<> inline constexpr std::string_view debug_command_name<enter_character> = "enter_character";
    template<> inline constexpr std::string_view debug_command_name<shuffle_deck> = "shuffle_deck";
    template<> inline constexpr std::string_view debug_command_name<set_active_character> = "set_active_character";
    template<> inline constexpr std::string_view debug_command_name<select_active_character_both> = "select_active_character_both";
    template<> inline constexpr std::string_view debug_command_name<draw_cards> = "draw_cards";
    template<> inline constexpr std::string_view debug_command_name<create_hand_card> = "create_hand_card";
    template<> inline constexpr std::string_view debug_command_name<discard_hand_card> = "discard_hand_card";
    template<> inline constexpr std::string_view debug_command_name<discard_deck_cards> = "discard_deck_cards";
    template<> inline constexpr std::string_view debug_command_name<add_support> = "add_support";
    template<> inline constexpr std::string_view debug_command_name<set_support_state> = "set_support_state";
    template<> inline constexpr std::string_view debug_command_name<modify_support_state> = "modify_support_state";
    template<> inline constexpr std::string_view debug_command_name<remove_support> = "remove_support";
    template<> inline constexpr std::string_view debug_command_name<summon> = "summon";
    template<> inline constexpr std::string_view debug_command_name<add_summon> = "add_summon";
    template<> inline constexpr std::string_view debug_command_name<set_summon_state> = "set_summon_state";
    template<> inline constexpr std::string_view debug_command_name<modify_summon_state> = "modify_summon_state";
    template<> inline constexpr std::string_view debug_command_name<remove_summon> = "remove_summon";
    template<> inline constexpr std::string_view debug_command_name<generate_combat_status> = "generate_combat_status";
    template<> inline constexpr std::string_view debug_command_name<add_combat_status> = "add_combat_status";
    template<> inline constexpr std::string_view debug_command_name<set_combat_status_state> = "set_combat_status_state";
    template<> inline constexpr std::string_view debug_command_name<modify_combat_status_state> = "modify_combat_status_state";
    template<> inline constexpr std::string_view debug_command_name<remove_combat_status> = "remove_combat_status";
    template<> inline constexpr std::string_view debug_command_name<attach> = "attach";
    template<> inline constexpr std::string_view debug_command_name<set_attachment_state> = "set_attachment_state";
    template<> inline constexpr std::string_view debug_command_name<modify_attachment_state> = "modify_attachment_state";
    template<> inline constexpr std::string_view debug_command_name<add_attachment> = "add_attachment";
    template<> inline constexpr std::string_view debug_command_name<transfer_attachment> = "transfer_attachment";
    template<> inline constexpr std::string_view debug_command_name<remove_attachment> = "remove_attachment";
    template<> inline constexpr std::string_view debug_command_name<replace_cards> = "replace_cards";
    template<> inline constexpr std::string_view debug_command_name<replace_cards_both> = "replace_cards_both";
    template<> inline constexpr std::string_view debug_command_name<start_round> = "start_round";
    template<> inline constexpr std::string_view debug_command_name<begin_action> = "begin_action";
    template<> inline constexpr std::string_view debug_command_name<use_skill> = "use_skill";
    template<> inline constexpr std::string_view debug_command_name<set_skill_state> = "set_skill_state";
    template<> inline constexpr std::string_view debug_command_name<set_energy> = "set_energy";
    template<> inline constexpr std::string_view debug_command_name<modify_energy> = "modify_energy";
    template<> inline constexpr std::string_view debug_command_name<end_round> = "end_round";
    template<> inline constexpr std::string_view debug_command_name<end_game> = "end_game";
    template<> inline constexpr std::string_view debug_command_name<start_dice_roll_phase> = "start_dice_roll_phase";
    template<> inline constexpr std::string_view debug_command_name<reroll_dice> = "reroll_dice";
    template<> inline constexpr std::string_view debug_command_name<add_dice> = "add_dice";
    template<> inline constexpr std::string_view debug_command_name<remove_dice> = "remove_dice";
    template<> inline constexpr std::string_view debug_command_name<start_battle> = "start_battle";
    template<> inline constexpr std::string_view debug_command_name<deal_damage> = "deal_damage";
    template<> inline constexpr std::string_view debug_command_name<apply_element> = "apply_element";
    template<> inline constexpr std::string_view debug_command_name<heal> = "heal";
    template<> inline constexpr std::string_view debug_command_name<increase_max_health> = "increase_max_health";

    inline constexpr auto debug_input_command_names = []
    {
        std::array<std::string_view, command_input_types::size()> result{};
        command_types::each([&]<class T>
        {
            if constexpr(requires { typename T::input_type; })
                result[command_input_types::index_of<typename T::input_type>()] = debug_command_name<T>;
        });
        return result;
    }();

    struct debug_input_requirement
    {
        std::size_t marker;
        std::size_t command_index;
        std::string_view command;
    };
    struct debug_program_info
    {
        std::optional<definition_name> source;
        std::size_t program_index;
        std::size_t position;
        std::size_t inputs_begin;
        std::size_t inputs_count;
    };
    struct program_debug_view
    {
        std::size_t library_identity = 0;
        std::span<const debug_program_info> programs;
        std::span<const debug_input_requirement> inputs;
    };
    inline std::atomic_size_t next_program_library_identity{ 1 };
}
#endif

#endif
