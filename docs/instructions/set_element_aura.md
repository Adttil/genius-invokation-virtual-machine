[返回](../events.md)

# set_element_aura

直接写入角色元素附着的底层命令。

## 字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `target` | `character_id` | 目标角色。 |
| `aura` | `element_aura` | 完整的新附着状态。 |

## 执行

除执行器维护的公共 `stage_t` 后缀外，本命令不要求额外栈输入，也不创建临时 frame。

直接将目标角色的 aura 替换为 `aura`，然后完成；不额外产生广播。它由已经完成反应判断的上层流程使用，扩展效果通常应在固定程序中声明 [`apply_element`](apply_element.md)。
