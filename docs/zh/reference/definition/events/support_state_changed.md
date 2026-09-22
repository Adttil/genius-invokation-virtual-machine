[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **support_state_changed**

# givm::support_state_changed

定义于头文件 `<givm/definition.hpp>`

支援的状态修改后的自身通知。设置和按增量修改状态都会在写入后通知该实体；响应读取牌桌时已经能看到新状态，返回的程序在修改命令结束前完成。

```cpp
struct support_state_changed
{
    const support_state previous;
    const support_state current;
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `previous` | `const support_state` | 本次修改前的状态 |
| `current` | `const support_state` | 本次修改并裁剪后的状态 |

层数或本回合次数为零时，定义可返回移除自身的程序，也可保留实体。每回合次数重置由定义响应适当的回合事件完成。
