[givm](../../reference.md) / [定义](../definition.md) / **program_entry**

# givm::program_entry

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TContext>
class program_entry;
```

一段效果的入口。事件响应可以用它选择接下来执行的效果，也可以表示没有后续效果。

## 模板参数

|  |  |
| --- | --- |
| `TContext` | 允许进入这段效果时的 context；通常使用 `handler_program_context_t<TEvent>` |

## 成员函数

|  |  |
| --- | --- |
| [(构造函数)](program_entry/constructor.md) | 构造空入口 |
| [`null`](program_entry/null.md) | 取得空入口 |
| [`is_null`](program_entry/is_null.md) | 检查入口是否为空 |
| [`operator bool`](program_entry/operator_bool.md) | 检查入口是否非空 |

## 注意

非空入口由 [`definition_compile_context::add_program`](definition_compile_context/add_program.md) 产生，不能跨定义库使用。context 不同的入口不能相互转换。空入口表示“不进入任何后续效果”。需要结束对局的响应可将 [`end_game`](../executor/instructions/end_game.md) 编入其程序。

## 非成员函数

|  |  |
| --- | --- |
| [`operator==`](program_entry/operator_eq.md) | 比较两个入口 |

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
        const givm::card_table&,
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
    const auto [library, ids] = sources.compile(
        std::tuple{}, std::tuple{ givm::start_round{} }
    );
}
```

输出

```text
已登记回合结束效果: true
```
