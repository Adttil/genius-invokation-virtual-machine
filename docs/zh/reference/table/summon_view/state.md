[givm](../../../reference.md) / [牌桌](../../table.md) / [summon_view](../summon_view.md) / **state**

# givm::summon_view::state

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr const summon_state& state() const;
```

访问该召唤物在对局中的当前状态。

## 返回值

实体持有的 [`summon_state`](../summon_state.md) 引用，只允许读取。

## 注意

实体必须有效。
