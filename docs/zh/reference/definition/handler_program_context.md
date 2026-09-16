[givm](../../reference.md) / [定义](../definition.md) / **handler_program_context**

# givm::handler_program_context

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TEvent>
struct handler_program_context
{
    using type = TEvent;
};

template<>
struct handler_program_context<cost_of_switch>;

template<>
struct handler_program_context<cost_of_card>;
```

事件类型与其响应效果所用 context 的映射。定义效果时应使用这层映射，让事件的语义限制与具体类型对应起来。

## 模板参数

|  |  |
| --- | --- |
| `TEvent` | 事件类型 |

## 成员类型

|  |  |
| --- | --- |
| [`type`](handler_program_context/type.md) | 该事件响应效果所用的 context |

## 注意

通常映射为事件本身；[`cost_of_switch`](events/cost_of_switch.md) 与 [`cost_of_card`](events/cost_of_card.md) 的响应效果在确认行动后执行，使用单独的响应语境。编写定义源时通过 [`handler_program_context_t`](handler_program_context_t.md) 取得相应类型，不需要依赖该语境的具体类型名。

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
