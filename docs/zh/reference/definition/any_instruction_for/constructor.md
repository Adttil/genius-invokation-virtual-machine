[givm](../../../reference.md) / [定义](../../definition.md) / [any_instruction_for](../any_instruction_for.md) / **(构造函数)**

# givm::any_instruction_for::any_instruction_for

定义于头文件 `<givm/definition.hpp>`

```cpp
template<instruction_compatible_with<TContext> TInstruction>
constexpr explicit any_instruction_for(const TInstruction& instruction) noexcept;
```

保存一条可用于本 context 的指令，使不同类型的指令能够放入同一种容器。

## 模板参数

|  |  |
| --- | --- |
| `TInstruction` | [核心给定集合](../../executor/instructions.md)中的具体指令类型，须满足 [`instruction_compatible_with`](../instruction_compatible_with.md) 及所属类的存储要求 |

## 参数

|  |  |
| --- | --- |
| `instruction` | 要保存的指令，包含本次操作的固定配置 |

## 返回值

（无）

## 注意

保存指令副本，不要求原对象继续存续；指令内部若含指针或 view，其引用对象仍须在执行时有效。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <tuple>
#include <vector>

#include <givm/givm.hpp>

int main()
{
    std::vector<givm::any_instruction_for<void>> initialization{};
    initialization.emplace_back(givm::shuffle_deck{ .player = givm::player_id{ 0 } });
    initialization.emplace_back(givm::shuffle_deck{ .player = givm::player_id{ 1 } });
    givm::definition_source_library sources{};
    const auto [library, ids] = sources.compile(
        initialization, std::tuple{ givm::start_round{ .max_rounds = 1 } }
    );
    givm::card_table table{ library };
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    execution.enter_entry(library);
    execution.run(table, random);
    std::println("初始化操作数量: {}", initialization.size());
    std::println("终局时的回合数: {}", table.state().round_number);
}
```

输出

```text
初始化操作数量: 2
终局时的回合数: 2
```
