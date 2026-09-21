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
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0
    };
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{ givm::start_round{ .max_rounds = 0 } }, givm::compile_mode::normal);
    givm::table table{};
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    execution.enter_entry(library);
    const auto state = execution.step(library, table, random);
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
