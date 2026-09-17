[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **skill_target_id**

# givm::skill_target_id

定义于头文件 `<givm/definition.hpp>`

```cpp
using skill_target_id = std::variant<std::monostate, character_id, support_id, summon_id>;
```

技能效果的目标标识。没有目标时保存 std::monostate，其余情况分别表示角色、支援或召唤物。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::skill_target_id target{};
    std::println("初始没有目标: {}", std::holds_alternative<std::monostate>(target));
    target = givm::character_id{ .player_id = givm::player_id{ 1 }, .index = 0 };
    std::println("选择角色目标: {}", std::holds_alternative<givm::character_id>(target));
}
```

输出

```text
初始没有目标: true
选择角色目标: true
```
