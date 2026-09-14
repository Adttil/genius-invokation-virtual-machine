[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<finished>](../finished.md) / **result**

# givm::execution_view<execution_state::finished>::result

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr game_result result() const noexcept;
```
[`game_result`](../../../enums/game_result.md)

取得本场对局的结果。

## 返回值

本场对局的胜负结果。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::table table{};
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    execution.enter_entry(library);
    const auto state = execution.run(library, table, random);
    if(state == givm::execution_state::finished)
    {
        std::println("双方告负: {}",
            execution.view_in<givm::execution_state::finished>().result() == givm::game_result::both_loss);
    }
}
```

输出

```text
双方告负: true
```
