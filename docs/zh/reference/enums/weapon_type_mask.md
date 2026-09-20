[givm](../../reference.md) / [枚举值](../enums.md) / **weapon_type_mask**

# givm::weapon_type_mask

定义于头文件 `<givm/enums/weapon_type.hpp>`

```cpp
class weapon_type_mask;
```

角色允许装备的武器类别集合，可以同时允许多种类别。

## 成员函数

|  |  |
| --- | --- |
| [`(构造函数)`](weapon_type_mask/constructor.md) | 构造空集合或只含一种武器类别的集合 |
| [`operator[]`](weapon_type_mask/operator_at.md) | 读取一种武器类别是否允许装备 |
| [`all`](weapon_type_mask/all.md) | 是否允许全部武器类别 |
| [`any`](weapon_type_mask/any.md) | 是否允许至少一种武器类别 |
| [`none`](weapon_type_mask/none.md) | 是否不允许任何武器类别 |
| [`count`](weapon_type_mask/count.md) | 取得允许装备的类别数量 |
| [`size`](weapon_type_mask/size.md) | 取得可表示的武器类别总数 |
| [`set`](weapon_type_mask/set.md) | 允许全部类别，或设置一种类别是否允许装备 |
| [`reset`](weapon_type_mask/reset.md) | 禁止全部类别，或禁止一种类别 |
| [`flip`](weapon_type_mask/flip.md) | 反转全部类别或一种类别的允许状态 |
| [`operator\|=`](weapon_type_mask/operator_or_assign.md) | 合入另一个集合允许的类别 |
| [`operator&=`](weapon_type_mask/operator_and_assign.md) | 只保留两个集合都允许的类别 |
| [`operator^=`](weapon_type_mask/operator_xor_assign.md) | 只保留恰有一个集合允许的类别 |
| [`operator==`](weapon_type_mask/operator_equal.md) | 比较两个集合是否相同 |

## 非成员函数

|  |  |
| --- | --- |
| [`operator\|`](weapon_type_mask/operator_or.md) | 取得两个集合的并集 |
| [`operator&`](weapon_type_mask/operator_and.md) | 取得两个集合的交集 |
| [`operator^`](weapon_type_mask/operator_xor.md) | 取得两个集合的对称差集 |
| [`operator~`](weapon_type_mask/operator_not.md) | 取得原集合不允许的类别集合 |

## 注意

默认构造时所有类别均为 false，表示不能装备武器。`weapon_type::none` 不属于可装备的类别，也不计入 `size()` 或全体类别操作的范围。

## 参阅

|  |  |
| --- | --- |
| [`weapon_type`](weapon_type.md) | 武器类别 |
| [`character_state`](../table/character_state.md) | 角色的当前状态，包括允许装备的武器类别 |
