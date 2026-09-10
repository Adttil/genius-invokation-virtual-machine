[givm](../../reference.md) / [事件](../events.md) / **character_will_be_defeated**

# givm::character_will_be_defeated

定义于头文件 `<givm/executor/events.hpp>`

```cpp
struct character_will_be_defeated;
```

角色即将被击倒。响应者可以阻止本次击倒，并指定恢复的生命。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `target` | `const character_id` | 将被击倒的角色；只读 |
| `prevented` | `bool` | 是否阻止击倒，初始为 false |
| `revive_health` | `std::uint32_t` | 阻止击倒时恢复的生命，初始为 0 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::character_will_be_defeated event{ .target = {} };
    event.prevented = true;
    event.revive_health = 1;
    std::println("阻止击倒: {}", event.prevented);
    std::println("恢复生命: {}", event.revive_health);
}
```

输出

```text
阻止击倒: true
恢复生命: 1
```
