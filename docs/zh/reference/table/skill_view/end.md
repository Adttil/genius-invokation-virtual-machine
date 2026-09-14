[givm](../../../reference.md) / [牌桌](../../table.md) / [skill_view](../skill_view.md) / **end**

# givm::skill_view::end

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr const skill_view* end() const;
```

取得这个技能的单实体范围终点。

## 返回值

实体有效时为单实体范围的尾后指针，否则与 `begin()` 相等。
