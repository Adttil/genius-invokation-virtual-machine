[givm](../../reference.md) / [定义](../definition.md) / **any_command_input**

# givm::any_command_input

定义于头文件 `<givm/definition.hpp>`

```cpp
using any_command_input = std::variant<
    set_active_character_input,
    create_hand_card_input,
    discard_hand_card_input,
    discard_deck_cards_input,
    add_support_input,
    set_support_state_input,
    modify_support_state_input,
    remove_support_input,
    summon_input,
    add_summon_input,
    set_summon_state_input,
    modify_summon_state_input,
    remove_summon_input,
    generate_combat_status_input,
    add_combat_status_input,
    set_combat_status_state_input,
    modify_combat_status_state_input,
    remove_combat_status_input,
    attach_input,
    set_attachment_state_input,
    modify_attachment_state_input,
    add_attachment_input,
    remove_attachment_input,
    use_skill_input,
    set_skill_state_input,
    set_energy_input,
    modify_energy_input,
    add_dice_input,
    deal_damage_input,
    apply_element_input,
    heal_input,
    increase_max_health_input
>;
```

所有公开 [命令输入](command_inputs.md) 的 variant，用于在运行时组织输入序列。Lua 等动态定义源适配器可以构造此类型的数组，并以 `std::span<const any_command_input>` 提交给 [`handle_context::invoke`](../executor/handle_context/invoke.md)。

## 注意

每个元素保存一条动态命令的完整输入对象，顺序与程序中的动态命令一致。固定模式命令不占元素。数组参数仍由对应输入对象中的 span 表达，`invoke` 会复制其内容；构造 variant 本身不会复制 span 引用的数组。两种构建模式使用同一接口，debug 检查由 C++ 适配接口完成，不要求脚本另外提供标记。

部分输入是含只读成员事件的别名，因此本 variant 可复制构造，但复制赋值和移动赋值运算符被删除。构建运行时序列时使用构造或 `emplace_back`；替换已有元素的内容时可统一使用 `emplace`，不能假定 `inputs[i] = value` 对所有输入类型都可用。
