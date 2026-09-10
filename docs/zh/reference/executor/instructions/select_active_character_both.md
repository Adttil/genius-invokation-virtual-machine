[givm](../../../reference.md) / [执行](../../executor.md) / [指令](../instructions.md) / **select_active_character_both**

# givm::select_active_character_both

定义于头文件 `<givm/executor/instructions/select_active_character_both.hpp>`

```cpp
struct select_active_character_both;
```

双方开局出战角色的选择指令。双方提交后，同时设置两边的出战角色，再通知相关效果。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，表示不依赖特定事件语境 |

## 成员函数

| | |
| --- | --- |
| [`execute`](select_active_character_both/execute.md) | 让双方选择开局出战角色 |

## 注意

等待输入时，栈顶可按 `top<character_id, stage_t>()` 取得角色输入槽和需保留的尾部状态。只修改角色输入槽；两次提交必须属于不同玩家，且各自指向有效角色。第一次提交后仍未设置出战角色，第二次提交后才同时生效。

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
        std::tuple{ givm::select_active_character_both{} },
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
    for(const auto chosen : { attacker, target })
    {
        auto&& [input, preserved] = execution.stack().top<givm::character_id, givm::stage_t>();
        input = chosen;
        while(execution.execute_next(table, random)) {}
    }
    std::println("玩家 0 已选出战角色: {}", table[givm::player_id{ 0 }].state().active_character == attacker);
    std::println("玩家 1 已选出战角色: {}", table[givm::player_id{ 1 }].state().active_character == target);
}
```

输出

```text
玩家 0 已选出战角色: true
玩家 1 已选出战角色: true
```

## 参阅

| | |
| --- | --- |
| [`active_character_changed`](../events/active_character_changed.md) | 出战角色设置完成后的通知 |
