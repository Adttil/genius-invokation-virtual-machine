[givm](../../../reference.md) / [执行](../../executor.md) / [definition_compile_context](../definition_compile_context.md) / **operator[]**

# givm::definition_compile_context::operator[]

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TCategory>
definition_view<TCategory> operator[](definition_id<TCategory> id) const noexcept;
```

按已有定义 ID 查看其名称、标签和能力信息。

## 模板参数

| | |
| --- | --- |
| `TCategory` | 由 ID 确定的定义类别 |

## 参数

| | |
| --- | --- |
| `id` | 本次编译集合中的有效定义 ID |

## 返回值

对应的 [`definition_view<TCategory>`](definition_view.md)，仅在本次编译期间有效。

## 注意

ID 必须属于本次编译，不能使用空 ID 或其他定义库发放的 ID。不检查输入是否有效。
