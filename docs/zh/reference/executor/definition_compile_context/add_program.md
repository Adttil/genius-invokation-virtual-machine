[givm](../../../reference.md) / [执行](../../executor.md) / [definition_compile_context](../definition_compile_context.md) / **add_program**

# givm::definition_compile_context::add_program

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TContext, class TInstructions>
program_entry<TContext> add_program(TInstructions&& instructions);
```

登记响应事件时需要依次执行的一段效果，并取得可在以后响应时返回的入口。

## 模板参数

|  |  |
| --- | --- |
| `TContext` | 这段效果适用的 context；事件处理通常使用 `handler_program_context_t<TEvent>` |
| `TInstructions` | 指令序列，可为 tuple-like 对象或可遍历范围 |

## 参数

|  |  |
| --- | --- |
| `instructions` | 按顺序执行的[核心指令](../instructions.md)，也可用 [`any_instruction_for`](../any_instruction_for.md) 保存；每项须与 `TContext` 兼容 |

## 返回值

登记效果的非空 [`program_entry<TContext>`](../program_entry.md)。

## 注意

返回入口只用于本次编译产生的定义库。效果正常完成后回到发起它的结算；若执行期间结束对局，则不再返回原结算。本函数只登记效果，不立即执行。

## 示例

```cpp
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct support_source
{
    using definition_category = givm::support_view;

    std::string_view name() const { return "洗牌助手"; }

    givm::handler_program_entry_t<givm::round_ended> compile(givm::definition_compile_context& context) const
    {
        auto entry = context.add_program<givm::handler_program_context_t<givm::round_ended>>(
            std::tuple{ givm::shuffle_deck{ .player = givm::player_id{ 0 } } }
        );
        std::println("已登记回合结束效果: {}", static_cast<bool>(entry));
        return entry;
    }

    static givm::handler_program_entry_t<givm::round_ended> handle(
        const givm::handler_program_entry_t<givm::round_ended>& entry,
        const givm::support_view&,
        givm::round_ended&,
        const givm::table&,
        givm::random_fn&
    )
    {
        return entry;
    }
};

int main()
{
    const support_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{ givm::start_round{} }
    );
}
```

输出

```text
已登记回合结束效果: true
```
