[返回](../events.md)

# roll_dice

掷出栈顶 selector 指定的骰子。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `player` | `player_id` | 执行投骰的玩家。 |

## 执行

进入本命令时，executor stack 顶部必须是：

```cpp
stack.top<selector>();
```

该 selector 通常由 [`push_selector`](push_selector.md) 产生，或由外层在阻塞后按当前协议直接写入。本命令假定 selector 已经填写完毕且合法。

本命令弹出 selector，栈顶恢复为压入该 selector 前的布局；然后移除选中的骰子，随机生成相同数量的骰子并加入玩家当前骰子。

## 注意

- 当前骰子依次按万能、冰、水、火、雷、风、岩、草排列，同种骰子连续排列；排列结果从 1 开始编号，第 `i` 个骰子对应选择 bitset 从低位到高位的第 `i` 位。
- 全零 bitset 表示本次不掷任何骰子。
- 本命令是单玩家、单 selector 的通用随机重投工具。
- 默认回合开始流程不使用本命令处理双方重投；双方投掷阶段由 [`process_dice_roll_phase`](process_dice_roll_phase.md) 预发重投随机池并推进。
- 本指令不发布投骰 event。

## 形式化语义

1. 读取 executor stack 顶部 selector，并要求其 `player` 等于本命令的 `player`。
2. 记 selector 中的 bitset 为 `M`，并弹出该 selector。
3. 按万能、冰、水、火、雷、风、岩、草的顺序排列玩家当前骰子，同种骰子保持连续。
4. 根据 `M` 移除所有被选择的骰子，记移除数量为 `X`。
5. 随机生成 `X` 个骰子并加入玩家当前骰子。
