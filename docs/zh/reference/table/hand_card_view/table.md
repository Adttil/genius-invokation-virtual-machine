[givm](../../../reference.md) / [牌桌](../../table.md) / [hand_card_view](../hand_card_view.md) / **table**

# givm::hand_card_view::table

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr const givm::table& table() const noexcept;
```

取得这个实体所属的牌桌，以查看同一对局中的其他实体和当前局面。

## 返回值

所属 [`table`](../table.md) 的只读引用。

## 注意

视图和返回的引用都不拥有牌桌。存活要求见[实体的身份与访问](../entity_access.md)。
