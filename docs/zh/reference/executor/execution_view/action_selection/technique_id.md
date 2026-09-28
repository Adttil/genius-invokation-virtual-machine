[givm](../../../../reference.md) / [行动选择](../action_selection.md) / **technique_id**

# technique_id

定义于头文件 `<givm/executor.hpp>`

```cpp
attachment_id technique_id() const noexcept(/* Release 为 true，Debug 为 false */);
```

返回当前特技装备的实体 ID。前提为 `has_technique()` 返回 true。

## 注意

Debug 下，视图不属于当前现场或已经失效时抛出 [`execution_view_error`](../../execution_view_error.md)；Release 保持 `noexcept` 且不检查这些条件。
