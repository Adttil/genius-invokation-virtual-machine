[givm](../../../../reference.md) / [执行](../../../executor.md) / [definition_library](../../definition_library.md) / [definition_view](../definition_view.md) / **can_handle**

# givm::definition_library::definition_view::can_handle

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TEvent, class TView>
bool can_handle() const noexcept;
```

检查该定义在给定实体形态下是否提供某个事件的响应。

## 模板参数

|  |  |
| --- | --- |
| `TEvent` | 属于该 view 可订阅范围的事件类型 |
| `TView` | 属于该定义类别的只读实体 view |

## 返回值

为该 view 和事件启用了响应函数时返回 `true`，否则返回 `false`。

## 注意

返回 `true` 不代表本次事件一定触发效果，响应函数仍可按事件条件决定是否生效。

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
    const auto definition = library[id];
    std::println("响应掷骰准备: {}", definition.can_handle<givm::dice_roll_preparation, givm::support_view>());
    std::println("响应回合结束: {}", definition.can_handle<givm::round_ended, givm::support_view>());
}
```

输出

```text
响应掷骰准备: true
响应回合结束: false
```
