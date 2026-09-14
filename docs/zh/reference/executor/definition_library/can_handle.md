[givm](../../../reference.md) / [执行](../../executor.md) / [definition_library](../definition_library.md) / **can_handle**

# givm::definition_library::can_handle

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TEvent, class TView, class TDefinitionType>
bool can_handle(definition_id<TDefinitionType> id) const noexcept;
```

检查某项定义在指定实体形态下是否能够响应一个事件。例如同一张卡牌处于手牌与牌堆时，可以具有不同的响应能力。

## 模板参数

|  |  |
| --- | --- |
| `TEvent` | 要查询的事件，须属于该 view 的 [`subscribed_events`](../../definition/subscribed_events.md) |
| `TView` | 响应实体的只读 view，须属于定义对应的 [`views_of_definition`](../../definition/views_of_definition.md) |
| `TDefinitionType` | 由 ID 推导的定义类别 |

## 参数

|  |  |
| --- | --- |
| `id` | 本定义库中的有效定义 ID |

## 返回值

编译时为这一 view 和事件启用了响应函数时返回 `true`，否则返回 `false`。

## 注意

结果表示定义是否提供响应函数，不代表当前事件一定触发效果；响应函数仍可检查条件后返回空入口。

## 示例

```cpp
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct support_source
{
    using definition_category = givm::support_view;

    std::string_view name() const { return "重投助手"; }
    int compile(givm::definition_compile_context&) const { return 1; }

    static givm::handler_program_entry_t<givm::dice_roll_preparation> handle(
        const int& extra_rerolls,
        const givm::support_view&,
        givm::dice_roll_preparation& event,
        const givm::table&,
        givm::random_fn&
    )
    {
        event.reroll_count[0] += extra_rerolls;
        return givm::handler_program_entry_t<givm::dice_roll_preparation>::null();
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
    const auto id = ids.get_id<givm::support_view>("重投助手");
    std::println("响应掷骰准备: {}", library.can_handle<givm::dice_roll_preparation, givm::support_view>(id));
    std::println("响应回合结束: {}", library.can_handle<givm::round_ended, givm::support_view>(id));
}
```

输出

```text
响应掷骰准备: true
响应回合结束: false
```
