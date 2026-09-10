[givm](../../../reference.md) / [执行](../../executor.md) / [execution_context](../execution_context.md) / **current_stage**

# givm::execution_context::current_stage

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr stage_t& current_stage() noexcept;
```

访问本条指令用于记录自身进度的值，以便在重新进入时继续此前的结算。

## 返回值

本条指令的进度值引用，类型为 [`stage_t`](../stage_t.md)。

## 注意

新进入的指令从零开始；推进到下一条指令时重置为零。数值含义由该指令自己决定，不构成相邻指令之间的数据协议。调用时须处于该指令自己的执行帧。
