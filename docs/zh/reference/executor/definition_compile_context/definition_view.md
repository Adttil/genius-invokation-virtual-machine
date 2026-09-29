[givm](../../../reference.md) / [执行](../../executor.md) / [definition_compile_context](../definition_compile_context.md) / **definition_view**

# givm::definition_compile_context::definition_view

定义于头文件 `<givm/definition_source.hpp>`

```cpp
template<class TCategory>
class definition_view;
```

本次编译集合中一项定义的元数据视图，提供名称、标签、名称依赖及响应和自定义查询的可用性。它用于构造效果配置，不执行查询或响应。

## 模板参数

| | |
| --- | --- |
| `TCategory` | 定义类别，见 [`definition_types`](../../definition/definition_types.md) |

## 成员函数

| | |
| --- | --- |
| [`id`](definition_view/id.md) | 取得本次编译的定义 ID |
| [`name`](definition_view/name.md) | 取得定义名称 |
| [`tags`](definition_view/tags.md) | 取得定义的标签名称范围 |
| [`dependencies`](definition_view/dependencies.md) | 取得指定类别的名称依赖 |
| [`has_tag`](definition_view/has_tag.md) | 按名称检查标签 |
| [`can_handle`](definition_view/can_handle.md) | 检查定义在指定实体形态下是否能响应事件 |
| [`has_query`](definition_view/has_query.md) | 检查定义是否提供自定义查询 |

## 注意

通过编译上下文的 [`definitions`](definitions.md)、[`find_definition`](find_definition.md) 或 [`operator[]`](operator_at.md) 取得。视图及 `tags()` 返回的借用范围只在本次编译期间有效；定义尚未执行 `compile` 也不妨碍查看其元数据。

需要保存筛选结果时，应保存定义 ID 或自行复制所需信息。进入对局后，使用 [`definition_library::definition_view`](../definition_library/definition_view.md) 查询编译结果，而不是保留本视图。
