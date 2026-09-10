[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **active_character_changed**

# givm::active_character_changed

定义于头文件 `<givm/executor/events.hpp>`

```cpp
struct active_character_changed;
```

出战角色设置完成后的通知。响应者可以通过新的出战角色找到对应玩家。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `current` | `const character_id` | 新的出战角色标识；只读 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::active_character_changed event{ .current = { .player_id = givm::player_id{ 1 }, .index = 2 } };
    std::println("玩家 1 更换出战角色: {}", event.current.player_id == givm::player_id{ 1 });
    std::println("出战角色序号: {}", event.current.index);
}
```

输出

```text
玩家 1 更换出战角色: true
出战角色序号: 2
```

## 参阅

| | |
| --- | --- |
| [`set_active_character`](../instructions/set_active_character.md) | 直接设置出战角色的指令 |
