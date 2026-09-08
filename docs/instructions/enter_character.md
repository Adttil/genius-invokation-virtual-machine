[返回](../events.md)

# enter_character

按定义为玩家创建一个角色，并通过该角色定义生成初始状态。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `player` | `player_id` | 角色所属玩家。 |
| `definition` | `definition_id<character_view>` | 角色定义。 |

## 栈

本指令不要求特定栈顶布局，也不读写 executor stack。

## 执行

1. 在 `player` 的角色区末尾创建一个定义为 `definition` 的角色，角色状态先使用默认 `character_state{}` 占位。
2. 创建局部 [`character_initialization`](../events/character_initialization.md) 工作区。
3. 只让这个新角色的定义处理该工作区。事件对象不压栈，因为它不需要跨指令保存。
4. 将工作区中的 `state` 写回新角色实体。
5. 忽略 handler 返回入口，并将工作区中的 `state` 写回新角色实体。

## 注意

`character_initialization` 当前约定只修改 event 并返回 null。该约定暂未由单独的静态 Context 类型强制；`enter_character` 不检查也不进入返回值。

本指令不决定初始出战角色；上层可在全部角色入场后使用 [`select_active_character_both`](select_active_character_both.md) 组织双方同步选择，或在固定初始化程序中使用 [`set_active_character`](set_active_character.md)。
