[givm](../../../reference.md) / [执行](../../executor.md) / [handle_context](../handle_context.md) / **random**

# givm::handle_context::random

定义于头文件 `<givm/definition_source.hpp>`

```cpp
std::uint32_t random() const requires (Category != event_category::preview);
```

从本次推进使用的随机源取得下一个值。

## 返回值

随机源本次产生的值，转换为 `std::uint32_t`。

## 注意

调用会使用原随机源，即使上下文本身为 const，也不会另建随机序列。随机源抛出的异常会传递给调用方。

预览上下文没有此成员可用，编写费用报价响应时不能取得随机值。后续预览效果程序实际执行时仍使用正常随机源。
