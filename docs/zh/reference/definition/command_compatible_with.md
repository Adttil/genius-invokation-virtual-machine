[givm](../../reference.md) / [定义](../definition.md) / **command_compatible_with**

# givm::command_compatible_with

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TCommand, class TContext>
concept command_compatible_with = /* 见下文 */;
```

命令或命令 variant 适用于给定 context 的约束。去掉 `TCommand` 和 `TContext` 的 cv/ref 修饰后，具体命令的 `context_type` 为 `void` 或与 `TContext` 相同即为兼容；`std::variant` 则要求每个候选类型均兼容。

## 模板参数

|  |  |
| --- | --- |
| `TCommand` | 要检查的命令类型 |
| `TContext` | 效果允许的 context |

## 注意

只检查 context 兼容性，不检查当前运行环境。满足本概念不表示该类型属于允许使用的[核心命令集合](commands.md)。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    std::println("洗牌可用于伤害效果: {}",
        givm::command_compatible_with<givm::shuffle_deck, givm::damage_effect>);
    std::println("抵挡伤害可用于伤害效果: {}",
        givm::command_compatible_with<givm::absorb_damage_by_count, givm::damage_effect>);
    std::println("抵挡伤害可用于回合结束: {}",
        givm::command_compatible_with<givm::absorb_damage_by_count, givm::round_ended>);
}
```

输出

```text
洗牌可用于伤害效果: true
抵挡伤害可用于伤害效果: true
抵挡伤害可用于回合结束: false
```
