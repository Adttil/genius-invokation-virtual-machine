[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action>](../action.md) / **request_cost**

# givm::execution_view<execution_state::action>::request_cost

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr void request_cost(stack_count_t index) const noexcept;
```
[`stack_count_t`](../../../utils/stack_count_t.md)

请求计算指定候选的费用。

## 参数

| | |
| --- | --- |
| `index` | 有效的切换候选下标。 |

## 返回值

(无)

## 注意

本操作只填写请求。下一次推进计算相应费用并再次返回 action；其他候选的缓存保留。
