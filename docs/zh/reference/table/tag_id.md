[givm](../../reference.md) / [牌桌](../table.md) / **tag_id**

# givm::tag_id

定义于头文件 `<givm/table.hpp>`

```cpp
class tag_id;
```

一个分类标签的身份，例如卡牌的用途或角色的元素分类。本类型只保存已取得的标签；允许尚未取得标签时使用 [`optional_tag_id`](optional_tag_id.md)。

## 成员函数

| 名称 | 说明 |
| --- | --- |
| [(构造函数)](tag_id/constructor.md) | 默认构造或从标签索引构造 |
| [`operator=`](tag_id/operator_assign.md) | 从标签索引赋值 |
| [`value`](tag_id/value.md) | 取得标签索引 |
| [`operator==`](tag_id/operator_equal.md) | 比较标签身份 |

默认构造保持平凡，未初始化的对象须先赋值。索引属于产生它的名称映射及定义库，不能跨不同编译结果混用。标签索引采用 `std::size_t`；整数构造在 Debug 拒绝可空类型使用的特殊值。

## 示例

```cpp
#include <print>
#include <givm/givm.hpp>
int main()
{
    givm::issued_id_map ids{ "治疗" };
    const givm::tag_id tag = ids.get_tag_id("治疗");
    std::println("标签名称: {}", ids.tag_name(tag));
}
```

输出

```text
标签名称: 治疗
```
