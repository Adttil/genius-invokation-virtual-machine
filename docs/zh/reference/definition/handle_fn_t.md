[givm](../../reference.md) / [定义](../definition.md) / **handle_fn_t**

# givm::handle_fn_t

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TEntity, class TEvent>
using handle_fn_t = handler_program_entry_t<TEvent> (*)(
    const definition_data&,
    const TEntity&,
    TEvent&,
    const card_table&,
    random_fn&
);
```

统一调用某类实体对某个事件的响应函数所用的函数指针类型。它携带已编译定义、响应实体、当前事件、牌桌和随机源，返回后续效果的入口。

## 模板参数

|  |  |
| --- | --- |
| `TEntity` | 响应实体的只读 view |
| `TEvent` | 事件类型 |

## 注意

编写普通定义源时，静态 `handle` 的第一个形参直接使用该源编译所得类型，不必手动接收 [`definition_data`](definition_data.md)。

## 示例

```cpp
#include <any>
#include <cstdint>
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
        const givm::card_table&,
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
    const auto [library, ids] = sources.compile(
        std::tuple{}, std::tuple{ givm::start_round{} }
    );
    const auto id = ids.get_id<givm::support_view>("重投助手");

    givm::card_table table{ library };
    const givm::support_view entity = table[givm::player_id{ 0 }].add(id, { .count = 1 });
    auto random_source = []() -> std::uint32_t { return 0; };
    givm::random_fn random{ random_source };
    givm::dice_roll_preparation event{ .count = 8 };
    const givm::definition_data data{ 2 };
    const givm::handle_fn_t<givm::support_view, givm::dice_roll_preparation> handler = +[](
        const givm::definition_data& data,
        const givm::support_view& entity,
        givm::dice_roll_preparation& event,
        const givm::card_table& table,
        givm::random_fn& random
    )
    {
        return support_source::handle(std::any_cast<const int&>(data), entity, event, table, random);
    };
    const auto entry = handler(data, entity, event, table, random);
    std::println("玩家 0 重投次数: {}", event.reroll_count[0]);
    std::println("无需额外结算: {}", entry.is_null());
}
```

输出

```text
玩家 0 重投次数: 3
无需额外结算: true
```
