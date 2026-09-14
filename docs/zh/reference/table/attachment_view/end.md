[givm](../../../reference.md) / [牌桌](../../table.md) / [attachment_view](../attachment_view.md) / **end**

# givm::attachment_view::end

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr const attachment_view* end() const;
```

取得这个角色附属实体的单实体范围终点。

## 返回值

实体有效时为单实体范围的尾后指针，否则与 `begin()` 相等。
