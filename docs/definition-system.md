# 编写定义源

definition source 是一个描述单项游戏规则的 C++ 对象。它可以代表一张卡牌、一个角色、一种状态、一个召唤物或其他一种 definition。核心先读取它的身份与依赖，再调用它编译出不可变的 definition；对局执行规则时只读取编译结果，不再调用原 source 对象。source 仍需保持存活，因为源库和编译库可以保存由它提供的非拥有字符串视图。

本文按照编写定义源时理解信息的顺序介绍接口：身份、依赖、编译和 handler。每个接口会分别说明是否必须、参数、返回值和语义。

## 身份标识

### 定义类别

```cpp
using definition_category = support_view;
```

`definition_category` 是必须提供的公开嵌套类型，用来指定这个 source 产生哪一类 definition。上例表示它产生 support definition。

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

核心会保留返回的 `std::string_view`。底层字符必须在保存该 source 的 `definition_source_library` 及由它编译出的 `definition_library` 使用期间保持有效。

## 标签与依赖

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

上述接口每次返回的 range 只需支持一次顺序遍历，数量不必在编译期确定。调用方会在本次接口调用后立即完整消费该 range，不保存 range 或元素对象的引用，因此 source 可以返回临时 range、容器或惰性 range。

元素转换所得 `std::string_view` 可能被源库保留。其底层字符必须覆盖相应源库及编译库的使用期。这些字符串只在添加 source 和编译规则库时查询，不会进入对局运行时的名称查找热路径。

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

依赖闭包和本次编译的 issued id 全部确定后，核心调用 source 的 `compile(...)`。

### `compile`

```cpp
auto compile(definition_compile_context& context) const;
```

`compile(...)` 是必须提供的 const 成员函数。

参数 `context` 是只在本次调用期间有效的编译上下文。它可以解析当前 source 已声明的依赖，并把响应程序加入正在构建的游戏规则程序。source 不得在返回对象或其他长期状态中保存该上下文的引用或指针。

返回值是编译后的 definition，必须按值返回。核心直接推导其准确类型、取得所有权，并在运行时以该类型的 const 引用传给 handler。接口不要求 source 提供 `data_type` 或其他用于重复说明返回类型的嵌套别名。本文示例通常把返回类型命名为 `definition_type`，但这只是 source 内部的命名习惯。

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

```cpp
template<class TCategory>
definition_id<TCategory> resolve_id(std::string_view name) const;
```

`TCategory` 指定依赖的定义类别。`name` 必须等于当前 source 的对应 `xxx_dependencies()` 返回的某个名称。

返回值是 `definition_id<TCategory>`，表示该 definition 在本次编译库中的强类型 issued id。它是拥有值，可以直接保存在编译后的 definition 中；不同类别的 definition id 不能混用。

名称没有声明、目标 definition 不存在或类别不匹配时，本次规则库编译失败。

### 解析标签 ID 依赖

```cpp
tag_id resolve_tag(std::string_view name) const;
```

`name` 必须等于当前 source 的 `tag_dependencies()` 返回的某个标签名称。

返回值是该标签在本次编译库中的 `tag_id`。它是拥有值，可以直接保存在编译后的 definition 中。名称没有声明时，本次规则库编译失败。

### 解析标签筛选依赖

```cpp
template<class TCategory>
std::vector<definition_id<TCategory>> resolve_ids_by_tag(std::string_view filter) const;
```

`TCategory` 指定筛选的定义类别。`filter` 必须等于当前 source 的对应 `xxx_dependencies_by_tag()` 返回的某个筛选表达式。

返回值是拥有自身存储的 `std::vector<definition_id<TCategory>>`，包含每个匹配 definition 的 issued id 一次；没有匹配项时返回空 vector。结果不引用编译上下文，可以移动并长期保存在编译后的 definition 中。结果按本次编译的 issued id 顺序排列，不表示名称排序。

筛选表达式没有声明或类别不匹配时，本次规则库编译失败。

### 加入响应程序

```cpp
template<class TContext, class TInstructions>
program_entry<TContext> add_program(TInstructions&& instructions);
```

`TContext` 指定这段程序进入时采用的栈 ABI。`instructions` 是一段公开指令序列：静态 C++ source 可以传入异构 tuple-like 对象或同构 input range；动态 adapter 可以传入 `any_instruction_for<TContext>` 的 input range。每条指令都必须与 `TContext` 兼容。

该函数在返回前顺序消费完整序列，不保存序列或元素的引用。返回值是新程序的 `program_entry<TContext>`，可以直接保存在编译后的 definition 中。程序执行完最后一条公开指令后会返回触发它的结算过程，source 不需要加入返回指令。

编译上下文不公开最终程序容器、入口的数值表示或核心用于连接程序的内部指令。固定程序与 Context 的通用语义见 [固定程序模型](fixed-program-design.md)。

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

handler 在对局运行时响应事件。此时 source 对象不再参与；handler 读取 `compile(...)` 返回的 definition，并可以修改当前事件或选择一段已经编译的响应程序。

### `handle`

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

handler 可以直接修改 event，但不能通过 const table 修改持久状态。需要修改 table 时，返回由 `compile(...)` 保存的程序入口；返回空入口只表示不进入响应程序，已经完成的 event 修改仍然保留。handler 也可以通过 `program_entry` 的具名结果直接结束对局。

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

`character_initialization` 沿用相同调用形状，但当前约定是 handler 只修改局部事件并返回空入口；`enter_character` 不进入它返回的程序。具体事件字段、响应时序和栈 ABI 见 [当前指令与事件目录](events.md)。

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

脚本中的 Context 标识需要由 adapter 分派到具体 C++ Context，再将相应 `any_instruction_for<TContext>` range 传给 `add_program<TContext>`。这仍然使用同一套公开指令和强类型程序入口。仓库目前尚未提供内置 Lua 或 Python adapter。

## 注册与生命周期

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

当调用方必须先取得 issued ID 才能构造初始化程序或回合程序中的指令时，可以使用 `make_issued_id_map(...)`。牌组链接发生在编译后，应直接使用编译结果中的 `id_map`，不需要再次生成映射。

## 编译库与持久化

`definition_library` 是定义源集和游戏流程规则共同编译出的不可变游戏规则，不是一局游戏的可变状态。table 引用一份 definition library；table 与 executor 共同构成对局状态。

definition library 按类别提供名称、issued id、编译后的 definition、标签和事件分派查询，并提供游戏主入口及按公开执行位置读取指令的能力。其内部容器和程序布局不是公开接口。

需要持久化定义库构建信息时，稳定描述包括按类别记录的 definition source 完整名称，以及初始化程序和回合程序中的公开指令。恢复时，上层注册表按“类别 + 完整名称”找到 source 并重新编译。同类别同名却实现不同属于拓展冲突，核心不尝试序列化或比较任意 C++、Lua 定义实现。

初始化程序和回合程序使用固定的公开指令集，上层格式可以为指令种类分配稳定编号并序列化其公开字段。游戏存档中的可变状态仍是与重建后定义库匹配的 table 和 executor。

## 背后的编译过程

一份规则库按以下顺序形成：

1. 读取每个 source 的定义类别、名称、标签和依赖声明。
2. 从 `definition_selection` 指定的定义求出依赖闭包，或选择全部定义。
3. 为选中的定义和标签建立 issued id 映射。
4. 为每个 source 建立受限的 `definition_compile_context` 并调用一次 `compile(...)`；依赖查询返回已经分配的 issued id，`add_program(...)` 立即返回相应强类型入口。
5. 根据有效 `handle` 调用和可选 `can_handle` 结果安装运行时分派。
6. 将 definition 响应程序与调用方提供的初始化程序、回合程序共同组成游戏规则程序。
7. 所有 definition 完整构造后，同时发布不可变的 `definition_library` 和本次编译使用的 `issued_id_map`。

编译期间的中间对象不是 `definition_library` 的可观察状态。程序段存放顺序、入口数值、内部连接指令和擦除存储也都不是定义源接口。definition source 与游戏流程只能提交核心公开指令；只要对局仍在运行，上层观察到的当前指令也必须来自公开指令集。

[返回文档入口](README.md)
