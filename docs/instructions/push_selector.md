[返回](../events.md)

# push_selector

向 executor stack 顶部推送一个选择对象。

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
- 需要等待 bitset 多选输入的流程通常由前置指令主动挂起，并把 frame 尾部 selector 作为输入槽暴露给外层；`push_selector` 更适合脚本、规则效果或随机选择直接提供已完成输入。
- 不是所有输入都使用 selector；例如开局出战角色选择直接使用 `character_id`。
