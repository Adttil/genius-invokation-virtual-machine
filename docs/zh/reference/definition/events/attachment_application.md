[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **attachment_application**

# givm::attachment_application

定义于头文件 `<givm/definition.hpp>`

[attach](../commands/attach.md) 的动态输入，指定本次角色附属实体的目标、定义和状态。

```cpp
struct attachment_application
{
    character_id target;
    definition_id<attachment_view> definition;
    attachment_state state{
        std::numeric_limits<std::uint32_t>::max(),
        std::numeric_limits<std::uint32_t>::max()
    };
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `target` | `character_id` | 目标角色 |
| `definition` | `definition_id<attachment_view>` | 本次使用的定义 |
| `state` | `attachment_state` | 本次请求的状态，各字段默认 `UINT32_MAX`；命令执行时裁剪至定义上限 |

显式指定 `.state = {}` 时，两个字段均为零；部分初始化 `state` 时，省略的字段也会初始化为零。
