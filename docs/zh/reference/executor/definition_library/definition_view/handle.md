[givm](../../../../reference.md) / [执行](../../../executor.md) / [definition_library](../../definition_library.md) / [definition_view](../definition_view.md) / **handle**

# givm::definition_library::definition_view::handle

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TEvent, class TView>
program_entry handle(const TView& entity, TEvent& event, handle_context& context) const;
```

请求该定义为一个实体响应当前事件。响应通过提供的调用对象提交后续效果。

## 模板参数

| | |
| --- | --- |
| `TEvent` | 当前事件类型 |
| `TView` | 该定义类别对应的只读实体 view |

## 参数

| | |
| --- | --- |
| `entity` | 响应事件的实体，只读 view 须属于该定义类别 |
| `event` | 要响应的事件，可修改的成员用于反馈本次事件的调整 |
| `context` | 执行器提供的 [`handle_context`](../../handle_context.md)，用于读取配套牌桌、取得随机值及提交后续效果 |

## 返回值

响应返回的后续效果入口；没有后续效果时为空入口。

## 注意

[`can_handle<TEvent, TView>()`](can_handle.md) 须为 `true`。实体、牌桌与定义视图须使用配套定义库，响应者遵守[定义源协议](../../../definition/source_protocol.md)中的调用与输入约定。
