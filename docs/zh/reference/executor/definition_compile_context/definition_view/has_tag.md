[givm](../../../../reference.md) / [执行](../../../executor.md) / [definition_compile_context](../../definition_compile_context.md) / [definition_view](../definition_view.md) / **has_tag**

# givm::definition_compile_context::definition_view::has_tag

定义于头文件 `<givm/source.hpp>`

```cpp
bool has_tag(std::string_view name) const noexcept;
```

按标签名称检查定义是否属于某个分类。

## 参数

| | |
| --- | --- |
| `name` | 要查找的标签名称 |

## 返回值

定义包含该标签时返回 `true`，否则返回 `false`。未知标签也返回 `false`。

## 注意

只匹配一个完整标签名称，不解析标签筛选表达式，也不需要标签依赖声明。
