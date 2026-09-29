[givm](../../../reference.md) / [定义](../../definition.md) / [issued_id_map](../issued_id_map.md) / **has**

# givm::issued_id_map::has

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TDefinition>
bool has(std::string_view name) const;
```

检查指定类别中是否存在某个名称的定义。

## 模板参数

|  |  |
| --- | --- |
| `TDefinition` | 定义类别，见 [`definition_types`](../definition_types.md) |

## 参数

|  |  |
| --- | --- |
| `name` | 要查找的定义名称 |

## 返回值

存在时返回 `true`，否则返回 `false`。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{};
    ids.add<givm::card_definition>("恢复药剂", {});
    std::println("存在卡牌定义: {}", ids.has<givm::card_definition>("恢复药剂"));
    std::println("存在同名角色定义: {}", ids.has<givm::character_view>("恢复药剂"));
}
```

输出

```text
存在卡牌定义: true
存在同名角色定义: false
```
