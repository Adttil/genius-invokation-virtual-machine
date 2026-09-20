[givm](../../../reference.md) / [定义](../../definition.md) / [program_entry](../program_entry.md) / **is_null**

# givm::program_entry::is_null

定义于头文件 `<givm/definition.hpp>`

```cpp
[[nodiscard]] constexpr bool is_null() const noexcept;
```

检查入口是否为空。

## 返回值

空入口返回 `true`，非空入口返回 `false`。

## 示例

```cpp
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct result_source
{
    using definition_category = givm::support_view;
    using entry_type = givm::program_entry;

    std::string_view name() const { return "终局判定"; }
    entry_type compile(givm::definition_compile_context& context) const
    {
        entry_type effect{};
        std::println("尚无后续效果: {}", effect.is_null());
        effect = context.add_program(
            std::tuple{ givm::end_game{ .result = givm::game_result::both_loss } });
        std::println("已选择终局效果: {}", !effect.is_null());
        return effect;
    }
    static givm::program_entry handle(
        const entry_type& entry, const givm::support_view&, givm::round_ended&,
        givm::handle_context& context)
    {
        return context.invoke(entry);
    }
};

int main()
{
    const result_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{ givm::start_round{} }, givm::compile_mode::normal);
}
```

输出

```text
尚无后续效果: true
已选择终局效果: true
```
