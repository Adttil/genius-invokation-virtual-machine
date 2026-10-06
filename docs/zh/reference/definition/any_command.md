[givm](../../reference.md) / [定义](../definition.md) / **any_command**

# givm::any_command

定义于头文件 `<givm/definition.hpp>`

```cpp
using any_command = std::variant</* 核心命令类型 */>;
```

核心命令集合的 `std::variant` 别名，容纳[核心给定集合](commands.md)中的不同具体命令。它适合在运行时组装命令序列，例如根据对局配置决定需要哪些初始化操作。

## 注意

直接使用 `std::variant` 的构造、赋值、`std::get`、`std::get_if` 和 `std::visit` 接口，没有额外包装类。保存的是命令值，原对象在构造后可以销毁。可用命令及其参数要求见各[命令页面](commands.md)。

命令中的 `span` 等借用成员仍引用原数据，不随 variant 复制其内容。相应数据须保持有效，直到接收该序列的 [`compile`](../executor/compile.md) 或 [`add_program`](../executor/definition_compile_context/add_program.md) 返回。

## 示例

```cpp
#include <utility>
#include <cstdint>
#include <print>
#include <tuple>
#include <vector>

#include <givm/givm.hpp>

int main()
{
    std::vector<givm::any_command> initialization{};
    initialization.emplace_back(givm::shuffle_deck{ .player = givm::player_id{ 0 } });
    initialization.emplace_back(givm::shuffle_deck{ .player = givm::player_id{ 1 } });
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    auto library_result = compile(
        sources, basics,
        initialization, std::tuple{ givm::start_round{}, givm::settle{} }, givm::compile_mode::normal
    );
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    givm::table table{ { .max_rounds = 1 } };
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    const auto initialized = execution.start(library, table);
    initialized.resume(library, table, random);
    std::println("初始化操作数量: {}", initialization.size());
    std::println("终局时的回合数: {}", table.state().round_number);
}
```

输出

```text
初始化操作数量: 2
终局时的回合数: 2
```
