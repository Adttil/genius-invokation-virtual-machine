[givm](../../reference.md) / [指令](../instructions.md) / **start_battle**

# givm::start_battle

定义于头文件 `<givm/executor/instructions/start_battle.hpp>`

```cpp
struct start_battle;
```

在第一回合发出战斗开始通知，让相关效果在双方进入战斗时生效。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，可用于不限定事件的操作 |

## 成员函数

| | |
| --- | --- |
| [`execute`](start_battle/execute.md) | 在第一回合发出战斗开始通知，让相关效果在双方进入战斗时生效 |

## 注意

只有牌桌回合数为 1 时发出 [`battle_started`](../events/battle_started.md)；在其他回合执行时直接继续。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct observer_source
{
    using definition_category = givm::support_view;
    struct definition_type { int* count; };
    int* count;
    std::string_view name() const { return "observer"; }
    definition_type compile(givm::definition_compile_context&) const { return { count }; }
    static givm::program_entry<givm::battle_started> handle(
        const definition_type& data, const givm::support_view&,
        givm::battle_started&, const givm::card_table&, givm::random_fn&)
    {
        ++*data.count;
        return givm::program_entry<givm::battle_started>::null();
    }
};

int main()
{
    int count = 0;
    observer_source source{ &count };
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::start_battle{} },
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::card_table table{ library };
    table[givm::player_id{ 0 }].add(ids.get_id<givm::support_view>("observer"), {});
    table.state().round_number = 1;
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}
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
| [`battle_started`](../events/battle_started.md) | 战斗开始 |
