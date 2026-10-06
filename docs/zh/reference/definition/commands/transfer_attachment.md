[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **transfer_attachment**

# givm::transfer_attachment

定义于头文件 `<givm/definition.hpp>`

将一个角色的附属状态或装备转移给另一个角色，可同时恢复它的本回合可用次数。

```cpp
struct transfer_attachment_error;

struct transfer_attachment
{
    using error_type = transfer_attachment_error;

    using input_type = transfer_attachment_input;

    relative_attachment_target source{};
    relative_character_target target{};
    bool reset_round_usages = false;
};
```

## 成员类型

| | |
| --- | --- |
| `input_type` | [`transfer_attachment_input`](../command_inputs/transfer_attachment_input.md)，动态模式下的输入类型 |
| [`error_type`](#编译检查) | `transfer_attachment_error` 的别名，即本命令的编译检查错误类型 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | [`relative_attachment_target`](attachment_target.md) | 固定模式下原附属实体的定位 |
| `target` | [`relative_character_target`](../events/relative_character_target.md) | 固定模式下目标角色的位置，只接受 `character` 范围 |
| `reset_round_usages` | `bool` | 固定模式下是否将本回合可用次数恢复至定义给出的上限，默认保留原值 |

## 输入

- 默认构造 `transfer_attachment{}` 使用动态模式，由 [`invoke`](../../executor/handle_context/invoke.md) 提交一个 [transfer_attachment_input](../command_inputs/transfer_attachment_input.md)。
- 在 `source.selector` 中显式指定定义或装备类别时使用固定模式，不消费响应输入。在执行时分别解析 `source.character` 和 `target`，从各自相对于出战角色的偏移位置向后循环寻找存活角色。按定义选择原角色上首个有效匹配实体，或按装备类别选择其当前装备；该实体必须存在。

动态输入的 `attachment` 可以是具体附属实体 ID，也可以是角色 ID 与装备类别，后者在命令执行时取得该角色当前的装备。完整规则见 [附属实体定位](attachment_target.md)。

开始执行时，原附属实体必须有效，目标角色必须有效且存活，并且不能是原所属角色。武器类型是否适合目标角色等用牌条件由定义源自行检查，命令不代替目标合法性查询。

## 结算

若原附属实体带 `control` 标签，而目标具有 `control_immunity` 附属，则停止转移，保留原实体及目标原有装备。

普通附属实体直接转移；目标已有同定义的普通附属实体不影响本次操作，也不产生重复附属通知。

装备类别由定义标签确定。目标已经装备同类装备时，先使目标旧装备离场，再完成本次转移和可选的次数恢复，最后为目标旧装备广播 [attachment_removed](../events/attachment_removed.md)。离场响应看到的是转移完成后的牌桌；被转移的装备可以以新身份参与这次广播。

离场响应若修改、移除或替换了已经转移的实体，这些结果会保留；命令不会在响应结束后重新安装实体或再次恢复次数。

转移总是保留 `count`。`reset_round_usages` 为 `false` 时也保留 `round_usages`；为 `true` 时，将它设为 [attachment_state_limit](../queries/attachment_state_limit.md) 返回的 `round_usages`。若定义用该字段的不同位段编码多个独立次数，上限查询应返回这些次数全部恢复后的编码值。

被转移的实体不产生自身离场、重复附属或入场效果；可选的次数恢复也不发送 [this_attachment_state_change](../events/this_attachment_state_change.md)，不借用回合开始事件。转移后的实体位于目标角色的附属列表末尾；装备仍遵守通常广播中的固定类别顺序。

转移后实体取得属于目标角色的新 ID，旧 ID 对应的实体失效。正在进行的外层广播不会因此补入新实体；实体身份及访问约定见 [实体的身份与访问](../../table/entity_access.md)。

## 编译检查

```cpp
struct transfer_attachment_error;
```

`transfer_attachment::error_type` 是 `givm::transfer_attachment_error` 的别名。`transfer_attachment_error` 是本命令的结构化编译错误，`transfer_attachment_error::reason` 是原因枚举。[`compile`](../../executor/compile.md) 根据本次定义集合与程序种类检查以下条件，并收集相应的结构化错误。

### 错误原因

| | |
| --- | --- |
| `dynamic_input_in_root` | 初始化或回合根流程使用了动态输入模式；该模式只允许出现在响应程序中 |
| `invalid_definition` | `source.selector` 的定义 ID 数值超出本次编译集合的 `attachment_view` 定义数量 |
| `invalid_equipment_type` | `source.selector` 不是有效装备类型，包括使用了 `equipment_type::none` |
| `invalid_source_character_player` | `source.character.player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_source_character_selection` | `source.character.selection` 不是 `character_selection::character`；此处只允许单个角色 |
| `invalid_target_player` | `target.player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_target_selection` | `target.selection` 不是 `character_selection::character`；此处只允许单个角色 |

### `transfer_attachment_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::size_t` | 出错字段的数值；定义 ID 使用其 `value()`，枚举使用其底层数值 |
| `limit` | `std::size_t` | `invalid_definition` 对应类别的定义数量，即有效 ID 数值范围的上界（不含） |

仅与当前 `cause` 对应的附加成员具有诊断含义。`dynamic_input_in_root` 不使用附加成员；动态模式不检查未使用的固定参数。

## 参阅

| | |
| --- | --- |
| [`add_attachment`](add_attachment.md) | 直接添加附属实体的命令 |
| [`remove_attachment`](remove_attachment.md) | 移除附属实体并广播离场的命令 |
| [`set_attachment_state`](set_attachment_state.md) | 设置附属实体完整状态的命令 |
