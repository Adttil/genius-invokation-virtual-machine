[givm](../../../reference.md) / [牌桌](../../table.md) / [support_view](../support_view.md) / **state**

# givm::support_view::state

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr const support_state& state() const;
```

访问该支援在对局中的当前状态。

## 返回值

实体持有的 [`support_state`](../support_state.md) 引用，只允许读取。

## 注意

实体必须有效。
