[givm](../../reference.md) / [牌桌](../table.md) / **attachment_entity**

# givm::attachment_entity

定义于头文件 `<givm/table.hpp>`

```cpp
template<class TStorage>
class attachment_entity;
```

角色身上一个附属实体的访问入口，例如随角色持续存在的效果或装备。具体用途由定义决定。

## 模板参数

|  |  |
| --- | --- |
| `TStorage` | 由牌桌决定的存储类型；是否具有 const 限定决定实体能否修改。通常通过牌桌取得对象并使用 `auto`，无需显式指定此参数 |

## 成员类型

|  |  |
| --- | --- |
| `table_type` | `TStorage` |
| `data_type` | 实体数据类型，随 `is_mutable` 保留只读限定 |
| `storage_type` | 实体访问所用的辅助类型 |
| `player_type` | `player_entity<TStorage>` |
| `character_type` | `character_entity<TStorage>` |

## 成员常量

|  |  |
| --- | --- |
| [`is_mutable`](attachment_entity/is_mutable.md) | 实体是否允许修改 |

## 成员函数

|  |  |
| --- | --- |
| [`operator attachment_view`](attachment_entity/conversion.md) | 取得同一实体的只读视图 |
| [`is_valid`](attachment_entity/is_valid.md) | 判断实体是否尚未移除 |
| [`operator bool`](attachment_entity/operator_bool.md) | 判断实体是否尚未移除 |
| [`size`](attachment_entity/size.md) | 取得单实体范围的元素数 |
| [`begin`](attachment_entity/begin.md) | 取得单实体范围的起点 |
| [`end`](attachment_entity/end.md) | 取得单实体范围的终点 |
| [`player`](attachment_entity/player.md) | 取得所属玩家 |
| [`id`](attachment_entity/id.md) | 取得实体 ID |
| [`definition_id`](attachment_entity/definition_id.md) | 取得实体的定义 ID |
| [`state`](attachment_entity/state.md) | 访问实体状态 |
| [`erase`](attachment_entity/erase.md) | 移除实体 |
| [`character`](attachment_entity/character.md) | 取得所属角色 |

## 注意

从牌桌或所属实体取得该对象；复制它仍然访问同一个角色附属实体。访问对象的存活、只读转换和移除约定见[实体的身份与访问](entity_access.md)。

## 示例

```cpp
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

template<class Category>
struct example_source
{
    using definition_category = Category;
    struct definition_type {};

    std::string_view name() const { return "示例"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }
};

int main()
{
    givm::definition_source_library sources{};
    const example_source<givm::character_view> character_source{};
    const example_source<givm::attachment_view> attachment_source{};
    sources.add(character_source, attachment_source);
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{};
    const auto player = table[givm::player_id{ 0 }];
    const auto character_definition = id_map.get_id<givm::character_view>("示例");
    const auto definition = id_map.get_id<givm::attachment_view>("示例");
    const auto character = player.add(character_definition, { .max_health = 10, .health = 10 });
    const auto entity = character.add(definition, { .count = 3 });
    std::println("计数: {}", entity.state().count);
}
```

输出

```text
计数: 3
```

## 参阅

|  |  |
| --- | --- |
| [`attachment_view`](attachment_view.md) | 对应的只读视图 |
| [`attachment_state`](attachment_state.md) | 该实体的状态 |
