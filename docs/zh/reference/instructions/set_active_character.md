[givm](../../reference.md) / [指令](../instructions.md) / **set_active_character**

# givm::set_active_character

定义于头文件 `<givm/executor/instructions/set_active_character.hpp>`

```cpp
struct set_active_character;
```

直接设置一名玩家的出战角色。它用于开局设置或规则强制的切换，不经过主动切换的费用计算。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，可用于不限定事件的操作 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `target` | [`character_id`](../table/character_id.md) | 新的出战角色，必须是牌桌上有效的角色 |

## 成员函数

| | |
| --- | --- |
| [`execute`](set_active_character/execute.md) | 直接设置一名玩家的出战角色 |

## 注意

设置后发出 [`active_character_changed`](../events/active_character_changed.md)，响应者能够读取新的出战角色。

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
        std::tuple{ givm::set_active_character{ .target = { .player_id = givm::player_id{ 1 }, .index = 0 } } },
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::card_table table{ library };
    const auto definition = ids.get_id<givm::character_view>("character");
    const auto attacker = table[givm::player_id{ 0 }].add(
        definition, { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 }).id();
    const auto target = table[givm::player_id{ 1 }].add(
        definition, { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 }).id();
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}
    std::println("出战角色设置成功: {}", table[givm::player_id{ 1 }].state().active_character == target);
}
```

输出

```text
出战角色设置成功: true
```

## 参阅

| | |
| --- | --- |
| [`active_character_changed`](../events/active_character_changed.md) | 出战角色已设置 |
