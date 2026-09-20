[givm](../../../reference.md) / [枚举值](../../enums.md) / [weapon_type_mask](../weapon_type_mask.md) / **flip**

# givm::weapon_type_mask::flip

定义于头文件 `<givm/enums/weapon_type.hpp>`

```cpp
constexpr weapon_type_mask& flip() noexcept;
constexpr weapon_type_mask& flip(weapon_type type) noexcept;
```

无参版本反转全部武器类别的允许状态；带参数版本只反转指定类别。原本允许的变为禁止，原本禁止的变为允许。

## 参数

| 名称 | 说明 |
| --- | --- |
| `type` | 武器类别，必须为 sword、claymore、polearm、bow 或 catalyst |

## 返回值

`*this`。

## 注意

传入 `weapon_type::none` 或其他非有效类别的值属于未定义行为。
