[givm](../../../reference.md) / [执行](../../executor.md) / [指令](../instructions.md) / **enter_character**

# givm::enter_character

定义于头文件 `<givm/executor/instructions/enter_character.hpp>`

```cpp
struct enter_character;
```

角色入场指令，指定加入哪一方队伍的角色，并按其定义准备初始状态。

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

新角色的 [`character_initialization`](../events/character_initialization.md) 由该角色定义直接响应。需要的初始状态在这次响应中填写；入场不自动将其设为出战角色。

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

    static givm::program_entry<givm::character_initialization> handle(
        const definition_type&, const givm::character_view&,
        givm::character_initialization& event, const givm::card_table&, givm::random_fn&)
    {
        event.state = { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 };
        return givm::program_entry<givm::character_initialization>::null();
    }
};

int main()
{
    character_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    const auto issued = sources.make_issued_id_map();
    const auto definition = issued.get_id<givm::character_view>("character");
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::enter_character{ .player = givm::player_id{ 0 }, .definition = definition } },
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::card_table table{ library };
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.run(table, random);
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
| [`character_initialization`](../events/character_initialization.md) | 角色初始状态的准备事件 |
