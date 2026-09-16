[givm](../../../reference.md) / [牌桌](../../table.md) / [deck_card_view](../deck_card_view.md) / **state**

# givm::deck_card_view::state

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr const card_state& state() const;
```

访问该牌库卡牌在对局中的当前状态。

## 返回值

实体持有的 [`card_state`](../card_state.md) 引用，只允许读取。

## 注意

实体须曾存在于此处，且未转移、未清理；删除后仍可读取本项信息。访问对象与 ID 的保存期限见[实体的身份与访问](../entity_access.md)。
