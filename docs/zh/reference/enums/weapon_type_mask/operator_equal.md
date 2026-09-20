[givm](../../../reference.md) / [枚举值](../../enums.md) / [weapon_type_mask](../weapon_type_mask.md) / **operator==**

# givm::weapon_type_mask::operator==

定义于头文件 `<givm/enums/weapon_type.hpp>`

```cpp
constexpr bool operator==(const weapon_type_mask&) const noexcept = default;
```

比较两个集合允许装备的武器类别是否完全相同。

## 返回值

集合相同返回 true，否则返回 false。也可以使用 `!=` 判断是否不同。
