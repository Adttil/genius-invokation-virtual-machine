[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **attachment_added**

# givm::attachment_added

定义于头文件 `<givm/definition.hpp>`

通知一个附属实体或装备已经加入角色。

```cpp
struct attachment_added
{
    const attachment_id attachment;
};
```

`attachment` 是刚创建的实体。广播开始时可通过牌桌读取其定义、所属角色、装备类别及初始状态，新实体自身也可以响应这个事件。

若其他响应在结算期间移除了它，旧 ID 在 cleanup 前仍可读取实体信息；继续广播时跳过已经移除的响应者。
