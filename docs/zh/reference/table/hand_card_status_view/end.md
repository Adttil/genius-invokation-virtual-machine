[givm](../../../reference.md) / [牌桌](../../table.md) / [hand_card_status_view](../hand_card_status_view.md) / **end**

# givm::hand_card_status_view::end

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr const hand_card_status_view* end() const;
```

取得这个手牌上的状态的单实体范围终点。

## 返回值

实体有效时为单实体范围的尾后指针，否则与 `begin()` 相等。
