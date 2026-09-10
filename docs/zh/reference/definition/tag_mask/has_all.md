[givm](../../../reference.md) / [定义](../../definition.md) / [tag_mask](../tag_mask.md) / **has_all**

# givm::tag_mask::has_all

定义于头文件 `<givm/definition.hpp>`

```cpp
bool has_all(std::span<const tag_id> tags) const;
```

检查集合是否具有给定标签中的全部标签。

## 参数

|  |  |
| --- | --- |
| `tags` | 有效标签 ID 的范围，各 ID 必须在集合容量范围内 |

## 返回值

满足条件时返回 `true`，否则返回 `false`。空范围返回 `true`。

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
    const std::array requested{ healing, food };
    std::println("满足标签条件: {}", tags.has_all(requested));
}
```

输出

```text
满足标签条件: false
```
