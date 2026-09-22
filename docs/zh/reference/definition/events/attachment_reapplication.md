[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **attachment_reapplication**

# givm::attachment_reapplication

定义于头文件 `<givm/definition.hpp>`

[attach](../commands/attach.md) 找到已有同定义角色附属实体时，仅交给首个匹配实体的重复请求。响应程序结算完成后，原命令才结束。

```cpp
struct attachment_reapplication
{
    const attachment_state state;
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `state` | `const attachment_state` | 本次请求经定义上限裁剪后的状态 |

当前状态由响应者的 `state()` 读取。响应可选择刷新、累加、独立添加等处理；未提供响应或返回空入口时，本次请求结束。
