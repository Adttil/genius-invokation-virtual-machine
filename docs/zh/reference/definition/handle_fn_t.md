[givm](../../reference.md) / [定义](../definition.md) / **handle_fn_t**

# givm::handle_fn_t

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TEntity, class TEvent>
using handle_fn_t = /* 由 TEvent::category 选择的函数指针类型 */;
```

统一调用某类实体对某个事件的响应函数所用的函数指针类型。它接收已编译定义、当前事件及对应类别的 [`handle_context`](../executor/handle_context.md)。普通与立即响应还接收 `std::uint32_t` 响应编号；预览响应没有该形参。通过 context 取得响应实体、读取牌桌、取得随机值和提交后续效果。

## 模板参数

| | |
| --- | --- |
| `TEntity` | 响应实体的只读 view |
| `TEvent` | 事件类型 |

## 注意

普通与立即响应的编号使用 `std::uint32_t`。函数指针类型不提供默认参数，通过此类型调用时须显式传入编号；首轮响应的编号为 0。

编写普通定义源时，静态 `handle` 的第一个形参直接使用该源编译所得类型，不必手动接收 [`definition_data`](definition_data.md)。响应函数返回 `effect<TEvent::category>`；没有后续效果时返回空入口，需要后续效果时，以 `return context.invoke(entry, inputs...);` 结束响应。完整约定见[定义源协议](source_protocol.md)。
