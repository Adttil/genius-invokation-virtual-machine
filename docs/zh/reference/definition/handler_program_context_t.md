[givm](../../reference.md) / [定义](../definition.md) / **handler_program_context_t**

# givm::handler_program_context_t

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TEvent>
using handler_program_context_t = typename handler_program_context<TEvent>::type;
```

一个事件的响应效果所使用的 context 类型，用于登记只在该事件下执行的命令序列。

## 模板参数

|  |  |
| --- | --- |
| `TEvent` | 事件类型 |

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
        std::tuple{}, std::tuple{ givm::start_round{} }, givm::compile_mode::normal
    );
}
```

输出

```text
已登记回合结束效果: true
```

## 参阅

|  |  |
| --- | --- |
| [`handler_program_context`](handler_program_context.md) | 事件与 context 的映射 |
