[givm](../../reference.md) / [定义](../definition.md) / **any_instruction_for**

# givm::any_instruction_for

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TContext>
class any_instruction_for;
```

适用于同一种 context 的同构指令值，容纳[核心给定集合](../executor/instructions.md)中的不同具体指令。它适合在运行时组装指令序列，例如根据对局配置决定需要哪些初始化操作。

## 模板参数

|  |  |
| --- | --- |
| `TContext` | 序列允许的 context；`void` 表示无需事件 context |

## 成员类型

|  |  |
| --- | --- |
| [`context_type`](any_instruction_for/context_type.md) | `TContext`，该对象可以用于的 context |

## 成员函数

|  |  |
| --- | --- |
| [(构造函数)](any_instruction_for/constructor.md) | 保存一条兼容指令 |

## 注意

保存的是指令副本。指令须为隐式生命周期类型，可平凡复制、可平凡析构，大小不超过 64 字节，对齐不超过 `alignof(std::max_align_t)`。

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

## 参阅

|  |  |
| --- | --- |
| [`instruction_compatible_with`](instruction_compatible_with.md) | 指令与 context 相容的约束 |
