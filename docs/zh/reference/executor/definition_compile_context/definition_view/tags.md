[givm](../../../../reference.md) / [执行](../../../executor.md) / [definition_compile_context](../../definition_compile_context.md) / [definition_view](../definition_view.md) / **tags**

# givm::definition_compile_context::definition_view::tags

定义于头文件 `<givm/definition_source.hpp>`

```cpp
std::span<const std::string_view> tags() const noexcept;
```

取得定义自身的标签名称，供编译时分类或筛选使用。

## 返回值

定义源 `tags()` 提供的标签名称范围；没有标签时为空。

## 注意

返回范围借用本次编译的元数据，仅在本次编译期间有效。单个标签名称也不拥有字符存储。
