[givm](../../../reference.md) / [定义](../../definition.md) / [issued_id_map](../issued_id_map.md) / **get_id**

# givm::issued_id_map::get_id

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TDefinition>
definition_id<TDefinition> get_id(std::string_view name) const;
```

取得某个名称的实体定义 ID，供准备牌组或指定实体效果时使用。

## 模板参数

|  |  |
| --- | --- |
| `TDefinition` | 定义类别，见 [`definition_types`](../definition_types.md) |

## 参数

|  |  |
| --- | --- |
| `name` | 已登记在本类别中的定义名称 |

## 返回值

名称对应的定义 ID。

## 注意

名称必须存在；不确定时先使用 [`has`](has.md) 检查。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{};
    ids.add<givm::card_definition>("恢复药剂", {});
    givm::linked_deck deck{};
    deck.cards.push_back(ids.get_id<givm::card_definition>("恢复药剂"));
    std::println("牌组卡牌数量: {}", deck.cards.size());
}
```

输出

```text
牌组卡牌数量: 1
```
