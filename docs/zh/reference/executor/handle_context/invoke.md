[givm](../../../reference.md) / [执行](../../executor.md) / [handle_context](../handle_context.md) / **invoke**

# givm::handle_context::invoke

定义于头文件 `<givm/definition_source.hpp>`

```cpp
template<class... T> // 每个 T 均须为核心命令声明的 input_type
program_entry invoke(program_entry entry, T&&... inputs);

program_entry invoke(program_entry entry, const program_inputs& inputs);

template<class... T> // 每个 T 均须为核心命令声明的 input_type
program_entry invoke(substack_t, program_entry entry, T&&... inputs);

program_entry invoke(substack_t, program_entry entry, const program_inputs& inputs);
```

提交要执行的效果，以及本次效果需要的全部 [命令输入](../../definition/command_inputs.md)。普通响应使用不带标记的重载；费用响应使用首参数为 `substack_t{}` 的重载，保留到确认行动后执行。

不需要输入的程序不传输入参数；其余程序按执行顺序逐项传入 `xxx_input` 对象。延迟命令的输入直接使用 [`defer_invoke`](../../definition/defer_invoke.md) 的返回值。Lua 等动态定义源适配器可以通过 [`pack_inputs`](../../definition/pack_inputs.md) 和 [`concat_inputs`](../../definition/concat_inputs.md) 准备 [`program_inputs`](../../definition/program_inputs.md)，再一次提交。

## 模板参数

| | |
| --- | --- |
| `T` | 核心命令通过 `input_type` 声明的输入类型；忽略引用和 cv 限定。显式复用为输入别名的事件类型同样可用 |

## 参数

| | |
| --- | --- |
| `substack_t{}` | 费用预览提交所用的标记 |
| `entry` | 当前定义库中通过 `add_program` 登记的非空入口 |
| `inputs` | 按命令执行顺序排列的输入对象；每个动态命令恰好对应一个对象，固定模式不占输入位置 |

## 返回值

原样返回 `entry`，响应函数须立即将其返回给调用方。

## 异常

未定义 `NDEBUG` 时，在写入本次输入前检查入口、提交方式、重复提交以及输入对象的数量、具体类型和顺序；违反协议时抛出 [`program_input_error`](../program_input_error.md)。其中 `reason` 区分具体错误，可直接读取结构化字段，也可用 `what()` 或 [`error_string`](../error_string.md) 取得文本。发布构建不进行这些检查，也不保留对应诊断元数据；违反输入约定属于未定义行为。

协议检查失败时不写入本次输入，但不回滚此前响应对事件的修改，也不回滚本次推进已执行的其他效果。捕获异常用于定位定义错误，不应在原现场继续推进执行器。

目标实体、资源数量及具体输入值等前提在命令实际执行时检查，错误以 [`command_input_error`](../command_input_error.md) 报告；不会因为提交时尚未满足、但前序命令会使其满足而拒绝提交。

## 注意

`cost_of_switch`、`cost_of_card`、`cost_of_skill` 和 `cost_of_technique` 响应若提交后续效果，必须调用 `context.invoke(substack_t{}, entry, inputs...)`，没有输入时也须传这个标记；普通响应使用 `context.invoke(entry, inputs...)`。调试构建检查是否选对重载。

程序要求的输入对象数量、类型和顺序由编译时的具体命令值决定。数组长度属于本次输入值，不参与类型匹配。例如 `deal_damage_input` 不论包含零条、一条还是多条伤害描述，都占一个输入位置。输入须与命令的 `input_type` 相同；字段相同本身不代表类型兼容，显式别名则是同一个类型。

第一条 [`return_response`](../../definition/commands/return_response.md) 之后的命令不属于有效程序，因此也不要求输入。动态返回输入不得使用 `return_response::dynamic`。提交 [`defer_program_input`](../../definition/command_inputs/defer_program_input.md) 时，目标入口和嵌套参数也在 Debug 下按同一规则检查。

入口必须非空并属于当前定义库，每次响应最多调用一次。定义库复制保留入口对应关系，原库的入口可用于其副本；独立重新编译的库不能混用入口。调试构建检查这些条件。

必须使用尾调用形式，例如 `return context.invoke(entry, set_active_character_input{ target });`。调用前完成对当前事件与现场的全部读取，调用后立即返回。尾调用与引用生命周期约定不自动检查，违反时行为未定义。

普通逐项输入对象在执行现场可能扩容前取得值快照；其中作为命令数组的 span，其内容也在调用时复制，返回后不再借用原数组。源数组须在复制期间保持有效，不能指向可能因本次调用而移动的执行现场。复制数组不会延长其元素中其他借用对象的生命周期。

[`defer_invoke`](../../definition/defer_invoke.md) 和 [`pack_inputs`](../../definition/pack_inputs.md) 已拥有准备好的参数及数组内容；提交它们时复制的是这份快照。它们在提交返回后可以销毁，无须存活到目标程序实际执行。普通逐项调用不要求预先使用 `pack_inputs`。

不需要后续效果时，响应返回空入口（`return {};`），不调用本函数。提交不保证效果立即执行：费用预览只保留效果，未被选择的候选效果不会执行。

返回程序实际执行时，以响应实体所属玩家作为 [`table_state::self_player`](../../table/table_state.md)，供相对效果命令使用；程序完成后恢复外层本方。`invoke` 本身及调用它的 `handle` 不改变本方，费用预览也不会改变它。

## 示例

下例由角色的被动技能响应事件，依次提交一次伤害和设置出战的目标。`invoke` 复制输入；两个动态命令各对应一个输入对象。

```cpp
#include <utility>
#include <array>
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>
#include <givm/givm.hpp>

struct passive_skill_source
{
    using definition_category = givm::skill_view;

    std::string_view name() const { return "响应选择出战"; }

    givm::program_entry compile(givm::definition_compile_context& context) const
    {
        return context.add_program(std::tuple{ givm::deal_damage{}, givm::deal_damage{}, givm::set_active_character{} });
    }

    static givm::program_entry handle(
        const givm::program_entry& entry,
        givm::round_started&, givm::handle_context<givm::skill_view>& context, std::uint32_t = 0)
    {
        const auto self = context.entity();
        const std::array damages{
            givm::deal_damage_input{ std::array{ givm::damage{ .source = self.id(), .target = self.character().id(),
                .value = 1, .type = givm::damage_type::physical } } },
            givm::deal_damage_input{ std::array{ givm::damage{ .source = self.id(), .target = self.character().id(),
                .value = 2, .type = givm::damage_type::physical } } }
        };
        return context.invoke(entry, damages[0], damages[1],
            givm::set_active_character_input{ .current = self.character().id() });
    }
};

struct character_source
{
    using definition_category = givm::character_view;
    using definition_type = givm::definition_id<givm::skill_view>;
    std::string_view name() const { return "角色"; }
    auto skill_dependencies() const { return std::array<std::string_view, 1>{ "响应选择出战" }; }
    definition_type compile(givm::definition_compile_context& context) const
    {
        return context.resolve_id<givm::skill_view>("响应选择出战");
    }
    static givm::character_state query(const definition_type&, const givm::character_initial_state&)
    {
        return { .max_health = 10, .health = 10 };
    }
    static definition_type query(const definition_type& skill, const givm::character_initial_skill& query)
    {
        return query.skill_index == 0 ? skill : definition_type{};
    }
};

int main()
{
    const passive_skill_source source{};
    const character_source character{};
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    if(not sources.add(source)) return 1;
    if(not sources.add(character)) return 1;
    auto library_result = compile(sources, basics,
        std::tuple{ givm::start_round{}, givm::settle{}, givm::end_game{ .result = givm::game_result::both_loss } },
        std::tuple{}, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    givm::table table{};
    load_deck(table, library, givm::linked_deck{
        .characters = { ids.get_id<givm::character_view>("角色") }
    }, {});

    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    const auto initialized = execution.start(library, table);
    initialized.resume(library, table, random);
    const givm::character_id selected{ givm::player_id{ 0 }, 0 };
    std::println("已按响应输入选择出战: {}", table[givm::player_id{ 0 }].state().active_character == selected);
    std::println("剩余生命: {}", table[selected].state().health);
}
```

输出

```text
已按响应输入选择出战: true
剩余生命: 7
```
