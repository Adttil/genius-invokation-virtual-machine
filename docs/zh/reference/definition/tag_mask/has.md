[givm](../../../reference.md) / [定义](../../definition.md) / [tag_mask](../tag_mask.md) / **has**

# givm::tag_mask::has

定义于头文件 `<givm/definition.hpp>`

```cpp
bool has(tag_id id) const;
```

检查集合是否具有某个标签。

## 参数

|  |  |
| --- | --- |
| `id` | 在集合容量范围内的有效标签 ID |

## 返回值

具有标签时返回 `true`，否则返回 `false`。

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
