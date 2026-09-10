[givm](../../reference.md) / [枚举值](../enums.md) / **skill_kind**

# givm::skill_kind

定义于头文件 `<givm/enums/skill_kind.hpp>`

```cpp
enum class skill_kind : std::uint8_t
{
    normal_attack,
    elemental_skill,
    elemental_burst,
    passive,
    technique
};
```

角色技能的类别，用于区分普通攻击、元素战技、元素爆发及其他能力。

## 枚举值

|  |  |
| --- | --- |
| `normal_attack` | 普通攻击 |
| `elemental_skill` | 元素战技 |
| `elemental_burst` | 元素爆发 |
| `passive` | 被动能力 |
| `technique` | 特技 |

## 示例

```cpp
#include <print>

#include <givm/executor/events.hpp>

int main()
{
    givm::skill_will_be_used event{
        .skill = { { { 0 }, 0 }, 0 },
        .kind = givm::skill_kind::elemental_burst,
        .speed = givm::action_speed::combat
    };
    // 示例效果：仅将元素爆发改为快速行动。
    if(event.kind == givm::skill_kind::elemental_burst)
    {
        event.speed = givm::action_speed::fast;
    }
    std::println("本次技能已改为快速行动: {}", event.speed == givm::action_speed::fast);
}
```

输出

```text
本次技能已改为快速行动: true
```
