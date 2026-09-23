[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **generate_combat_status**

# givm::generate_combat_status

定义于头文件 `<givm/definition.hpp>`

生成出战状态；已有同定义实体时，由它决定本次请求如何影响现有效果。

```cpp
struct generate_combat_status
{
    relative_player player = relative_player::self;
    definition_id<combat_status_view> definition{};
    combat_status_state state{
        std::numeric_limits<std::uint32_t>::max(),
        std::numeric_limits<std::uint32_t>::max()
    };
};
```

## 输入

- 默认构造 `generate_combat_status{}` 使用动态模式，由 `invoke` 提交一个 [combat_status_generation](../events/combat_status_generation.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定的一方。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前效果的本方。动态输入明确指定目标玩家和定义，两者须合法。

## 结算

执行时读取 [combat_status_state_limit](../queries/combat_status_state_limit.md)，将提供的 `state` 各字段分别裁剪至对应上限。`state` 省略时，两个字段均为 `UINT32_MAX`，经同样的裁剪后得到该定义的上限；显式指定较小的值可以创建较少层数或次数的实体。

在目标范围查找首个有效、定义 ID 相同的实体：

- 没有匹配实体时，使用裁剪后的状态创建实体。
- 已有匹配实体时，仅向该实体发送 [combat_status_regeneration](../events/combat_status_regeneration.md)，携带本次裁剪后的状态；响应可从自身读取原有状态。
- 返回的响应程序完整结算后，本命令结束。未提供响应或返回空入口时，不再追加实体。多个同定义实体也只通知首个。

已有实体的响应可以通过 [modify_combat_status_state](modify_combat_status_state.md)、[set_combat_status_state](set_combat_status_state.md)、[remove_combat_status](remove_combat_status.md) 或 [add_combat_status](add_combat_status.md) 表达累加、刷新、删除重建和独立创建。

显式指定 `.state = {}` 时，两个字段均为零；部分初始化 `state` 时，省略的字段也会初始化为零。
