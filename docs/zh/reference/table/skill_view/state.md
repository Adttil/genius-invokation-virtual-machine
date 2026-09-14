[givm](../../../reference.md) / [牌桌](../../table.md) / [skill_view](../skill_view.md) / **state**

# givm::skill_view::state

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr const skill_state& state() const;
```

访问该技能在对局中的当前状态。

## 返回值

实体持有的 [`skill_state`](../skill_state.md) 引用，只允许读取。

## 注意

实体必须有效。
