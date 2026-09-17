[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **initialize_characters**

# givm::initialize_characters

定义于头文件 `<givm/definition.hpp>`

```cpp
struct initialize_characters;
```

已有角色的初始化命令，用于按角色定义准备指定玩家全部角色的初始状态与技能。它适用于角色已装入牌桌后的开局初始化。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，表示不依赖特定事件语境 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`player_id`](../../table/player_id.md) | 要初始化角色的玩家 |

## 注意

每个角色使用定义库已保存的 [`character_initial_state`](../queries/character_initial_state.md) 结果。已有角色状态会被此初始状态替换；本命令不创建角色。

随后重建角色技能：移除原有技能，从零开始逐项查询 [`character_initial_skill`](../queries/character_initial_skill.md)，直到首次得到无效 ID；新技能使用默认状态。重复初始化也会重建技能集合，但不改变角色附着实体。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct character_source
{
    using definition_category = givm::character_view;
    struct definition_type {};
    std::string_view name() const { return "character"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }

    static givm::character_state query(const definition_type&, const givm::character_initial_state&)
    {
        return { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 };
    }
};

int main()
{
    character_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::initialize_characters{ .player = givm::player_id{ 0 } } },
        std::tuple{ givm::start_round{ .max_rounds = 0 } }, givm::compile_mode::normal);
    givm::table table{};
    table.load_deck(givm::player_id{ 0 }, givm::linked_deck{
        .characters = { ids.get_id<givm::character_view>("character") }
    });
    const auto character = table[givm::character_id{ givm::player_id{ 0 }, 0 }];
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.step(library, table, random);
    std::println("初始化后的生命: {}", character.state().health);
}
```

输出

```text
初始化后的生命: 10
```

## 参阅

| | |
| --- | --- |
| [`character_initial_state`](../queries/character_initial_state.md) | 角色初始状态查询 |
| [`character_initial_skill`](../queries/character_initial_skill.md) | 角色初始技能查询 |
