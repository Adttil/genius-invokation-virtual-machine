[givm](../../../reference.md) / [执行](../../executor.md) / [definition_compile_context](../definition_compile_context.md) / **operator[]**

# givm::definition_compile_context::operator[]

定义于头文件 `<givm/source.hpp>`

```cpp
template<class TCategory>
definition_view<TCategory> operator[](definition_id<TCategory> id) const;
```

按已有定义 ID 查看其名称、标签和能力信息。

## 模板参数

| | |
| --- | --- |
| `TCategory` | 由 ID 确定的定义类别 |

## 参数

| | |
| --- | --- |
| `id` | 待查询的定义 ID |

## 返回值

对应的 [`definition_view<TCategory>`](definition_view.md)，仅在本次编译期间有效。

输入为空 ID 或其数值超出本次类别的定义数量时，记录 [`definition_metadata_error`](../definition_metadata_error.md) 并返回空元数据视图。该视图的 `id()` 无效，`name()`、`tags()`、`dependencies<T>()` 为空，`has_tag()`、`can_handle<...>()` 和 `has_query<...>()` 均返回 `false`。

## 注意

有效 ID 须属于本次编译集合；此检查只能确认数值范围，不能识别另一个定义库发放的相同数值 ID。错误最终通过 [`compile`](../compile.md) 的返回值报告。
