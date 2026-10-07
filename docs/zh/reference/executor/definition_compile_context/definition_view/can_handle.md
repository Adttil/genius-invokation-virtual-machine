[givm](../../../../reference.md) / [执行](../../../executor.md) / [definition_compile_context](../../definition_compile_context.md) / [definition_view](../definition_view.md) / **can_handle**

# givm::definition_compile_context::definition_view::can_handle

定义于头文件 `<givm/definition_source.hpp>`

```cpp
template<class TEvent, entity_category Entity = /* 根据 TCategory 推导 */>
bool can_handle() const noexcept;
```

检查定义在指定实体形态下是否提供某个事件的响应，用于筛选能被使用或参与某类规则的定义。

## 模板参数

| | |
| --- | --- |
| `TEvent` | 要检查的事件类型 |
| `Entity` | 要检查的实体类别；定义只有一种实体形态时默认采用该形态，否则默认 `null` |

## 返回值

为给定实体形态及事件启用了响应时返回 `true`。定义不支持该实体形态、实体形态不能订阅该事件，或者没有启用响应时返回 `false`。

## 注意

卡牌和卡牌状态有两种实体形态，调用时应显式指定类别，例如 `can_handle<givm::this_card_play, givm::entity_category::hand_card>()`。历史摘要没有实体形态，默认的 `null` 参数用于查询摘要自身的订阅能力。

本函数只报告响应能力，不执行响应，也不保证某次实际事件满足效果触发条件。动态定义源是否启用响应由其能力声明决定。
