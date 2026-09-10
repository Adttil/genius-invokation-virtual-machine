[givm](../../reference.md) / [执行](../executor.md) / **stage_t**

# givm::stage_t

定义于头文件 `<givm/executor.hpp>`

```cpp
using stage_t = std::uint8_t;
```

指令用来记录自身结算进度的数值类型。

## 注意

各值的含义由使用它的指令决定。外部调用方仅在某个输入槽约定要求该类型时使用它，不应据此推断其他结算状态的布局。

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
        givm::stage_t& stage = context.current_stage();
        if(stage == 0)
        {
            stage = 1;
            return context.yield();
        }
        return context.end_game(givm::game_result::both_loss);
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
    std::println("首次执行请求继续: {}", continued);
    execution.execute_next(table, random);
    std::println("再次执行后双方告负: {}", execution.status() == givm::game_result::both_loss);
}
```

输出

```text
首次执行请求继续: false
再次执行后双方告负: true
```

## 参阅

|  |  |
| --- | --- |
| [`execution_context::current_stage`](execution_context/current_stage.md) | 访问本条指令的进度 |
