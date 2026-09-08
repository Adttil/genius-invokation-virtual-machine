[返回](../events.md)

# start_dice_roll_phase

开始并完整推进本回合投掷阶段。

## 字段

| 字段 | 类型 | 默认值 | 含义 |
| --- | --- | ---: | --- |
| `count` | `std::uint32_t` | 8 | 双方首次投掷的骰子数量。 |
| `reroll_count` | `std::array<std::uint32_t, 2>` | `{1, 1}` | 玩家 0、玩家 1 的基础重投次数。 |

## 栈

本指令入口不要求特定栈顶布局。执行期间会临时压入 `dice_roll_preparation` 广播 frame；若双方有重投次数，还会压入：

```cpp
stack.top<std::uint32_t[], dice_reroll_phase, selector, stage_t>();
```

其中 `stage_t` 位于物理栈顶，`selector` 是它前面的外层输入槽，`dice_reroll_phase` 和预发随机池由本指令维护。彻底完成时，本指令会弹出所有临时 frame，恢复入口栈形状。

## 执行

1. 用指令字段构造 [`dice_roll_preparation`](../events/dice_roll_preparation.md) `{ .count = count, .reroll_count = reroll_count }`。
2. 按当前 table 枚举可响应该事件的实体，连同游标和事件对象一起压入广播 frame。
3. 推进该广播；响应者返回固定入口时，本指令会在响应程序完成后继续广播。
4. 广播完成后，根据最终事件写入双方固定骰和首次随机投骰。
5. 若双方均无重投次数，弹出广播 frame 并完成。
6. 否则预发双方所有重投随机数，压入重投 frame 并挂起。外层每次填写 `stage_t` 前的 `selector` 输入槽后继续执行；本指令会结算该玩家重投，直到双方都结束。

## 注意

本指令是干净指令：虽然会在等待重投输入时暴露临时 frame，但完成后会恢复入口栈形状。
