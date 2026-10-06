[givm](../../reference.md) / [定义](../definition.md) / **effect**

# givm::effect

定义于头文件 `<givm/definition.hpp>`

```cpp
template<event_category Category>
class effect;

using normal_effect = effect<event_category::normal>;
using immediate_effect = effect<event_category::immediate>;
using preview_effect = effect<event_category::preview>;
```

一段效果的强类型入口，类别与响应事件一致。普通效果完成独立结算，立即效果产生的记录归入外层段，预览效果在确认操作后执行。默认构造表示没有后续效果。

## 成员函数

|  |  |
| --- | --- |
| [(构造函数)](effect/constructor.md) | 构造空入口 |
| [`null`](effect/null.md) | 取得空入口 |
| [`is_null`](effect/is_null.md) | 检查入口是否为空 |
| [`operator bool`](effect/operator_bool.md) | 检查入口是否非空 |

## 注意

非空入口由 [`definition_compile_context::add_normal_effect`](../executor/definition_compile_context/add_effect.md) 产生，可以用于该定义库及其副本，不能用于另一份独立编译的定义库。入口通过 [`event_category`](event_category.md) 区分类别，同类别入口可以用于不同事件与实体；调用方须通过 [`handle_context::invoke`](../executor/handle_context/invoke.md) 提供完整匹配的输入。空入口表示“不进入任何后续效果”。需要结束对局的响应可将 [`end_game`](commands/end_game.md) 编入其程序。

## 非成员函数

|  |  |
| --- | --- |
| [`operator==`](effect/operator_eq.md) | 比较两个入口 |

## 示例

```cpp
#include <cstdint>
#include <utility>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct support_source
{
    using definition_category = givm::support_view;

    std::string_view name() const { return "洗牌助手"; }

    givm::normal_effect compile(givm::definition_compile_context& context) const
    {
        auto entry = context.add_normal_effect(
            std::tuple{ givm::shuffle_deck{ .player = givm::player_id{ 0 } } }
        );
        std::println("已登记回合结束效果: {}", static_cast<bool>(entry));
        return entry;
    }

    static givm::normal_effect handle(
        const givm::normal_effect& entry,
        givm::round_ended&,
        givm::handle_context<givm::support_view>& context, std::uint32_t = 0)
    {
        return context.invoke(entry);
    }
};

int main()
{
    const support_source source{};
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    if(not sources.add(source)) return 1;
    auto library_result = compile(
        sources, basics,
        std::tuple{}, std::tuple{ givm::start_round{}, givm::settle{} }, givm::compile_mode::normal
    );
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
}
```

输出

```text
已登记回合结束效果: true
```
