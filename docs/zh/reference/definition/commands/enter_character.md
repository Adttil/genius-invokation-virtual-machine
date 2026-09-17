[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **enter_character**

# givm::enter_character

定义于头文件 `<givm/definition.hpp>`

```cpp
struct enter_character;
```

角色入场命令，指定加入哪一方队伍的角色，并按其定义准备初始状态与技能。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，表示不依赖特定事件语境 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`player_id`](../../table/player_id.md) | 角色所属的玩家 |
| `definition` | `definition_id<character_view>` | 要入场的角色定义 |

## 注意

新角色使用定义库已保存的 [`character_initial_state`](../queries/character_initial_state.md) 结果作为初始状态，再从零开始逐项查询 [`character_initial_skill`](../queries/character_initial_skill.md)，直到首次得到无效 ID，并以默认技能状态加入角色。入场不自动将其设为出战角色。

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
    const auto issued = sources.make_issued_id_map();
    const auto definition = issued.get_id<givm::character_view>("character");
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::enter_character{ .player = givm::player_id{ 0 }, .definition = definition } },
        std::tuple{ givm::start_round{ .max_rounds = 0 } }, givm::compile_mode::normal);
    givm::table table{};
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.step(library, table, random);
    const auto character = table[givm::character_id{ .player_id = givm::player_id{ 0 }, .index = 0 }];
    std::println("初始生命: {}", character.state().health);
}
```

输出

```text
初始生命: 10
```

## 参阅

| | |
| --- | --- |
| [`character_initial_state`](../queries/character_initial_state.md) | 角色初始状态查询 |
| [`character_initial_skill`](../queries/character_initial_skill.md) | 角色初始技能查询 |
