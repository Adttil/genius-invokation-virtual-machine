[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **summoning**

# givm::summoning

定义于头文件 `<givm/definition.hpp>`

[summon](../commands/summon.md) 的动态输入，指定本次召唤物的目标、定义和状态。

```cpp
struct summoning
{
    player_id player;
    definition_id<summon_view> definition;
    summon_state state{
        std::numeric_limits<std::uint32_t>::max(),
        std::numeric_limits<std::uint32_t>::max()
    };
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | `player_id` | 目标玩家 |
| `definition` | `definition_id<summon_view>` | 本次使用的定义 |
| `state` | `summon_state` | 本次请求的状态，各字段默认 `UINT32_MAX`；命令执行时裁剪至定义上限 |

显式指定 `.state = {}` 时，两个字段均为零；部分初始化 `state` 时，省略的字段也会初始化为零。
