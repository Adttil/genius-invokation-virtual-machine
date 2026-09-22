[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **technique_effect**

# givm::technique_effect

定义于头文件 `<givm/definition.hpp>`

```cpp
struct technique_effect;
```

执行所选特技自身效果时，提供这项特技及其目标。特技定义在响应中通过 `invoke` 提交程序，实现特技效果。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `technique` | `const attachment_id` | 本次使用的特技；只读。 |
| `targets` | `const std::array<technique_target_id, 2>` | 本次采用的两个目标位置；只读，未使用的位置忽略。 |

## 注意

行动选择只将出战角色的特技装备在支持本事件时列为候选。不支持本事件的特技仍可响应其他事件，不会成为主动使用候选。支持响应不要求每次都提交后续效果。

本事件仅调用所选特技自己的定义。在支付和 [`technique_will_be_used`](technique_will_be_used.md) 的响应完成后，只有未被取消的特技效果才执行本事件。其效果完成后再广播 [`technique_used`](technique_used.md)。

只有带 `technique` 标签且提供本事件响应的装备附件，才会成为特技行动候选。
