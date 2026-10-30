# givm::perspective_table_view

定义于头文件 `<givm/table.hpp>`

```cpp
template<role Role>
class perspective_table_view;
```

供玩家或观众读取一场对局的牌桌视图。通过 [`table::view_for`](table/view_for.md) 创建，借用原牌桌。

| 成员 | 用途 |
| --- | --- |
| [`state`](perspective_table_view/state.md) | 读取公共对局状态 |
| [`player<Player>`](perspective_table_view/player.md) | 按编译期玩家索引取得玩家视图 |
| [`players`](perspective_table_view/players.md) | 同时取得双方视图 |
| [`operator[]`](perspective_table_view/operator_at.md) | 读取公开实体 |

玩家归属影响返回类型，因此玩家和手牌通过 `player<0>()`、`player<1>()` 路径访问。视图不提供牌堆明细和任意历史字段读取。`access_role` 为静态身份。

`role::full` 包装转发原牌桌的完整只读索引和玩家范围；其 `player<Player>()` 返回原有 `player_view`。
