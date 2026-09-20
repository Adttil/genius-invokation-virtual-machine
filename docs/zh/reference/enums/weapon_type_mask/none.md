[givm](../../../reference.md) / [枚举值](../../enums.md) / [weapon_type_mask](../weapon_type_mask.md) / **none**

# givm::weapon_type_mask::none

定义于头文件 `<givm/enums/weapon_type.hpp>`

```cpp
constexpr bool none() const noexcept;
```

没有任何武器类别允许装备时返回 true。

## 注意

只考虑 sword、claymore、polearm、bow 和 catalyst；`weapon_type::none` 不计入其中。
