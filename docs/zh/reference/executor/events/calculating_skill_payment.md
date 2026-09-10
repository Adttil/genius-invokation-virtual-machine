[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **calculating_skill_payment**

# givm::calculating_skill_payment

定义于头文件 `<givm/executor/events.hpp>`

```cpp
struct calculating_skill_payment;
```

使用技能时的支付计算事件。事件标识这次要使用的技能。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `skill` | `const skill_id` | 本次使用的技能标识；只读 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::calculating_skill_payment event{ .skill = {} };
    std::println("技能属于玩家 0: {}", event.skill.character_id.player_id == givm::player_id{ 0 });
}
```

输出

```text
技能属于玩家 0: true
```
