[givm](../../../../reference.md) / [行动选择](../action_selection.md) / **technique_cost**

# technique_cost

定义于头文件 `<givm/executor.hpp>`

```cpp
const cost_of_technique& technique_cost() const noexcept(/* Release 为 true，Debug 为 false */);
```

读取已计算的特技报价，可以重复读取。前提为当前现场已经计算特技报价。

## 注意

Debug 下，视图不属于当前现场或已经失效时抛出 [`execution_view_error`](../../execution_view_error.md)；Release 保持 `noexcept` 且不检查这些条件。
