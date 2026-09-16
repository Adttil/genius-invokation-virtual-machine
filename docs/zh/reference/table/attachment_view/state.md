[givm](../../../reference.md) / [牌桌](../../table.md) / [attachment_view](../attachment_view.md) / **state**

# givm::attachment_view::state

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr const attachment_state& state() const;
```

访问该角色附属实体在对局中的当前状态。

## 返回值

实体持有的 [`attachment_state`](../attachment_state.md) 引用，只允许读取。

## 注意

实体须曾存在于此处，且未转移、未清理；删除后仍可读取本项信息。访问对象与 ID 的保存期限见[实体的身份与访问](../entity_access.md)。
