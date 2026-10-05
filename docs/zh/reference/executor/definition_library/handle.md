[givm](../../../reference.md) / [执行](../../executor.md) / [definition_library](../definition_library.md) / **handle**

# givm::definition_library::handle

定义于头文件 `<givm/runtime.hpp>`

```cpp
template<class TEvent, class TDefinitionType, class TView>
program_entry handle(
    definition_id<TDefinitionType> id, TEvent& event,
    handle_context<TView>& context, std::uint32_t response_index = 0) const;
```

请求一个实体的定义响应当前事件。响应可以直接调整事件允许修改的内容，并通过提供的调用对象提交后续效果。

## 模板参数

| | |
| --- | --- |
| `TEvent` | 当前事件类型 |
| `TDefinitionType` | 由 ID 推导的定义类别 |
| `TView` | 该定义类别对应的只读实体 view |

## 参数

| | |
| --- | --- |
| `id` | 本定义库中的有效定义 ID |
| `event` | 要响应的事件，可修改的成员用于反馈本次事件的调整 |
| `context` | 执行器提供的 [`handle_context<TView>`](../handle_context.md)，持有该定义类别对应的响应实体 |
| `response_index` | 本次响应的编号，首轮为 0 |

## 返回值

响应返回的后续效果入口；没有后续效果时为空入口。

## 注意

对应的 [`can_handle<TEvent, TView>`](can_handle.md) 必须为 `true`。通常由命令触发事件分发；响应者遵守[定义源协议](../../definition/source_protocol.md)中的调用与输入约定。
