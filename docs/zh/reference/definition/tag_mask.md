[givm](../../reference.md) / [定义](../definition.md) / **tag_mask**

# givm::tag_mask

定义于头文件 `<givm/definition.hpp>`

```cpp
class tag_mask;
```

一组定义分类标签的记录，用于判断定义是否符合某种筛选条件。

## 成员函数

|  |  |
| --- | --- |
| [(构造函数)](tag_mask/constructor.md) | 构造一个空标签集合 |
| [`take`](tag_mask/take.md) | 借用标签位集前缀 |
| [`set`](tag_mask/set.md) | 设置或清除一个标签 |
| [`has`](tag_mask/has.md) | 检查一个标签 |
| [`has_all`](tag_mask/has_all.md) | 检查是否包含所有标签 |
| [`has_any`](tag_mask/has_any.md) | 检查是否包含任一标签 |
| [`matches`](tag_mask/matches.md) | 检查必需与排除标签条件 |

## 注意

构造时指定可用标签数量；之后使用的标签 ID 必须来自对应映射，并在这个数量范围内。

内部使用 [`small_bitset<192>`](../utils/small_bitset.md)：最多 192 个标签位时使用内联存储，超过时分配整个位集。编译准备阶段的 ID 映射与运行期定义库共用 `tag_mask`。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{ "治疗", "料理" };
    const auto healing = ids.get_tag_id("治疗");
    const auto food = ids.get_tag_id("料理");
    givm::tag_mask tags{ ids.tag_names().size() };
    tags.set(healing);
    std::println("具有治疗标签: {}", tags.has(healing));
    std::println("具有料理标签: {}", tags.has(food));
}
```

输出

```text
具有治疗标签: true
具有料理标签: false
```
