[givm](../../../../reference.md) / [执行](../../../executor.md) / [definition_compile_context](../../definition_compile_context.md) / [definition_view](../definition_view.md) / **can_handle**

# givm::definition_compile_context::definition_view::can_handle

定义于头文件 `<givm/source.hpp>`

```cpp
template<class TEvent, class TView = TCategory>
bool can_handle() const noexcept;
```

检查定义在指定实体形态下是否提供某个事件的响应，用于筛选能被使用或参与某类规则的定义。

## 模板参数

| | |
| --- | --- |
| `TEvent` | 要检查的事件类型 |
| `TView` | 要检查的实体 view，默认等于定义类别 `TCategory` |

## 返回值

为给定实体形态及事件启用了响应时返回 `true`。定义不支持该实体形态、实体形态不能订阅该事件，或者没有启用响应时返回 `false`。

## 注意

卡牌和卡牌状态的定义类别与实体 view 不同，调用时应显式指定形态，例如 `can_handle<givm::card_effect, givm::hand_card_view>()`。历史摘要使用自身的定义类别作为默认形态。

本函数只报告响应能力，不执行响应，也不保证某次实际事件满足效果触发条件。动态定义源是否启用响应由其能力声明决定。
