[givm](../../../reference.md) / [执行](../../executor.md) / [execution_context](../execution_context.md) / **stack**

# givm::execution_context::stack

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr auto& stack(this auto& self) noexcept;
```

访问本次结算使用的临时数据。

## 模板参数

|  |  |
| --- | --- |
| `self` 的推导类型 | 调用对象类型，保留 cv 限定 |

## 参数

|  |  |
| --- | --- |
| `self` | 本次执行上下文 |

## 返回值

所持有的 [`frame_stack`](../../utils/frame_stack.md) 的引用，保留对象的 cv 限定。

## 注意

指令彻底结束时，应恢复进入前的栈形状；跨调用保存的数据由该指令自己管理。外部输入仅按相应指令约定的输入槽访问。
