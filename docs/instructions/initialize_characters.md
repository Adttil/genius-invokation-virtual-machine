[返回](../events.md)

# initialize_characters

初始化指定玩家已经装入 table 的全部角色。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `player` | `player_id` | 需要初始化角色的玩家。 |

## 栈 ABI

本指令不要求特定栈顶布局，也不读写 executor stack。

## 执行

指令按角色槽位顺序遍历 `player` 的所有有效角色。对每个角色依次执行：

1. 创建局部 [`character_initialization`](../events/character_initialization.md) 工作区。
2. 只调用该角色自身 definition 的 `character_initialization` handler，并向它传递本次执行的随机函数。
3. 忽略 handler 返回的入口，把工作区中的 `state` 写回角色实体。

整个遍历在一次指令执行中完成，不产生中间挂起点，也不进行普通事件广播。所有角色初始化完成后进入下一条固定指令。

`character_initialization` 当前约定只修改 event 并返回空入口。该约定暂未由单独的静态 Context 类型强制。
