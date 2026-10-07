[givm](../../../reference.md) / [牌桌](../../table.md) / [optional_tag_id](../optional_tag_id.md) / **operator==**

# givm::optional_tag_id::operator==

定义于头文件 `<givm/table.hpp>`

```cpp
friend constexpr bool operator==(optional_tag_id, optional_tag_id) noexcept = default;
```

两个空值相等；否则比较标签身份。
