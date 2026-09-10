[givm](../../../../reference.md) / [执行](../../../executor.md) / [指令](../../instructions.md) / [start_battle](../start_battle.md) / **execute**

# givm::start_battle::execute

定义于头文件 `<givm/executor/instructions/start_battle.hpp>`

```cpp
bool execute(card_table& table, execution_context& context, random_fn& random) const;
```

[`card_table`](../../../table/card_table.md)
[`execution_context`](../../execution_context.md)
[`random_fn`](../../random_fn.md)

在第一回合发出战斗开始通知，让相关效果在双方进入战斗时生效。

由 [`executor::execute_next`](../../executor/execute_next.md) 在对局推进过程中调用。

## 参数

| | |
| --- | --- |
| `table` | 本场对局的牌桌 |
| `context` | 本次指令执行的上下文；调用须满足本指令的事件或输入约定 |
| `random` | 本次执行可使用的随机源 |

## 返回值

返回 `true` 表示可以继续自动推进，`false` 表示应停止推进，包括等待外部输入或对局结束。对局结果由 [`executor::status`](../../executor/status.md) 判断。

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
