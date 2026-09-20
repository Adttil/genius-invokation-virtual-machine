[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **set_active_character_from_input**

# givm::set_active_character_from_input

定义于头文件 `<givm/definition.hpp>`

```cpp
struct set_active_character_from_input;
```

按响应提交的角色 ID 设置出战角色，并处理出战角色变化通知。适用于目标在响应时才能确定的效果。

## 成员类型

| | |
| --- | --- |
| `input_type` | [`active_character_changed`](../events/active_character_changed.md)，通过 `current` 指定目标角色 |

## 注意

输入目标须符合 [`set_active_character`](set_active_character.md) 的要求。执行时将 `current` 设置为该方出战角色，并使用这个事件通知变化。

固定目标可继续使用 `set_active_character`。本命令不读取产生响应的事件或响应者，所需目标须显式提交。
