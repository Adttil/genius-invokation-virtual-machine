[givm](../../../reference.md) / [枚举值](../../enums.md) / [weapon_type_mask](../weapon_type_mask.md) / **operator~**

# givm::operator~

定义于头文件 `<givm/enums/weapon_type.hpp>`

```cpp
constexpr weapon_type_mask operator~(weapon_type_mask value) noexcept;
constexpr weapon_type_mask operator~(weapon_type type) noexcept;
```

返回只允许原集合所禁止类别的新集合。

## 参数

| 名称 | 说明 |
| --- | --- |
| `value` | 原集合 |
| `type` | 有效武器类别，按只含该类别的集合处理 |

## 注意

只反转五种可装备的武器类别，`weapon_type::none` 不计入其中。运算不修改实参。
