[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **apply_element**

# givm::apply_element

定义于头文件 `<givm/definition.hpp>`

```cpp
struct apply_element;
```

元素附着命令。没有反应时更新角色附着；发生反应时提供反应前后两次响应时机。

## 成员类型

| | |
| --- | --- |
| `input_type` | [`apply_element_input`](../command_inputs/apply_element_input.md)，动态模式下的输入类型 |
| [`error_type`](#编译检查) | `apply_element_error` 的别名，即本命令的编译检查错误类型 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | [`relative_character_target`](../events/relative_character_target.md) | 固定模式下的来源角色位置 |
| `target` | [`relative_character_target`](../events/relative_character_target.md) | 固定模式下的目标位置 |
| `element` | [`element`](../../enums/element.md) | 施加的元素 |
| `cause` | [`element_application_cause`](../../enums/element_application_cause.md) | 附着来源的类别，初始为 effect |

## 输入

默认构造 `apply_element{}` 使用动态输入，消费响应通过 `invoke` 提交的一个 [`apply_element_input`](../command_inputs/apply_element_input.md)。显式填写固定目标时不消费输入，执行时解析来源和目标的位置；任一位置不存在时，本次附着无效。固定定位允许生命为零但尚未离场的角色。

## 编译检查

```cpp
struct apply_element_error;
```

`apply_element::error_type` 是 `givm::apply_element_error` 的别名。`apply_element_error` 是本命令的结构化编译错误，`apply_element_error::reason` 是原因枚举。[`compile`](../../executor/compile.md) 根据本次定义集合与程序种类检查以下条件，并收集相应的结构化错误。

### 错误原因

| | |
| --- | --- |
| `dynamic_input_in_root` | 初始化或回合根流程使用了动态输入模式；该模式只允许出现在响应程序中 |
| `invalid_source_player` | `source.player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_source_selection` | `source.selection` 不是 `character_selection::character`；此处只允许单个角色 |
| `invalid_target_player` | `target.player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_target_selection` | `target.selection` 不是 `character_selection::character`；此处只允许单个角色 |
| `invalid_element` | `element` 不是已声明的 `element` 枚举值；`element::none` 本身合法 |
| `invalid_cause` | `cause` 不是 `element_application_cause::effect` 或 `element_application_cause::damage` |

### `apply_element_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::size_t` | 出错字段的数值；定义 ID 使用其 `value()`，枚举使用其底层数值 |

仅与当前 `cause` 对应的附加成员具有诊断含义。`dynamic_input_in_root` 不使用附加成员；动态模式不检查未使用的固定参数。

## 注意

反应判定后先发出 [`elemental_reaction_will_occur`](../events/elemental_reaction_will_occur.md)。其 `new_aura` 已由选中反应定义预填，普通响应可直接修改最终附着；将 `cancel_default_effects` 设为 true 可取消反应定义的默认后续。写入附着后执行未取消的默认效果，反应事实及反应后通知始终保留。

独立附着没有主伤害或反应加伤，但仍执行反应定义的后续；超导、感电和扩散可因此产生派生伤害。所有这些伤害的记录归当前段，后续在结算点处理。来源玩家从原始 source 取得，反应定义表则使用目标对手的本局映射。

默认结晶在反应目标的对方生成一点护盾，采用所选结晶反应定义声明依赖并按名称解析的出战状态定义；独立附着不产生结晶的 1 点反应加伤。随库提供的护盾重复生成时累加至两层，已有超出上限的层数不会因此降低。

默认超载同样支持强制切换。反应判定及标签选择完成时，若目标是其所属玩家的出战角色，就登记该玩家；附着处理完成后，以该玩家当时的出战位置为起点，循环选择下一个存活角色，完成切换及其通知后再广播反应后通知。原目标后来死亡或中途换人不取消已登记的切换；若唯一存活角色已出战则不切换、不通知。观察模式也会报告实际切换产生的 `active_character_changed` 现场。

执行超载时，若当时的出战角色具有 `control_immunity` 附属，则取消此次切换，不产生通知或切人观察现场。该保护不撤销已完成的附着处理。

默认冻结在附着处理后，向仍存活的目标施加定义库指定的冻结附属；同样遵守 [`attach`](attach.md) 的免控与重复施加规则。独立附着没有主伤害，因而不额外扣除冻结反应的 1 点加伤。随库提供的冻结具有 `control` 标签，物理或火伤害会触发其加伤与解除，详见[基础定义源](../../basic_definitions.md#冻结与控制)。

激化、燃烧、绽放分别在发生反应角色的对方请求生成定义库指定的激化领域、燃烧烈焰、草原核。归属由目标决定，与附着来源和当前行动玩家无关；为己方角色附着元素引起反应时，默认实体也生成在对方。已有同定义实体时按其重复生成响应处理，燃烧烈焰遵守召唤物容量限制。默认生成在对应反应的附着处理时完成；重复生成响应若提交程序，先完成它再继续后续伤害或完成通知。替代标签非空时不执行默认生成。其他默认反应后果尚未全部实现。

## 示例

```cpp
#include <utility>
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct character_source
{
    using definition_category = givm::character_view;
    struct definition_type {};
    std::string_view name() const { return "character"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }

    static givm::character_state query(const definition_type&, const givm::character_initial_state&)
    {
        return { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 };
    }
};

int main()
{
    character_source source{};
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    if(not sources.add(source)) return 1;
    auto library_result = compile(
        sources, basics,
        std::tuple{ givm::select_active_character_both{}, givm::apply_element{ .source = givm::relative_character_target{ givm::relative_player::self, 0 }, .target = givm::relative_character_target{ givm::relative_player::opponent, 0 }, .element = givm::element::hydro } },
        std::tuple{}, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    givm::table table{ { .max_rounds = 0, .self_player = givm::player_id{ 0 } } };
    const auto definition = ids.get_id<givm::character_view>("character");
    load_deck(table, library,
        givm::linked_deck{ .characters = { definition } },
        givm::linked_deck{ .characters = { definition } });
    const givm::character_id attacker{ givm::player_id{ 0 }, 0 };
    const givm::character_id target{ givm::player_id{ 1 }, 0 };
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    const auto initialized = execution.start(library, table);
    initialized.resume(library, table, random);
    execution.view_in<givm::execution_state::initial_active_character_selection>().select(library, table, random, givm::character_id{ givm::player_id{ 0 }, 0 });
    execution.view_in<givm::execution_state::remaining_active_character_selection>().select(library, table, random, givm::character_id{ givm::player_id{ 1 }, 0 });
    std::println("目标附着水元素: {}", table[target].state().aura == givm::element_aura::hydro);
}
```

输出

```text
目标附着水元素: true
```

## 参阅

| | |
| --- | --- |
| [`elemental_reaction_will_occur`](../events/elemental_reaction_will_occur.md) | 反应判定后选择替代效果的事件 |
| [`after_elemental_reaction`](../events/after_elemental_reaction.md) | 元素反应处理完成后的通知 |
