[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **element_application**

# givm::element_application

定义于头文件 `<givm/definition.hpp>`

响应通过 `invoke` 向默认构造的 [`apply_element`](../commands/apply_element.md) 提交的直接附着输入。此结构仅用于命令输入，不广播，也不在任何类别的订阅列表中；命令根据目标此时的附着判定反应。

```cpp
struct element_application;
```

| 成员 | 类型 | 说明 |
| --- | --- | --- |
| `source` | [`element_application_source_id`](element_application_source_id.md) | 附着来源 |
| `target` | [`character_id`](../../table/character_id.md) | 目标角色 |
| `element` | [`element`](../../enums/element.md) | 施加的元素 |
| `cause` | [`element_application_cause`](../../enums/element_application_cause.md) | 附着原因；默认 `effect` |
