[givm](../../../reference.md) / [执行](../../executor.md) / [executor](../executor.md) / **view_in**

# givm::executor::view_in

定义于头文件 `<givm/executor.hpp>`

```cpp
template<execution_state State>
constexpr auto view_in() noexcept;
```
[`execution_state`](../execution_state.md)
[`execution_view`](../execution_view.md)

取得当前指定种类的执行现场视图。

`State` 必须与最近一次 [`run`](run.md) 或 [`step`](step.md) 返回的现场种类一致。调用方已知该条件时，可以直接取得视图；本函数不执行对局，也不改变现场。

## 模板参数

| | |
| --- | --- |
| `State` | 当前执行现场的种类 |

## 返回值

与 `State` 对应的现场访问对象，可用成员见相应[现场视图](../execution_view.md)的说明；纯通知现场返回空对象。调用方可用 `auto` 接收结果。

## 注意

有数据的视图借用当前执行器现场。开始下一次推进、重新进入、移动或替换执行器后，原有借用失效。复制视图仍借用原执行器；复制执行器后，应从副本重新取得视图。

即使下一次推进返回相同种类，先前借用的现场也不能继续访问。指定错误种类或访问已经失效的现场属于未定义行为。

## 参阅

| | |
| --- | --- |
| [`execution_view`](../execution_view.md) | 对局执行现场的视图 |
