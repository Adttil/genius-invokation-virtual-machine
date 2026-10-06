[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **attachment_removal_effect**

# givm::attachment_removal_effect

```cpp
struct attachment_removal_effect {};
```

实体已离场后仅告知它自身的效果事件。通过 `context.entity()` 读取离场实体的 ID、定义和状态，事件不重复携带 ID。

普通离场通知的候选响应者在自身效果前采样；先完成自身效果及其独立结算域，再更新普通离场历史并向该候选集合发送 `attachment_removed`。自身效果中新建实体不会追溯加入本次普通离场候选集合。
