[givm](../../../reference.md) / [执行](../../executor.md) / [handle_context](../handle_context.md) / **random**

# givm::handle_context::random

定义于头文件 `<givm/executor.hpp>`

```cpp
std::uint32_t random() const;
```

从本次推进使用的随机源取得下一个值。

## 返回值

随机源本次产生的值，转换为 `std::uint32_t`。

## 注意

调用会使用原随机源，即使上下文本身为 const，也不会另建随机序列。随机源抛出的异常会传递给调用方。

[`cost_of_switch`](../../definition/events/cost_of_switch.md)、[`cost_of_card`](../../definition/events/cost_of_card.md) 与 [`cost_of_skill`](../../definition/events/cost_of_skill.md) 费用响应不得使用随机数；在这些响应中调用本函数属于未定义行为，不进行运行期检查。
