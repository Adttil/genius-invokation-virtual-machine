[givm](../../reference.md) / [枚举值](../enums.md) / **damage_type_mask**

# givm::damage_type_mask

定义于 `<givm/enums/damage_type.hpp>`。

记录一段中出现过的伤害类型。默认集合为空，按值使用位或合并；不记录各值的小计或先后次序。物理伤害和穿透伤害均占用自己的集合位。

## 成员和运算

| 名称 | 说明 |
| --- | --- |
| [`构造函数`](damage_type_mask/constructor.md) | 构造空集合或只含一个值的集合 |
| [`operator[]`](damage_type_mask/operator_at.md) | 查询指定值是否出现 |
| [`all`](damage_type_mask/all.md) | 查询是否包含全部值 |
| [`any`](damage_type_mask/any.md) | 查询是否至少包含一个值 |
| [`none`](damage_type_mask/none.md) | 查询是否为空 |
| [`count`](damage_type_mask/count.md) | 取得集合中的值数量 |
| [`size`](damage_type_mask/size.md) | 取得可表示的值总数 10 |
| [`set`](damage_type_mask/set.md) | 设置全体或指定值 |
| [`reset`](damage_type_mask/reset.md) | 清除全体或指定值 |
| [`flip`](damage_type_mask/flip.md) | 反转全体或指定值 |
| [`operator\|=`](damage_type_mask/operator_or_assign.md) | 合入另一个集合 |
| [`operator&=`](damage_type_mask/operator_and_assign.md) | 保留交集 |
| [`operator^=`](damage_type_mask/operator_xor_assign.md) | 保留对称差集 |
| [`operator==`](damage_type_mask/operator_equal.md) | 比较两个集合 |
| [`operator\|`](damage_type_mask/operator_or.md) | 取得并集 |
| [`operator&`](damage_type_mask/operator_and.md) | 取得交集 |
| [`operator^`](damage_type_mask/operator_xor.md) | 取得对称差集 |
| [`operator~`](damage_type_mask/operator_not.md) | 取得补集 |
