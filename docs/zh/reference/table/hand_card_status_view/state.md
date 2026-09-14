[givm](../../../reference.md) / [牌桌](../../table.md) / [hand_card_status_view](../hand_card_status_view.md) / **state**

# givm::hand_card_status_view::state

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr const status_state& state() const;
```

访问该手牌上的状态在对局中的当前状态。

## 返回值

实体持有的 [`status_state`](../status_state.md) 引用，只允许读取。

## 注意

实体必须有效。
