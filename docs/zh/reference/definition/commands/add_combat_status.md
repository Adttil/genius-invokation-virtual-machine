[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **add_combat_status**

# givm::add_combat_status

定义于头文件 `<givm/definition.hpp>`

直接添加一个出战状态。可用于定义希望独立保留多个同类效果，或在移除旧实体后创建新实体的情况。

```cpp
struct add_combat_status
{
    relative_player player = relative_player::current;
    definition_id<combat_status_view> definition{};
    combat_status_state state{
        std::numeric_limits<std::uint32_t>::max(),
        std::numeric_limits<std::uint32_t>::max()
    };
};
```

## 输入

- 默认构造 `add_combat_status{}` 使用动态模式，由 `invoke` 提交一个 [combat_status_addition](../events/combat_status_addition.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定的一方。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前行动玩家。动态输入明确指定目标玩家和定义，两者须合法。

## 结算

执行时读取 [combat_status_state_limit](../queries/combat_status_state_limit.md)，将提供的 `state` 各字段分别裁剪至对应上限。`state` 省略时，两个字段均为 `UINT32_MAX`，经同样的裁剪后得到该定义的上限；显式指定较小的值可以创建较少层数或次数的实体。

直接创建独立实体，同定义实体的存在不改变本次操作。

显式指定 `.state = {}` 时，两个字段均为零；部分初始化 `state` 时，省略的字段也会初始化为零。
