[givm](../../reference.md) / [定义](../definition.md) / **定义源协议**

# 定义源协议

定义源描述一种卡牌、角色或其他实体具有什么规则。每个源都可以使用适合自己的 C++ 类型保存配置；多个对局实体共享这份定义，而生命值、可用次数等会随对局变化的状态另由牌桌保存。

源先加入 [`definition_source_library`](definition_source_library.md)，再与本场对局需要的其他源一起编译。源对象不由库拥有，应在源库使用期间保持有效；名称、标签及依赖字符串的字符存储也必须保持有效，编译后名称和标签仍由定义库使用。

最终编译由执行模块的 [`givm::compile`](../executor/compile.md) 完成。[`program_entry`](program_entry.md) 由定义模块提供，定义源协议使用的 [`definition_compile_context`](../executor/definition_compile_context.md) 在执行模块中完整定义；编写需要调用编译上下文的定义源时可包含 `<givm/givm.hpp>`，取得这些类型、公开命令和事件。

## 必需成员

| | |
| --- | --- |
| `using definition_category = ...;` | 定义所属类别，取 [`definition_types`](definition_types.md) 中的类型 |
| `name() const` | 返回同一类别内唯一的完整名称 |
| `compile(definition_compile_context&) const` | 返回这项定义的配置数据，供查询与事件响应使用 |

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

登记后，名称、标签和依赖声明必须保持不变。登记、遍历和编译可以分别调用这些接口；每次返回的范围只消费一次，但多次调用须提供相同内容。

attachment 的装备类别使用 `weapon`、`artifact`、`talent`、`technique` 标签，分别表示武器、圣遗物、天赋、特技；没有这些标签时为普通附属实体。编译后的定义库通过 [`equipment_type`](../executor/definition_library/equipment_type.md) 提供这一分类。武器类型使用 `sword`、`claymore`、`polearm`、`bow`、`catalyst` 标签。同一组内的标签互斥，由定义源保证，不进行冲突检查。装备添加与替换行为见 [`add_attachment`](commands/add_attachment.md)。

附属的 `control` 标签表示控制状态，例如冻结、石化、眩晕或水泡；`control_immunity` 表示阻止施加控制附属及效果引发的切人。控制查询检查当前仍在场的附属，移除其中一个不会解除其他实体提供的控制。免控不解除已经存在的控制，也不妨碍玩家在行动选择时主动切换。具体入口见 [`is_controlled`](../executor/definition_library/is_controlled.md)、[`is_control_immune`](../executor/definition_library/is_control_immune.md) 与 [`attach`](commands/attach.md)。标签只声明分类；到期移除等行为仍由定义响应实现。

当调用方只选择部分定义时，[`definition_source_library::make_issued_id_map`](definition_source_library/make_issued_id_map.md) 和 [`compile`](../executor/compile.md) 会保留源库构造时选定的四个默认反应定义，并自动加入这些定义和所选定义直接或间接依赖的所有定义。按标签匹配的定义也参与这一过程，因此选择一张会生成召唤物的卡牌时，无须再手动选择其召唤物定义。

## 事件响应

源可以为所属类别的只读实体 view 和可订阅事件提供以下静态函数，其中 `definition_type` 代表本源 `compile` 的实际返回类型：

```cpp
static givm::program_entry handle(
    const definition_type& definition, const TView& entity, TEvent& event,
    givm::handle_context& context);
```

`TView` 必须属于 [`views_of_definition`](views_of_definition.md)，`TEvent` 必须属于该 view 的 [`subscribed_events`](subscribed_events.md)。可按具体类型编写重载，也可用受约束的函数模板覆盖多个事件。普通静态源只按匹配的函数判断是否响应，没有匹配函数就不响应；已有函数的返回类型必须正确。

响应函数可以读取实体，通过 `context.table()` 读取牌桌、`context.random()` 取得随机值，并修改事件允许调整的成员。返回类型须为 `program_entry`；不需要后续效果时返回空入口（`return {};`）。需要后续操作时，先在 `compile` 中组合[核心命令](commands.md)，通过 [`add_program`](../executor/definition_compile_context/add_program.md) 登记入口；响应时准备这段程序所需的全部输入，普通响应以 `return context.invoke(entry, inputs...);` 结束响应，费用响应则以 `return context.invoke(givm::substack_t{}, entry, inputs...);` 提交延迟效果。

响应返回的程序执行期间，牌桌的 [`self_player`](../table/table_state.md) 表示该响应实体所属玩家；固定效果命令中的 [`relative_player::self`](commands/relative_player.md) 与 `opponent` 据此确定双方。嵌套响应的程序结束后恢复外层本方，费用响应延迟执行时也使用原响应实体所属玩家。`handle` 调用本身不切换本方；读取自身所属玩家应使用实体参数的 `player().id()`，不能把 `context.table().state().self_player` 当作当前响应者的所属玩家。

[`handle_context`](../executor/handle_context.md) 由执行器提供，不由定义源构造。输入按命令执行顺序提供，每个输入须与相应命令要求的初始事件类型完全匹配；是否消费输入由编译时的具体命令值决定，使用固定参数的命令不占输入位置。输入一般是命令开始时所需的初始事件，所有值都在响应期间确定。每个已编译程序所需输入的数量、类型和顺序都是固定的；响应选择入口后，通过 `context.invoke(entry, events...)` 提供本次输入值。动态定义源适配器也可以传入 `std::span<const unsigned char>`，提交为该入口准备好的完整不透明输入字节段，无须提供逐事件描述符或类型元信息。

一次响应至多调用一次 `invoke`，且必须立即返回其结果。调用可能使当前事件及借用的执行现场引用失效，因此必须先完成全部计算；输入中借用的对象也必须满足相应命令的生命周期要求。定义源须保证输入数量、类型、顺序及所属定义库都与入口匹配。未定义 `NDEBUG` 时只检查输入总字节长度，不符则在写入前抛出 `std::invalid_argument`；不要求类型元信息，也不能识别同长度输入的错误类型或顺序。发布构建不检查，违反约定属于未定义行为。字节 span 的数据须在整个调用期间保持有效，不能依赖可能因本次调用而失效的执行现场存储。

入口是否执行以及何时执行由触发事件的操作决定。切换的 [`cost_of_switch`](events/cost_of_switch.md)、出牌的 [`cost_of_card`](events/cost_of_card.md)、技能的 [`cost_of_skill`](events/cost_of_skill.md) 与特技的 [`cost_of_technique`](events/cost_of_technique.md) 响应在报价时准备后续效果，确认行动后才执行。这些费用响应提交时必须使用首参数为 `givm::substack_t{}` 的 `invoke` 重载，没有输入的程序也不例外；使用普通重载属于未定义行为，不进行运行期检查。报价期间牌桌不变，先前响应只通过费用事件影响后续响应；费用响应不得使用随机数，违反此前提属于未定义行为。当前行动窗口内每个候选只允许计算一次报价，已计算结果可以反复读取；不进行运行期检查。

可打出的牌提供 [`card_effect`](events/card_effect.md) 原效果响应。原效果在费用结算与反制响应完成后执行，没有后续效果时也可返回空入口。主动技能提供 [`skill_effect`](events/skill_effect.md) 原效果响应，未提供时不会成为行动候选；技能分类使用定义标签。主动特技由特技装备提供 [`technique_effect`](events/technique_effect.md) 原效果响应，未提供时不能通过行动选择主动使用。卡牌初始状态、技能与特技初始费用和目标检查采用下述查询接口。

准备技能附属提供 [`prepared_skill_effect`](events/prepared_skill_effect.md) 响应。行动阶段依据是否提供此响应识别准备技能；它在角色未受控制时自动代替行动选择，无需支付，也不产生技能或特技使用通知。响应可修改本次行动速度，并提交效果程序；即使返回空入口，本次准备技能仍会被消耗。

角色定义不订阅事件，负责提供初始状态、初始技能组和分类标签。角色被动能力定义为角色持有的技能，通过相应事件响应参与[全场广播](events.md#全场广播)，不提供 `skill_effect` 时不会成为主动技能候选。

每次事件是否实际生效，由响应函数根据事件和对局状态判断。需要按源对象配置选择响应能力时，使用下述[动态定义源](#动态定义源)协议。

## 查询

查询取得规则信息、检查结果，或修改参数中明确允许写入的状态；不返回效果入口，也不接收随机源。源可为所属类别的 [`supported_queries`](supported_queries.md) 提供以下静态函数；`definition_type` 仍是本源 `compile` 的实际返回类型，`Q` 是具体查询类型：

```cpp
static Q::result_t query(const definition_type& definition, const Q& parameters);
```

可按查询类型编写重载或受约束的函数模板，返回类型必须正好是 `Q::result_t`。查询类型除了嵌套的结果类型，还携带所需的全部参数；需要当前实体、牌桌或目标时，都通过参数对象提供。通常查询只读取参数；[`card_state_modification`](queries/card_state_modification.md) 返回 `void`，只修改显式传入的卡牌 state。

当 `std::is_empty_v<Q>` 为 `true` 时，查询结果只由编译后的定义决定。每次编译定义库时，在该项定义的 `compile` 完成后查询一次并保存结果；游戏运行期间读取已保存的结果，不再调用定义源的 `query`。查询类型须能以 `Q{}` 构造；结果不要求是 C++ 常量表达式。非空查询按每次提供的参数求值。

缺少对应 `query` 时，使用通过参数相关查找（ADL）找到的 [`query_default(parameters)`](query_default.md)，返回类型同样必须是 `Q::result_t`。既没有源查询也没有默认方法时，定义源不满足协议。当前[查询列表](queries.md)中的每种查询均有默认方法；其中卡牌、技能与特技的目标检查仅在目标数量为零时默认返回 `valid_complete`，非零数量返回 `invalid`。

卡牌初始属性由 [`card_initial_state`](queries/card_initial_state.md) 给出，牌自身的费用与是否允许调和保存在 `card_state`。卡牌附属状态通过 [`card_state_modification`](queries/card_state_modification.md) 修改这些属性；此查询接收卡牌 state 的可变引用与该附属状态的只读 state，不读取牌外的动态状态。

支援、召唤物、出战状态和角色附属实体分别通过 [`support_state_limit`](queries/support_state_limit.md)、[`summon_state_limit`](queries/summon_state_limit.md)、[`combat_status_state_limit`](queries/combat_status_state_limit.md)、[`attachment_state_limit`](queries/attachment_state_limit.md) 提供各状态字段的上限，并在编译定义库时缓存。生成和直接添加命令的 `state` 成员默认将各字段设为 `UINT32_MAX`，执行时与显式输入一样按上限裁剪；显式的 `state{}` 仍将各字段初始化为零。召唤、生成和附属请求在实际执行时决定创建新实体，或通知首个已有同定义实体；支援则直接添加独立实体。重复请求和状态修改后的自身通知仍使用普通 `handle` 接口。

角色初始技能通过有参查询 [`character_initial_skill`](queries/character_initial_skill.md) 按索引逐个取得，首次返回无效 ID 时结束。定义源自行决定如何产生和保存这些结果，不要求使用特定容器。

查询结果若包含引用、指针或视图，所引用的数据必须在结果使用期间保持有效。空查询的结果会随定义库保存与复制，定义源须相应保证其所借用数据的生命周期。

`name`、`tags` 及其他各类别共用的分类与依赖接口仍采用各自的具名形式。

## 动态定义源

未声明 `is_dynamic` 或声明为 `static constexpr bool is_dynamic = false;` 时，定义源按上述静态规则提供响应与查询，不调用 `can_handle` 或 `can_query`。Lua 等动态来源的适配器可以声明以下成员，按每个源对象实际提供的能力选择响应与查询：

```cpp
static constexpr bool is_dynamic = true;

template<class TView, class TEvent>
bool can_handle() const;

template<class Q>
bool can_query() const;
```

动态源须为所属类别支持的每个实体 view 与事件组合提供 `can_handle`，并为每种支持的查询提供 `can_query`；两者的返回类型都必须为 `bool`。实际调用仍由前述静态 `handle`、`query` 函数实现，适配器所需的脚本状态、回调引用等数据由 `compile` 返回的配置保存。

| 判断结果 | 编译定义库时的行为 |
| --- | --- |
| `can_handle<TView, TEvent>()` 为 `true` | 启用对应 `handle` |
| `can_handle<TView, TEvent>()` 为 `false` | 不提供该响应 |
| `can_query<Q>()` 为 `true` | 使用对应 `query` |
| `can_query<Q>()` 为 `false` | 使用 `query_default` |

能力判断在编译定义库时对具体源对象进行。若返回 `true` 却没有匹配的实现，编译定义库抛出 `std::invalid_argument`；已有实现返回类型错误则属于 C++ 编译错误。能力判断须与源实际提供的实现一致。查询选定后仍遵守空参数查询求值一次、非空参数查询按本次参数求值的规则。

游戏运行期间不再调用源对象的能力判断。定义库公开的 [`can_handle`](../executor/definition_library/can_handle.md) 查询返回本次编译确定的响应能力。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct passive_skill_source
{
    using definition_category = givm::skill_view;

    std::string_view name() const { return "重投助手"; }
    int compile(givm::definition_compile_context&) const { return 1; }

    static givm::program_entry handle(
        const int& extra_rerolls,
        const givm::skill_view&,
        givm::dice_roll_preparation& event,
        givm::handle_context& context)
    {
        event.reroll_count[0] += extra_rerolls;
        return {};
    }
};

int main()
{
    const passive_skill_source source{};
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{ givm::start_round{} }, givm::compile_mode::normal
    );
    const auto id = ids.get_id<givm::skill_view>("重投助手");

    std::println("响应掷骰准备: {}", library.can_handle<givm::dice_roll_preparation, givm::skill_view>(id));
}
```

输出

```text
响应掷骰准备: true
```
