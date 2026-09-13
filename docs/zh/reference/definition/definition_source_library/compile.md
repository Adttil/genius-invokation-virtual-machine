[givm](../../../reference.md) / [定义](../../definition.md) / [definition_source_library](../definition_source_library.md) / **compile**

# givm::definition_source_library::compile

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TInitializationSequence, class TRoundSequence>
definition_compile_result compile(
    TInitializationSequence&& initialization_program,
    TRoundSequence&& round_program
) const; // (1)

template<class TInitializationSequence, class TRoundSequence>
definition_compile_result compile(
    const definition_selection& selection,
    TInitializationSequence&& initialization_program,
    TRoundSequence&& round_program
) const; // (2)
```

准备一场对局要使用的实体定义和对局流程。初始化部分只进行一次，回合部分随后反复进行，直到流程主动暂停或结束对局。

(1) 使用全部已登记定义。(2) 从指定定义出发，自动包含直接和间接依赖的定义；未选中的定义不会编译。

## 模板参数

|  |  |
| --- | --- |
| `TInitializationSequence` | 初始化指令序列，可为 tuple-like 对象或可遍历范围 |
| `TRoundSequence` | 每回合的指令序列，可为 tuple-like 对象或可遍历范围 |

## 参数

|  |  |
| --- | --- |
| `selection` | 各类别首先选择的定义名称 |
| `initialization_program` | 对局开始时依次执行的指令 |
| `round_program` | 每回合依次执行的指令 |

## 返回值

包含定义库及配套 ID 映射的 [`definition_compile_result`](../definition_compile_result.md)。

## 异常

|  |  |
| --- | --- |
| `std::invalid_argument` | 选择了未知定义，或定义源编译时查询了未声明的依赖 |

## 注意

两段流程只能使用[核心给定的指令](../../executor/instructions.md)，也可用 [`any_instruction_for`](../any_instruction_for.md) 保存。指令须与 `void` context 兼容，并满足该容器的存储要求。回合流程必须能够暂停或结束，避免空流程无限运行。定义源的编译操作抛出的异常继续向调用者传播。

指令与编译所得定义数据由返回的定义库持有；源名称和标签的字符存储仍须保持有效。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, ids] = sources.compile(
        std::tuple{}, std::tuple{ givm::start_round{ .max_rounds = 2 } }
    );
    givm::card_table table{ library };
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    execution.enter_entry(library);
    execution.run(table, random);
    std::println("终局时的回合数: {}", table.state().round_number);
    std::println("以双败结束: {}", execution.view_in<givm::execution_state::finished>().result() == givm::game_result::both_loss);
}
```

输出

```text
终局时的回合数: 3
以双败结束: true
```

## 参阅

|  |  |
| --- | --- |
| [`make_issued_id_map`](make_issued_id_map.md) | 在编译前取得配套 ID |
| [`executor::enter_entry`](../../executor/executor/enter_entry.md) | 开始执行一场游戏 |
