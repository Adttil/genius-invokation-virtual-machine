[givm](../reference.md) / **执行**

# 执行

将定义源与游戏流程编译为对局规则，推进一场对局，并在需要输入、观察结果或结束对局时把控制权交回调用方。

## 类

### 编译与定义库

|  |  |
| --- | --- |
| [`definition_compile_context`](executor/definition_compile_context.md) | 单项定义的编译上下文 |
| [`definition_library`](executor/definition_library.md) | 对局使用的定义与流程 |

### 对局执行

|  |  |
| --- | --- |
| [`executor`](executor/executor.md) | 游戏对局的执行器 |
| [`execution_view`](executor/execution_view.md) | 一处对局执行现场的视图 |
| [`random_fn`](executor/random_fn.md) | 随机函数视图 |

### 行动输入

|  |  |
| --- | --- |
| [`action_argument`](executor/action_argument.md) | 行动支付参数 |
| [`action_target`](executor/action_target.md) | 行动目标 |

## 函数

|  |  |
| --- | --- |
| [`compile`](executor/compile.md) | 编译选定定义与对局流程 |

## 枚举

|  |  |
| --- | --- |
| [`compile_mode`](executor/compile_mode.md) | 编译时选择普通推进或额外观察 |
| [`execution_state`](executor/execution_state.md) | 执行器交回控制权时的执行现场种类 |
| [`initial_card_selection_check_result`](executor/initial_card_selection_check_result.md) | 首次换牌选择检查的结果 |
| [`initial_active_character_selection_check_result`](executor/initial_active_character_selection_check_result.md) | 首次出战角色选择检查的结果 |
| [`remaining_active_character_selection_check_result`](executor/remaining_active_character_selection_check_result.md) | 剩余一方出战角色选择检查的结果 |
| [`dice_selection_check_result`](executor/dice_selection_check_result.md) | 显式指定玩家的骰子重投选择检查结果 |
| [`switch_payment_check_result`](executor/switch_payment_check_result.md) | 切换出战角色的骰子支付检查结果 |
| [`card_payment_check_result`](executor/card_payment_check_result.md) | 出牌的骰子支付检查结果 |
| [`action_target_kind`](executor/action_target_kind.md) | 行动目标种类 |

## 常量

|  |  |
| --- | --- |
| [`selection_capacity`](executor/selection_capacity.md) | 单次选择可表示的位置数量 |
