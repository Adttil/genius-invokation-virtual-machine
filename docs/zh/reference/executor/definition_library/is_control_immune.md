[givm](../../../reference.md) / [执行](../../executor.md) / [definition_library](../definition_library.md) / **is_control_immune**

# givm::definition_library::is_control_immune

定义于头文件 `<givm/executor.hpp>`

```cpp
bool is_control_immune(character_view character) const noexcept;
```

检查角色是否具有免控保护。保护会阻止控制附属的施加，以及使该角色离开出战位置的非主动效果切换。

## 参数

| | |
| --- | --- |
| `character` | 与本定义库配套牌桌中的有效角色 |

## 返回值

角色当前仍在场的附属中，存在定义带 `control_immunity` 标签的实体时返回 `true`，否则返回 `false`。

## 注意

[`attach`](../../definition/commands/attach.md) 与 [`add_attachment`](../../definition/commands/add_attachment.md) 在施加 `control` 附属时检查此保护，已有同定义实体的重复施加也会被阻止。保护不移除已有控制，因此本函数与 [`is_controlled`](is_controlled.md) 可以同时返回 `true`。

[`set_active_character`](../../definition/commands/set_active_character.md) 与组末超载检查当时出战角色的保护。玩家在行动选择中主动切换不受此保护限制。

每次查询读取角色当前仍在场的附属；保护附属被移除后，后续查询立即反映变化。附属状态值为零不会自行取消标签效果，规则需要通过移除附属结束保护。
