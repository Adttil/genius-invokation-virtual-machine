[givm](../../../reference.md) / [枚举值](../../enums.md) / [weapon_type_mask](../weapon_type_mask.md) / **operator^=**

# givm::weapon_type_mask::operator^=

定义于头文件 `<givm/enums/weapon_type.hpp>`

```cpp
constexpr weapon_type_mask& operator^=(const weapon_type_mask& other) noexcept;
```

改为只允许当前集合和另一个集合中恰有一个允许的武器类别。

## 参数

| 名称 | 说明 |
| --- | --- |
| `other` | 参与运算的另一个集合，也可通过隐式转换传入一个有效武器枚举 |

## 返回值

`*this`。
