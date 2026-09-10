[givm](../../../reference.md) / [执行](../../executor.md) / [execution_context](../execution_context.md) / **yield**

# givm::execution_context::yield

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr bool yield() noexcept;
```

暂停自动推进，保留本条指令以便下次重新执行。

## 返回值

`false`。
