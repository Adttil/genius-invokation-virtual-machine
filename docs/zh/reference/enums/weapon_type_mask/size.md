[givm](../../../reference.md) / [枚举值](../../enums.md) / [weapon_type_mask](../weapon_type_mask.md) / **size**

# givm::weapon_type_mask::size

定义于头文件 `<givm/enums/weapon_type.hpp>`

```cpp
constexpr std::size_t size() const noexcept;
```

返回可表示的武器类别总数，目前为 5。

## 注意

只考虑 sword、claymore、polearm、bow 和 catalyst；`weapon_type::none` 不计入其中。
