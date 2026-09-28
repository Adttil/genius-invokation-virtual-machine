[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **附属实体定位**

# givm::attachment_target、givm::equipment_target、givm::relative_attachment_target

定义于头文件 `<givm/definition.hpp>`

附属实体定位描述状态修改、移除和转移命令要操作的角色附属实体。动态输入可以指定具体实体，也可以指定某角色当前的一类装备；固定命令使用相对角色位置，再按定义或装备类别定位。

```cpp
struct equipment_target
{
    character_id character;
    equipment_type type;
};

using attachment_target = std::variant<attachment_id, equipment_target>;

struct relative_attachment_target
{
    relative_character_target character{};
    std::variant<definition_id<attachment_view>, equipment_type> selector{};
};
```

## 动态定位

[`set_attachment_state_input`](../command_inputs/set_attachment_state_input.md)、[`modify_attachment_state_input`](../command_inputs/modify_attachment_state_input.md)、[`remove_attachment_input`](../command_inputs/remove_attachment_input.md) 和 [`transfer_attachment_input`](../command_inputs/transfer_attachment_input.md) 的 `attachment` 使用 `attachment_target`。

| 候选类型 | 含义 |
| --- | --- |
| `attachment_id` | 指定的具体附属实体，执行时必须仍有效 |
| `equipment_target` | 执行时 `character` 角色当前的 `type` 类装备；该角色和装备必须有效，`type` 不能是 `equipment_type::none` |

装备定位不绑定提交输入时的具体实体。例如，响应可为转移后的下一条修改命令提前提交目标角色与 `equipment_type::weapon`；执行到修改命令时才取得该角色当前的武器，无须预先知道转移后的新 ID。若中间效果更换了装备，后续命令操作更换后的装备。

## 固定定位

`relative_attachment_target::character` 使用 [`relative_character_target`](../events/relative_character_target.md)，仅接受 `character_selection::character`。执行时，以指定一方的出战角色为基准进行循环偏移；调用方必须保证能定位到有效角色。

设置状态、按增量修改状态和移除命令直接使用偏移后的位置，不跳过生命值为零但仍有效的角色，因此濒死响应仍可操作该角色尚未移除的附属实体。转移命令则从原角色和目标角色各自的偏移位置向后循环寻找存活角色。

| `selector` 候选类型 | 含义 |
| --- | --- |
| `definition_id<attachment_view>` | 此角色上首个有效、定义相同的附属实体；该实体必须存在 |
| `equipment_type` | 此角色当前的指定类别装备；该装备必须存在，类别不能是 `none` |

使用这些定位对象的命令默认构造时采用动态输入。固定模式显式指定 `selector` 中的定义或装备类别；具体成员位置见各命令页面。

## 参阅

| | |
| --- | --- |
| [`equipment_type`](../../enums/equipment_type.md) | 装备类别 |
| [`transfer_attachment`](transfer_attachment.md) | 转移角色附属实体的命令 |
| [`set_attachment_state`](set_attachment_state.md) | 设置附属实体完整状态的命令 |
| [`modify_attachment_state`](modify_attachment_state.md) | 按增量修改附属实体状态的命令 |
| [`remove_attachment`](remove_attachment.md) | 移除附属实体并广播离场的命令 |
