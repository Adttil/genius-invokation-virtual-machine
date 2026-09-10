[givm](../../../reference.md) / [执行](../../executor.md) / [指令](../instructions.md) / **action_target_kind**

# givm::action_target_kind

定义于头文件 `<givm/executor/instructions/begin_action.hpp>`

```cpp
enum class action_target_kind : std::uint8_t
{
    none,
    character,
    support,
    summon
};
```

行动参数中的目标种类，决定读取哪一个目标标识。

## 枚举值

| | |
| --- | --- |
| `none` | 没有目标 |
| `character` | 角色目标 |
| `support` | 支援目标 |
| `summon` | 召唤物目标 |

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::action_target target{ .kind = givm::action_target_kind::character, .character = { .player_id = givm::player_id{ 1 }, .index = 0 } };
    std::println("使用角色目标: {}", target.kind == givm::action_target_kind::character);
}
```

输出

```text
使用角色目标: true
```
