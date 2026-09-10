[givm](../../../reference.md) / [定义](../../definition.md) / [tag_mask](../tag_mask.md) / **matches**

# givm::tag_mask::matches

定义于头文件 `<givm/definition.hpp>`

```cpp
bool matches(
    std::span<const tag_id> required_tags,
    std::span<const tag_id> excluded_tags = {}
) const;
```

同时检查必需标签和排除标签，用于组合筛选条件。

## 参数

|  |  |
| --- | --- |
| `required_tags` | 必须全部具有的标签 |
| `excluded_tags` | 必须全部不具有的标签 |

## 返回值

全部条件满足时返回 `true`，否则返回 `false`。两个范围均为空时返回 `true`。

## 注意

所有标签 ID 必须有效且在集合容量范围内。

## 示例

```cpp
#include <array>
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{ "治疗", "料理" };
    const auto healing = ids.get_tag_id("治疗");
    const auto food = ids.get_tag_id("料理");
    givm::tag_mask tags{ ids.tag_names().size() };
    tags.set(healing);
    std::println("治疗但非料理: {}", tags.matches(std::array{ healing }, std::array{ food }));
}
```

输出

```text
治疗但非料理: true
```
