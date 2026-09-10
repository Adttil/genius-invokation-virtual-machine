[givm](../../reference.md) / [指令](../instructions.md) / **action_target**

# givm::action_target

定义于头文件 `<givm/executor/instructions/begin_action.hpp>`

```cpp
struct action_target;
```

行动所指定的目标。使用 kind 指出目标种类，再填写对应的角色、支援或召唤物标识。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `kind` | [`action_target_kind`](action_target_kind.md) | 目标种类，初始为 none |
| `character` | [`character_id`](../table/character_id.md) | 角色目标 |
| `support` | [`support_id`](../table/support_id.md) | 支援目标 |
| `summon` | [`summon_id`](../table/summon_id.md) | 召唤物目标 |

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

## 参阅

| | |
| --- | --- |
| [`begin_action`](begin_action.md) | 提交玩家行动 |
