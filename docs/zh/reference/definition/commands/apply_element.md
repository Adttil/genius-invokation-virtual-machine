[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **apply_element**

# givm::apply_element

定义于头文件 `<givm/definition.hpp>`

```cpp
struct apply_element;
```

元素附着命令。没有反应时更新角色附着；发生反应时提供反应前后两次响应时机。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | [`element_application_source_id`](../events/element_application_source_id.md) | 施加元素的来源 |
| `target` | [`character_id`](../../table/character_id.md) | 受到附着的有效角色 |
| `element` | [`element`](../../enums/element.md) | 施加的元素 |
| `cause` | [`element_application_cause`](../../enums/element_application_cause.md) | 附着来源的类别，初始为 effect |

## 注意

反应判定后先发出 [`elemental_reaction_will_occur`](../events/elemental_reaction_will_occur.md)，由响应者选择 `replacement_reaction`。后写标签无条件覆盖前写；空标签表示默认反应效果，非空标签取消默认派生伤害、实体生成和超载切人。两者都按原始反应处理附着消耗，并保留原始反应种类及反应后通知。替代效果可在后续已有事件中根据标签执行，其中另行造成的伤害独立结算。

独立附着不产生伤害类默认效果：不应用反应加伤，超导、感电不产生后台穿透伤害，扩散也不产生后台元素伤害。因此不会广播伤害属性、数值、抵消或伤害后事件，也不会产生 `health_reduced` 观察现场。元素消耗、默认实体生成、冻结、超载切人及反应前后通知仍然有效，与伤害引发的反应相同。

默认超载同样支持强制切换。反应判定及标签选择完成时，若目标是其所属玩家的出战角色，就登记该玩家；附着处理完成后，以该玩家当时的出战位置为起点，循环选择下一个存活角色，完成切换及其通知后再广播反应后通知。原目标后来死亡或中途换人不取消已登记的切换；若唯一存活角色已出战则不切换、不通知。观察模式也会报告实际切换产生的 `active_character_changed` 现场。

组末执行超载时，若当时的出战角色具有 `control_immunity` 附属，则取消此次切换，不产生通知或切人观察现场。该保护不撤销已完成的附着处理。

默认冻结在附着处理后，向仍存活的目标施加定义库指定的冻结附属；同样遵守 [`attach`](attach.md) 的免控与重复施加规则。独立附着没有主伤害，因而不额外扣除冻结反应的 1 点加伤。随库提供的冻结具有 `control` 标签，物理或火伤害会触发其加伤与解除，详见[基础定义源](../../basic_definitions.md#冻结与控制)。

激化、燃烧、绽放分别在发生反应角色的对方请求生成定义库指定的激化领域、燃烧烈焰、草原核。归属由目标决定，与附着来源和当前行动玩家无关；为己方角色附着元素引起反应时，默认实体也生成在对方。已有同定义实体时按其重复生成响应处理，燃烧烈焰遵守召唤物容量限制。默认生成在对应反应的附着处理时完成；重复生成响应若提交程序，先完成它再继续后续伤害或完成通知。替代标签非空时不执行默认生成。其他默认反应后果尚未全部实现。

## 示例

```cpp
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
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::apply_element{ .source = givm::character_id{ .player_id = givm::player_id{ 0 }, .index = 0 }, .target = { .player_id = givm::player_id{ 1 }, .index = 0 }, .element = givm::element::hydro } },
        std::tuple{}, givm::compile_mode::normal);
    givm::table table{ { .max_rounds = 0 } };
    const auto definition = ids.get_id<givm::character_view>("character");
    load_deck(table, library,
        givm::linked_deck{ .characters = { definition } },
        givm::linked_deck{ .characters = { definition } });
    const givm::character_id attacker{ givm::player_id{ 0 }, 0 };
    const givm::character_id target{ givm::player_id{ 1 }, 0 };
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.step(library, table, random);
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
