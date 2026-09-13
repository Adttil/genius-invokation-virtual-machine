[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action>](../action.md) / **costs**

# givm::execution_view<execution_state::action>::costs

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr std::span<const cost_of_switch> costs() const noexcept;
```
[`cost_of_switch`](../../events/cost_of_switch.md)

取得当前切换候选的费用。

## 返回值

借用当前现场的只读费用范围，下标与行动候选一致。
