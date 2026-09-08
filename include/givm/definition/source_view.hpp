#ifndef GIVM_DEFINITION_SOURCE_VIEW_HPP
#define GIVM_DEFINITION_SOURCE_VIEW_HPP

#include <any>
#include <array>
#include <concepts>
#include <cstddef>
#include <ranges>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "instruction.hpp"
#include "subscribed_events.hpp"
#include "types.hpp"
#include "../enums/game_result.hpp"

namespace givm
{
    class definition_compile_context;
    class definition_library;
    class definition_source_library;

    template<class TCategory>
    class definition_source_view;

    template<class TContext>
    class program_entry
    {
    public:
        constexpr program_entry() noexcept = default;

        [[nodiscard]] static constexpr program_entry null() noexcept
        {
            return {};
        }

        [[nodiscard]] static constexpr program_entry player_0_win() noexcept
        {
            return program_entry{ game_result::player_0_win };
        }

        [[nodiscard]] static constexpr program_entry player_1_win() noexcept
        {
            return program_entry{ game_result::player_1_win };
        }

        [[nodiscard]] static constexpr program_entry both_loss() noexcept
        {
            return program_entry{ game_result::both_loss };
        }

        [[nodiscard]] constexpr bool is_null() const noexcept
        {
            return position_ == static_cast<std::size_t>(game_result::no_result);
        }

        [[nodiscard]] constexpr explicit operator bool() const noexcept
        {
            return not is_null();
        }

        friend constexpr bool operator==(program_entry, program_entry) noexcept = default;

    private:
        constexpr explicit program_entry(std::size_t position) noexcept
        : position_{ position }
        {}

        constexpr explicit program_entry(game_result result) noexcept
        : position_{ static_cast<std::size_t>(result) }
        {}

        std::size_t position_ = static_cast<std::size_t>(game_result::no_result);

        friend class definition_compile_context;
        friend class execution_context;
    };

    namespace detail
    {
        using definition_dependency_lists =
            std::array<std::vector<std::string_view>, definition_types::size()>;

        struct definition_source_declarations
        {
            std::vector<std::string_view> tags;
            definition_dependency_lists dependencies;
            std::vector<std::string_view> tag_dependencies;
            definition_dependency_lists dependencies_by_tag;
        };

        template<class TItems>
        std::vector<std::string_view> collect_definition_strings(TItems&& items)
        {
            std::vector<std::string_view> result;
            for(auto&& item : items)
            {
                result.emplace_back(std::string_view{ item });
            }
            return result;
        }

        template<class TSource>
        constexpr decltype(auto) definition_tags(const TSource& source)
        {
            if constexpr(requires { source.tags(); })
            {
                return source.tags();
            }
            else
            {
                return std::array<std::string_view, 0>{};
            }
        }

        template<class TSource>
        constexpr decltype(auto) definition_tag_dependencies(const TSource& source)
        {
            if constexpr(requires { source.tag_dependencies(); })
            {
                return source.tag_dependencies();
            }
            else
            {
                return std::array<std::string_view, 0>{};
            }
        }

        template<class TCategory, class TSource>
        constexpr decltype(auto) definition_dependencies(const TSource& source)
        {
            if constexpr(std::same_as<TCategory, card_definition>)
            {
                if constexpr(requires { source.card_dependencies(); }) return source.card_dependencies();
                else return std::array<std::string_view, 0>{};
            }
            else if constexpr(std::same_as<TCategory, status_definition>)
            {
                if constexpr(requires { source.status_dependencies(); }) return source.status_dependencies();
                else return std::array<std::string_view, 0>{};
            }
            else if constexpr(std::same_as<TCategory, support_view>)
            {
                if constexpr(requires { source.support_dependencies(); }) return source.support_dependencies();
                else return std::array<std::string_view, 0>{};
            }
            else if constexpr(std::same_as<TCategory, summon_view>)
            {
                if constexpr(requires { source.summon_dependencies(); }) return source.summon_dependencies();
                else return std::array<std::string_view, 0>{};
            }
            else if constexpr(std::same_as<TCategory, combat_status_view>)
            {
                if constexpr(requires { source.combat_status_dependencies(); })
                    return source.combat_status_dependencies();
                else return std::array<std::string_view, 0>{};
            }
            else if constexpr(std::same_as<TCategory, character_view>)
            {
                if constexpr(requires { source.character_dependencies(); }) return source.character_dependencies();
                else return std::array<std::string_view, 0>{};
            }
            else if constexpr(std::same_as<TCategory, skill_view>)
            {
                if constexpr(requires { source.skill_dependencies(); }) return source.skill_dependencies();
                else return std::array<std::string_view, 0>{};
            }
            else
            {
                static_assert(std::same_as<TCategory, attachment_view>);
                if constexpr(requires { source.attachment_dependencies(); })
                    return source.attachment_dependencies();
                else return std::array<std::string_view, 0>{};
            }
        }

        template<class TCategory, class TSource>
        constexpr decltype(auto) definition_dependencies_by_tag(const TSource& source)
        {
            if constexpr(std::same_as<TCategory, card_definition>)
            {
                if constexpr(requires { source.card_dependencies_by_tag(); })
                    return source.card_dependencies_by_tag();
                else return std::array<std::string_view, 0>{};
            }
            else if constexpr(std::same_as<TCategory, status_definition>)
            {
                if constexpr(requires { source.status_dependencies_by_tag(); })
                    return source.status_dependencies_by_tag();
                else return std::array<std::string_view, 0>{};
            }
            else if constexpr(std::same_as<TCategory, support_view>)
            {
                if constexpr(requires { source.support_dependencies_by_tag(); })
                    return source.support_dependencies_by_tag();
                else return std::array<std::string_view, 0>{};
            }
            else if constexpr(std::same_as<TCategory, summon_view>)
            {
                if constexpr(requires { source.summon_dependencies_by_tag(); })
                    return source.summon_dependencies_by_tag();
                else return std::array<std::string_view, 0>{};
            }
            else if constexpr(std::same_as<TCategory, combat_status_view>)
            {
                if constexpr(requires { source.combat_status_dependencies_by_tag(); })
                    return source.combat_status_dependencies_by_tag();
                else return std::array<std::string_view, 0>{};
            }
            else if constexpr(std::same_as<TCategory, character_view>)
            {
                if constexpr(requires { source.character_dependencies_by_tag(); })
                    return source.character_dependencies_by_tag();
                else return std::array<std::string_view, 0>{};
            }
            else if constexpr(std::same_as<TCategory, skill_view>)
            {
                if constexpr(requires { source.skill_dependencies_by_tag(); })
                    return source.skill_dependencies_by_tag();
                else return std::array<std::string_view, 0>{};
            }
            else
            {
                static_assert(std::same_as<TCategory, attachment_view>);
                if constexpr(requires { source.attachment_dependencies_by_tag(); })
                    return source.attachment_dependencies_by_tag();
                else return std::array<std::string_view, 0>{};
            }
        }

        template<class TContext, class TSequence>
        void append_instructions(std::vector<any_instruction>& program, TSequence&& instructions)
        {
            const auto append = [&]<class TInstruction>(TInstruction&& instruction)
            {
                const any_instruction_for<TContext> contextual_instruction{
                    std::forward<TInstruction>(instruction)
                };
                program.push_back(any_instruction{ contextual_instruction });
            };

            if constexpr(std::ranges::range<TSequence>)
            {
                for(auto&& instruction : instructions)
                {
                    append(std::forward<decltype(instruction)>(instruction));
                }
            }
            else
            {
                [&]<std::size_t... I>(std::index_sequence<I...>)
                {
                    using std::get;
                    (append(get<I>(std::forward<TSequence>(instructions))), ...);
                }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<TSequence>>>{});
            }
        }

        template<class TSource>
        using definition_for_source_t = std::remove_cvref_t<decltype(
            std::declval<const TSource&>().compile(std::declval<definition_compile_context&>())
        )>;
    }

    template<class TCategory>
    class definition_source_view
    {
    public:
        template<class TSource>
        constexpr definition_source_view(const TSource& source)
        : source_{ &source }, rtti_{ &rtti_for<TSource> }
        {}

        std::string_view name() const
        {
            return rtti_->name(source_);
        }

        std::vector<std::string_view> tags() const
        {
            return rtti_->tags(source_);
        }

        template<class TDependencyCategory>
        std::vector<std::string_view> dependencies() const
        {
            return rtti_->dependencies[definition_types::index_of<TDependencyCategory>()](source_);
        }

        std::vector<std::string_view> tag_dependencies() const
        {
            return rtti_->tag_dependencies(source_);
        }

        template<class TDependencyCategory>
        std::vector<std::string_view> dependencies_by_tag() const
        {
            return rtti_->dependencies_by_tag[definition_types::index_of<TDependencyCategory>()](source_);
        }

    private:
        template<class TView, class TEvent>
        using handle_fn_getter_t = handle_fn_t<TView, TEvent> (*)(const void*);

        template<class TView, class TSequence>
        struct handle_fn_getter_tuple_for_view_impl;

        template<class TView, std::size_t... I>
        struct handle_fn_getter_tuple_for_view_impl<TView, std::index_sequence<I...>>
        {
            using type = std::tuple<
                handle_fn_getter_t<TView, typename subscribed_events<TView>::template type_at<I>>...
            >;
        };

        template<class TView>
        using handle_fn_getter_tuple_for_view_t = typename handle_fn_getter_tuple_for_view_impl<
            TView,
            std::make_index_sequence<subscribed_events<TView>::size()>
        >::type;

        template<class TSequence>
        struct handle_fn_getter_tuple_impl;

        template<std::size_t... I>
        struct handle_fn_getter_tuple_impl<std::index_sequence<I...>>
        {
            using type = std::tuple<
                handle_fn_getter_tuple_for_view_t<typename views_of_definition<TCategory>::template type_at<I>>...
            >;
        };

        using handle_fn_getter_tuple_t = typename handle_fn_getter_tuple_impl<
            std::make_index_sequence<views_of_definition<TCategory>::size()>
        >::type;

        struct rtti_t
        {
            std::string_view(*name)(const void*);
            std::vector<std::string_view>(*tags)(const void*);
            std::array<std::vector<std::string_view>(*)(const void*), definition_types::size()> dependencies;
            std::vector<std::string_view>(*tag_dependencies)(const void*);
            std::array<std::vector<std::string_view>(*)(const void*), definition_types::size()>
                dependencies_by_tag;
            definition_data(*compile)(const void*, definition_compile_context&);
            handle_fn_getter_tuple_t handle_fn_getters;
        };

        definition_data compile(definition_compile_context& context) const
        {
            return rtti_->compile(source_, context);
        }

        detail::definition_source_declarations declarations() const
        {
            detail::definition_source_declarations result{
                .tags = tags(),
                .tag_dependencies = tag_dependencies()
            };
            [&]<std::size_t... I>(std::index_sequence<I...>)
            {
                ((result.dependencies[I] =
                    rtti_->dependencies[I](source_)), ...);
                ((result.dependencies_by_tag[I] =
                    rtti_->dependencies_by_tag[I](source_)), ...);
            }(std::make_index_sequence<definition_types::size()>{});
            return result;
        }

        template<class TView, class TEvent>
        handle_fn_t<TView, TEvent> get_handle_fn() const
        {
            const auto getter = std::get<subscribed_events<TView>::template index_of<TEvent>()>(
                std::get<views_of_definition<TCategory>::template index_of<TView>()>(
                    rtti_->handle_fn_getters
                )
            );
            return getter(source_);
        }

        template<class TSource>
        static constexpr rtti_t make_rtti()
        {
            return {
                .name = +[](const void* source)
                {
                    return static_cast<const TSource*>(source)->name();
                },
                .tags = +[](const void* source)
                {
                    return detail::collect_definition_strings(
                        detail::definition_tags(*static_cast<const TSource*>(source))
                    );
                },
                .dependencies = make_dependencies<TSource>(
                    std::make_index_sequence<definition_types::size()>{}
                ),
                .tag_dependencies = +[](const void* source)
                {
                    return detail::collect_definition_strings(
                        detail::definition_tag_dependencies(*static_cast<const TSource*>(source))
                    );
                },
                .dependencies_by_tag = make_dependencies_by_tag<TSource>(
                    std::make_index_sequence<definition_types::size()>{}
                ),
                .compile = +[](const void* source, definition_compile_context& context)
                {
                    using result_type = decltype(static_cast<const TSource*>(source)->compile(context));
                    static_assert(not std::is_reference_v<result_type>);
                    static_assert(not std::is_void_v<result_type>);
                    return definition_data{ static_cast<const TSource*>(source)->compile(context) };
                },
                .handle_fn_getters = make_handle_fn_getters<TSource>(
                    std::make_index_sequence<views_of_definition<TCategory>::size()>{}
                )
            };
        }

        template<class TSource>
        static constexpr rtti_t rtti_for = make_rtti<TSource>();

        template<class TSource, std::size_t... I>
        static constexpr auto make_dependencies(std::index_sequence<I...>)
        {
            return std::array<std::vector<std::string_view>(*)(const void*), definition_types::size()>{
                (+[](const void* source)
                {
                    using dependency_category = typename definition_types::template type_at<I>;
                    return detail::collect_definition_strings(
                        detail::definition_dependencies<dependency_category>(
                            *static_cast<const TSource*>(source)
                        )
                    );
                })...
            };
        }

        template<class TSource, std::size_t... I>
        static constexpr auto make_dependencies_by_tag(std::index_sequence<I...>)
        {
            return std::array<std::vector<std::string_view>(*)(const void*), definition_types::size()>{
                (+[](const void* source)
                {
                    using dependency_category = typename definition_types::template type_at<I>;
                    return detail::collect_definition_strings(
                        detail::definition_dependencies_by_tag<dependency_category>(
                            *static_cast<const TSource*>(source)
                        )
                    );
                })...
            };
        }

        template<class TSource, std::size_t... I>
        static constexpr handle_fn_getter_tuple_t make_handle_fn_getters(std::index_sequence<I...>)
        {
            return {
                make_handle_fn_getters_for_view<
                    TSource,
                    typename views_of_definition<TCategory>::template type_at<I>
                >()...
            };
        }

        template<class TSource, class TView, std::size_t... I>
        static constexpr handle_fn_getter_tuple_for_view_t<TView> make_handle_fn_getters_for_view(
            std::index_sequence<I...>
        )
        {
            return {
                make_handle_fn_getter<
                    TSource,
                    TView,
                    typename subscribed_events<TView>::template type_at<I>
                >()...
            };
        }

        template<class TSource, class TView>
        static constexpr handle_fn_getter_tuple_for_view_t<TView> make_handle_fn_getters_for_view()
        {
            return make_handle_fn_getters_for_view<TSource, TView>(
                std::make_index_sequence<subscribed_events<TView>::size()>{}
            );
        }

        template<class TSource, class TView, class TEvent>
        static constexpr handle_fn_getter_t<TView, TEvent> make_handle_fn_getter()
        {
            return +[](const void* source)
            {
                return make_handle_fn<TSource, TView, TEvent>(*static_cast<const TSource*>(source));
            };
        }

        template<class TSource, class TView, class TEvent>
        static handle_fn_t<TView, TEvent> make_handle_fn(const TSource& source)
        {
            using definition_type = detail::definition_for_source_t<TSource>;
            if constexpr(requires(
                const definition_type& definition,
                const TView& entity,
                TEvent& event,
                const card_table& table,
                random_fn& random
            )
            {
                TSource::handle(definition, entity, event, table, random);
            })
            {
                using result_type = decltype(TSource::handle(
                    std::declval<const definition_type&>(),
                    std::declval<const TView&>(),
                    std::declval<TEvent&>(),
                    std::declval<const card_table&>(),
                    std::declval<random_fn&>()
                ));
                static_assert(std::same_as<result_type, handler_program_entry_t<TEvent>>);

                if constexpr(requires { source.template can_handle<TView, TEvent>(); })
                {
                    static_assert(std::same_as<
                        decltype(source.template can_handle<TView, TEvent>()),
                        bool
                    >);
                    if(not source.template can_handle<TView, TEvent>())
                    {
                        return nullptr;
                    }
                }

                return +[](
                    const definition_data& data,
                    const TView& entity,
                    TEvent& event,
                    const card_table& table,
                    random_fn& random
                )
                {
                    return TSource::handle(
                        std::any_cast<const definition_type&>(data),
                        entity,
                        event,
                        table,
                        random
                    );
                };
            }
            else
            {
                return nullptr;
            }
        }

        const void* source_;
        const rtti_t* rtti_;

        friend class definition_library;
        friend class definition_source_library;
    };
}

#endif
