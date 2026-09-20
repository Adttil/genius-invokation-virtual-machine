[givm](../../../reference.md) / [枚举值](../../enums.md) / [weapon_type_mask](../weapon_type_mask.md) / **count**

# givm::weapon_type_mask::count

定义于头文件 `<givm/enums/weapon_type.hpp>`

```cpp
constexpr std::size_t count() const noexcept;
```

返回当前允许装备的武器类别数量。

## 注意

只考虑 sword、claymore、polearm、bow 和 catalyst；`weapon_type::none` 不计入其中。
