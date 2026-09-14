[givm](../../../reference.md) / [牌桌](../../table.md) / [support_view](../support_view.md) / **end**

# givm::support_view::end

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr const support_view* end() const;
```

取得这个支援的单实体范围终点。

## 返回值

实体有效时为单实体范围的尾后指针，否则与 `begin()` 相等。
