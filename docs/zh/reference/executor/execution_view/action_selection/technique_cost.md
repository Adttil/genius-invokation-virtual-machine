[givm](../../../../reference.md) / [行动选择](../action_selection.md) / **technique_cost**

# technique_cost

定义于头文件 `<givm/executor.hpp>`

```cpp
const cost_of_technique& technique_cost() const noexcept;
```

读取已计算的特技报价，可以重复读取。前提为当前现场已经计算特技报价。
