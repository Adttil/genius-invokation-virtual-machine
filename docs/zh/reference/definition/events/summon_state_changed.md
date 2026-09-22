[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **summon_state_changed**

# givm::summon_state_changed

定义于头文件 `<givm/definition.hpp>`

召唤物的状态修改后的自身通知。设置和按增量修改状态都会在写入后通知该实体；响应读取牌桌时已经能看到新状态，返回的程序在修改命令结束前完成。

```cpp
struct summon_state_changed
{
    const summon_state previous;
    const summon_state current;
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `previous` | `const summon_state` | 本次修改前的状态 |
| `current` | `const summon_state` | 本次修改并裁剪后的状态 |

可用次数变为零时直接移除实体并广播 [summon_removed](summon_removed.md)，不发送本通知。
