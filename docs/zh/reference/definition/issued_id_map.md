[givm](../../reference.md) / [定义](../definition.md) / **issued_id_map**

# givm::issued_id_map

定义于头文件 `<givm/definition.hpp>`

```cpp
class issued_id_map;
```

定义名称、分类标签与 ID 之间的对应表。准备牌组或编写实体效果时，可以用熟悉的名称找到定义，也可以按标签选出一组定义。

## 成员类型

|  |  |
| --- | --- |
| [`id_type`](issued_id_map/id_type.md) | 指定类别的定义 ID |

## 成员函数

|  |  |
| --- | --- |
| [(构造函数)](issued_id_map/constructor.md) | 指定可用标签并构造映射 |
| [`add`](issued_id_map/add.md) | 为定义分配 ID |
| [`has`](issued_id_map/has.md) | 检查定义名称是否存在 |
| [`has_tag`](issued_id_map/has_tag.md) | 检查标签名称是否存在 |
| [`get_id`](issued_id_map/get_id.md) | 按名称取得定义 ID |
| [`get_tag_id`](issued_id_map/get_tag_id.md) | 按名称取得标签 ID |
| [`tag_name`](issued_id_map/tag_name.md) | 取得标签名称 |
| [`tag_names`](issued_id_map/tag_names.md) | 取得全部标签名称 |
| [`query_by_tag`](issued_id_map/query_by_tag.md) | 按标签筛选定义 |

## 注意

映射不拥有名称字符串；名称、标签对应的字符存储应在映射及其使用者存续期间保持有效。通常使用 [`definition_source_library::compile`](definition_source_library/compile.md) 返回的配套映射。

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
