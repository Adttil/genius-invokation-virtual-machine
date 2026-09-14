# 定义源协议的编译边界与设计理由

本篇保留定义源从身份声明、依赖解析到运行期响应的完整设计脉络，以及动态 adapter、生命周期和持久化的选择理由。公开接口以[定义源协议](../reference/definition/source_protocol.md)及各 API 页为准；原有声明、表格、约束和配套代码片段在相应主题中继续保留，避免只留下接口名称而丢掉当时的限制和原因。片段用于讨论协议，不是可单独运行的 reference 示例。

对照基线为 `b3d6c50`。旧记录的 Context/栈 ABI 称法、源对象生命周期概括、逻辑编译步骤与实际写入顺序在相应位置标明差异。持久化一节记录上层格式的设计方向，不声称核心已经提供现成序列化器。

definition source 是一个描述单项游戏规则的 C++ 对象。它可以代表一张卡牌、一个角色、一种状态、一个召唤物或其他一种 definition。核心先读取它的身份与依赖，再调用它编译出不可变的 definition；对局执行规则时只读取编译结果，不再调用原 source 对象。原记录由此概括“source 仍需保持存活，因为源库和编译库可以保存由它提供的非拥有字符串视图”；源对象、字符存储和编译结果的具体拥有边界在下文“注册与生命周期”中分别核对。

本文按照编写定义源时理解信息的顺序介绍接口：身份、依赖、编译和 handler。每个接口会分别说明是否必须、参数、返回值和语义。

## 身份标识

对应 API 入口是[定义源协议的必需成员](../reference/definition/source_protocol.md#必需成员)。下面保留单类别及不解析完整名称的设计边界。

### 定义类别

```cpp
using definition_category = support_view;
```

`definition_category` 是必须提供的公开嵌套类型，用来指定这个 source 产生哪一类 definition。一个 source 只产生该类别的一项 definition；需要关联卡牌与支援等不同类别时，分别注册 source 并声明依赖。上例表示它产生 support definition。

可用类别及其运行时实体 view 如下：

| 定义类别 | `definition_category` | 可能传给 handler 的实体 view |
| --- | --- | --- |
| card | `card_definition` | `hand_card_view`、`deck_card_view` |
| card status | `status_definition` | `hand_card_status_view`、`deck_card_status_view` |
| support | `support_view` | `support_view` |
| summon | `summon_view` | `summon_view` |
| combat status | `combat_status_view` | `combat_status_view` |
| character | `character_view` | `character_view` |
| skill | `skill_view` | `skill_view` |
| attachment | `attachment_view` | `attachment_view` |

card 和 card status 的定义类别不是实体 view，因为同一项定义可能分别形成手牌区和牌库区实体。其余类别目前直接使用对应 view 类型作为类别标记。

### 名称

```cpp
std::string_view name() const;
```

`name()` 是必须提供的 const 成员函数，没有参数，返回这个 definition 的完整名称。

名称对核心是不透明的字符串。核心不会解析其中的显示名、版本、作者或其他片段；这些格式约定属于上层和拓展生态。名称的作用域是定义类别，同一类别内不能重名，不同类别可以使用相同名称。

核心会保留返回的 `std::string_view`。底层字符必须在保存该 source 的 `definition_source_library`、由它编译出的 `definition_library` 及相应 `issued_id_map` 使用期间保持有效。

## 标签与依赖

类别间依赖的公开成员表见[可选的分类与依赖](../reference/definition/source_protocol.md#可选的分类与依赖)。以下保留三种依赖不能互相替代的前置声明设计。

依赖在 issued id 产生前声明，因此使用“目标类别 + 名称”或“目标类别 + 标签筛选表达式”标识。编译器先用这些声明检查定义集合并求出依赖闭包，之后 `compile(...)` 才能把声明过的内容解析为 issued id。

同一个 source 的名称、标签和依赖接口在加入源库后必须保持结果一致。

### 定义标签

```cpp
auto tags() const;
```

`tags()` 是可选的 const 成员函数，没有参数。返回值可以是任意 input range，其元素必须可转换为 `std::string_view`；每个字符串表示当前 definition 拥有的一个标签。省略该接口等价于返回空 range。

标签供依赖筛选和编译库查询使用。标签名称同样是不透明字符串，核心只解释标签筛选表达式的组合语法。

### 直接名称依赖

```cpp
auto support_dependencies() const;
```

直接依赖接口都是可选的 const 成员函数，没有参数。返回值可以是任意 input range，其元素必须可转换为 `std::string_view`；每个字符串是目标类别下一个 definition 的完整名称。上例声明 support 依赖。

每种目标类别对应一个接口：

| 目标类别 | 接口 |
| --- | --- |
| card | `card_dependencies()` |
| card status | `status_dependencies()` |
| support | `support_dependencies()` |
| summon | `summon_dependencies()` |
| combat status | `combat_status_dependencies()` |
| character | `character_dependencies()` |
| skill | `skill_dependencies()` |
| attachment | `attachment_dependencies()` |

省略某个接口等价于返回空 range。直接名称依赖是硬依赖：对应 source 不存在时，定义源集合无效。单个添加 source 时，除同类别自依赖外，依赖必须已经存在；原子批量添加允许同一批 source 互相依赖。

### 标签 ID 依赖

```cpp
auto tag_dependencies() const;
```

`tag_dependencies()` 是可选的 const 成员函数，没有参数。返回值可以是任意 input range，其元素必须可转换为 `std::string_view`；每个字符串是 `compile(...)` 稍后需要解析为 `tag_id` 的标签名称。省略该接口等价于返回空 range。

标签 ID 依赖本身不会选入某个 definition。它只声明编译后的 definition 需要保存或使用这个标签的 issued id。

### 标签筛选依赖

```cpp
auto card_dependencies_by_tag() const;
```

标签筛选依赖接口都是可选的 const 成员函数，没有参数。返回值可以是任意 input range，其元素必须可转换为 `std::string_view`；每个字符串是针对目标类别的标签筛选表达式。上例筛选 card definition。

每种目标类别对应一个接口：

| 目标类别 | 接口 |
| --- | --- |
| card | `card_dependencies_by_tag()` |
| card status | `status_dependencies_by_tag()` |
| support | `support_dependencies_by_tag()` |
| summon | `summon_dependencies_by_tag()` |
| combat status | `combat_status_dependencies_by_tag()` |
| character | `character_dependencies_by_tag()` |
| skill | `skill_dependencies_by_tag()` |
| attachment | `attachment_dependencies_by_tag()` |

筛选表达式由 `&` 连接若干标签条件。未否定的标签必须存在，以 `!` 否定的标签必须不存在。例如 `"food & !event"` 选择具有 `food` 且不具有 `event` 标签的定义。匹配的定义全部进入依赖闭包。省略某个接口等价于返回空 range。

### Range 与字符串生命周期

`definition_source_view` 的 [tags](../reference/definition/definition_source_view/tags.md)、[dependencies](../reference/definition/definition_source_view/dependencies.md)、[tag_dependencies](../reference/definition/definition_source_view/tag_dependencies.md) 和 [dependencies_by_tag](../reference/definition/definition_source_view/dependencies_by_tag.md) 都拥有返回的视图容器，但不拥有字符。这里“可返回临时容器”不包括让字符串本体随临时容器一起销毁：例如临时 `vector<string_view>` 可指向稳定字符，临时 `vector<string>` 的字符却会一起消失。

上述接口每次返回的 range 只需支持一次顺序遍历，数量不必在编译期确定。调用方会在本次接口调用后立即完整消费该 range，不保存 range 或元素对象的引用，因此 source 可以返回临时 range、容器或惰性 range。

元素转换所得 `std::string_view` 可能被源库保留，部分名称和标签也由编译库及 `issued_id_map` 借用。其底层字符必须覆盖这些对象的使用期。字符串用于注册、编译和对局前的名称链接；对局规则执行使用 issued ID，不要求运行时按名称查找。

### 依赖声明示例

下面的片段声明一个 support 名称依赖、一个标签 ID 依赖，以及一组由标签筛选的 card 依赖。相同字符串常量稍后会传给编译上下文：

```cpp
static constexpr std::string_view bonus_support_name = "BonusSupport";
static constexpr std::string_view food_tag_name = "food";
static constexpr std::string_view food_filter = "food & !event";

std::array<std::string_view, 1> support_dependencies() const
{
    return { bonus_support_name };
}

std::array<std::string_view, 1> tag_dependencies() const
{
    return { food_tag_name };
}

std::array<std::string_view, 1> card_dependencies_by_tag() const
{
    return { food_filter };
}
```

## 编译定义

入口见 [definition_compile_context](../reference/definition/definition_compile_context.md)。依赖闭包和 ID 先确定，再允许 source 一次构造完整结果，是以下接口共同依赖的顺序。

依赖闭包和本次编译的 issued id 全部确定后，核心调用 source 的 `compile(...)`。

### `compile`

```cpp
auto compile(definition_compile_context& context) const;
```

`compile(...)` 是必须提供的 const 成员函数。

参数 `context` 是只在本次调用期间有效的编译上下文。它可以解析当前 source 已声明的依赖，并把响应程序加入正在构建的游戏规则程序。source 不得在返回对象或其他长期状态中保存该上下文的引用或指针。

返回值是编译后的 definition，必须按值返回一个非 `void`、可复制构造的对象类型。核心直接推导其准确类型、取得所有权，并在运行时以该类型的 const 引用传给 handler。接口不要求 source 提供 `data_type` 或其他用于重复说明返回类型的嵌套别名。本文示例通常把返回类型命名为 `definition_type`，但这只是 source 内部的命名习惯。

`compile(...)` 一次性返回完整对象。普通配置、issued id 和程序入口都在返回前确定，不存在后续 setter 或分阶段补写。

一个没有依赖、程序和额外数据的 source 可以返回空类型：

```cpp
struct definition_type {};

definition_type compile(definition_compile_context&) const
{
    return {};
}
```

### 解析名称依赖

当前完整声明见 [resolve_id](../reference/definition/definition_compile_context/resolve_id.md)。

```cpp
template<class TCategory>
definition_id<TCategory> resolve_id(std::string_view name) const;
```

`TCategory` 指定依赖的定义类别。`name` 必须等于当前 source 的对应 `xxx_dependencies()` 返回的某个名称。

返回值是 `definition_id<TCategory>`，表示该 definition 在本次编译库中的强类型 issued id。它是拥有值，可以直接保存在编译后的 definition 中；不同类别的 definition id 不能混用。

名称未在对应类别中声明，或目标 definition 不存在时，抛出 `std::invalid_argument`，本次规则库编译失败。

### 解析标签 ID 依赖

当前完整声明见 [resolve_tag](../reference/definition/definition_compile_context/resolve_tag.md)。

```cpp
tag_id resolve_tag(std::string_view name) const;
```

`name` 必须等于当前 source 的 `tag_dependencies()` 返回的某个标签名称。

返回值是该标签在本次编译库中的 `tag_id`。它是拥有值，可以直接保存在编译后的 definition 中。名称没有声明时抛出 `std::invalid_argument`，本次规则库编译失败。

### 解析标签筛选依赖

返回与异常见 [resolve_ids_by_tag](../reference/definition/definition_compile_context/resolve_ids_by_tag.md)。下文“不表示名称排序”强调返回顺序由配套 ID 映射决定，不能据此断言当前结果一定与名称序不同：当前 `ordered_selected_indices` 在各类别内按名称排序后分配 ID。筛选表达式必须与声明字符串完全一致，逻辑等价而拼写不同不够。

```cpp
template<class TCategory>
std::vector<definition_id<TCategory>> resolve_ids_by_tag(std::string_view filter) const;
```

`TCategory` 指定筛选的定义类别。`filter` 必须等于当前 source 的对应 `xxx_dependencies_by_tag()` 返回的某个筛选表达式。

返回值是拥有自身存储的 `std::vector<definition_id<TCategory>>`，包含每个匹配 definition 的 issued id 一次；没有匹配项时返回空 vector。结果不引用编译上下文，可以移动并长期保存在编译后的 definition 中。结果按本次编译的 issued id 顺序排列，不表示名称排序。

筛选表达式未在对应类别中声明时，抛出 `std::invalid_argument`，本次规则库编译失败。

### 加入响应程序

**旧称谓与当前理解：**原记录把 `TContext` 概括为“这段程序进入时采用的栈 ABI”。这里保留这个实现来源，但不能由此推出指令作者要在相邻指令之间配合完整帧布局。现在公开的约束是这段程序在哪种事件 context 下可执行；事件映射到内部栈形状是实现。比如 `absorb_damage_by_count` 在 `damage_effect` 广播触发的响应程序中工作，并非在结算伤害指令后直接拼接并接管它的局部帧。每次执行一条指令都是完整的一次执行；同一指令可能自重入或进入子程序，并在彻底退出时恢复进入前的栈形状。参阅 [add_program](../reference/definition/definition_compile_context/add_program.md) 和[固定程序模型](fixed_program.md)。

原记录：

```cpp
template<class TContext, class TInstructions>
program_entry<TContext> add_program(TInstructions&& instructions);
```

`TContext` 指定这段程序进入时采用的栈 ABI。`instructions` 是一段公开指令序列：静态 C++ source 可以传入异构 tuple-like 对象或同构 input range；动态 adapter 可以传入 `any_instruction_for<TContext>` 的 input range。每条指令都必须与 `TContext` 兼容。

该函数在返回前顺序消费完整序列，不保存序列或元素的引用。返回值是新程序的 `program_entry<TContext>`，可以直接保存在编译后的 definition 中。程序执行完最后一条公开指令后会返回触发它的结算过程，source 不需要加入返回指令。

编译上下文不公开最终程序容器、入口的数值表示或核心用于连接程序的内部指令。固定程序与 Context 的通用语义见 [固定程序模型](fixed_program.md)。

### 编译示例

接着前面的依赖声明，编译后的 definition 可以直接保存三种依赖结果：

```cpp
struct definition_type
{
    definition_id<support_view> bonus_support;
    tag_id food_tag;
    std::vector<definition_id<card_definition>> food_cards;
};

definition_type compile(definition_compile_context& context) const
{
    return {
        .bonus_support = context.resolve_id<support_view>(bonus_support_name),
        .food_tag = context.resolve_tag(food_tag_name),
        .food_cards = context.resolve_ids_by_tag<card_definition>(food_filter)
    };
}
```

需要响应程序时，definition 同样直接保存 `add_program` 返回的入口：

```cpp
struct definition_type
{
    std::uint32_t maximum_count;
    program_entry<damage_effect> absorption;
};

definition_type compile(definition_compile_context& context) const
{
    return {
        .maximum_count = maximum_count,
        .absorption = context.add_program<damage_effect>(std::tuple{
            absorb_damage_by_count{ .maximum_count = maximum_count }
        })
    };
}
```

`definition_id<TCategory>` 和 `tag_id` 只在产生它们的 `definition_library` 内有意义。definition id 标识不可变规则定义，不是 `support_id`、`character_id` 等一局对局中的实体 id。

## Handler

当前使用入口见[定义源协议的事件响应](../reference/definition/source_protocol.md#事件响应)。下文保留静态函数选择和动态 adapter 兼容的实现理由。

handler 在对局运行时响应事件。此时 source 对象不再参与；handler 读取 `compile(...)` 返回的 definition，并可以修改当前事件或选择一段已经编译的响应程序。

### `handle`

费用事件的 `onpay_context` 名称在下面只作为现有映射实现记录，不另立一种公开“on pay 事件”体系。面向定义源应使用 [handler_program_context_t](../reference/definition/handler_program_context_t.md) 和 [handler_program_entry_t](../reference/definition/handler_program_entry_t.md)；同一事件只有相应的响应 context。

```cpp
static handler_program_entry_t<TEvent> handle(
    const TDefinition& definition,
    const TEntityView& self,
    TEvent& event,
    const card_table& table,
    random_fn& random
);
```

`handle` 是可选的静态函数。source 可以提供普通重载或函数模板；核心针对每个允许的实体 view 与事件组合使用普通重载解析检查这个调用是否存在。

参数含义如下：

- `definition` 的类型必须准确等于 `compile(...)` 的返回类型，用于读取该 definition 的固定配置、issued id 和程序入口。
- `self` 是当前响应者的只读实体 view。其类型由 `definition_category` 和事件订阅关系决定。
- `event` 是当前事件，可以读取和修改。
- `table` 是只读牌桌，用于查询持久游戏状态。
- `random` 是本次执行使用的随机函数。

返回类型必须准确等于 `handler_program_entry_t<TEvent>`，即 `program_entry<handler_program_context_t<TEvent>>`。普通事件的 handler Context 通常就是事件类型；费用事件可以映射到 `onpay_context<TCostEvent>`。

没有匹配调用时，该 definition 不响应这一 view 与事件组合。调用存在但返回类型错误时，source 不合法，不能静默退化为无 handler。

handler 可以直接修改 event，但不能通过 const table 修改持久状态。需要修改 table 时，返回由 `compile(...)` 保存的程序入口；返回空入口只表示不进入响应程序，已经完成的 event 修改仍然保留。需要结束对局时，在 `compile(...)` 中把公开 `end_game` 指令编入相应 Context 的程序，由 handler 返回保存的入口。

计数护盾可以根据当前事件和实体状态决定是否返回前面编译的吸收程序：

```cpp
static program_entry<damage_effect> handle(
    const definition_type& definition,
    const combat_status_view& self,
    damage_effect& event,
    const card_table&,
    random_fn&
)
{
    if(
        event.value == 0
        || event.target.player_id != self.player().id()
        || event.flags.contains(damage_flag_bits::ignore_shield)
        || self.state().count == 0
    )
    {
        return program_entry<damage_effect>::null();
    }
    return definition.absorption;
}
```

`character_initialization` 沿用相同调用形状，但原记录约定 handler 只修改局部事件并返回空入口；`enter_character` 不进入它返回的程序。当前 `initialize_characters` 也遵循直接填写状态、不进入响应程序的处理方式，见 [character_initialization](../reference/executor/events/character_initialization.md)。原文将“具体事件字段、响应时序和栈 ABI”一并指向事件目录；现在前两者由事件及指令 reference 说明，完整内部映射和帧结构留在[事件分派](event_dispatch.md)与[栈布局备忘](stack_layout.md)，不作为事件使用者的完整栈协议。

handler 使用静态函数，是因为运行时持有编译后的 definition，而不保留原 source。动态 adapter 需要的 Lua 状态引用、回调索引或其他稳定句柄应由 `compile(...)` 放进 definition，再由静态 handler 读取。

### `can_handle`

```cpp
template<class TEntityView, class TEvent>
bool can_handle() const;
```

`can_handle` 是可选的 const 成员函数，没有运行时参数。`TEntityView` 和 `TEvent` 指定要判断的实体 view 与事件组合，返回值表示这个具体 source 是否实际提供该响应。

最终只有同时满足以下条件时，编译库才为该组合安装 handler：存在返回类型正确的静态 `handle` 调用，并且 `can_handle<TEntityView, TEvent>()` 未提供或返回 `true`。

普通静态 source 通常只需省略不支持的 `handle`，无需提供 `can_handle`。动态 adapter 可能拥有覆盖全部事件的通用 handler 模板，此时可根据脚本实际注册的回调返回准确结果。该判断只在编译定义库时发生，不增加对局运行时的字符串查询或脚本能力检查。

`can_handle` 表示“存在这一类响应”，不保证 handler 每次调用都会返回非空入口。card 和 card status 可能为不同区域 view 提供不同响应，所以接口同时区分 view 与 event。

## 动态定义源

Lua 等动态来源通过 C++ adapter 实现与静态 source 相同的接口，不使用另一套定义协议。adapter 可以从脚本元数据返回名称、标签和依赖 range，在 `compile(...)` 中解析依赖并加入脚本提供的程序，再把运行时回调所需的稳定句柄放进 definition。

脚本中的 Context 标识需要由 adapter 分派到具体 C++ Context，再将相应 `any_instruction_for<TContext>` range 传给 `add_program<TContext>`。这仍然使用同一套公开指令和强类型程序入口。

## 注册与生命周期

对应公开接口为 [add](../reference/definition/definition_source_library/add.md)、[compile](../reference/definition/definition_source_library/compile.md)、[definition_selection](../reference/definition/definition_selection.md)、[definition_compile_result](../reference/definition/definition_compile_result.md) 和 [make_issued_id_map](../reference/definition/definition_source_library/make_issued_id_map.md)。

**生命周期表述的细化：**下面原记录要求 source 覆盖源库及其编译库的全部使用期，是把源对象与它可能拥有的字符串一并保活的保守约束。当前源码中，源库的 `definition_source_view::source_` 非拥有地指向源对象；编译后的库保存 definition 数据、字符视图和不捕获 source 的静态 handler 函数指针，不再保存这个源对象指针。因此需要分别保证：源库使用期间 source 有效；所有仍借用的名称/标签字符在相应库或映射使用期间有效；definition 若另存 Lua 状态、回调句柄或其他非拥有对象，其目标也必须存活。字符属于 source 自身时，source 当然仍要覆盖字符使用期。不能因为运行期不调用 source 就把尚被借用的数据销毁。

原注册说明与约束：

```cpp
template<class TSource>
bool definition_source_library::add(const TSource& source);
```

`add` 保存 source 的非拥有引用。返回 `true` 表示添加成功；同类别重名或依赖不满足时返回 `false`。source 对象必须在保存它的 `definition_source_library` 以及由该源库编译出的所有 `definition_library` 的使用期间保持存活。

同时添加多个 source 的重载是原子的：它允许同一批 source 互相依赖，任一名称或依赖检查失败时整批都不加入。源库之间也可以在没有同类别名称冲突时合并。

源库可以编译全部定义，也可以通过 `definition_selection` 按类别指定需要的 definition，并自动包含它们的依赖闭包。初始化程序和回合程序必须在同一次编译中提供。编译后的库不能通过合并增补定义；改变定义集合后需要重新编译。

```cpp
auto [library, id_map] = source_library.compile(initialization_program, round_program);
```

`initialization_program` 只执行一次；随后 `round_program` 会反复执行，直到游戏结束被触发。两者都是无 Context 依赖的公开指令序列。`compile(...)` 不提供省略这两段程序的重载。

```cpp
using definition_selection = std::array<std::span<const std::string_view>, definition_types::size()>;
```

需要只编译部分定义时，使用接受 `const definition_selection& selection` 的重载。`selection` 按 definition 类别保存名称序列；每个选中的 definition 及其传递依赖都会进入编译结果。

返回类型是 `definition_compile_result`。其 `library` 成员是编译后的游戏规则，`id_map` 成员是同一次编译使用的名称映射，供上层在对局开始前把名称形式的牌组或其他输入链接为 issued ID。两者对应同一个定义集合和 ID 分配结果，也可以按该顺序结构化绑定；对局运行时只需要 `library`。

当调用方必须先取得 issued ID 才能构造初始化程序或回合程序中的指令时，可以使用 `make_issued_id_map(...)`。提前生成映射与随后 `compile(...)` 必须使用相同的定义集合、标签声明及选择范围；生成映射后改变源库或选择范围可能改变 ID 分配。牌组链接发生在编译后，应直接使用编译结果中的 `id_map`，不需要再次生成映射。

## 编译库与持久化

**上层格式的设计记录。**以下描述哪些信息适合作为可重建的稳定描述；当前核心没有因此提供任意 C++/Lua definition 的序列化器，也没有承诺 `table`、`executor` 的内存布局可直接作为存档格式。

`definition_library` 是定义源集和游戏流程规则共同编译出的不可变游戏规则，不是一局游戏的可变状态。table 与 executor 共同构成对局状态，都不保存 definition library 指针。执行器每次推进显式接收配套的定义库，table 仅保存实体的定义 ID。

definition library 通过 issued id 提供 definition view、名称、标签和事件分派查询；游戏入口和取指仅供内部执行器使用。编译后的具体 definition 对象由核心传给对应 handler；名称到 issued id 的查找由 `issued_id_map` 提供。其内部容器和程序布局不是公开接口。

需要持久化定义库构建信息时，稳定描述包括按类别记录的 definition source 完整名称，以及初始化程序和回合程序中的公开指令。恢复时，上层注册表按“类别 + 完整名称”找到 source 并重新编译。同类别同名却实现不同属于拓展冲突，核心不尝试序列化或比较任意 C++、Lua 定义实现。

初始化程序和回合程序使用固定的公开指令集，上层格式可以为指令种类分配稳定编号并序列化其公开字段。游戏存档中的可变状态仍是与重建后定义库匹配的 table 和 executor。

## 背后的编译过程

**逻辑阶段与实际写入顺序。**下面七步保留原来的依赖关系解释，不声称每一步对应一次独立容器遍历。当前实现先发放 ID、建立带内部前缀的局部 `definition_library`，写入初始化、回合和回跳连接，再逐项建立受限 context、编译 definition、追加其响应程序并安装 handler，最后一并返回 `library` 和 `id_map`。响应程序由 `add_program` 当场追加并补内部返回连接。尚未完成的库不对外发布；循环依赖能够成立，也依赖于解析时读取预先分配的 ID，而非要求对方 definition 已构造。

一份规则库的构建包含以下工作：

1. 读取每个 source 的定义类别、名称、标签和依赖声明。
2. 从 `definition_selection` 指定的定义求出依赖闭包，或选择全部定义。
3. 为选中的定义和标签建立 issued id 映射。
4. 为每个选中的 source 建立受限的 `definition_compile_context` 并调用一次 `compile(...)`；依赖查询返回已经分配的 issued id，`add_program(...)` 立即返回相应强类型入口。
5. 根据有效 `handle` 调用和可选 `can_handle` 结果安装运行时分派。
6. 将 definition 响应程序与调用方提供的初始化程序、回合程序共同组成游戏规则程序。
7. 所有 definition 完整构造后，同时发布不可变的 `definition_library` 和本次编译使用的 `issued_id_map`。

编译期间的中间对象不是 `definition_library` 的可观察状态。程序段存放顺序、入口数值、内部连接指令和擦除存储也都不是定义源接口。definition source 与游戏流程只能提交核心公开指令描述；上层运行期间通过 execution_view 观察领域现场，而不是查看当前指令。

[开发备忘](../notes.md)
