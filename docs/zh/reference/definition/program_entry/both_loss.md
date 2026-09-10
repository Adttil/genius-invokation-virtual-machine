[givm](../../../reference.md) / [定义](../../definition.md) / [program_entry](../program_entry.md) / **both_loss**

# givm::program_entry::both_loss

定义于头文件 `<givm/definition.hpp>`

```cpp
[[nodiscard]] static constexpr program_entry both_loss() noexcept;
```

取得使对局以双方失败结束的入口。取得入口本身不会结束对局，进入该入口后才产生终局结果。

## 返回值

表示双方失败的非空终局入口。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct result_source
{
    using definition_category = givm::support_view;
    struct definition_type {};

    std::string_view name() const { return "终局判定"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }

    static givm::program_entry<givm::test_event> handle(
        const definition_type&,
        const givm::support_view&,
        givm::test_event&,
        const givm::card_table&,
        givm::random_fn&
    )
    {
        return givm::program_entry<givm::test_event>::both_loss();
    }
};

int main()
{
    const result_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::test_command{} },
        std::tuple{ givm::start_round{ .max_rounds = 0 } }
    );
    givm::card_table table{ library };
    table[givm::player_id{ 0 }].add(
        ids.get_id<givm::support_view>("终局判定"), givm::support_state{}
    );
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}
    std::println("双方失败: {}", execution.status() == givm::game_result::both_loss);
}
```

输出

```text
双方失败: true
```
