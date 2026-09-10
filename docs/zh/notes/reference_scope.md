# 文档范围与源码核对

本轮 reference 以三个核心模块的公开使用方式为主线：定义、牌桌和执行。具体指令与事件单独组织，公共游戏类型及实际使用的栈、类型工具另设主题。公开说明与示例按当前头文件和单测核对；旧文档仅作为理解背景。

## 公开边界

- 牌桌包含实体、只读 view、ID、公开状态、参数和成员操作。`card_data` 用于实际的卡牌搬运接口，因此包含在 reference 中。其余后台 `*_data`、`status_slot`、`invalid_status_index` 和 `detail` 存储辅助对象不作为独立的用户接口推广。
- 定义库返回的指令观察对象虽使用内部类型名，但调用方需要使用其 `is`、`as`、`type_index` 等操作，因此在返回该对象的公开接口文档中说明。
- `execution_context` 用于编写指令，包含继续、暂停、进入效果和结束对局等操作。指令作者使用的自身进度值与外部输入所需的最小后缀有文档；完整调用帧、广播帧和 `return_info` 排列不属于 reference。
- `onpay_item` 只用于 `begin_action` 保存费用响应及其减费记录，不是定义源响应参数，也不在公开输入后缀中，因此不单列 reference。旧测试访问完整费用缓存时使用它，不能据此把缓存布局作为公开契约。
- `broadcast.hpp` 中的辅助函数位于 `givm::detail`，不能据此把其广播顺序写成所有事件的强制约定。
- 未从指令公共入口导出的 `push_selector`、`roll_dice` 仍使用旧式栈配合，`process_dice_roll_phase` 是旧名称。这些遗留指令不列入新 reference。
- `assume_enabled_t`、`assume_enabled` 没有实际调用点；`utils/optional.hpp`、`stable_vector.hpp`、`inplace_vector.hpp` 没有被三个公共入口引用，其中还有未完成的声明或接口。这些遗留文件需另行整理，本轮没有为其建立看似可用的公开 API 页。
- `type_list` 的实现基类、调试辅助函数和成对展开/撤销的内部宏没有独立的 reference 页面。

## 独立于文档整理的源码问题

以下是本轮阅读源码时记录的事项，尚未在本轮修改实现。它们不是新的公开契约。

- `aura_after_reaction` 恒返回无附着，源码保留了附着消耗规则的 TODO；`aura_without_reaction` 也有非反应附着保留规则的 TODO。不能据此宣称完整实现了全部元素附着规则。
- `apply_element` 的默认反应后果、`deal_damage` 的若干反应伤害加成仍有 TODO。文档不补写源码尚未执行的游戏效果。
- `reaction_between` 对水雷组合返回感电；区分月感电所需状态仍待实现。
- `player_data::clean_up()` 压缩角色数据后，没有同步重映射 `player_state::active_character`。这是源码阅读发现的风险，需要结合角色移除和清理场景另行验证。
- 从非 const 的 `random_fn` 左值构造另一个 `random_fn` 时，模板构造可能优先于隐式复制构造；此时经模板构造的新包装会引用前一个包装对象。已经用独立小程序验证：重新给原包装赋值后，新包装也改用新的来源。后续若修改构造约束，需要同时检视生命周期说明。
- 两个 `damage_flag_bits` 参数的 `operator|` 定义为 `damage_flags` 的隐藏友元；仅有枚举实参时，不能依赖它自然被实参相关查找找到。使用 `damage_flags` 对象作为左操作数可正常组合标志。
- `begin_action` 可以通过最小输入后缀提交行动，但尚无单独读取计算后候选费用的公开访问函数。读取这部分费用仍会涉及内部缓存的布局；reference 不把完整内部帧变成调用方的约定。

## 维护方式

新增或修改公开接口时，同步检查对应 reference 页面、类或主题索引和文档示例。生成的聚合源码和独立验证夹具保存在 `out/`，不在仓库另存示例副本。文档表达以[写作规范](../reference/style_guide.md)及用户调整后的模板为准。
