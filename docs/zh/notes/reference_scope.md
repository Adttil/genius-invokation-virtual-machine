# 文档范围与源码核对

本轮 reference 以三个核心模块的公开使用方式为主线：定义、牌桌和执行。公开 command 与事件归 definition，公共游戏类型及实际使用的栈、类型工具另设主题。公开说明与示例按当前头文件和单测核对；旧文档仅作为理解背景。

## 公开边界

- 定义源和游戏流程必须使用核心给定的公开命令集合；编译入口检查核心集合，命令通过 input_type 声明调用输入。指令字段供定义源构造，事件字段供响应函数访问，二者都是公开接口，不因采用结构体或参与内部结算而变成仅供实现使用的数据。
- 牌桌公开 `table`、独立的只读 `xxx_view`、ID、状态值类型和参数。`table` 只提供只读访问及 `clean_up()` 等规定入口；牌组由 `load_deck()` 装载，完整修改操作由内部 `unrestricted_table` 和 `basic_xxx_handle<TStorage>` 提供。`xxx_handle<TStorage>` 是内部类型选择别名，不是公开 view 的定义方式。`card_data`、其余后台 `*_data`、`status_slot`、`invalid_status_index` 和存储辅助对象均不作为独立用户接口。
- 定义库取指、执行位置与指令类型标识只供内部使用。公开运行接口返回 execution_state，由 view_in 取得相应访问对象；纯通知使用空视图，相关数据直接读取 table。
- execution_context 与行动选择实现类型位于 `givm::detail` 命名空间，文件按所属功能组织。换牌现场直接保存玩家 ID 与选择位集，公开读写通过 execution_view 的读取方法和参数式输入。executor 不公开栈访问，不维护外部输入槽或完整帧 ABI。utils 中的栈工具可以独立使用。
- 支付缓存保存入口与参数块的位置、长度，只属于执行器实现；输入通过公开初始事件和 handle_context::invoke 提交，不公开完整缓存布局。
- `broadcast.hpp` 中的辅助函数位于 `givm::detail`，不能据此把其广播顺序写成所有事件的强制约定。
- 旧 `push_selector`、`roll_dice` 和 `process_dice_roll_phase` 已移除，输入与结算由完整 command 负责。
- `assume_enabled_t`、`assume_enabled` 没有实际调用点；`utils/optional.hpp`、`stable_vector.hpp`、`inplace_vector.hpp` 没有被三个公共入口引用，其中还有未完成的声明或接口。这些遗留文件需另行整理，本轮没有为其建立看似可用的公开 API 页。
- `type_list` 的实现基类、调试辅助函数和成对展开/撤销的内部宏没有独立的 reference 页面。

## 独立于文档整理的源码问题

以下是本轮阅读源码时记录的事项，尚未在本轮修改实现。它们不是新的公开契约。

- `aura_after_reaction` 恒返回无附着，源码保留了附着消耗规则的 TODO；`aura_without_reaction` 也有非反应附着保留规则的 TODO。不能据此宣称完整实现了全部元素附着规则。
- `apply_element` 的默认反应后果、`deal_damage` 的若干反应伤害加成仍有 TODO。文档不补写源码尚未执行的游戏效果。
- 回合开始的掷骰子仍由独立指令安排，与回合开始广播的整合尚未完成；本次只调整回合计数及观察边界，目标顺序和未修范围见[执行观察与输入](execution_observation.md#本次确定的观察边界)。
- `reaction_between` 对水雷组合返回感电；区分月感电所需状态仍待实现。
- `player_data::clean_up()` 压缩角色数据后，没有同步重映射 `player_state::active_character`。这是源码阅读发现的风险，需要结合角色移除和清理场景另行验证。
- 从非 const 的 `random_fn` 左值构造另一个 `random_fn` 时，模板构造可能优先于隐式复制构造；此时经模板构造的新包装会引用前一个包装对象。已经用独立小程序验证：重新给原包装赋值后，新包装也改用新的来源。后续若修改构造约束，需要同时检视生命周期说明。
- 两个 `damage_flag_bits` 参数的 `operator|` 定义为 `damage_flags` 的隐藏友元；仅有枚举实参时，不能依赖它自然被实参相关查找找到。使用 `damage_flags` 对象作为左操作数可正常组合标志。
- 行动费用原本缺少独立的公开读取入口；如今由 execution_view<action_selection> 的 switch_cost(target_index) 与 card_cost(card_index) 提供指定候选的只读费用引用。内部缓存布局仍不属于公开契约。
- 费用候选的目标只读；每个窗口内只允许计算一次报价，可反复读取已计算结果。提交使用缓存入口和输入，不重跑响应。前一支付效果可能影响后续输入目标的有效性，定义源须遵守命令目标前提，详见[费用缓存](event_dispatch/payment_commit.md#按原顺序执行缓存调用)。

## 维护方式

新增或修改公开接口时，同步检查对应 reference 页面、类或主题索引和文档示例。生成的聚合源码和独立验证夹具保存在 `out/`，不在仓库另存示例副本。文档表达以[写作规范](../reference_stype.md)及用户调整后的模板为准。
