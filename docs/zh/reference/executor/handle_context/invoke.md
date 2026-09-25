[givm](../../../reference.md) / [执行](../../executor.md) / [handle_context](../handle_context.md) / **invoke**

# givm::handle_context::invoke

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class... T> // 每个 T 均须为核心命令声明的 input_type
program_entry invoke(program_entry entry, T&&... inputs);

program_entry invoke(program_entry entry, std::span<const any_command_input> inputs);

template<class... T> // 每个 T 均须为核心命令声明的 input_type
program_entry invoke(substack_t, program_entry entry, T&&... inputs);

program_entry invoke(substack_t, program_entry entry, std::span<const any_command_input> inputs);
```

提交要执行的效果，以及本次效果需要的全部 [命令输入](../../definition/command_inputs.md)。普通响应使用不带标记的重载；费用响应使用首参数为 `substack_t{}` 的重载，保留到确认行动后执行。

不需要输入的程序不传输入参数；其余程序按执行顺序逐项传入 `xxx_input` 对象。Lua 等动态定义源适配器也可以提交 [`any_command_input`](../../definition/any_command_input.md) 的 span。

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

未定义 `NDEBUG` 时，在写入前检查输入对象的数量、具体类型和顺序；与入口要求不符时抛出 `std::invalid_argument`。发布构建不进行这些检查，违反输入约定属于未定义行为。

## 注意

`cost_of_switch`、`cost_of_card`、`cost_of_skill` 和 `cost_of_technique` 响应若提交后续效果，必须调用 `context.invoke(substack_t{}, entry, inputs...)`，没有输入时也须传这个标记；普通响应使用 `context.invoke(entry, inputs...)`。库不检查是否选对重载，违反此前提属于未定义行为。

程序要求的输入对象数量、类型和顺序由编译时的具体命令值决定。数组长度属于本次输入值，不参与类型匹配。例如 `deal_damage_input` 不论包含零条、一条还是多条伤害描述，都占一个输入位置。输入须与命令的 `input_type` 相同；字段相同本身不代表类型兼容，显式别名则是同一个类型。

入口必须非空，每次响应最多调用一次，且必须使用尾调用形式，例如 `return context.invoke(entry, set_active_character_input{ target });`。调用前完成对当前事件与现场的全部读取，调用后立即返回。这些使用前提不进行运行期检查，违反时行为未定义。

逐项输入对象在提交前按值取得；输入对象中作为命令数组的 span，其内容也在调用时复制，返回后不再借用原数组。源数组以及 variant span 本身须在复制期间保持有效，不能指向可能因本次调用而移动的执行现场。复制数组不会延长其元素中其他借用对象的生命周期。

不需要后续效果时，响应返回空入口（`return {};`），不调用本函数。提交不保证效果立即执行：费用预览只保留效果，未被选择的候选效果不会执行。

返回程序实际执行时，以响应实体所属玩家作为 [`table_state::self_player`](../../table/table_state.md)，供相对效果命令使用；程序完成后恢复外层本方。`invoke` 本身及调用它的 `handle` 不改变本方，费用预览也不会改变它。

## 示例

下例由角色的被动技能响应事件，依次提交一组伤害和设置出战的目标。伤害数组在响应内准备，`invoke` 复制其内容；两个动态命令各对应一个输入对象。

```cpp
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
        return context.add_program(std::tuple{ givm::deal_damage{}, givm::set_active_character{} });
    }

    static givm::program_entry handle(
        const givm::program_entry& entry, const givm::skill_view& self,
        givm::test_event&, givm::handle_context& context)
    {
        const std::array damages{
            givm::damage{ .source = self.id(), .target = self.character().id(),
                .value = 1, .type = givm::damage_type::physical },
            givm::damage{ .source = self.id(), .target = self.character().id(),
                .value = 2, .type = givm::damage_type::physical }
        };
        return context.invoke(entry, givm::deal_damage_input{ damages },
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
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    sources.add(source);
    sources.add(character);
    const auto [library, ids] = compile(sources,
        std::tuple{ givm::test_command{}, givm::end_game{ .result = givm::game_result::both_loss } },
        std::tuple{}, givm::compile_mode::normal);
    givm::table table{};
    load_deck(table, library, givm::linked_deck{
        .characters = { ids.get_id<givm::character_view>("角色") }
    }, {});

    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    execution.enter_entry(library);
    execution.step(library, table, random);
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
