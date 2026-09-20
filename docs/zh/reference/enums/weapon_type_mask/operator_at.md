[givm](../../../reference.md) / [枚举值](../../enums.md) / [weapon_type_mask](../weapon_type_mask.md) / **operator[]**

# givm::weapon_type_mask::operator[]

定义于头文件 `<givm/enums/weapon_type.hpp>`

```cpp
constexpr bool operator[](weapon_type type) const noexcept;
```

读取指定武器类别是否允许装备。

## 参数

| 名称 | 说明 |
| --- | --- |
| `type` | 要访问的武器类别，必须为 sword、claymore、polearm、bow 或 catalyst |

## 返回值

bool 值，表示是否允许装备该类别。

## 注意

传入 `weapon_type::none` 或其他非有效类别的值属于未定义行为。
