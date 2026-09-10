[givm](../../../reference.md) / [定义](../../definition.md) / [issued_id_map](../issued_id_map.md) / **query_by_tag**

# givm::issued_id_map::query_by_tag

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TDefinition>
std::vector<definition_id<TDefinition>> query_by_tag(std::string_view expression) const;
```

找出同时满足标签条件的定义，例如具有治疗标签但不属于料理的卡牌。

## 模板参数

|  |  |
| --- | --- |
| `TDefinition` | 定义类别，见 [`definition_types`](../definition_types.md) |

## 参数

|  |  |
| --- | --- |
| `expression` | 以 `&` 连接的标签条件；`!标签` 表示排除该标签，可在条件两侧添加空白 |

## 返回值

所有满足条件的定义 ID，按 ID 分配顺序排列。

## 注意

表达式中的标签必须全部存在；条件不能为空。不支持括号或 `|`。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{ "治疗", "料理" };
    ids.add<givm::card_definition>("恢复药剂", { "治疗" });
    ids.add<givm::card_definition>("恢复料理", { "治疗", "料理" });
    const auto cards = ids.query_by_tag<givm::card_definition>("治疗 & !料理");
    std::println("非料理治疗牌数量: {}", cards.size());
    std::println("包含恢复药剂: {}", ids.has<givm::card_definition>("恢复药剂"));
}
```

输出

```text
非料理治疗牌数量: 1
包含恢复药剂: true
```
