# givm::tag_mask_view

定义于头文件 `<givm/definition.hpp>`

对标签位集前缀的只读借用。公开标签ID连续位于所有标签之前，因此公开标签集合只需引用原位集及公开位数。

| 成员 | 用途 |
| --- | --- |
| [`size`](tag_mask_view/size.md) | 可见标签槽位数量 |
| [`empty`](tag_mask_view/empty.md) | 是否没有可见槽位 |
| [`has`](tag_mask_view/has.md) | 查询一个可见标签 |
| [`has_all`](tag_mask_view/has_all.md) | 查询全部标签 |
| [`has_any`](tag_mask_view/has_any.md) | 查询任意标签 |
| [`matches`](tag_mask_view/matches.md) | 查询必需及排除条件 |
| [`begin`](tag_mask_view/begin.md)、[`end`](tag_mask_view/end.md) | 按ID顺序遍历只读标签位 |

超出可见前缀的标签在查询中视为不存在。视图不拥有位集，原位集必须存活且未被替换。

通过 [`tag_mask::take`](tag_mask/take.md) 取得前缀；迭代器只读且支持随机访问，不依赖视图对象本身的生命周期。
