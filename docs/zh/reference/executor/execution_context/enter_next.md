[givm](../../../reference.md) / [执行](../../executor.md) / [execution_context](../execution_context.md) / **enter_next**

# givm::execution_context::enter_next

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr bool enter_next() noexcept;
```

结束本条指令，随后继续执行下一条指令。

## 返回值

`true`。

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
        table.state().round_number = 3;
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
    std::println("继续自动推进: {}", continued);
    std::println("下一条是回合开始: {}", library.instruction(execution.position()).is<givm::start_round>());
}
```

输出

```text
继续自动推进: true
下一条是回合开始: true
```
