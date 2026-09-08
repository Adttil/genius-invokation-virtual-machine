[返回](../events.md)

# start_battle

广播战斗开始时机。

## 字段

无。

## 栈

本指令不要求特定栈顶布局，也不读写 executor stack。

## 执行

若当前回合编号不是 1，直接完成。否则推进 [`battle_started`](../events/battle_started.md) 广播，然后完成当前指令。

## 注意

默认回合程序每轮都包含本指令，因此无需为首回合建立不同程序形状；只有第 1 回合实际广播。广播完整结算后，固定程序继续进入 [`begin_action`](begin_action.md)。
