[givm](../../reference.md) / [定义](../definition.md) / **event_category**

# givm::event_category

定义于头文件 `<givm/definition.hpp>`。

```cpp
enum class event_category : std::uint8_t { normal, immediate, preview };
```

指定事件如何调用响应及处理后续效果。每个事件通过类内 `static constexpr event_category category` 指定类别；类别不限制告知哪些实体或采用什么顺序。

| 值 | 响应与结算方式 |
| --- | --- |
| `normal` | 事件可进入结算队列，也可按触发流程直接告知，响应返回 `normal_effect`，每次效果执行拥有独立结算域并在返回前完成结算 |
| `immediate` | 事件在当前操作中立即处理，响应返回 `immediate_effect`，效果产生的记录归入外层段，不允许分段或结算 |
| `preview` | 事件在报价时处理，响应不接收编号，返回的 `preview_effect` 保留到确认操作后独立执行和结算 |

普通与立即响应从编号 0 开始，效果返回非空编号时再调用同一响应者。预览响应仅调用一次，上下文没有随机数接口；稍后执行的预览效果仍可使用正常随机源。

效果入口见 [`effect<Category>`](effect.md)，登记方法见 [`add_effect`](../executor/definition_compile_context/add_effect.md)。
