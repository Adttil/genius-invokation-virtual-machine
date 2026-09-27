[givm](../../../reference.md) / [定义](../../definition.md) / [source_conflict](../source_conflict.md) / **reason**

# givm::source_conflict::reason

定义于头文件 `<givm/definition.hpp>`

```cpp
enum class source_conflict::reason
{
    different_object,
    different_type
};
```

同类别同名的两个定义源无法视为同一源的原因。

## 枚举值

| | |
| --- | --- |
| `different_object` | C++ 类型相同，但源对象不同 |
| `different_type` | C++ 类型不同，优先于对象差异报告 |
