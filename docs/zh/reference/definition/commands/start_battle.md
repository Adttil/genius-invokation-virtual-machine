[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **start_battle**

# givm::start_battle

定义于头文件 `<givm/definition.hpp>`

```cpp
struct start_battle;
```

首回合战斗开始的通知命令，供相关效果在双方进入战斗时生效。

## 成员类型

| | |
| --- | --- |
| [`error_type`](#编译检查) | `start_battle_error` 的别名，即本命令的编译检查错误类型 |

## 编译检查

```cpp
enum class start_battle_error {};
```

`start_battle::error_type` 是 `givm::start_battle_error` 的别名。这是没有枚举项的空枚举类型，本命令没有编译期参数错误。

## 注意

只有牌桌回合数为 1 时发出 [`battle_started`](../events/battle_started.md)；在其他回合执行时直接继续。本命令不随机选择先手，自身不调用随机源；事件响应可以使用随机值。

## 示例

```cpp
#include <utility>
#include <array>
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct observer_source
{
    static constexpr auto category = givm::definition_category::skill;
    struct definition_type { int* count; };
    int* count;
    std::string_view name() const { return "observer"; }
    definition_type compile(givm::definition_compile_context&) const { return { count }; }
    static givm::normal_effect handle(
        const definition_type& data,
        givm::battle_started&, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
    {
        ++*data.count;
        return {};
    }
};

struct character_source
{
    static constexpr auto category = givm::definition_category::character;
    using definition_type = givm::optional_definition_id<givm::definition_category::skill>;
    std::string_view name() const { return "character"; }
    auto skill_dependencies() const { return std::array<std::string_view, 1>{ "observer" }; }
    definition_type compile(givm::definition_compile_context& context) const
    {
        return context.resolve_id<givm::definition_category::skill>("observer");
    }
    static givm::character_state query(const definition_type&, const givm::character_initial_state&)
    {
        return { .max_health = 10, .health = 10 };
    }
    static definition_type query(const definition_type& skill, const givm::character_initial_skill& query)
    {
        return query.skill_index == 0 ? skill : definition_type{};
    }
};

int main()
{
    int count = 0;
    observer_source source{ &count };
    character_source character{};
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    if(not sources.add(source)) return 1;
    if(not sources.add(character)) return 1;
    auto library_result = compile(
        sources, basics,
        std::tuple{},
        std::tuple{
            givm::start_dice_roll_phase{ .count = 0, .reroll_count = { 0, 0 } },
            givm::start_round{}, givm::settle{}, givm::start_battle{}, givm::settle{}
        }, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    givm::table table{ { .max_rounds = 2 } };
    load_deck(table, library, givm::linked_deck{
        .characters = { ids.get_id<givm::definition_category::character>("character") }
    }, {});
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    const auto initialized = execution.start(library, table);
    initialized.resume(library, table, random);
    std::println("收到事件次数: {}", count);
}
```

输出

```text
收到事件次数: 1
```

## 参阅

| | |
| --- | --- |
| [`battle_started`](../events/battle_started.md) | 对局首次进入战斗的通知 |
