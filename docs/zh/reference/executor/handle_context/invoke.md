[givm](../../../reference.md) / [执行](../../executor.md) / [handle_context](../handle_context.md) / **invoke**

# givm::handle_context::invoke

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class... T>
requires (not std::is_convertible_v<T, std::span<const unsigned char>> && ...)
program_entry invoke(program_entry entry, T&&... inputs);

program_entry invoke(program_entry entry, std::span<const unsigned char> inputs);

template<class... T>
requires (not std::is_convertible_v<T, std::span<const unsigned char>> && ...)
program_entry invoke(substack_t, program_entry entry, T&&... inputs);

program_entry invoke(substack_t, program_entry entry, std::span<const unsigned char> inputs);
```

提交要执行的效果，以及本次效果需要的全部初始输入。普通响应使用不带标记的重载；费用响应使用首参数为 `substack_t{}` 的重载，保留到确认行动后执行。

不需要输入的程序不传输入参数；其余程序可以按执行顺序逐项传入初始事件，或者提交为该固定入口准备好的完整输入字节段。

## 参数

| | |
| --- | --- |
| `substack_t{}` | 费用预览提交所用的标记 |
| `entry` | 当前定义库中通过 `add_program` 登记的非空入口 |
| `inputs` | 逐项传入时是按命令执行顺序排列的初始事件，跳过不消费响应输入的命令；字节 span 则是与入口匹配的完整不透明输入段 |

## 返回值

原样返回 `entry`，响应函数须立即将其返回给调用方。

## 异常

未定义 `NDEBUG` 时，所有重载都会在写入前比较输入的总字节长度；与入口要求不符时抛出 `std::invalid_argument`。发布构建不进行这项检查。

## 注意

`cost_of_switch`、`cost_of_card` 和 `cost_of_skill` 响应若提交后续效果，必须调用 `context.invoke(substack_t{}, entry, inputs...)`，没有输入时也须传这个标记；普通响应使用 `context.invoke(entry, inputs...)`。库不检查是否选对重载，违反此前提属于未定义行为。

逐项传入的输入须平凡可复制。所选程序要求的输入数量、具体类型和顺序，由编译时给出的具体命令值确定，调用时只提供相应的输入值。字节重载接收的是已经准备完成的一整段输入，不要求逐事件描述符或类型元信息。所有调用都由定义源保证输入与入口匹配。debug 构建只检查总字节长度，不要求源提供类型元信息，也不能识别同长度的错误类型或顺序；这些错误仍属于未定义行为。发布构建不校验输入，违反约定属于未定义行为。

入口必须非空，每次响应最多调用一次，且必须使用尾调用形式，例如 `return context.invoke(entry, initial_event);`。调用前完成对当前事件与现场的全部读取，调用后立即返回。上述约定不进行运行期检查，违反时行为未定义。逐项传入的初始事件会在提交前按值复制，供后续命令消费。

字节 span 借用整段输入，数据必须在整个调用期间保持有效，不能依赖可能因本次 `invoke` 而失效的执行现场存储。提交会复制这段字节，不接管原存储。

输入中的指针、span 等仍是借用对象，不因提交而复制其指向的数据。相应数据须满足命令执行所需的生命周期；费用效果可能延迟到最终选择行动后才执行。

不需要后续效果时，响应返回空入口（`return {};`），不调用本函数。提交不保证效果立即执行：费用预览只保留效果，未被选择的候选效果不会执行。

## 示例

下例在响应时根据当前实体选定角色。默认构造的 `set_active_character{}` 使用动态输入，响应通过 `active_character_changed` 提供目标；源无需为不同角色登记不同入口。

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>
#include <givm/givm.hpp>

struct character_source
{
    using definition_category = givm::character_view;

    std::string_view name() const { return "响应选择出战"; }

    givm::program_entry compile(givm::definition_compile_context& context) const
    {
        return context.add_program(std::tuple{ givm::set_active_character{} });
    }

    static givm::program_entry handle(
        const givm::program_entry& entry, const givm::character_view& self,
        givm::test_event&, givm::handle_context& context)
    {
        return context.invoke(entry, givm::active_character_changed{ .current = self.id() });
    }
};

int main()
{
    const character_source source{};
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0
    };
    sources.add(source);
    const auto [library, ids] = compile(sources,
        std::tuple{ givm::test_command{}, givm::end_game{ .result = givm::game_result::both_loss } },
        std::tuple{}, givm::compile_mode::normal);
    givm::table table{};
    load_deck(table, library, givm::linked_deck{
        .characters = { ids.get_id<givm::character_view>("响应选择出战") }
    }, {});

    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    execution.enter_entry(library);
    execution.step(library, table, random);
    const givm::character_id selected{ givm::player_id{ 0 }, 0 };
    std::println("已按响应输入选择出战: {}", table[givm::player_id{ 0 }].state().active_character == selected);
}
```

输出

```text
已按响应输入选择出战: true
```
