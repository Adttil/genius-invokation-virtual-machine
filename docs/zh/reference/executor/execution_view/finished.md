[givm](../../../reference.md) / [执行](../../executor.md) / [execution_view](../execution_view.md) / **execution_view<finished>**

# givm::execution_view<execution_state::finished>

定义于头文件 `<givm/executor.hpp>`

```cpp
template<>
class execution_view<execution_state::finished>;
```

已结束对局的结果视图。

## 成员函数

| | |
| --- | --- |
| [`result`](finished/result.md) | 取得本场对局的结果。 |

## 参阅

| | |
| --- | --- |
| [`executor::view_in`](../executor/view_in.md) | 取得对应执行现场的视图 |
