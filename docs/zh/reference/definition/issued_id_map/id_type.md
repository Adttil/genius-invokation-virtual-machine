[givm](../../../reference.md) / [定义](../../definition.md) / [issued_id_map](../issued_id_map.md) / **id_type**

# givm::issued_id_map::id_type

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TDefinition>
using id_type = definition_id<TDefinition>;
```

指定定义类别对应的 ID 类型。

## 模板参数

|  |  |
| --- | --- |
| `TDefinition` | 定义类别，见 [`definition_types`](../definition_types.md) |

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{};
    const givm::issued_id_map::id_type<givm::card_definition> card =
        ids.add<givm::card_definition>("恢复药剂", {});
    std::println("已取得定义 ID: {}", card.is_valid());
}
```

输出

```text
已取得定义 ID: true
```
