[givm](../../../reference.md) / [执行](../../executor.md) / [definition_library](../definition_library.md) / **remove_at_zero_usages**

# givm::definition_library::remove_at_zero_usages

定义于头文件 `<givm/executor.hpp>`

```cpp
bool remove_at_zero_usages(definition_id<summon_view> id) const noexcept;
```

检查召唤物定义是否采用可用次数耗尽后离场的规则。

## 参数

| | |
| --- | --- |
| `id` | 本定义库中的有效召唤物定义 ID |

## 返回值

定义具有 `remove_at_zero_usages` 标签时返回 `true`，否则返回 `false`。

## 注意

本函数只查询定义的规则，不读取具体召唤物的当前次数。[`modify_summon_state`](../../definition/commands/modify_summon_state.md) 用此规则处理次数归零后的离场；[`set_summon_state`](../../definition/commands/set_summon_state.md) 始终仅写入状态。强制移除使用 [`remove_summon`](../../definition/commands/remove_summon.md)，不受此标签限制。
