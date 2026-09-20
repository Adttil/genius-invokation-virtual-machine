[givm](../../../reference.md) / [枚举值](../../enums.md) / [weapon_type_mask](../weapon_type_mask.md) / **reset**

# givm::weapon_type_mask::reset

定义于头文件 `<givm/enums/weapon_type.hpp>`

```cpp
constexpr weapon_type_mask& reset() noexcept;
constexpr weapon_type_mask& reset(weapon_type type) noexcept;
```

无参版本禁止装备全部武器类别；带参数版本禁止装备指定武器类别。

## 参数

| 名称 | 说明 |
| --- | --- |
| `type` | 要修改的武器类别，必须为 sword、claymore、polearm、bow 或 catalyst |

## 返回值

`*this`。

## 注意

传入 `weapon_type::none` 或其他非有效类别的值属于未定义行为。
