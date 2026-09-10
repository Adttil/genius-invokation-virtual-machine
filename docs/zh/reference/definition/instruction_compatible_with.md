[givm](../../reference.md) / [定义](../definition.md) / **instruction_compatible_with**

# givm::instruction_compatible_with

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TInstruction, class TContext>
concept instruction_compatible_with =
    std::same_as<
        std::remove_cvref_t<typename std::remove_cvref_t<TInstruction>::context_type>,
        void
    >
    || std::same_as<
        std::remove_cvref_t<typename std::remove_cvref_t<TInstruction>::context_type>,
        std::remove_cvref_t<TContext>
    >;
```

一条指令适用于给定 context 的约束。无需 context 的自由指令可用于任何效果；有 context 要求的指令只能用于对应的效果。

## 模板参数

|  |  |
| --- | --- |
| `TInstruction` | 要检查的指令类型 |
| `TContext` | 效果允许的 context |

## 注意

只检查 `context_type` 的兼容性，不检查指令大小、执行函数或当前运行环境。满足本概念不表示该类型属于允许使用的[核心指令集合](../executor/instructions.md)。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    std::println("洗牌可用于伤害效果: {}",
        givm::instruction_compatible_with<givm::shuffle_deck, givm::damage_effect>);
    std::println("抵挡伤害可用于伤害效果: {}",
        givm::instruction_compatible_with<givm::absorb_damage_by_count, givm::damage_effect>);
    std::println("抵挡伤害可用于回合结束: {}",
        givm::instruction_compatible_with<givm::absorb_damage_by_count, givm::round_ended>);
}
```

输出

```text
洗牌可用于伤害效果: true
抵挡伤害可用于伤害效果: true
抵挡伤害可用于回合结束: false
```
