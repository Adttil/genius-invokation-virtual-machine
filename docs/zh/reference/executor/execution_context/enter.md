[givm](../../../reference.md) / [执行](../../executor.md) / [execution_context](../execution_context.md) / **enter**

# givm::execution_context::enter

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TContext>
constexpr bool enter(program_entry<TContext> entry);
```

进入另一段效果。该段效果返回后，会重新进入发起调用的指令，由它继续自己的结算。

## 模板参数

|  |  |
| --- | --- |
| `TContext` | 被进入效果的 context 类型 |

## 参数

|  |  |
| --- | --- |
| `entry` | 有效的程序入口，不能是空入口 |

## 返回值

`true`。

## 注意

入口须属于与本次执行配套的定义库，且调用方须满足该段效果所要求的 context。[`program_entry`](../../definition/program_entry.md) 的终局入口可用于直接结束对局。
