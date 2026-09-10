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
#include <tuple>

#include <givm/givm.hpp>

struct finish_game
{
    using context_type = void;
    givm::program_entry<void> entry;

    bool execute(givm::card_table&, givm::execution_context& context, givm::random_fn&) const
    {
        return context.enter(entry);
    }
};

int main()
{
    givm::definition_source_library sources{};
    const auto [library, ids] = sources.compile(
        std::tuple{},
        std::tuple{ finish_game{ givm::program_entry<void>::both_loss() } }
    );
    givm::card_table table{ library };
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
