[givm](../../reference.md) / [牌桌](../table.md) / **combat_status_entity**

# givm::combat_status_entity

定义于头文件 `<givm/table.hpp>`

```cpp
template<class TStorage>
class combat_status_entity;
```

一方出战状态的访问入口。它属于玩家，可在出战角色变化后继续存在。

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

## 成员常量

|  |  |
| --- | --- |
| [`is_mutable`](combat_status_entity/is_mutable.md) | 实体是否允许修改 |

## 成员函数

|  |  |
| --- | --- |
| [`operator combat_status_view`](combat_status_entity/conversion.md) | 取得同一实体的只读视图 |
| [`is_valid`](combat_status_entity/is_valid.md) | 判断实体是否尚未移除 |
| [`operator bool`](combat_status_entity/operator_bool.md) | 判断实体是否尚未移除 |
| [`size`](combat_status_entity/size.md) | 取得单实体范围的元素数 |
| [`begin`](combat_status_entity/begin.md) | 取得单实体范围的起点 |
| [`end`](combat_status_entity/end.md) | 取得单实体范围的终点 |
| [`player`](combat_status_entity/player.md) | 取得所属玩家 |
| [`id`](combat_status_entity/id.md) | 取得实体 ID |
| [`definition_id`](combat_status_entity/definition_id.md) | 取得实体的定义 ID |
| [`state`](combat_status_entity/state.md) | 访问实体状态 |
| [`erase`](combat_status_entity/erase.md) | 移除实体 |

## 注意

从牌桌或所属实体取得该对象；复制它仍然访问同一个出战状态。访问对象的存活、只读转换和移除约定见[实体的身份与访问](entity_access.md)。

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
    const example_source<givm::combat_status_view> combat_status_source{};
    sources.add(combat_status_source);
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{};
    const auto player = table[givm::player_id{ 0 }];
    const auto definition = id_map.get_id<givm::combat_status_view>("示例");
    const auto entity = player.add(definition, { .count = 3 });
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
| [`combat_status_view`](combat_status_view.md) | 对应的只读视图 |
| [`combat_status_state`](combat_status_state.md) | 该实体的状态 |
