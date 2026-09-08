#ifndef GIVM_DEFINITION_LIBRARY_HPP
#define GIVM_DEFINITION_LIBRARY_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "source_view.hpp"
#include "issued_id.hpp"
#include "issued_id_map.hpp"
#include "subscribed_events.hpp"
#include "types.hpp"
#include "../enums/game_result.hpp"
#include "../utils/debug.hpp"
#include "../utils/type_list.hpp"

#include "../macro_define.hpp"

namespace givm
{
    using execution_position = std::size_t;

    namespace detail
    {
        inline constexpr execution_position null_program_position =
            static_cast<execution_position>(game_result::no_result);
        inline constexpr execution_position player_0_win_position =
            static_cast<execution_position>(game_result::player_0_win);
        inline constexpr execution_position player_1_win_position =
            static_cast<execution_position>(game_result::player_1_win);
        inline constexpr execution_position both_loss_position =
            static_cast<execution_position>(game_result::both_loss);
        inline constexpr execution_position program_prefix_size = both_loss_position + 1;
        inline constexpr execution_position entry_position = program_prefix_size;

        struct null_instruction
        {
            bool execute(card_table&, execution_context&, random_fn&) const
            {
                GIVM_ASSERT(false);
                return false;
            }
        };

        struct end_game_instruction
        {
            bool execute(card_table&, execution_context&, random_fn&) const noexcept
            {
                return false;
            }
        };

        struct return_instruction
        {
            bool execute(card_table&, execution_context&, random_fn&) const
            {
                GIVM_ASSERT(false);
                return false;
            }
        };

        struct jump_instruction
        {
            execution_position target;

            bool execute(card_table&, execution_context&, random_fn&) const
            {
                GIVM_ASSERT(false);
                return false;
            }
        };
    }

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

        template<class TContext, class TInstructions>
        program_entry<TContext> add_program(TInstructions&& instructions)
        {
            const program_entry<TContext> result{ program_.size() };
            detail::append_instructions<TContext>(
                program_,
                std::forward<TInstructions>(instructions)
            );
            program_.push_back(detail::any_instruction{ detail::return_instruction{} });
            return result;
        }

    private:
        definition_compile_context(
            const issued_id_map& id_map,
            std::vector<detail::any_instruction>& program,
            const detail::definition_source_declarations& declarations
        )
        : id_map_{ id_map }, program_{ program }, declarations_{ declarations }
        {}

        static bool contains(const std::vector<std::string_view>& values, std::string_view value)
        {
            return std::ranges::find(values, value) != values.end();
        }

        const issued_id_map& id_map_;
        std::vector<detail::any_instruction>& program_;
        const detail::definition_source_declarations& declarations_;

        friend class definition_library;
    };

    class tag_mask
    {
    public:
        tag_mask() = default;

        explicit tag_mask(size_t size)
        : bits_(size)
        {}

        void set(tag_id id, bool value = true)
        {
            bits_[id.value()] = value;
        }

        bool has(tag_id id) const
        {
            return bits_[id.value()];
        }

        bool has_all(std::span<const tag_id> tags) const
        {
            for(tag_id tag : tags)
            {
                if(not has(tag))
                {
                    return false;
                }
            }
            return true;
        }

        bool has_any(std::span<const tag_id> tags) const
        {
            for(tag_id tag : tags)
            {
                if(has(tag))
                {
                    return true;
                }
            }
            return false;
        }

        bool matches(
            std::span<const tag_id> required_tags,
            std::span<const tag_id> excluded_tags = {}
        ) const
        {
            if(not has_all(required_tags))
            {
                return false;
            }

            for(tag_id tag : excluded_tags)
            {
                if(has(tag))
                {
                    return false;
                }
            }
            return true;
        }

    private:
        std::vector<bool> bits_;
    };

    class definition_library
    {
        friend class definition_source_library;

    public:
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

            template<class TEvent, class TView>
            handler_program_entry_t<TEvent> handle(
                const TView& entity,
                TEvent& event,
                const card_table& table,
                random_fn& random
            ) const
            {
                return library_->template handle<TEvent>(
                    id_, entity, event, table, random
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

        static constexpr execution_position entry() noexcept
        {
            return detail::entry_position;
        }

        constexpr decltype(auto) instruction(execution_position position) const noexcept
        {
            GIVM_ASSERT(position < program_.size());
            return static_cast<detail::any_instruction_view>(program_[position]);
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

        template<class TEvent, class TDefinitionType, class TView>
        handler_program_entry_t<TEvent> handle(
            definition_id<TDefinitionType> id,
            const TView& entity,
            TEvent& event,
            const card_table& table,
            random_fn& random
        ) const
        {
            const auto& bucket = bucket_for<TDefinitionType>();
            const size_t index = id.value();
            const auto handle_fn = get_handle_fn<TEvent, TView>(id);
            return handle_fn(bucket.data[index], entity, event, table, random);
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

        template<class TDefinitionType>
        struct bucket
        {
            std::vector<std::string_view> names;
            std::vector<definition_data> data;
            std::vector<tag_mask> tags;
            handle_fn_vector_groups_t<TDefinitionType> handle_fns;
        };

        template<class...TDefinition>
        using bucket_tuple_for = std::tuple<bucket<TDefinition>...>;

        using bucket_tuple = definition_type_list::apply<bucket_tuple_for>;

        explicit definition_library(std::span<const std::string_view> tag_names)
        : tag_names_(tag_names.begin(), tag_names.end())
        {
            program_.push_back(detail::any_instruction{ detail::null_instruction{} });
            program_.push_back(detail::any_instruction{ detail::end_game_instruction{} });
            program_.push_back(detail::any_instruction{ detail::end_game_instruction{} });
            program_.push_back(detail::any_instruction{ detail::end_game_instruction{} });
            GIVM_ASSERT(program_.size() == detail::program_prefix_size);
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
            std::string_view name,
            const issued_id_map& id_map,
            const detail::definition_source_declarations& declarations
        )
        {
            auto& bucket = bucket_for<TDefinitionType>();
            definition_compile_context context{ id_map, program_, declarations };
            definition_data data = source.compile(context);

            bucket.names.push_back(name);
            bucket.data.push_back(std::move(data));
            bucket.tags.push_back(make_tag_mask(declarations.tags, id_map));
            append_handle_fns(bucket, source,
                              std::make_index_sequence<views_of_definition<TDefinitionType>::size()>{});
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

        std::vector<detail::any_instruction> program_;
        std::vector<std::string_view> tag_names_;
        bucket_tuple buckets_;
    };
}

#include "../macro_undef.hpp"

#endif
