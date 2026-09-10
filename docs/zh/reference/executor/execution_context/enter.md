[givm](../../../reference.md) / [执行](../../executor.md) / [execution_context](../execution_context.md) / **enter**

# givm::execution_context::enter

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TContext>
constexpr bool enter(program_entry<TContext> entry);
```

进入另一段效果。该段效果返回后，会重新进入发起调用的指令，由它继续自己的结算。

## 模板参数

|  |  |
| --- | --- |
| `TContext` | 被进入效果的 context 类型 |

## 参数

|  |  |
| --- | --- |
| `entry` | 有效的程序入口，不能是空入口 |

## 返回值

`true`。

## 注意

入口须属于与本次执行配套的定义库，且调用方须满足该段效果所要求的 context。[`program_entry`](../../definition/program_entry.md) 的终局入口可用于直接结束对局。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct grant_die
{
    using context_type = void;

    bool execute(givm::card_table& table, givm::execution_context& context, givm::random_fn&) const
    {
        auto& dice = table[givm::player_id{ 0 }].state().dice;
        ++dice[givm::elemental_dice::omni];
        std::println("子效果中的万能骰数量: {}", dice[givm::elemental_dice::omni]);
        return context.enter_next();
    }
};

struct call_effect
{
    using context_type = givm::test_event;
    givm::program_entry<void> entry;

    bool execute(givm::card_table& table, givm::execution_context& context, givm::random_fn&) const
    {
        if(context.current_stage() == 0)
        {
            // 记住已经进入过子效果，返回后不再重复调用。
            context.current_stage() = 1;
            return context.enter(entry);
        }

        auto& dice = table[givm::player_id{ 0 }].state().dice;
        ++dice[givm::elemental_dice::pyro];
        std::println("返回后增加的火骰数量: {}", dice[givm::elemental_dice::pyro]);
        return context.enter_next();
    }
};

struct support_source
{
    using definition_category = givm::support_view;

    std::string_view name() const { return "效果调用示例"; }

    givm::program_entry<givm::test_event> compile(givm::definition_compile_context& context) const
    {
        const auto entry = context.add_program<void>(std::tuple{ grant_die{} });
        return context.add_program<givm::test_event>(std::tuple{ call_effect{ entry } });
    }

    static givm::program_entry<givm::test_event> handle(
        const givm::program_entry<givm::test_event>& entry, const givm::support_view&,
        givm::test_event&, const givm::card_table&, givm::random_fn&)
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
        std::tuple{ givm::test_command{} },
        std::tuple{ givm::start_round{ .max_rounds = 0 } }
    );
    givm::card_table table{ library };
    table[givm::player_id{ 0 }].add(ids.get_id<givm::support_view>(source.name()), {});

    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    for(execution.enter_entry(library); execution.execute_next(table, random);)
    {}

    std::println("结算结束后的骰子总数: {}", table[givm::player_id{ 0 }].state().dice.total());
}
```

输出

```text
子效果中的万能骰数量: 1
返回后增加的火骰数量: 1
结算结束后的骰子总数: 2
```
