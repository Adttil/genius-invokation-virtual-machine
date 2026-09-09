[返回](../events.md)

# push_selector

向 executor stack 顶部推送一个选择对象。

本指令由独立头文件 `givm/executor/instructions/push_selector.hpp` 提供，不由 `givm/executor/instructions.hpp` 聚合导出。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `player` | `player_id` | 选择所属玩家。 |
| `selected` | `std::bitset<selection_capacity>` | 按消费该选择的 command 所定义顺序选择若干项。 |

## 执行

本命令不要求特定栈顶布局。执行时将 `selector { player, selected }` 压入 executor stack 顶部，完成后产生：

```cpp
stack.top<selector>();
```

后续消费选择的 command 直接读取该 selector，并假定其中的玩家与 bitset 已经由上层或前置指令按协议写好。

## 注意

- 消费 selector 的 command 会在成功读取后直接弹出 selector，不需要单独的 pop command。
- 全零 bitset 是有效选择，表示没有选择任何候选项。
- 每一位对应哪个候选项由消费选择的 command 定义。
- 本指令用于直接提供已完成的选择，不自行挂起等待输入。
- 不是所有输入都使用 selector；例如开局出战角色选择直接使用 `character_id`。
