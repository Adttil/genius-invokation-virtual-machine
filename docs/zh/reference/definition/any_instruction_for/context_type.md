[givm](../../../reference.md) / [定义](../../definition.md) / [any_instruction_for](../any_instruction_for.md) / **context_type**

# givm::any_instruction_for::context_type

定义于头文件 `<givm/definition.hpp>`

```cpp
using context_type = TContext;
```

所保存指令允许使用的 context。编写通用的效果组装代码时，可以用它保持指令容器与登记效果的 context 一致。

## 示例

```cpp
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct support_source
{
    using definition_category = givm::support_view;

    std::string_view name() const { return "洗牌助手"; }

    givm::handler_program_entry_t<givm::round_ended> compile(givm::definition_compile_context& context) const
    {
        auto entry = context.add_program<givm::any_instruction_for<givm::round_ended>::context_type>(
            std::tuple{ givm::shuffle_deck{ .player = givm::player_id{ 0 } } }
        );
        std::println("已登记回合结束效果: {}", static_cast<bool>(entry));
        return entry;
    }

    static givm::handler_program_entry_t<givm::round_ended> handle(
        const givm::handler_program_entry_t<givm::round_ended>& entry,
        const givm::support_view&,
        givm::round_ended&,
        const givm::table&,
        givm::random_fn&
    )
    {
        return entry;
    }
};

int main()
{
    const support_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = sources.compile(
        std::tuple{}, std::tuple{ givm::start_round{} }
    );
}
```

输出

```text
已登记回合结束效果: true
```
