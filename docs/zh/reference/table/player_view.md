[givm](../../reference.md) / [牌桌](../table.md) / **player_view**

# givm::player_view

定义于头文件 `<givm/table.hpp>`

```cpp
using player_view = player_entity<const detail::table_storage>;
```

玩家的只读视图。它可以查看实体的状态和所属关系，不能修改该实体。

该视图仍然访问原牌桌中的实体；创建视图不会冻结或复制对局状态。

## 示例

```cpp
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{};
    const auto entity = table[givm::player_id{ 0 }];
    const givm::player_view view = entity;
    std::println("骰子数: {}", view.state().dice.total());
}
```

输出

```text
骰子数: 0
```

## 参阅

|  |  |
| --- | --- |
| [`player_entity`](player_entity.md) | 实体的完整访问接口 |
| [实体的身份与访问](entity_access.md) | 只读访问与存活约定 |
