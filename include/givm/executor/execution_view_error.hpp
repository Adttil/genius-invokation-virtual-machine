#ifndef GIVM_EXECUTOR_EXECUTION_VIEW_ERROR_HPP
#define GIVM_EXECUTOR_EXECUTION_VIEW_ERROR_HPP

#include <cstddef>
#include <exception>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "execution_state.hpp"

namespace givm::detail
{
#ifdef NDEBUG
    inline constexpr bool view_checks_disabled = true;
#else
    inline constexpr bool view_checks_disabled = false;
#endif

    constexpr std::string_view execution_state_name(execution_state state) noexcept
    {
        switch(state)
        {
        case execution_state::finished: return "finished";
        case execution_state::card_selection: return "card_selection";
        case execution_state::initial_card_selection: return "initial_card_selection";
        case execution_state::initial_active_character_selection: return "initial_active_character_selection";
        case execution_state::remaining_active_character_selection: return "remaining_active_character_selection";
        case execution_state::dice_selection: return "dice_selection";
        case execution_state::action_selection: return "action_selection";
        case execution_state::health_reduced: return "health_reduced";
        case execution_state::deck_cards_discarded: return "deck_cards_discarded";
        case execution_state::active_character_changed: return "active_character_changed";
        case execution_state::initial_active_characters_selected: return "initial_active_characters_selected";
        case execution_state::round_started: return "round_started";
        case execution_state::action_started: return "action_started";
        case execution_state::round_end_declared: return "round_end_declared";
        case execution_state::round_ending: return "round_ending";
        case execution_state::dice_reroll_selection: return "dice_reroll_selection";
        case execution_state::initialized: return "initialized";
        default: return "unavailable";
        }
    }
}

namespace givm
{
    struct unexpected_execution_state
    {
        execution_state expected;
        std::optional<execution_state> actual;
    };

    struct expired_execution_view
    {
        std::size_t expected_version;
        std::size_t actual_version;
    };

    using execution_view_error_reason = std::variant<unexpected_execution_state, expired_execution_view>;

    inline std::string error_string(const unexpected_execution_state& error)
    {
        return "expected execution state " + std::string{ detail::execution_state_name(error.expected) }
            + ", actual " + (error.actual ? std::string{ detail::execution_state_name(*error.actual) } : "unavailable");
    }

    inline std::string error_string(const expired_execution_view& error)
    {
        return "expired execution view: version " + std::to_string(error.expected_version)
            + ", current version " + std::to_string(error.actual_version);
    }

    inline std::string error_string(const execution_view_error_reason& error)
    {
        return std::visit([](const auto& reason) { return error_string(reason); }, error);
    }

    class execution_view_error : public std::exception
    {
    public:
        const execution_view_error_reason reason;

        explicit execution_view_error(execution_view_error_reason reason)
        : reason{ std::move(reason) }, message_{ error_string(this->reason) }
        {}

        const char* what() const noexcept override { return message_.c_str(); }

    private:
        std::string message_;
    };

    inline std::string error_string(const execution_view_error& error) { return error.what(); }

    struct view_index_out_of_range
    {
        std::string field;
        std::size_t index;
        std::size_t count;
    };

    inline std::string error_string(const view_index_out_of_range& error)
    {
        return error.field + " index " + std::to_string(error.index)
            + " is outside [0, " + std::to_string(error.count) + ")";
    }

    template<class Reason>
    class view_input_error : public std::exception
    {
    public:
        const std::string operation;
        const Reason reason;

        view_input_error(std::string_view operation, Reason reason)
        : operation{ operation }, reason{ std::move(reason) }, message_{ this->operation + ": " + error_string(this->reason) }
        {}

        const char* what() const noexcept override { return message_.c_str(); }

    private:
        std::string message_;
    };

    template<class Reason>
    inline std::string error_string(const view_input_error<Reason>& error) { return error.what(); }
}

#endif
