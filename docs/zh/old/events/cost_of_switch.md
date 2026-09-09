[返回](../events.md)

# cost_of_switch

计算一次主动切换出战角色的费用时广播的可修改事件。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `target` | `character_id` | 正在预览或执行的切换目标，可修改；实际切换读取最终字段值。 |
| `requirement` | `action_cost_requirement` | 当前费用需求，包含 `dice_requirement` 与 `speed`。 |
| `effect_argument` | `cost_effect_argument<cost_of_switch>` | 当前 handler 为其 onpay 入口生成的最小提交参数；调用下一个 handler 前会被清空。 |

## 默认值

主动切人默认需要 1 个任意元素骰，行动速度为 `action_speed::combat`。

## 响应

`begin_action` 在准备当前行动输入 frame 时采样费用响应者。每次计算指定候选的费用时，先恢复该候选的默认费用，清空该行旧 onpay，再按这一快照调用 handler。

响应者可以直接修改 `requirement`。五参数 handler 若需要在实际支付时产生副作用，应把本 handler 的实际减费量写入 `effect_argument`，并返回 `program_entry<onpay_context<cost_of_switch>>`；返回空入口表示没有 onpay。`begin_action` 会把入口和参数一起记入当前 action、当前 handler 对应的矩阵槽位，不在费用预览时执行它。

玩家确认支付后，`begin_action` 才按原 handler 顺序进入选中行中的非空 onpay。每个固定入口读取 `detail::handler_id<cost_of_switch>, cost_effect_argument<cost_of_switch>, stage_t` 调用 frame。参数保存实际减少的骰子；onpay 不依赖费用计算时的事件工作区。

费用 handler 只支持上述五参数固定入口协议，不存在运行时生成 onpay 程序的兼容路径。
