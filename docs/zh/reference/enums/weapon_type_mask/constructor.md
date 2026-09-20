[givm](../../../reference.md) / [枚举值](../../enums.md) / [weapon_type_mask](../weapon_type_mask.md) / **weapon_type_mask**

# givm::weapon_type_mask::weapon_type_mask

定义于头文件 `<givm/enums/weapon_type.hpp>`

```cpp
constexpr weapon_type_mask() noexcept;
constexpr weapon_type_mask(weapon_type type) noexcept;
```

构造不允许任何武器的空集合，或只允许指定武器类别的集合。

## 参数

| 名称 | 说明 |
| --- | --- |
| `type` | 武器类别，必须为 sword、claymore、polearm、bow 或 catalyst |

## 注意

传入 `weapon_type::none` 或其他非有效类别的值属于未定义行为。

单类型构造允许隐式转换，因此可以直接用有效武器枚举初始化集合，也可以让集合运算接受武器枚举。
