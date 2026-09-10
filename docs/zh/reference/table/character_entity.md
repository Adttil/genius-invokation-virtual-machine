[givm](../../reference.md) / [牌桌](../table.md) / **character_entity**

# givm::character_entity

定义于头文件 `<givm/table.hpp>`

```cpp
template<class TStorage>
class character_entity;
```

对局中一名角色的访问入口。它承载角色的生命、充能、元素附着，以及角色拥有的技能和附属实体。

## 模板参数

|  |  |
| --- | --- |
| `TStorage` | 由取得实体的牌桌决定；是否具有 const 限定决定实体能否修改。通常通过牌桌取得对象并使用 `auto`，无需显式指定此参数 |

## 成员类型

|  |  |
| --- | --- |
| `table_type` | `TStorage` |
| `data_type` | 实体数据类型，随 `is_mutable` 保留只读限定 |
| `storage_type` | 实体访问所用的辅助类型 |
| `player_type` | `player_entity<TStorage>` |

## 成员常量

|  |  |
| --- | --- |
| [`is_mutable`](character_entity/is_mutable.md) | 实体是否允许修改 |

## 成员函数

|  |  |
| --- | --- |
| [`operator character_view`](character_entity/conversion.md) | 取得同一实体的只读视图 |
| [`is_valid`](character_entity/is_valid.md) | 判断实体是否尚未移除 |
| [`operator bool`](character_entity/operator_bool.md) | 判断实体是否尚未移除 |
| [`size`](character_entity/size.md) | 取得单实体范围的元素数 |
| [`begin`](character_entity/begin.md) | 取得单实体范围的起点 |
| [`end`](character_entity/end.md) | 取得单实体范围的终点 |
| [`player`](character_entity/player.md) | 取得所属玩家 |
| [`id`](character_entity/id.md) | 取得实体 ID |
| [`definition`](character_entity/definition.md) | 取得实体定义 |
| [`can_handle`](character_entity/can_handle.md) | 判断是否具有某事件的处理程序 |
| [`state`](character_entity/state.md) | 访问实体状态 |
| [`erase`](character_entity/erase.md) | 移除实体 |
| [`skills`](character_entity/skills.md) | 遍历角色的技能 |
| [`attachments`](character_entity/attachments.md) | 遍历角色的附属实体 |
| [`add`](character_entity/add.md) | 增加技能或附属实体 |
| [`clean_up`](character_entity/clean_up.md) | 清理角色已移除的技能和附属实体 |

## 注意

从牌桌或所属实体取得该对象；复制它仍然访问同一个角色。访问对象的存活、只读转换和移除约定见[实体的身份与访问](entity_access.md)。

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
    sources.add(character_source);
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{ library };
    const auto player = table[givm::player_id{ 0 }];
    const auto definition = id_map.get_id<givm::character_view>("示例");
    const auto entity = player.add(definition, { .max_health = 10, .health = 10 });
    std::println("生命值: {}", entity.state().health);
}
```

输出

```text
生命值: 10
```

## 参阅

|  |  |
| --- | --- |
| [`character_view`](character_view.md) | 对应的只读视图 |
| [`character_state`](character_state.md) | 该实体的状态 |
