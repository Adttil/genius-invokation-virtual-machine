[givm](../../../reference.md) / [执行](../../executor.md) / [execution_context](../execution_context.md) / **current_stage**

# givm::execution_context::current_stage

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr stage_t& current_stage() noexcept;
```

访问本条指令用于记录自身进度的值，以便在重新进入时继续此前的结算。

## 返回值

本条指令的进度值引用，类型为 [`stage_t`](../stage_t.md)。

## 注意

新进入的指令从零开始；推进到下一条指令时重置为零。数值含义由该指令自己决定，不构成相邻指令之间的数据协议。调用时须处于该指令自己的执行帧。

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
        if(context.current_stage() == 0)
        {
            context.current_stage() = 1;
            return context.yield();
        }
        ++table.state().round_number;
        return context.enter_next();
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
    std::println("再次执行请求继续: {}", execution.execute_next(table, random));
    std::println("完成的次数: {}", table.state().round_number);
}
```

输出

```text
首次执行请求继续: false
再次执行请求继续: true
完成的次数: 1
```
