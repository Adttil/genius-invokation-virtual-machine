[返回](../events.md)

# start_battle

广播战斗开始时机。

## 字段

无。

## 栈

本指令入口不要求额外栈输入。实际广播时会临时压入 `battle_started` 广播 frame；广播完整结算后弹出该 frame，恢复入口栈形状。

## 执行

若当前回合编号不是 1，直接完成。否则推进 [`battle_started`](../events/battle_started.md) 广播，然后完成当前指令。

## 注意

调用方可以在循环执行的回合程序中放置本指令，因此无需为首回合建立不同程序形状；只有第 1 回合实际广播。广播完整结算后，固定程序继续进入调用方安排的下一条指令，例如 [`begin_action`](begin_action.md)。
