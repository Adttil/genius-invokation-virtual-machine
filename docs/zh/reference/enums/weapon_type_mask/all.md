[givm](../../../reference.md) / [枚举值](../../enums.md) / [weapon_type_mask](../weapon_type_mask.md) / **all**

# givm::weapon_type_mask::all

定义于头文件 `<givm/enums/weapon_type.hpp>`

```cpp
constexpr bool all() const noexcept;
```

所有武器类别都允许装备时返回 true。

## 注意

只考虑 sword、claymore、polearm、bow 和 catalyst；`weapon_type::none` 不计入其中。
