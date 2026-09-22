[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **technique_will_be_used**

# givm::technique_will_be_used

定义于头文件 `<givm/definition.hpp>`

```cpp
struct technique_will_be_used;
```

特技效果生效前的事件。响应者可以调整行动速度或取消特技效果。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `technique` | `const attachment_id` | 本次使用的特技标识；只读 |
| `targets` | `const std::array<technique_target_id, 2>` | 本次采用的两个目标位置；只读，未使用的位置忽略 |
| `speed` | [`action_speed`](../../enums/action_speed.md) | 行动速度 |
| `effect_cancelled` | `bool` | 是否取消特技效果，初始为 false |

## 注意

费用结算完成后广播本事件。响应者可以将 `effect_cancelled` 设为 `true`，跳过特技自身的 [`technique_effect`](technique_effect.md)；这不会退还费用或取消之后的 [`technique_used`](technique_used.md) 广播。特技分类通过定义标签查询。
