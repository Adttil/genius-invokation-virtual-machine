[givm](../../reference.md) / [枚举值](../enums.md) / **elemental_reaction_mask**

# givm::elemental_reaction_mask

定义于 `<givm/enums/elemental_reaction.hpp>`。

记录一段中出现过的反应槽位。默认集合为空，按值使用位或合并；不记录各值的小计或先后次序。`none` 表示没有反应，查询为 false，设置、清除、反转该值均不改变集合。

## 成员和运算

| 名称 | 说明 |
| --- | --- |
| [`构造函数`](elemental_reaction_mask/constructor.md) | 构造空集合或只含一个值的集合 |
| [`operator[]`](elemental_reaction_mask/operator_at.md) | 查询指定值是否出现 |
| [`all`](elemental_reaction_mask/all.md) | 查询是否包含全部值 |
| [`any`](elemental_reaction_mask/any.md) | 查询是否至少包含一个值 |
| [`none`](elemental_reaction_mask/none.md) | 查询是否为空 |
| [`count`](elemental_reaction_mask/count.md) | 取得集合中的值数量 |
| [`size`](elemental_reaction_mask/size.md) | 取得可表示的值总数 17 |
| [`set`](elemental_reaction_mask/set.md) | 设置全体或指定值 |
| [`reset`](elemental_reaction_mask/reset.md) | 清除全体或指定值 |
| [`flip`](elemental_reaction_mask/flip.md) | 反转全体或指定值 |
| [`operator\|=`](elemental_reaction_mask/operator_or_assign.md) | 合入另一个集合 |
| [`operator&=`](elemental_reaction_mask/operator_and_assign.md) | 保留交集 |
| [`operator^=`](elemental_reaction_mask/operator_xor_assign.md) | 保留对称差集 |
| [`operator==`](elemental_reaction_mask/operator_equal.md) | 比较两个集合 |
| [`operator\|`](elemental_reaction_mask/operator_or.md) | 取得并集 |
| [`operator&`](elemental_reaction_mask/operator_and.md) | 取得交集 |
| [`operator^`](elemental_reaction_mask/operator_xor.md) | 取得对称差集 |
| [`operator~`](elemental_reaction_mask/operator_not.md) | 取得补集 |
