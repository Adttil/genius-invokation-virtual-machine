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

可用次数变为零时仍发送本通知。是否离场由该召唤物的响应决定；响应程序可执行 [remove_summon](../commands/remove_summon.md) 并产生 [summon_removed](summon_removed.md) 广播。未提供响应或返回空入口时，召唤物保留修改后的状态。
