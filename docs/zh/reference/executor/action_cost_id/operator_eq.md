[givm](../../../reference.md) / [执行](../../executor.md) / [action_cost_id](../action_cost_id.md) / **operator==**

# givm::operator==(action_cost_id, action_cost_id)

```cpp
friend bool operator==(action_cost_id, action_cost_id) = default;
```

比较同一窗口缓存中的两个同类报价标识是否相同，不读取报价或检查窗口有效性。不同窗口或各自新增报价的副本之间不比较标识。

## 返回值

相同返回 `true`，否则返回 `false`。不同类别的报价标识不能比较。
