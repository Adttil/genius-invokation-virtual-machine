[givm](../../../../reference.md) / [执行](../../../executor.md) / [definition_compile_context](../../definition_compile_context.md) / [definition_view](../definition_view.md) / **dependencies**

# givm::definition_compile_context::definition_view::dependencies

定义于头文件 `<givm/definition_source.hpp>`

```cpp
template<class TDependencyCategory>
std::span<const std::string_view> dependencies() const noexcept;
```

查看这项定义在指定类别中声明的名称依赖。

## 模板参数

| | |
| --- | --- |
| `TDependencyCategory` | 所查询依赖的定义类别 |

## 返回值

该定义源通过相应 `*_dependencies()` 声明的名称范围；未声明该类别的依赖时为空。

## 注意

本函数读取被查看定义自己的声明，不改变当前编译源的依赖，也不会授权当前源通过 [`resolve_id`](../resolve_id.md) 解析这些名称。要查看本次集合中名称对应的元数据，可使用 [`find_definition`](../find_definition.md)。

返回范围仅在本次编译期间有效，名称字符存储遵守[定义源生命周期](../../../definition/source_protocol.md)约定。
