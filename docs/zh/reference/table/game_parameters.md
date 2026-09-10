[givm](../../reference.md) / [牌桌](../table.md) / **game_parameters**

# givm::game_parameters

定义于头文件 `<givm/table.hpp>`

```cpp
struct game_parameters;
```

对局采用的容量限制。不同对局可以设置不同的手牌、支援和召唤物上限。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `hand_limit` | `std::uint32_t` | 手牌上限，默认 10 |
| `support_limit` | `std::uint32_t` | 支援上限，默认 4 |
| `summon_limit` | `std::uint32_t` | 召唤物上限，默认 4 |

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::game_parameters value{ .hand_limit = 12 };
    std::println("手牌上限: {}", value.hand_limit);
    std::println("召唤物上限: {}", value.summon_limit);
}
```

输出

```text
手牌上限: 12
召唤物上限: 4
```
