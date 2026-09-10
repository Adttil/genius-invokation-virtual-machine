[givm](../../reference.md) / [牌桌](../table.md) / **character_state**

# givm::character_state

定义于头文件 `<givm/table.hpp>`

```cpp
struct character_state;
```

角色在对局中的生命值、充能和元素附着。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `max_health` | `std::uint32_t` | 生命值上限 |
| `max_energy` | `std::uint32_t` | 充能上限 |
| `health` | `std::uint32_t` | 当前生命值 |
| `energy` | `std::uint32_t` | 当前充能 |
| `aura` | [`element_aura`](../enums/element_aura.md) | 当前元素附着，默认 none |

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::character_state value{ .max_health = 10, .max_energy = 3, .health = 8, .energy = 1 };
    std::println("生命值: {}/{}", value.health, value.max_health);
    std::println("充能: {}/{}", value.energy, value.max_energy);
}
```

输出

```text
生命值: 8/10
充能: 1/3
```
