[givm](../../reference.md) / [游戏用语](../enums.md) / **attachment_slot**

# givm::attachment_slot

定义于头文件 `<givm/enums/attachment_slot.hpp>`

```cpp
enum class attachment_slot : std::uint8_t
{
    none,
    weapon,
    artifact,
    talent,
    technique
};
```

角色装备所占用的位置，用于区分武器、圣遗物等装备类别。

## 枚举值

|  |  |
| --- | --- |
| `none` | 不占装备位置 |
| `weapon` | 武器 |
| `artifact` | 圣遗物 |
| `talent` | 天赋 |
| `technique` | 特技 |

## 示例

```cpp
#include <map>
#include <print>
#include <string_view>

#include <givm/enums/attachment_slot.hpp>

int main()
{
    const std::map<givm::attachment_slot, std::string_view> equipment{
        { givm::attachment_slot::weapon, "示例武器" },
        { givm::attachment_slot::artifact, "示例圣遗物" }
    };
    std::println("武器位置: {}", equipment.at(givm::attachment_slot::weapon));
    std::println("已配置天赋: {}", equipment.contains(givm::attachment_slot::talent));
}
```

输出

```text
武器位置: 示例武器
已配置天赋: false
```
