[givm](../../../reference.md) / [牌桌](../../table.md) / [skill_view](../skill_view.md) / **operator bool**

# givm::skill_view::operator bool

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr explicit operator bool() const;
```

判断这个技能是否仍然有效。

## 返回值

实体尚未移除时为 `true`，已经移除时为 `false`。

## 注意

访问对象本身必须仍可安全访问；本函数不能用来检查已经悬空的对象。
