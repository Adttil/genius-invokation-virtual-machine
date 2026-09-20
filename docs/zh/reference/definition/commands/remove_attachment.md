[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **remove_attachment**

# givm::remove_attachment

定义于头文件 `<givm/definition.hpp>`

使一个普通附属实体或装备离场。

```cpp
struct remove_attachment
{
    using input_type = attachment_removal;
};
```

响应通过 `invoke` 提交一个 [attachment_removal](../events/attachment_removal.md)，目标须为有效实体。

先广播 [entity_will_leave](../events/entity_will_leave.md)，再移除实体并广播 [entity_left](../events/entity_left.md)。若离场前的嵌套响应已经移除了该实体，本次操作不重复移除和发出离场完成通知。

移除装备后，所属角色的 [`has(type)`](../../table/character_view/has.md) 对该装备类别立即返回 false。移除后实体不再出现在通常的遍历和广播中，但其 ID、定义、所属角色、状态和装备类别在 cleanup 前仍可读取。
