[givm](../../reference.md) / [执行](../executor.md) / **execution_context**

# givm::execution_context

定义于头文件 `<givm/executor.hpp>`

```cpp
class execution_context;
```

供指令处理对局推进的上下文。指令通过它选择继续处理、等待外部输入、进入另一段效果或结束对局。

## 成员函数

|  |  |
| --- | --- |
| [`enter_next`](execution_context/enter_next.md) | 继续执行下一条指令 |
| [`yield_next`](execution_context/yield_next.md) | 暂停并在下次执行下一条指令 |
| [`yield`](execution_context/yield.md) | 暂停并在下次重新执行本条指令 |
| [`enter`](execution_context/enter.md) | 进入另一段效果 |
| [`end_game`](execution_context/end_game.md) | 结束对局 |
| [`stack`](execution_context/stack.md) | 访问临时数据 |
| [`current_stage`](execution_context/current_stage.md) | 访问本条指令记录的进度 |

## 注意

执行器在调用指令时提供此对象。编写指令时，从 `execute` 接收并使用它；对局的外层驱动使用 [`executor`](executor.md)。

## 示例

```cpp
#include <print>
#include <cstdint>
#include <tuple>

#include <givm/givm.hpp>

struct effect
{
    using context_type = void;

    bool execute(givm::card_table& table, givm::execution_context& context, givm::random_fn&) const
    {
        return context.end_game(givm::game_result::player_0_win);
    }
};

int main()
{
    givm::definition_source_library sources{};
    const auto [library, ids] = sources.compile(
        std::tuple{ effect{} },
        std::tuple{ givm::start_round{ .max_rounds = 0 } }
    );
    givm::card_table table{ library };
    givm::executor execution{};
    execution.enter_entry(library);
    auto random = []() -> std::uint32_t { return 0; };
    const bool continued = execution.execute_next(table, random);
    std::println("请求继续执行: {}", continued);
    std::println("玩家 0 获胜: {}", execution.status() == givm::game_result::player_0_win);
}
```

输出

```text
请求继续执行: false
玩家 0 获胜: true
```
