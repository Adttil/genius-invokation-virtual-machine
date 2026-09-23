[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **add_summon**

# givm::add_summon

定义于头文件 `<givm/definition.hpp>`

直接添加一个召唤物。可用于定义希望独立保留多个同类效果，或在移除旧实体后创建新实体的情况。

```cpp
struct add_summon
{
    relative_player player = relative_player::self;
    definition_id<summon_view> definition{};
    summon_state state{
        std::numeric_limits<std::uint32_t>::max(),
        std::numeric_limits<std::uint32_t>::max()
    };
};
```

## 输入

- 默认构造 `add_summon{}` 使用动态模式，由 `invoke` 提交一个 [summon_addition](../events/summon_addition.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定的一方。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前效果的本方。动态输入明确指定目标玩家和定义，两者须合法。

## 结算

执行时读取 [summon_state_limit](../queries/summon_state_limit.md)，将提供的 `state` 各字段分别裁剪至对应上限。`state` 省略时，两个字段均为 `UINT32_MAX`，经同样的裁剪后得到该定义的上限；显式指定较小的值可以创建效果量或可用次数较少的召唤物。

目标玩家的有效召唤物数量小于其当前 [`player_state::summon_limit`](../../table/player_state.md) 时，直接创建独立实体；同定义实体的存在不改变本次操作，但同样占用容量。创建时裁剪后的 `usages` 必须大于零。

容量按命令实际执行时目标玩家的有效召唤物计数；已移除的实体和另一位玩家的召唤物不占用该玩家的容量。每位玩家的上限默认为 4。达到或超过上限时（包括上限为零），本命令不创建实体，也不移除已有召唤物或广播 [summon_removed](../events/summon_removed.md)。从 [resummoning](../events/resummoning.md) 响应中执行本命令时，也遵循相同的容量规则。

显式指定 `.state = {}` 时，两个字段均为零；部分初始化 `state` 时，省略的字段也会初始化为零。
