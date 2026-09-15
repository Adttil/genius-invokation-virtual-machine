[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **character_defeated**

# givm::character_defeated

定义于头文件 `<givm/definition.hpp>`

```cpp
struct character_defeated;
```

角色被击倒后的通知。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `target` | `const character_id` | 已被击倒的角色；只读 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::character_defeated event{ .target = { .player_id = givm::player_id{ 0 }, .index = 1 } };
    std::println("被击倒的角色序号: {}", event.target.index);
}
```

输出

```text
被击倒的角色序号: 1
```
