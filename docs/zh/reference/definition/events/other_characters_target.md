[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **other_characters_target**

# givm::other_characters_target

定义于头文件 `<givm/definition.hpp>`

对指定角色以外的同方存活角色分别造成伤害。不要求被排除的角色是当前出战角色。

```cpp
struct other_characters_target
{
    character_id excluded;
};
```

从 `excluded` 的下一个角色位置开始，循环一圈，记录其余有效且存活的角色，按此顺序逐个处理。`excluded` 已经被扣至 0 生命也可以用于排除和确定顺序。

范围展开后不重新定位角色。轮到某个已记录目标时若它已被移除或生命为 0，跳过该目标；期间复活的其他角色不会加入已经展开的范围。

参见 [`damage`](damage.md)。
