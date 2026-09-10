[givm](../../../reference.md) / [定义](../../definition.md) / [tag_mask](../tag_mask.md) / **(构造函数)**

# givm::tag_mask::tag_mask

定义于头文件 `<givm/definition.hpp>`

```cpp
tag_mask() = default; // (1)
explicit tag_mask(size_t size); // (2)
```

构造一个不具有任何标签的集合。默认构造没有可用标签；指定数量后，可以设置对应 ID 的标签。

## 参数

|  |  |
| --- | --- |
| `size` | 可用标签的数量，通常取配套映射的 `tag_names().size()` |

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
    std::println("具有治疗标签: {}", tags.has(healing));
    std::println("具有料理标签: {}", tags.has(food));
}
```

输出

```text
具有治疗标签: true
具有料理标签: false
```
