[givm](../../../reference.md) / [牌桌](../../table.md) / [combat_status_view](../combat_status_view.md) / **state**

# givm::combat_status_view::state

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr const combat_status_state& state() const;
```

访问该出战状态在对局中的当前状态。

## 返回值

实体持有的 [`combat_status_state`](../combat_status_state.md) 引用，只允许读取。

## 注意

实体须曾存在于此处，且未转移、未清理；删除后仍可读取本项信息。访问对象与 ID 的保存期限见[实体的身份与访问](../entity_access.md)。
