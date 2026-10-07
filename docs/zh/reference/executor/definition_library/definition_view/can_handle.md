[givm](../../../../reference.md) / [执行](../../../executor.md) / [definition_library](../../definition_library.md) / [definition_view](../definition_view.md) / **can_handle**

# givm::definition_library::definition_view::can_handle

定义于头文件 `<givm/runtime.hpp>`

```cpp
template<class TEvent, entity_category Entity>
bool can_handle() const noexcept;
```

检查该定义在给定实体形态下是否提供某个事件的响应。

## 模板参数

|  |  |
| --- | --- |
| `TEvent` | 属于该 view 可订阅范围的事件类型 |
| `Entity` | 属于该定义类别的实体类别 |

## 返回值

为该 view 和事件启用了响应函数时返回 `true`，否则返回 `false`。

## 注意

返回 `true` 不代表本次事件一定触发效果，响应函数仍可按事件条件决定是否生效。

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
    static constexpr auto category = givm::definition_category::support;

    std::string_view name() const { return "重投助手"; }
    int compile(givm::definition_compile_context&) const { return 1; }

    static givm::immediate_effect handle(
        const int& extra_rerolls,
        givm::dice_roll_preparation& event,
        givm::handle_context<givm::support_view, givm::event_category::immediate>& context, std::uint32_t = 0)
    {
        event.reroll_count[0] += extra_rerolls;
        return {};
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
    const auto id = ids.get_id<givm::definition_category::support>("重投助手");
    const auto definition = library[id];
    std::println("响应掷骰准备: {}", definition.can_handle<givm::dice_roll_preparation, givm::entity_category::support>());
    std::println("响应回合结束: {}", definition.can_handle<givm::round_ended, givm::entity_category::support>());
}
```

输出

```text
响应掷骰准备: true
响应回合结束: false
```
