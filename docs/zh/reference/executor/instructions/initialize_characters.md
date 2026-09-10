[givm](../../../reference.md) / [执行](../../executor.md) / [指令](../instructions.md) / **initialize_characters**

# givm::initialize_characters

定义于头文件 `<givm/executor/instructions/initialize_characters.hpp>`

```cpp
struct initialize_characters;
```

已有角色的初始化指令，用于按角色定义准备指定玩家全部角色的初始状态。它适用于角色已装入牌桌后的开局初始化。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，表示不依赖特定事件语境 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`player_id`](../../table/player_id.md) | 要初始化角色的玩家 |

## 成员函数

| | |
| --- | --- |
| [`execute`](initialize_characters/execute.md) | 按角色定义重新准备指定玩家所有角色的初始状态 |

## 注意

每个角色分别响应 [`character_initialization`](../events/character_initialization.md)。已有角色状态会被这次准备的状态替换；本指令不创建角色。

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
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::initialize_characters{ .player = givm::player_id{ 0 } } },
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::card_table table{ library };
    const auto character = table[givm::player_id{ 0 }].add(
        ids.get_id<givm::character_view>("character"), givm::character_state{});
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}
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
| [`character_initialization`](../events/character_initialization.md) | 角色初始状态的准备事件 |
