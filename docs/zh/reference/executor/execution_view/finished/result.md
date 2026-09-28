[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<finished>](../finished.md) / **result**

# givm::execution_view<execution_state::finished>::result

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr game_result result() const noexcept(/* Release 为 true，Debug 为 false */);
```
[`game_result`](../../../enums/game_result.md)

取得本场对局的结果。

## 返回值

本场对局的胜负结果。

## 注意

Debug 下，视图不属于当前现场或已经失效时抛出 [`execution_view_error`](../../execution_view_error.md)；Release 保持 `noexcept` 且不检查这些条件。

## 示例

```cpp
#include <utility>
#include <cstdint>
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    givm::definition_source_library sources{};
    auto library_result = compile(
        sources, basics,
        std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    givm::table table{ { .max_rounds = 0 } };
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    const auto initialized = execution.start(library, table);
    const auto state = initialized.resume(library, table, random);
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
