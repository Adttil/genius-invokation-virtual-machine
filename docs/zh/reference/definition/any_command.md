[givm](../../reference.md) / [定义](../definition.md) / **any_command**

# givm::any_command

定义于头文件 `<givm/definition.hpp>`

```cpp
using any_command = std::variant</* 核心命令类型 */>;
```

核心命令集合的 `std::variant` 别名，容纳[核心给定集合](commands.md)中的不同具体命令。它适合在运行时组装命令序列，例如根据对局配置决定需要哪些初始化操作。

## 注意

直接使用 `std::variant` 的构造、赋值、`std::get`、`std::get_if` 和 `std::visit` 接口，没有额外包装类。保存的是命令值，原对象在构造后可以销毁。可用命令及其参数要求见各[命令页面](commands.md)。

## 示例

```cpp
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
    givm::definition_source_library sources{};
    const auto [library, ids] = compile(
        sources,
        initialization, std::tuple{ givm::start_round{ .max_rounds = 1 } }, givm::compile_mode::normal
    );
    givm::table table{};
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    execution.enter_entry(library);
    execution.step(library, table, random);
    std::println("初始化操作数量: {}", initialization.size());
    std::println("终局时的回合数: {}", table.state().round_number);
}
```

输出

```text
初始化操作数量: 2
终局时的回合数: 2
```
