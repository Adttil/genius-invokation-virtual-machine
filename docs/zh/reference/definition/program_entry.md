[givm](../../reference.md) / [定义](../definition.md) / **program_entry**

# givm::program_entry

定义于头文件 `<givm/definition.hpp>`

```cpp
class program_entry;
```

一段效果的入口。事件响应通过 `invoke` 选择接下来执行的效果；默认构造的空入口表示尚未登记效果。

## 成员函数

|  |  |
| --- | --- |
| [(构造函数)](program_entry/constructor.md) | 构造空入口 |
| [`null`](program_entry/null.md) | 取得空入口 |
| [`is_null`](program_entry/is_null.md) | 检查入口是否为空 |
| [`operator bool`](program_entry/operator_bool.md) | 检查入口是否非空 |

## 注意

非空入口由 [`definition_compile_context::add_program`](../executor/definition_compile_context/add_program.md) 产生，不能跨定义库使用。入口不绑定外层响应事件或实体类别；调用方须通过 [`handle_context::invoke`](../executor/handle_context/invoke.md) 提供完整匹配的输入。空入口表示“不进入任何后续效果”。需要结束对局的响应可将 [`end_game`](commands/end_game.md) 编入其程序。

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

    givm::program_entry compile(givm::definition_compile_context& context) const
    {
        auto entry = context.add_program(
            std::tuple{ givm::shuffle_deck{ .player = givm::player_id{ 0 } } }
        );
        std::println("已登记回合结束效果: {}", static_cast<bool>(entry));
        return entry;
    }

    static givm::program_entry handle(
        const givm::program_entry& entry,
        const givm::support_view&,
        givm::round_ended&,
        givm::handle_context& context)
    {
        return context.invoke(entry);
    }
};

int main()
{
    const support_source source{};
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0
    };
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
