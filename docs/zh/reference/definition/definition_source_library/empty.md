[givm](../../../reference.md) / [定义](../../definition.md) / [definition_source_library](../definition_source_library.md) / **empty**

# givm::definition_source_library::empty

定义于头文件 `<givm/definition_source_interface.hpp>`

```cpp
bool empty() const noexcept;
```

检查源库是否为空。

## 返回值

所有定义类别均为空时返回 `true`，否则返回 `false`。

## 注意

标签随定义源保存，没有独立的标签登记；空源库也不包含标签。此查询不访问原始源对象，也不修改源库。
