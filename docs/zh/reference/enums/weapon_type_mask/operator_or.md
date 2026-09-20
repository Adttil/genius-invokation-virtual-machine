[givm](../../../reference.md) / [枚举值](../../enums.md) / [weapon_type_mask](../weapon_type_mask.md) / **operator|**

# givm::operator|

定义于头文件 `<givm/enums/weapon_type.hpp>`

```cpp
constexpr weapon_type_mask operator|(weapon_type_mask lhs, const weapon_type_mask& rhs) noexcept;
constexpr weapon_type_mask operator|(weapon_type lhs, weapon_type rhs) noexcept;
```

返回允许任一集合所允许类别的新集合。

## 参数

| 名称 | 说明 |
| --- | --- |
| `lhs` | 左侧集合 |
| `rhs` | 右侧集合 |

## 注意

支持两个集合、两个有效武器枚举，或一个集合和一个有效武器枚举。枚举表示只含该类别的集合；混合运算通过隐式转换处理。运算不修改实参。
