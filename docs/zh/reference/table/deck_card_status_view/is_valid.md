[givm](../../../reference.md) / [牌桌](../../table.md) / [deck_card_status_view](../deck_card_status_view.md) / **is_valid**

# givm::deck_card_status_view::is_valid

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr bool is_valid() const;
```

判断这个牌库卡牌上的状态是否仍然有效。

## 返回值

实体尚未移除时为 `true`，已经移除时为 `false`。

## 注意

访问对象本身必须仍可安全访问；本函数不能用来检查已经悬空的对象。
