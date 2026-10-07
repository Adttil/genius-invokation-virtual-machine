[givm](../../../reference.md) / [定义](../../definition.md) / [effect](../effect.md) / **(构造函数)**

# givm::effect::effect

定义于头文件 `<givm/definition.hpp>`

```cpp
constexpr effect() noexcept = default;
```

构造一个表示没有后续效果的空入口。

## 返回值

（无）

## 示例

```cpp
#include <cstdint>
#include <utility>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct result_source
{
    static constexpr auto category = givm::definition_category::support;
    using entry_type = givm::normal_effect;

    std::string_view name() const { return "终局判定"; }
    entry_type compile(givm::definition_compile_context& context) const
    {
        entry_type effect{};
        std::println("尚无后续效果: {}", effect.is_null());
        effect = context.add_normal_effect(
            std::tuple{ givm::settle{}, givm::end_game{ .result = givm::game_result::both_loss } });
        std::println("已选择终局效果: {}", static_cast<bool>(effect));
        return effect;
    }
    static givm::normal_effect handle(
        const entry_type& entry, givm::round_ended&,
        givm::handle_context<givm::support_view>& context, std::uint32_t = 0)
    {
        return context.invoke(entry);
    }
};

int main()
{
    const result_source source{};
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    if(not sources.add(source)) return 1;
    auto library_result = compile(
        sources, basics,
        std::tuple{}, std::tuple{ givm::start_round{}, givm::settle{} }, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
}
```

输出

```text
尚无后续效果: true
已选择终局效果: true
```
