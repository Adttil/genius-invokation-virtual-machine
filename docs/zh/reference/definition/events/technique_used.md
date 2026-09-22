[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **technique_used**

# givm::technique_used

定义于头文件 `<givm/definition.hpp>`

```cpp
struct technique_used;
```

特技使用完成后的通知。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `technique` | `const attachment_id` | 本次使用的特技标识；只读 |
| `targets` | `const std::array<technique_target_id, 2>` | 本次采用的两个目标位置；只读，未使用的位置忽略 |
| `speed` | [`const action_speed`](../../enums/action_speed.md) | 行动速度；只读 |
| `effect_cancelled` | `const bool` | 本次特技效果是否已被取消；只读 |

## 注意

本次使用的特技及目标与确认的行动一致。无论特技原效果是否被取消，本事件都会在原效果结算之后广播；`effect_cancelled` 保留取消结果。特技分类可通过该特技的定义标签查询。
