[givm](../../../reference.md) / [定义](../../definition.md) / [tag_mask](../tag_mask.md) / **set**

# givm::tag_mask::set

定义于头文件 `<givm/definition.hpp>`

```cpp
void set(tag_id id, bool value = true);
```

设置或清除某个标签。

## 参数

|  |  |
| --- | --- |
| `id` | 在集合容量范围内的有效标签 ID |
| `value` | `true` 表示具有该标签，`false` 表示不具有 |

## 返回值

（无）

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
    std::println("设置后具有治疗标签: {}", tags.has(healing));
    tags.set(healing, false);
    std::println("清除后具有治疗标签: {}", tags.has(healing));
}
```

输出

```text
设置后具有治疗标签: true
清除后具有治疗标签: false
```
