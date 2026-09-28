[givm](../../../reference.md) / [执行](../../executor.md) / [definition_library](../definition_library.md) / **definition_count**

# givm::definition_library::definition_count

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class T>
std::size_t definition_count() const noexcept;
```

取得定义库中指定类别的定义数量。

## 模板参数

|  |  |
| --- | --- |
| `T` | 定义类别，须为 [`definition_types`](../../definition/definition_types.md) 中的一种类型 |

## 返回值

本定义库中属于 `T` 类别的定义总数。

## 参阅

|  |  |
| --- | --- |
| [`operator[]`](operator_at.md) | 查看指定定义 |
| [`tag_count`](tag_count.md) | 取得标签数量 |
