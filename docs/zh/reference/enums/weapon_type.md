[givm](../../reference.md) / [游戏用语](../enums.md) / **weapon_type**

# givm::weapon_type

定义于头文件 `<givm/enums/weapon_type.hpp>`

```cpp
enum class weapon_type : std::uint8_t
{
    none,
    sword,
    claymore,
    polearm,
    bow,
    catalyst
};
```

角色或武器所属的武器类别。

## 枚举值

|  |  |
| --- | --- |
| `none` | 无武器类别 |
| `sword` | 单手剑 |
| `claymore` | 双手剑 |
| `polearm` | 长柄武器 |
| `bow` | 弓 |
| `catalyst` | 法器 |

## 示例

```cpp
#include <algorithm>
#include <array>
#include <print>

#include <givm/enums/weapon_type.hpp>

int main()
{
    const auto character_weapon = givm::weapon_type::bow;
    const std::array candidates{
        givm::weapon_type::sword,
        givm::weapon_type::bow,
        givm::weapon_type::catalyst,
        givm::weapon_type::bow
    };
    std::println("武器类别匹配的候选项: {}", std::ranges::count(candidates, character_weapon));
}
```

输出

```text
武器类别匹配的候选项: 2
```
