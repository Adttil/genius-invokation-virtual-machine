[givm](../../../reference.md) / [枚举值](../../enums.md) / [weapon_type_mask](../weapon_type_mask.md) / **set**

# givm::weapon_type_mask::set

定义于头文件 `<givm/enums/weapon_type.hpp>`

```cpp
constexpr weapon_type_mask& set() noexcept;
constexpr weapon_type_mask& set(weapon_type type, bool value = true) noexcept;
```

无参版本允许装备全部武器类别；带参数版本设置指定武器类别是否允许装备。

## 参数

| 名称 | 说明 |
| --- | --- |
| `type` | 要修改的武器类别，必须为 sword、claymore、polearm、bow 或 catalyst |
| `value` | true 表示允许，false 表示禁止；默认为 true |

## 返回值

`*this`。

## 注意

传入 `weapon_type::none` 或其他非有效类别的值属于未定义行为。
