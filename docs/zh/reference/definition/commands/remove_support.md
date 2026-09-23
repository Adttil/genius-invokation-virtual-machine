[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **remove_support**

# givm::remove_support

定义于头文件 `<givm/definition.hpp>`

使一个支援离场，并通知其他实体处理相应效果。

```cpp
struct remove_support
{
    relative_player player = relative_player::self;
    definition_id<support_view> definition{};
};
```

## 输入

- 默认构造 `remove_support{}` 使用动态模式，由 `invoke` 提交一个 [support_removal](../events/support_removal.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定的一方。在该玩家的支援中选取首个有效、定义 ID 相同的实体；该实体必须存在。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前效果的本方。动态输入直接指定要操作的有效实体。

## 结算

移除指定实体，再广播 [support_removed](../events/support_removed.md)，完整结算离场响应后继续下一条命令。实体离场后不再参与通常的遍历和广播；在 cleanup 前，其定义和状态仍可由旧 ID 读取。
