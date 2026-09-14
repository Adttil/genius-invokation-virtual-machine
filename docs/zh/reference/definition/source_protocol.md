[givm](../../reference.md) / [定义](../definition.md) / **定义源协议**

# 定义源协议

定义源描述一种卡牌、角色或其他实体具有什么规则。每个源都可以使用适合自己的 C++ 类型保存配置；多个对局实体共享这份定义，而生命值、可用次数等会随对局变化的状态另由牌桌保存。

源先加入 [`definition_source_library`](definition_source_library.md)，再与本场对局需要的其他源一起编译。源对象不由库拥有，应在源库使用期间保持有效；名称、标签及依赖字符串的字符存储也必须保持有效，编译后名称和标签仍由定义库使用。

## 必需成员

| | |
| --- | --- |
| `using definition_category = ...;` | 定义所属类别，取 [`definition_types`](definition_types.md) 中的类型 |
| `name() const` | 返回同一类别内唯一的完整名称 |
| `compile(definition_compile_context&) const` | 返回这项定义的配置数据，供后续事件响应使用 |

`compile` 的返回值不能是 `void` 或引用，须能存入 `std::any`，因此其类型须可复制构造。返回类型由源自行决定，不必命名为 `definition_type`，也不必与 `definition_category` 相同。

## 可选的分类与依赖

下列成员均为无参 `const` 成员，返回可遍历的字符串范围，每个元素可构造 `std::string_view`。省略成员表示相应列表为空。

| | |
| --- | --- |
| `tags()` | 返回定义自身所属的分类标签 |
| `tag_dependencies()` | 返回编译时需要直接取得 ID 的标签名称 |
| `card_dependencies()` / `card_dependencies_by_tag()` | 返回所依赖的卡牌定义名称或标签筛选条件 |
| `status_dependencies()` / `status_dependencies_by_tag()` | 返回所依赖的卡牌状态定义名称或标签筛选条件 |
| `support_dependencies()` / `support_dependencies_by_tag()` | 返回所依赖的支援定义名称或标签筛选条件 |
| `summon_dependencies()` / `summon_dependencies_by_tag()` | 返回所依赖的召唤物定义名称或标签筛选条件 |
| `combat_status_dependencies()` / `combat_status_dependencies_by_tag()` | 返回所依赖的出战状态定义名称或标签筛选条件 |
| `character_dependencies()` / `character_dependencies_by_tag()` | 返回所依赖的角色定义名称或标签筛选条件 |
| `skill_dependencies()` / `skill_dependencies_by_tag()` | 返回所依赖的技能定义名称或标签筛选条件 |
| `attachment_dependencies()` / `attachment_dependencies_by_tag()` | 返回所依赖的角色附着实体定义名称或标签筛选条件 |

`*_dependencies()` 列出名称，`*_dependencies_by_tag()` 列出形如 `治疗 & !料理` 的筛选表达式。每种查询须使用相应的声明：名称依赖通过 `resolve_id` 查询，标签依赖通过 `resolve_tag` 查询，筛选依赖通过 `resolve_ids_by_tag` 查询。只声明名称依赖不会顺带授权标签查询，反之亦然。

当调用方只选择部分定义时，源库会自动加入所选定义直接或间接依赖的所有定义。按标签匹配的定义也参与这一过程，因此选择一张会生成召唤物的卡牌时，无须再手动选择其召唤物定义。

## 事件响应

源可以为所属类别的只读实体 view 和可订阅事件提供以下静态函数，其中 `definition_type` 代表本源 `compile` 的实际返回类型：

```cpp
static givm::handler_program_entry_t<TEvent> handle(
    const definition_type& definition,
    const TView& entity,
    TEvent& event,
    const givm::table& card_table,
    givm::random_fn& random
);
```

`TView` 必须属于 [`views_of_definition`](views_of_definition.md)，`TEvent` 必须属于该 view 的 [`subscribed_events`](subscribed_events.md)。可按具体类型编写重载，也可用受约束的函数模板覆盖多个事件。没有匹配的函数就表示不响应。

响应函数可以读取实体与牌桌，修改事件允许调整的成员，然后返回后续效果的入口；不需要执行额外效果时返回空入口。若需执行后续操作，先在 `compile` 中组合[核心给定的指令](../executor/instructions.md)，通过 [`add_program`](definition_compile_context/add_program.md) 登记，并把取得的入口保存在定义数据中。返回入口的类型必须正好是 `handler_program_entry_t<TEvent>`。

入口是否执行以及何时执行由触发该事件的操作决定。例如，[角色初始化](../executor/events/character_initialization.md)要求在响应函数内直接填写初始状态。

还可以提供 `template<class TView, class TEvent> bool can_handle() const`，按源对象配置禁用某个已经存在的响应函数。返回 `false` 时该响应不进入编译后的定义。这个选择在编译时确定；每次事件是否实际生效，由响应函数根据事件和对局状态判断。

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

    std::string_view name() const { return "重投助手"; }
    int compile(givm::definition_compile_context&) const { return 1; }

    static givm::handler_program_entry_t<givm::dice_roll_preparation> handle(
        const int& extra_rerolls,
        const givm::character_view&,
        givm::dice_roll_preparation& event,
        const givm::table&,
        givm::random_fn&
    )
    {
        event.reroll_count[0] += extra_rerolls;
        return givm::handler_program_entry_t<givm::dice_roll_preparation>::null();
    }
};

int main()
{
    const character_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = sources.compile(
        std::tuple{}, std::tuple{ givm::start_round{} }
    );
    const auto id = ids.get_id<givm::character_view>("重投助手");

    givm::table table{};
    table.load_deck(givm::player_id{ 0 }, givm::linked_deck{ .characters = { id } });
    const auto entity = table[givm::character_id{ givm::player_id{ 0 }, 0 }];
    auto random_source = []() -> std::uint32_t { return 0; };
    givm::random_fn random{ random_source };
    givm::dice_roll_preparation event{ .count = 8 };
    const auto entry = library.handle<givm::dice_roll_preparation>(id, entity, event, table, random);
    std::println("玩家 0 重投次数: {}", event.reroll_count[0]);
    std::println("无需额外结算: {}", entry.is_null());
}
```

输出

```text
玩家 0 重投次数: 2
无需额外结算: true
```
