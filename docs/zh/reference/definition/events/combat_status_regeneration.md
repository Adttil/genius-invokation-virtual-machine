[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **combat_status_regeneration**

# givm::combat_status_regeneration

定义于头文件 `<givm/definition.hpp>`

[generate_combat_status](../commands/generate_combat_status.md) 找到已有同定义出战状态时，仅交给首个匹配实体的重复请求。响应程序结算完成后，原命令才结束。

```cpp
struct combat_status_regeneration
{
    const combat_status_state state;
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `state` | `const combat_status_state` | 本次请求经定义上限裁剪后的状态 |

当前状态由响应者的 `state()` 读取。响应可选择刷新、累加、独立添加等处理；未提供响应或返回空入口时，本次请求结束。
