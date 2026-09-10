[givm](../../../reference.md) / [执行](../../executor.md) / [execution_context](../execution_context.md) / **yield**

# givm::execution_context::yield

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr bool yield() noexcept;
```

暂停自动推进，保留本条指令以便下次重新执行。

## 返回值

`false`。

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
        return context.yield();
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
    std::println("下次仍是本条指令: {}", library.instruction(execution.position()).is<effect>());
}
```

输出

```text
继续自动推进: false
下次仍是本条指令: true
```
