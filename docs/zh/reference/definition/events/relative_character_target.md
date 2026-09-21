[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **relative_character_target**

# givm::relative_character_target

定义于头文件 `<givm/definition.hpp>`

相对于某一方当前出战角色的伤害目标。在相应伤害描述开始处理时定位角色，不在响应提交输入时提前绑定。

```cpp
struct relative_character_target
{
    player_id player;
    std::int32_t offset = 0;
};
```

| 成员 | 说明 |
| --- | --- |
| `player` | 目标所属的一方 |
| `offset` | 相对于出战角色位置的循环偏移；0 表示出战位置，1 表示下一个，-1 表示上一个 |

先按角色位置进行循环偏移，再从该位置向后循环寻找有效且存活的角色，最多检查一圈。偏移按位置计算，不能理解为跳过若干名存活角色；没有存活角色时不产生伤害。

目标方须有可作为定位基准的出战角色 ID；该角色生命已经为 0 不妨碍定位其他存活角色。

参见 [`damage`](damage.md)。
