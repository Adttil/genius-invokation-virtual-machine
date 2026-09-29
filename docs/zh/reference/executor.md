[givm](../reference.md) / **执行**

# 执行

将定义源与游戏流程编译为对局规则，推进一场对局，并在需要输入、观察结果或结束对局时把控制权交回调用方。

## 类

### 编译与定义库

|  |  |
| --- | --- |
| [`definition_compile_context`](executor/definition_compile_context.md) | 单项定义的编译上下文 |
| [`definition_compile_context::definition_view`](executor/definition_compile_context/definition_view.md) | 编译集合中一项定义的元数据视图 |
| [`definition_library`](executor/definition_library.md) | 对局使用的定义与流程 |
| [`definition_compile_result`](executor/definition_compile_result.md) | 编译成功的定义库与名称映射 |
| [`compile_error`](executor/compile_error.md) | 一项编译诊断 |
| [`compile_location`](executor/compile_location.md) | 编译诊断的发生位置 |
| [`definition_resolution_error`](executor/definition_resolution_error.md) | 定义硬依赖的解析诊断 |
| [`definition_metadata_error`](executor/definition_metadata_error.md) | 定义元数据查询的 ID 诊断 |

历史字段相关诊断类型见 [`compile_error_reason`](executor/compile_error_reason.md)。

### 对局执行

|  |  |
| --- | --- |
| [`executor`](executor/executor.md) | 游戏对局的执行器 |
| [`execution_view`](executor/execution_view.md) | 一处对局执行现场的视图 |
| [`execution_view_error`](executor/execution_view_error.md) | 现场种类或视图生命周期的调试异常 |
| [`view_input_error`](executor/view_input_error.md) | 视图参数或查询前提的强类型调试异常 |
| [`random_fn`](executor/random_fn.md) | 随机函数视图 |
| [`handle_context`](executor/handle_context.md) | 事件响应使用的牌桌、随机源及效果提交接口 |
| [`program_invoker`](executor/program_invoker.md) | 响应提交后续效果的调用对象 |
| [`program_input_error`](executor/program_input_error.md) | 程序入口或输入协议的调试异常 |
| [`command_input_error`](executor/command_input_error.md) | 命令输入值或执行前提的调试异常 |
| [`history_access_error`](executor/history_access_error.md) | 历史摘要字段访问的调试异常 |

### 行动输入

|  |  |
| --- | --- |
| [`action_argument`](executor/action_argument.md) | 行动支付参数 |
| [`action_target`](executor/action_target.md) | 行动目标 |

## 类型别名

| | |
| --- | --- |
| [`compile_error_reason`](executor/compile_error_reason.md) | 全部具体编译错误的 variant |
| [`program_input_error_reason`](executor/program_input_error_reason.md) | 程序提交协议错误的 variant |
| [`command_input_error_reason`](executor/command_input_error_reason.md) | 命令执行前提错误的 variant |
| [`command_entity_id`](executor/command_input_error_reason.md#command_entity_id) | 命令诊断中的实体 ID variant |
| [`execution_view_error_reason`](executor/execution_view_error.md#相关类型别名) | 现场与视图有效性错误的 variant |
| [`history_access_error_reason`](executor/history_access_error.md#相关类型别名) | 历史字段访问错误的 variant |

## 函数

|  |  |
| --- | --- |
| [`compile`](executor/compile.md) | 编译选定定义与对局流程 |
| [`error_string`](executor/error_string.md) | 格式化编译诊断 |
| [`load_deck`](executor/load_deck.md) | 装载双方牌组并初始化角色状态与技能 |

## 枚举

|  |  |
| --- | --- |
| [`compile_mode`](executor/compile_mode.md) | 编译时选择普通推进或额外观察 |
| [`compile_stage`](executor/compile_stage.md) | 编译诊断的发生阶段 |
| [`program_kind`](executor/program_kind.md) | 编译诊断中的程序类别 |
| [`execution_state`](executor/execution_state.md) | 执行器交回控制权时的执行现场种类 |
| [`initial_card_selection_validation`](executor/initial_card_selection_validation.md) | 首次换牌选择检查的结果 |
| [`initial_active_character_selection_validation`](executor/initial_active_character_selection_validation.md) | 首次出战角色选择检查的结果 |
| [`remaining_active_character_selection_validation`](executor/remaining_active_character_selection_validation.md) | 剩余一方出战角色选择检查的结果 |
| [`dice_selection_validation`](executor/dice_selection_validation.md) | 显式指定玩家的骰子重投选择检查结果 |
| [`switch_payment_validation`](executor/switch_payment_validation.md) | 切换出战角色的支付检查结果 |
| [`card_payment_validation`](executor/card_payment_validation.md) | 出牌的支付检查结果 |
| [`skill_payment_validation`](executor/skill_payment_validation.md) | 技能使用的支付检查结果 |
| [`technique_payment_validation`](executor/technique_payment_validation.md) | 特技使用的支付检查结果 |
| [`action_target_kind`](executor/action_target_kind.md) | 行动目标种类 |

## 常量

|  |  |
| --- | --- |
| [`selection_capacity`](executor/selection_capacity.md) | 单次选择可表示的位置数量 |
