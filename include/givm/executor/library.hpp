#ifndef GIVM_EXECUTOR_LIBRARY_HPP
#define GIVM_EXECUTOR_LIBRARY_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <ranges>
#include <stdexcept>
#include <span>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "instruction.hpp"
#include "../definition/program_entry.hpp"
#include "../definition.hpp"
#include "../enums/equipment_type.hpp"
#include "../utils/stack.hpp"

#include "../macro_define.hpp"

namespace givm::detail
{
    template<class TExecutionContext>
    execution_state execute_return(
        const definition_library&, unrestricted_table&, TExecutionContext& context, random_fn&)
    {
        return context.return_from_subroutine();
    }
    template<class TExecutionContext>
    execution_state execute_jump(
        const definition_library& library, unrestricted_table&, TExecutionContext& context, random_fn&)
    {
        return context.jump(context.template instruction_data<1, execution_position>(library));
    }

    template<class TSequence>
    std::size_t append_commands(program_writer& writer, TSequence&& commands, compile_mode mode)
    {
        std::size_t inputs_size = 0;
        const auto append_command = [&](const auto& command)
        {
#ifndef NDEBUG
            using input_type = typename std::remove_cvref_t<decltype(command)>::input_type;
            if constexpr(not std::is_void_v<input_type>)
            {
                inputs_size += (sizeof(input_type) + max_alignment - 1) / max_alignment * max_alignment;
            }
#endif
            compile(writer, command, mode);
        };
        const auto append = [&]<class TCommand>(TCommand&& command)
        {
            if constexpr(requires { std::variant_size<std::remove_cvref_t<TCommand>>::value; })
            {
                std::visit(append_command, command);
            }
            else
            {
                append_command(command);
            }
        };

        if constexpr(std::ranges::range<TSequence>)
        {
            for(auto&& command : commands)
            {
                append(std::forward<decltype(command)>(command));
            }
        }
        else
        {
            [&]<std::size_t... I>(std::index_sequence<I...>)
            {
                using std::get;
                (append(get<I>(std::forward<TSequence>(commands))), ...);
            }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<TSequence>>>{});
        }
        return inputs_size;
    }
}

namespace givm
{
    class definition_compile_context
    {
    public:
        template<class TCategory>
        definition_id<TCategory> resolve_id(std::string_view name) const
        {
            const auto& declared = declarations_.dependencies[definition_types::index_of<TCategory>()];
            if(not contains(declared, name) || not id_map_.has<TCategory>(name))
            {
                throw std::invalid_argument{ "undeclared definition dependency" };
            }
            return id_map_.get_id<TCategory>(name);
        }

        tag_id resolve_tag(std::string_view name) const
        {
            if(not contains(declarations_.tag_dependencies, name) || not id_map_.has_tag(name))
            {
                throw std::invalid_argument{ "undeclared tag dependency" };
            }
            return id_map_.get_tag_id(name);
        }

        template<class TCategory>
        std::vector<definition_id<TCategory>> resolve_ids_by_tag(std::string_view filter) const
        {
            const auto& declared =
                declarations_.dependencies_by_tag[definition_types::index_of<TCategory>()];
            if(not contains(declared, filter))
            {
                throw std::invalid_argument{ "undeclared definition tag dependency" };
            }
            return id_map_.query_by_tag<TCategory>(filter);
        }

        template<class TCommands>
        program_entry add_program(TCommands&& commands)
        {
            program_entry result{ program_.size() };
            detail::program_writer writer{ program_ };
            [[maybe_unused]] const auto inputs_size =
                detail::append_commands(writer, std::forward<TCommands>(commands), mode_);
#ifndef NDEBUG
            result.inputs_size_ = inputs_size;
#endif
            writer.write(detail::execute_fn{ detail::execute_return });
            return result;
        }

    private:
        definition_compile_context(
            const issued_id_map& id_map,
            detail::program_bytes& program,
            const detail::definition_source_declarations& declarations,
            compile_mode mode
        )
        : id_map_{ id_map }, program_{ program }, declarations_{ declarations }, mode_{ mode }
        {}

        static bool contains(const std::vector<std::string_view>& values, std::string_view value)
        {
            return std::ranges::find(values, value) != values.end();
        }

        const issued_id_map& id_map_;
        detail::program_bytes& program_;
        const detail::definition_source_declarations& declarations_;

        compile_mode mode_;

        friend class definition_library;
    };

    class definition_library
    {
        friend class executor;
        friend class detail::execution_context;

    public:
        definition_library(const definition_library& other)
        : program_{ other.program_ }, tag_names_{ other.tag_names_ },
          equipment_tags_{ other.equipment_tags_ }, buckets_{ other.buckets_ }
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

        static constexpr size_t definition_count = definition_types::size();

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
                const TView& entity,
                TEvent& event,
                handle_context& context
            ) const
            {
                return library_->template handle<TEvent>(
                    id_, entity, event, context
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
            const TView& entity,
            TEvent& event,
            handle_context& context
        ) const
        {
            const auto& bucket = bucket_for<TDefinitionType>();
            const size_t index = id.value();
            const auto handle_fn = get_handle_fn<TEvent, TView>(id);
            return handle_fn(bucket.data[index], entity, event, context);
        }

    private:
        using definition_type_list = definition_types;

        template<class TView, class TSequence>
        struct handle_fn_vectors_impl;

        template<class TView, size_t...I>
        struct handle_fn_vectors_impl<TView, std::index_sequence<I...>>
        {
            using type = std::tuple<
                std::vector<handle_fn_t<TView, typename subscribed_events<TView>::template type_at<I>>>...
            >;
        };

        template<class TView>
        using handle_fn_vectors_t = typename handle_fn_vectors_impl<
            TView,
            std::make_index_sequence<subscribed_events<TView>::size()>
        >::type;

        template<class TDefinitionType, class TSequence>
        struct handle_fn_vector_groups_impl;

        template<class TDefinitionType, size_t...I>
        struct handle_fn_vector_groups_impl<TDefinitionType, std::index_sequence<I...>>
        {
            using type = std::tuple<
                handle_fn_vectors_t<typename views_of_definition<TDefinitionType>::template type_at<I>>...
            >;
        };

        template<class TDefinitionType>
        using handle_fn_vector_groups_t = typename handle_fn_vector_groups_impl<
            TDefinitionType,
            std::make_index_sequence<views_of_definition<TDefinitionType>::size()>
        >::type;

        template<class... TQueries>
        using query_vectors_for = std::tuple<std::vector<std::conditional_t<
            std::is_empty_v<TQueries>, typename TQueries::result_t, detail::query_fn_t<TQueries>
        >>...>;

        template<class TDefinitionType>
        struct bucket
        {
            std::vector<std::string_view> names;
            std::vector<definition_data> data;
            std::vector<tag_mask> tags;
            handle_fn_vector_groups_t<TDefinitionType> handle_fns;
#ifdef _MSC_VER
            [[msvc::no_unique_address]]
#else
            [[no_unique_address]]
#endif
            typename supported_queries<TDefinitionType>::template apply<query_vectors_for> queries;
        };

        template<class...TDefinition>
        using bucket_tuple_for = std::tuple<bucket<TDefinition>...>;

        using bucket_tuple = definition_type_list::apply<bucket_tuple_for>;

        explicit definition_library(const issued_id_map& id_map)
        : program_(sizeof(detail::execute_fn), 0), tag_names_(id_map.tag_names().begin(), id_map.tag_names().end())
        {
            constexpr std::array<std::string_view, static_cast<size_t>(givm::equipment_type::none)> equipment_tag_names{
                "weapon", "artifact", "talent", "technique"
            };
            for(size_t index = 0; index != equipment_tags_.size(); ++index)
            {
                if(id_map.has_tag(equipment_tag_names[index]))
                {
                    equipment_tags_[index] = id_map.get_tag_id(equipment_tag_names[index]);
                }
            }
        }

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

        template<class TEvent, class TView, class TDefinitionType>
        handle_fn_t<TView, TEvent> get_handle_fn(definition_id<TDefinitionType> id) const noexcept
        {
            const auto& bucket = bucket_for<TDefinitionType>();
            return std::get<subscribed_events<TView>::template index_of<TEvent>()>(
                std::get<views_of_definition<TDefinitionType>::template index_of<TView>()>(bucket.handle_fns)
            )[id.value()];
        }

        template<class TDefinitionType>
        void append(
            const definition_source_view<TDefinitionType>& source,
            const issued_id_map& id_map,
            compile_mode mode
        )
        {
            const auto declarations = source.declarations();
            auto& bucket = bucket_for<TDefinitionType>();
            definition_compile_context context{ id_map, program_, declarations, mode };
            definition_data data = source.compile(context);

            bucket.names.push_back(source.name());
            bucket.data.push_back(std::move(data));
            bucket.tags.push_back(make_tag_mask(declarations.tags, id_map));
            append_handle_fns(bucket, source,
                              std::make_index_sequence<views_of_definition<TDefinitionType>::size()>{});
            if constexpr(supported_queries<TDefinitionType>::size() != 0)
            {
                supported_queries<TDefinitionType>::each([&]<class TQuery>
                {
                    auto& queries =
                        std::get<supported_queries<TDefinitionType>::template index_of<TQuery>()>(bucket.queries);
                    const auto query_fn = source.template get_query_fn<TQuery>();
                    if constexpr(std::is_empty_v<TQuery>)
                    {
                        queries.push_back(query_fn(bucket.data.back(), TQuery{}));
                    }
                    else
                    {
                        queries.push_back(query_fn);
                    }
                });
            }
        }

        static tag_mask make_tag_mask(
            const std::vector<std::string_view>& tags,
            const issued_id_map& id_map
        )
        {
            tag_mask result{ id_map.tag_names().size() };
            for(std::string_view tag : tags)
            {
                result.set(id_map.get_tag_id(tag));
            }
            return result;
        }

        template<class TDefinitionType, size_t...I>
        static void append_handle_fns(
            bucket<TDefinitionType>& bucket,
            const definition_source_view<TDefinitionType>& source,
            std::index_sequence<I...>
        )
        {
            (append_handle_fns_for_view<
                TDefinitionType,
                typename views_of_definition<TDefinitionType>::template type_at<I>
            >(bucket, source), ...);
        }

        template<class TDefinitionType, class TView, size_t...I>
        static void append_handle_fns_for_view(
            bucket<TDefinitionType>& bucket,
            const definition_source_view<TDefinitionType>& source,
            std::index_sequence<I...>
        )
        {
            auto& handle_fns =
                std::get<views_of_definition<TDefinitionType>::template index_of<TView>()>(bucket.handle_fns);
            ((std::get<I>(handle_fns).push_back(
                source.template get_handle_fn<TView, typename subscribed_events<TView>::template type_at<I>>()
            )), ...);
        }

        template<class TDefinitionType, class TView>
        static void append_handle_fns_for_view(
            bucket<TDefinitionType>& bucket,
            const definition_source_view<TDefinitionType>& source
        )
        {
            append_handle_fns_for_view<TDefinitionType, TView>(
                bucket,
                source,
                std::make_index_sequence<subscribed_events<TView>::size()>{}
            );
        }

        template<class TCategory>
        static void append_sources(
            const definition_source_library& sources,
            const issued_id_map& id_map,
            definition_library& library,
            compile_mode mode
        )
        {
            std::vector<definition_source_view<TCategory>> selected;
            for(auto source : sources.source_views<TCategory>())
            {
                if(id_map.has<TCategory>(source.name()))
                {
                    selected.push_back(source);
                }
            }
            std::ranges::sort(selected, [&](const auto& left, const auto& right)
            {
                return id_map.get_id<TCategory>(left.name()).value()
                    < id_map.get_id<TCategory>(right.name()).value();
            });
            for(const auto& source : selected)
            {
                library.append(source, id_map, mode);
            }
        }

    public:
        template<class TInitializationSequence, class TRoundSequence>
        static auto compile(
            const definition_source_library& sources,
            issued_id_map id_map,
            TInitializationSequence&& initialization_program,
            TRoundSequence&& round_program,
            compile_mode mode
        )
        {
            struct compile_result
            {
                definition_library library;
                issued_id_map id_map;
            };

            definition_library library{ id_map };
            detail::program_writer writer{ library.program_ };
            [[maybe_unused]] const auto initialization_inputs_size =
                detail::append_commands(writer, std::forward<TInitializationSequence>(initialization_program), mode);
#ifndef NDEBUG
            if(initialization_inputs_size != 0)
            {
                throw std::invalid_argument{ "root programs cannot consume invocation inputs" };
            }
#endif
            const detail::execution_position round_entry = library.program_.size();
            [[maybe_unused]] const auto round_inputs_size =
                detail::append_commands(writer, std::forward<TRoundSequence>(round_program), mode);
#ifndef NDEBUG
            if(round_inputs_size != 0)
            {
                throw std::invalid_argument{ "root programs cannot consume invocation inputs" };
            }
#endif
            writer.write(detail::execute_fn{ detail::execute_jump });
            writer.write(round_entry);

            [&]<std::size_t... I>(std::index_sequence<I...>)
            {
                (append_sources<definition_types::type_at<I>>(sources, id_map, library, mode), ...);
            }(std::make_index_sequence<definition_types::size()>{});

            detail::finalize_program(library.program_);
            return compile_result{ .library = std::move(library), .id_map = std::move(id_map) };
        }

    private:
        detail::program_bytes program_;
        std::vector<std::string_view> tag_names_;
        std::array<tag_id, static_cast<size_t>(givm::equipment_type::none)> equipment_tags_{};
        bucket_tuple buckets_;
    };

    template<class TInitializationSequence, class TRoundSequence>
    auto compile(
        const definition_source_library& sources,
        TInitializationSequence&& initialization_program,
        TRoundSequence&& round_program,
        compile_mode mode
    )
    {
        return definition_library::compile(
            sources, sources.make_issued_id_map(),
            std::forward<TInitializationSequence>(initialization_program),
            std::forward<TRoundSequence>(round_program), mode
        );
    }

    template<class TInitializationSequence, class TRoundSequence>
    auto compile(
        const definition_source_library& sources,
        const definition_selection& selection,
        TInitializationSequence&& initialization_program,
        TRoundSequence&& round_program,
        compile_mode mode
    )
    {
        return definition_library::compile(
            sources, sources.make_issued_id_map(selection),
            std::forward<TInitializationSequence>(initialization_program),
            std::forward<TRoundSequence>(round_program), mode
        );
    }
}

#include "../macro_undef.hpp"
#endif
