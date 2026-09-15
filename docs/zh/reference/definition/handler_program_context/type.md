[givm](../../../reference.md) / [定义](../../definition.md) / [handler_program_context](../handler_program_context.md) / **type**

# givm::handler_program_context::type

定义于头文件 `<givm/definition.hpp>`

```cpp
using type = TEvent; // 通常的事件
using type = /* 未指定的响应语境类型 */; // cost_of_switch 特化
```

该事件的响应效果所使用的 context。

[`cost_of_switch`](../events/cost_of_switch.md) 使用确认行动后执行效果的语境，其具体类型不作为使用约定。定义源可以通过 [`handler_program_context_t`](../handler_program_context_t.md) 取得相应类型。

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
        auto entry = context.add_program<givm::handler_program_context<givm::round_ended>::type>(
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
| [`handler_program_context_t`](../handler_program_context_t.md) | 事件响应所用的 context 类型 |
