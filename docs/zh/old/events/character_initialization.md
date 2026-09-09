[返回](../events.md)

# character_initialization

用于生成角色初始状态的工作区。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `state` | `character_state` | 将写回角色实体的初始状态，默认值初始化；数值字段均为 0，aura 为 `element_aura::none`。 |

## 响应

[`enter_character`](../instructions/enter_character.md) 会在创建单个角色后立即使用该工作区。[`initialize_characters`](../instructions/initialize_characters.md) 则按槽位顺序为已经装入 table 的角色逐个创建工作区。两条指令都只让当前角色自己的 definition 处理事件，最后把 `state` 写回该角色实体。

这个流程是普通 handler 机制的一种特例：每次调用只涉及当前角色，事件对象不跨指令，因此可以放在局部变量而不是 executor stack 上。handler 使用五参数固定接口，约定只修改 `state` 并返回 null；调用指令会忽略返回入口。
