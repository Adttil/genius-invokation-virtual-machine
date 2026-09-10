[givm](../../../reference.md) / [执行](../../executor.md) / [execution_context](../execution_context.md) / **yield_next**

# givm::execution_context::yield_next

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr bool yield_next() noexcept;
```

结束本条指令并暂停，让调用方先处理观察或其他外部工作；下次从下一条指令继续。

## 返回值

`false`。
