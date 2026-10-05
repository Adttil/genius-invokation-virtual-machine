#ifndef GIVM_EXECUTOR_LIBRARY_HPP
#define GIVM_EXECUTOR_LIBRARY_HPP

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <expected>
#include <limits>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "instruction.hpp"
#include "program_input_error.hpp"
#include "history_access_error.hpp"
#include "../definition_common.hpp"
#include "../enums/equipment_type.hpp"
#include "../utils/stack.hpp"

#include "../macro_define.hpp"

namespace givm::detail
{
    struct basic_definition_ids
    {
        definition_id<combat_status_view> dendro_core;
        definition_id<combat_status_view> catalyzing_field;
        definition_id<summon_view> burning_flame;
        definition_id<attachment_view> frozen;
        definition_id<combat_status_view> shield;
    };

    template<class TView>
    struct definition_handle_vectors
    {
        template<class... TEvents>
        using tuple_for = std::tuple<std::vector<handle_fn_t<TView, TEvents>>...>;

        using type = subscribed_events<TView>::template apply<tuple_for>;
    };

    template<class... TViews>
    using definition_handle_groups = std::tuple<typename definition_handle_vectors<TViews>::type...>;

    template<class... TQueries>
    using definition_query_vectors = std::tuple<std::vector<std::conditional_t<
        std::is_empty_v<TQueries>, typename TQueries::result_t, query_fn_t<TQueries>>>...>;

    template<class TCategory>
    struct definition_bucket
    {
        std::vector<std::string_view> names;
        std::vector<definition_data> data;
        std::vector<tag_mask> tags;
        views_of_definition<TCategory>::template apply<definition_handle_groups> handle_fns;
#ifdef _MSC_VER
        [[msvc::no_unique_address]]
#else
        [[no_unique_address]]
#endif
        typename supported_queries<TCategory>::template apply<definition_query_vectors> queries;
    };

    template<class... TCategories>
    using definition_bucket_tuple = std::tuple<definition_bucket<TCategories>...>;

    template<class TEvent>
    struct history_handler
    {
        std::size_t index;
        history_handle_fn_t<TEvent> function;
    };

    template<class... TEvents>
    using history_handler_lists = std::tuple<std::vector<history_handler<TEvents>>...>;

    using definition_history_handlers = subscribed_events<history_summary_definition>::apply<history_handler_lists>;

    struct basic_definition_names;

    template<class TCategory>
    struct compile_definition;

    template<class... TCategories>
    using compile_definition_vectors = std::tuple<std::vector<compile_definition<TCategories>>...>;

    using compile_definitions = definition_types::apply<compile_definition_vectors>;

    struct compiled_history_field
    {
        std::string name;
        dynamic_history_field field;
    };

    struct compiled_history_summary
    {
        std::size_t offset{};
        std::size_t size{};
        std::vector<compiled_history_field> fields;

        const dynamic_history_field* find(std::string_view name) const noexcept
        {
            const auto found = std::ranges::find(fields, name, &compiled_history_field::name);
            return found == fields.end() ? nullptr : &found->field;
        }
    };

    struct response_return
    {
        player_id previous_player;
        execution_position position;
        std::uint32_t result = return_response::null;
#ifndef NDEBUG
        bool previous_inline = false;
#endif
    };

    template<class T>
    concept command_sequence = std::ranges::input_range<T>
        || requires { std::tuple_size<std::remove_cvref_t<T>>::value; };

    template<class TCommand>
    inline any_command make_any_command(TCommand&& command)
    {
        if constexpr(std::constructible_from<any_command, TCommand>)
            return any_command{ std::forward<TCommand>(command) };
        else
            return std::visit([](auto&& value) -> any_command
            {
                return any_command{ std::forward<decltype(value)>(value) };
            }, std::forward<TCommand>(command));
    }

    template<command_sequence TSequence>
    inline auto make_command_sequence(TSequence&& commands)
    {
        if constexpr(std::convertible_to<TSequence, std::span<const any_command>>)
            return std::span<const any_command>{ commands };
        else if constexpr(std::ranges::input_range<TSequence>)
        {
            std::vector<any_command> result;
            if constexpr(std::ranges::sized_range<TSequence>) result.reserve(std::ranges::size(commands));
            for(auto&& command : commands)
                result.push_back(make_any_command(std::forward<decltype(command)>(command)));
            return result;
        }
        else
            return [&]<std::size_t... I>(std::index_sequence<I...>)
            {
                using std::get;
                return std::array<any_command, sizeof...(I)>{ make_any_command(get<I>(std::forward<TSequence>(commands)))... };
            }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<TSequence>>>{});
    }

}

namespace givm
{
    class definition_source_library;
    struct basic_definition_sources;
    struct compile_error;
    struct definition_compile_result;

    class definition_library
    {
        friend class executor;
        friend class detail::execution_context;

    public:
        definition_library(const definition_library& other)
        : program_{ other.program_ },
#ifndef NDEBUG
          input_markers_{ other.input_markers_ }, debug_programs_{ other.debug_programs_ }, debug_library_identity_{ other.debug_library_identity_ },
#endif
          tag_names_{ other.tag_names_ },
          equipment_tags_{ other.equipment_tags_ }, skill_tags_{ other.skill_tags_ }, control_tag_{ other.control_tag_ },
          control_immunity_tag_{ other.control_immunity_tag_ },
          remove_at_zero_usages_tag_{ other.remove_at_zero_usages_tag_ }, basic_ids_{ other.basic_ids_ },
          buckets_{ other.buckets_ },
          history_layouts_{ other.history_layouts_ }, history_size_{ other.history_size_ },
          history_handlers_{ other.history_handlers_ }
        {
            detail::finalize_program(program_);
        }

        definition_library(definition_library&&) noexcept = default;

        definition_library& operator=(const definition_library& other)
        {
            if(this != &other)
            {
                auto replacement = other;
                *this = std::move(replacement);
            }
            return *this;
        }

        definition_library& operator=(definition_library&&) noexcept = default;
        ~definition_library() = default;

        template<class T>
        std::size_t definition_count() const noexcept { return bucket_for<T>().data.size(); }
        std::size_t tag_count() const noexcept { return tag_names_.size(); }

        definition_id<combat_status_view> dendro_core_id() const noexcept
        {
            return basic_ids_.dendro_core;
        }

        definition_id<combat_status_view> catalyzing_field_id() const noexcept
        {
            return basic_ids_.catalyzing_field;
        }

        definition_id<summon_view> burning_flame_id() const noexcept
        {
            return basic_ids_.burning_flame;
        }

        definition_id<attachment_view> frozen_id() const noexcept
        {
            return basic_ids_.frozen;
        }

        definition_id<combat_status_view> shield_id() const noexcept
        {
            return basic_ids_.shield;
        }

        template<class TDefinitionType>
        class definition_view
        {
        public:
            definition_id<TDefinitionType> id() const
            {
                return id_;
            }

            std::string_view name() const
            {
                return library_->name(id_);
            }

            givm::equipment_type equipment_type() const noexcept requires std::same_as<TDefinitionType, attachment_view>
            {
                return library_->equipment_type(id_);
            }

            bool has_tag(tag_id tag) const
            {
                return library_->has_tag(id_, tag);
            }

            bool has_all_tags(std::span<const tag_id> tags) const
            {
                return library_->has_all_tags(id_, tags);
            }

            bool has_any_tag(std::span<const tag_id> tags) const
            {
                return library_->has_any_tag(id_, tags);
            }

            bool matches_tags(
                std::span<const tag_id> required_tags,
                std::span<const tag_id> excluded_tags = {}
            ) const
            {
                return library_->matches_tags(id_, required_tags, excluded_tags);
            }

            template<class TEvent, class TView>
            bool can_handle() const noexcept
            {
                return library_->template can_handle<TEvent, TView>(id_);
            }

            template<class TQuery>
            TQuery::result_t query(const TQuery& parameters) const
            {
                return library_->query(id_, parameters);
            }

            template<class TEvent, class TView>
            program_entry handle(
                TEvent& event,
                handle_context<TView>& context,
                std::uint32_t response_index = 0
            ) const
            {
                return library_->template handle<TEvent>(
                    id_, event, context, response_index
                );
            }

        private:
            friend class definition_library;

            definition_view(const definition_library& library, definition_id<TDefinitionType> id)
            : library_{ &library }, id_{ id }
            {}

            const definition_library* library_;
            definition_id<TDefinitionType> id_;
        };

        template<class TDefinitionType>
        definition_view<TDefinitionType> operator[](definition_id<TDefinitionType> id) const
        {
            return { *this, id };
        }

        friend void load_deck(
            table& card_table, const definition_library& library,
            const linked_deck& first_deck, const linked_deck& second_deck
        )
        {
            library.load_deck(card_table, player_id{ 0 }, first_deck);
            library.load_deck(card_table, player_id{ 1 }, second_deck);
        }

    private:
        void load_deck(table& card_table, player_id player, const linked_deck& deck) const
        {
            auto& writable_table = static_cast<detail::unrestricted_table&>(card_table);
            writable_table.load_deck(player, deck);
            for(const auto card : writable_table[player].deck_cards())
            {
                card.state() = (*this)[card.definition_id()].query(card_initial_state{});
            }
            for(const auto character : writable_table[player].characters())
            {
                const auto definition = (*this)[character.definition_id()];
                character.state() = definition.query(character_initial_state{});
                for(std::size_t skill_index = 0; ; ++skill_index)
                {
                    const auto skill = definition.query(character_initial_skill{ skill_index });
                    if(not skill)
                    {
                        break;
                    }
                    character.add(skill, {});
                }
            }
        }

        static constexpr detail::execution_position entry() noexcept
        {
            return detail::entry_position;
        }

    public:
        dynamic_history_value history_field(definition_id<history_summary_definition> id, std::string_view name) const
        {
#ifndef NDEBUG
            if(id.value() >= history_layouts_.size())
                throw history_access_error{ definition_metadata_error{ definition_types::index_of<history_summary_definition>(), id.value(), history_layouts_.size() } };
#endif
            const auto& layout = history_layouts_[id.value()];
            const auto* field = layout.find(name);
#ifndef NDEBUG
            if(not field) throw history_access_error{ history_field_not_found{ std::string{ this->name(id) }, std::string{ name } } };
#endif
            return std::visit([&](auto key) -> dynamic_history_value
            {
                return history_value_key<typename decltype(key)::value_type>{ key.offset() + layout.offset, key.count() };
            }, *field);
        }

        template<class T>
        history_value_key<T> history_field(definition_id<history_summary_definition> id, std::string_view name) const
        {
            const auto field = history_field(id, name);
            const auto* key = std::get_if<history_value_key<T>>(&field);
#ifndef NDEBUG
            if(not key) throw history_access_error{ history_field_type_mismatch{ std::string{ this->name(id) }, std::string{ name },
                detail::history_type_name<T>(), std::visit([](auto value) { return detail::history_type_name<typename decltype(value)::value_type>(); }, field) } };
#endif
            return *key;
        }

        template<class TDefinitionType>
        std::string_view name(definition_id<TDefinitionType> id) const
        {
            return bucket_for<TDefinitionType>().names[id.value()];
        }

        std::string_view tag_name(tag_id id) const
        {
            return tag_names_[id.value()];
        }

        givm::equipment_type equipment_type(definition_id<attachment_view> id) const noexcept
        {
            for(size_t index = 0; index != equipment_tags_.size(); ++index)
            {
                if(equipment_tags_[index] && has_tag(id, equipment_tags_[index]))
                {
                    return static_cast<givm::equipment_type>(index);
                }
            }
            return givm::equipment_type::none;
        }

        givm::skill_flags skill_flags(definition_id<skill_view> id) const noexcept
        {
            givm::skill_flags result;
            constexpr std::array bits{ skill_flag_bits::normal_attack, skill_flag_bits::elemental_skill,
                                       skill_flag_bits::elemental_burst };
            for(size_t index = 0; index != skill_tags_.size(); ++index)
                if(skill_tags_[index] && has_tag(id, skill_tags_[index])) result.set(bits[index]);
            return result;
        }

        bool is_control(definition_id<attachment_view> id) const noexcept
        {
            return control_tag_ && has_tag(id, control_tag_);
        }

        bool remove_at_zero_usages(definition_id<summon_view> id) const noexcept
        {
            return remove_at_zero_usages_tag_ && has_tag(id, remove_at_zero_usages_tag_);
        }

        bool is_controlled(character_view character) const noexcept
        {
            if(not control_tag_) return false;
            for(const auto attachment : character.attachments())
                if(has_tag(attachment.definition_id(), control_tag_)) return true;
            return false;
        }

        bool is_control_immune(character_view character) const noexcept
        {
            if(not control_immunity_tag_) return false;
            for(const auto attachment : character.attachments())
                if(has_tag(attachment.definition_id(), control_immunity_tag_)) return true;
            return false;
        }

        template<class TDefinitionType>
        bool has_tag(definition_id<TDefinitionType> id, tag_id tag) const
        {
            return bucket_for<TDefinitionType>().tags[id.value()].has(tag);
        }

        template<class TDefinitionType>
        bool has_all_tags(definition_id<TDefinitionType> id, std::span<const tag_id> tags) const
        {
            return bucket_for<TDefinitionType>().tags[id.value()].has_all(tags);
        }

        template<class TDefinitionType>
        bool has_any_tag(definition_id<TDefinitionType> id, std::span<const tag_id> tags) const
        {
            return bucket_for<TDefinitionType>().tags[id.value()].has_any(tags);
        }

        template<class TDefinitionType>
        bool matches_tags(
            definition_id<TDefinitionType> id,
            std::span<const tag_id> required_tags,
            std::span<const tag_id> excluded_tags = {}
        ) const
        {
            return bucket_for<TDefinitionType>().tags[id.value()].matches(required_tags, excluded_tags);
        }

        template<class TEvent, class TView, class TDefinitionType>
        bool can_handle(definition_id<TDefinitionType> id) const noexcept
        {
            return get_handle_fn<TEvent, TView>(id) != nullptr;
        }

        template<class TDefinitionType, class TQuery>
        TQuery::result_t query(definition_id<TDefinitionType> id, const TQuery& parameters) const
        {
            const auto& bucket = bucket_for<TDefinitionType>();
            const auto& queries =
                std::get<supported_queries<TDefinitionType>::template index_of<TQuery>()>(bucket.queries);
            if constexpr(std::is_empty_v<TQuery>)
            {
                return queries[id.value()];
            }
            else
            {
                return queries[id.value()](bucket.data[id.value()], parameters);
            }
        }

        template<class TEvent, class TDefinitionType, class TView>
        program_entry handle(
            definition_id<TDefinitionType> id,
            TEvent& event,
            handle_context<TView>& context,
            std::uint32_t response_index = 0
        ) const
        {
            const auto& bucket = bucket_for<TDefinitionType>();
            const size_t index = id.value();
            const auto handle_fn = get_handle_fn<TEvent, TView>(id);
            return handle_fn(bucket.data[index], event, context, response_index);
        }

    private:
        using definition_type_list = definition_types;

        definition_library(const issued_id_map& id_map, const detail::basic_definition_names& basics);

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

        void initialize_history(table& card_table) const
        {
            auto& writable = static_cast<detail::unrestricted_table&>(card_table);
            writable.reset_history(history_size_);
            record_history(history_summary_initialization{}, writable);
        }

    public:
        template<class TEvent>
        void record_history(const TEvent& event, detail::unrestricted_table& card_table) const
        {
            if constexpr(requires { subscribed_events<history_summary_definition>::template index_of<TEvent>(); })
            {
                const auto& handlers = std::get<subscribed_events<history_summary_definition>::template index_of<TEvent>()>(
                    history_handlers_);
                const auto& definitions = bucket_for<history_summary_definition>().data;
                for(const auto& handler : handlers)
                {
                    const auto& layout = history_layouts_[handler.index];
                    handler.function(definitions[handler.index], card_table.history_summary(layout.offset, layout.size),
                        event, static_cast<const table&>(card_table), *this);
                }
            }
        }

    private:
        template<class TEvent, class TView, class TDefinitionType>
        handle_fn_t<TView, TEvent> get_handle_fn(definition_id<TDefinitionType> id) const noexcept
        {
            const auto& bucket = bucket_for<TDefinitionType>();
            return std::get<subscribed_events<TView>::template index_of<TEvent>()>(
                std::get<views_of_definition<TDefinitionType>::template index_of<TView>()>(bucket.handle_fns)
            )[id.value()];
        }

        template<class TDefinitionType>
        void compile_source(
            const detail::compile_definition<TDefinitionType>& definition,
            const issued_id_map& id_map,
            const detail::compile_definitions& definitions,
            compile_mode mode,
            std::vector<compile_error>& errors
        );

        void prepare_history_layouts(const detail::compile_definitions& definitions, const issued_id_map& ids, compile_mode mode,
            std::vector<compile_error>& errors);

        static tag_mask make_tag_mask(
            const std::vector<std::string_view>& tags,
            const issued_id_map& id_map
        );

        detail::compile_definitions prepare_definitions(const definition_source_library& sources, const issued_id_map& ids);

        void complete_dynamic_queries();

    public:
        static std::expected<definition_compile_result, std::vector<compile_error>> compile(
            const definition_source_library& sources, const basic_definition_sources& basics,
            std::span<const any_command> initialization_program, std::span<const any_command> round_program, compile_mode mode);

        static std::expected<definition_compile_result, std::vector<compile_error>> compile(
            const definition_source_library& sources, const basic_definition_sources& basics,
            const std::array<std::span<const std::string_view>, definition_types::size()>& selection,
            std::span<const any_command> initialization_program, std::span<const any_command> round_program, compile_mode mode);

        template<detail::command_sequence TInitializationSequence, detail::command_sequence TRoundSequence>
        requires (not std::convertible_to<TInitializationSequence, std::span<const any_command>>
            || not std::convertible_to<TRoundSequence, std::span<const any_command>>)
        static auto compile(const definition_source_library& sources, const basic_definition_sources& basics,
            TInitializationSequence&& initialization_program, TRoundSequence&& round_program, compile_mode mode)
        {
            const auto initialization = detail::make_command_sequence(std::forward<TInitializationSequence>(initialization_program));
            const auto round = detail::make_command_sequence(std::forward<TRoundSequence>(round_program));
            return compile(sources, basics, std::span{ initialization }, std::span{ round }, mode);
        }

        template<detail::command_sequence TInitializationSequence, detail::command_sequence TRoundSequence>
        requires (not std::convertible_to<TInitializationSequence, std::span<const any_command>>
            || not std::convertible_to<TRoundSequence, std::span<const any_command>>)
        static auto compile(const definition_source_library& sources, const basic_definition_sources& basics,
            const std::array<std::span<const std::string_view>, definition_types::size()>& selection,
            TInitializationSequence&& initialization_program,
            TRoundSequence&& round_program, compile_mode mode)
        {
            const auto initialization = detail::make_command_sequence(std::forward<TInitializationSequence>(initialization_program));
            const auto round = detail::make_command_sequence(std::forward<TRoundSequence>(round_program));
            return compile(sources, basics, selection, std::span{ initialization }, std::span{ round }, mode);
        }

    private:
        static std::expected<definition_compile_result, std::vector<compile_error>> compile_prepared(
            const definition_source_library& sources, const detail::basic_definition_names& basics, issued_id_map id_map,
            std::span<const any_command> initialization_program, std::span<const any_command> round_program,
            compile_mode mode, std::vector<source_preparation_error> preparation_errors);

    private:
        detail::program_bytes program_;
#ifndef NDEBUG
        std::vector<detail::debug_input_requirement> input_markers_;
        std::vector<detail::debug_program_info> debug_programs_;
        std::size_t debug_library_identity_ = detail::next_program_library_identity.fetch_add(1, std::memory_order_relaxed);
#endif
        std::vector<std::string_view> tag_names_;
        std::array<tag_id, static_cast<size_t>(givm::equipment_type::none)> equipment_tags_{};
        std::array<tag_id, 3> skill_tags_{};
        tag_id control_tag_{};
        tag_id control_immunity_tag_{};
        tag_id remove_at_zero_usages_tag_{};
        detail::basic_definition_ids basic_ids_;
        definition_types::apply<detail::definition_bucket_tuple> buckets_;
        std::vector<detail::compiled_history_summary> history_layouts_;
        std::size_t history_size_{};
        detail::definition_history_handlers history_handlers_;
    };


}

#include "../macro_undef.hpp"
#endif
