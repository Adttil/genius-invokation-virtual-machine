# givm::role

定义于头文件 `<givm/table.hpp>`

```cpp
enum class role : std::uint8_t { full, player0, player1, spectator };
```

读取对局信息和提交玩家选择时采用的身份。玩家只提交本人的选择；观众读取双方手牌及骰子，但不提交输入。各受限牌桌视图都隐藏牌堆明细。

| 值 | 能力 |
| --- | --- |
| `full` | 原有完整读取及驱动接口；执行视图的默认身份 |
| `player0` | 玩家0可读的信息及本人当前可提交的输入 |
| `player1` | 玩家1可读的信息及本人当前可提交的输入 |
| `spectator` | 双方玩家的可读信息，不含牌堆明细；没有提交和推进接口 |

身份用于选择接口类型，不在对局中维护权限状态。它与 [`compile_mode`](../executor/compile_mode.md) 独立。
