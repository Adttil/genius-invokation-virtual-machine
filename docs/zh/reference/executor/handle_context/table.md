[givm](../../../reference.md) / [执行](../../executor.md) / [handle_context](../handle_context.md) / **table**

# givm::handle_context::table

定义于头文件 `<givm/executor.hpp>`

```cpp
const givm::table& table() const noexcept;
```

读取当前事件发生的牌桌。

## 返回值

当前牌桌的只读引用。响应可以据此计算事件调整与后续效果的输入；对牌桌的修改由提交的命令执行。
