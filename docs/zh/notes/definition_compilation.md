# 定义源协议的编译边界与设计理由

本篇保留定义源从身份声明、依赖解析到运行期响应的完整设计脉络，以及动态 adapter、生命周期和持久化的选择理由。公开接口以[定义源协议](../reference/definition/source_protocol.md)及各 API 页为准；原有声明、表格、约束和配套代码片段在相应主题中继续保留，避免只留下接口名称而丢掉当时的限制和原因。片段用于讨论协议，不是可单独运行的 reference 示例。

对照基线为 `b3d6c50`。旧记录的 Context/栈 ABI 称法、源对象生命周期概括、逻辑编译步骤与实际写入顺序在相应位置标明差异。持久化一节记录上层格式的设计方向，不声称核心已经提供现成序列化器。

当前模块边界：definition 保留定义源协议、源库、source view、ID 准备、command、命令 variant、事件与程序入口；executor 提供最终编译的非成员 `givm::compile`，并拥有完整的编译上下文及编译后的定义库。source 协议只需前置声明上下文，程序入口则由 definition 提供完整类型；定义拓展者通过 `source.hpp` 取得完整编译与响应上下文，整库编译通过 `compile.hpp` 使用。入口索引的生成与解释仍由 executor 负责。本文的 `source.compile(context)` 始终指单项源的编译操作，整库编译则通过 `compile(source_library, ...)` 调用。

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

类别间依赖的公开成员表见[可选的分类与依赖](../reference/definition/source_protocol.md#可选的分类与依赖)。名称依赖决定编译集合；标签和能力查询只查看已经确定的集合。

依赖在 issued id 产生前声明，使用“目标类别 + 名称”标识。编译器复用源库登记时缓存的名称、标签和依赖声明，先求出闭包并分配 ID，再把名称、标签位集、响应函数表、历史摘要响应列表和非空查询函数直接写入最终 `definition_library` 的存储。空查询需要编译后的 definition 才能求值，因此暂存其函数指针。编译上下文借用这些信息，不另建一套随后搬运的元数据或重复的能力表。source 的 `compile(...)` 和历史摘要 `layout(...)` 都可以查看本次集合，不依赖其他定义的编译先后顺序。

同一个 source 的名称、标签和依赖接口在加入源库后必须保持结果一致。

### 定义标签

```cpp
auto tags() const;
```

`tags()` 是可选的 const 成员函数，没有参数。返回值可以是任意 input range，其元素必须可转换为 `std::string_view`；每个字符串表示当前 definition 拥有的一个标签。省略该接口等价于返回空 range。

标签供编译集合筛选和编译库查询使用，不扩充依赖闭包。标签名称同样是不透明字符串，核心只解释标签筛选表达式的组合语法。

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

### 标签与元数据查询

不再为标签 ID 或标签筛选声明依赖。`context.find_tag(name)` 返回本次集合中的标签 ID，未知标签返回 `nullopt`。`context.find_ids_by_tag<T>(expression)` 只筛选已选择的定义；未知正向标签无匹配，未知排除标签不限制结果。

筛选表达式由 `&` 连接若干条件，以 `!` 排除标签。例如 `"food & !event"` 选择本次集合中具有 `food` 且不具有 `event` 标签的定义，不会把源库中未选择的匹配项加入依赖闭包。

`context.definitions<T>()` 遍历按 ID 排列、可取得数量的元数据范围。`find_definition<T>(name)` 返回可为空的元数据视图，`context[id]` 按本次有效 ID 访问。视图提供 `id/name/tags/dependencies/has_tag/can_handle/has_query`，仅查看声明与能力，不执行查询或响应；`has_query` 不把默认查询结果算作自定义实现。这些视图只在本次编译期间有效。

### Range 与字符串生命周期

`definition_source_view` 的 [tags](../reference/definition/definition_source_view/tags.md) 和 [dependencies](../reference/definition/definition_source_view/dependencies.md) 都拥有返回的视图容器，但不拥有字符。这里“可返回临时容器”不包括让字符串本体随临时容器一起销毁：例如临时 `vector<string_view>` 可指向稳定字符，临时 `vector<string>` 的字符却会一起消失。

上述接口每次返回的 range 只需支持一次顺序遍历，数量不必在编译期确定。调用方会在本次接口调用后立即完整消费该 range，不保存 range 或元素对象的引用，因此 source 可以返回临时 range、容器或惰性 range。

元素转换所得 `std::string_view` 可能被源库保留，部分名称和标签也由编译库及 `issued_id_map` 借用。其底层字符必须覆盖这些对象的使用期。字符串用于注册、编译和对局前的名称链接；对局规则执行使用 issued ID，不要求运行时按名称查找。

### 依赖声明示例

下面的片段声明一个 support 名称依赖与定义自身的标签；标签筛选表达式只是稍后查询本次集合时使用的条件：

```cpp
static constexpr std::string_view bonus_support_name = "BonusSupport";
static constexpr std::string_view food_tag_name = "food";
static constexpr std::string_view food_filter = "food & !event";

std::array<std::string_view, 1> support_dependencies() const
{
    return { bonus_support_name };
}

std::array<std::string_view, 1> tags() const
{
    return { food_tag_name };
}

```

## 编译定义

入口见 [definition_compile_context](../reference/executor/definition_compile_context.md)。依赖闭包和 ID 先确定，再允许 source 一次构造完整结果，是以下接口共同依赖的顺序。

依赖闭包和本次编译的 issued id 全部确定后，核心调用 source 的 `compile(...)`。

### `compile`

```cpp
auto compile(definition_compile_context& context) const;
```

`compile(...)` 是必须提供的 const 成员函数。

参数 `context` 是只在本次调用期间有效的编译上下文。它可以解析当前 source 已声明的硬依赖、自由查询本次集合的元数据，并把响应程序加入正在构建的游戏规则程序。source 不得在返回对象或其他长期状态中保存该上下文或其元数据视图。

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

当前完整声明见 [resolve_id](../reference/executor/definition_compile_context/resolve_id.md)。

```cpp
template<class TCategory>
definition_id<TCategory> resolve_id(std::string_view name) const;
```

`TCategory` 指定依赖的定义类别。`name` 必须等于当前 source 的对应 `xxx_dependencies()` 返回的某个名称。

返回值是 `definition_id<TCategory>`，表示该 definition 在本次编译库中的强类型 issued id。它是拥有值，可以直接保存在编译后的 definition 中；不同类别的 definition id 不能混用。

名称未在对应类别中声明，或目标 definition 不存在时，记录 `definition_resolution_error` 并返回无效 ID。源的编译函数可以继续收集其他独立错误，但不能用该无效 ID 访问元数据；最终 `compile` 返回错误列表，不发布部分定义库。验证过程不借助抛出异常，因此源的 `noexcept compile` 也可以报告这些错误。

### 查找标签 ID

当前完整声明见 [find_tag](../reference/executor/definition_compile_context/find_tag.md)。

```cpp
std::optional<tag_id> find_tag(std::string_view name) const;
```

`name` 可为任意标签名称，不需要依赖声明，也不会引入标签或定义。

已存在时返回该标签在本次编译库中的 `tag_id`，否则返回 `nullopt`。结果可以保存在编译后的 definition 中。

### 按标签筛选

返回规则见 [find_ids_by_tag](../reference/executor/definition_compile_context/find_ids_by_tag.md)。返回顺序由配套 ID 映射决定：当前 ID 准备实现在各类别内按名称排序后分配 ID。筛选表达式不需要提前声明。

```cpp
template<class TCategory>
std::vector<definition_id<TCategory>> find_ids_by_tag(std::string_view filter) const;
```

`TCategory` 指定筛选的定义类别，`filter` 使用 `&` 连接标签条件、以 `!` 排除标签。查询范围只包含本次选定定义及其名称依赖和基础定义。

返回值是拥有自身存储的 `std::vector<definition_id<TCategory>>`，包含每个匹配 definition 的 issued id 一次；没有匹配项时返回空 vector。结果不引用编译上下文，可以移动并长期保存在编译后的 definition 中。结果按本次编译的 issued id 顺序排列，不表示名称排序。

未知正向标签使结果为空，未知排除标签不限制结果。筛选不修改本次编译集合。

### 加入响应程序

```cpp
program_entry add_program(std::span<const any_command> commands);

template<class TCommands>
    requires /* 命令序列，且不能隐式转换为 span<const any_command> */
program_entry add_program(TCommands&& commands);

template<class... TCommands>
    requires (std::constructible_from<any_command, TCommands> && ...)
program_entry add_program(TCommands&&... commands);
```

`commands` 可为异构 tuple-like、同构 input range、包含 `any_command` 的范围，或直接提供零个及多个命令。头文件包装将命令统一为 `span<const any_command>`，非模板后端在 cpp 中逐项检查和编译，不保存调用方序列或元素引用。编译及编译前允许类型擦除与动态分派；游戏执行仍使用编译出的 opcode，不遍历命令 variant。入口不绑定外层事件类型，所需输入由具体命令值按执行顺序确定；不消费响应输入的命令不占输入位置，编译后入口的输入数量、类型和顺序固定。

响应源通过 `context.invoke` 准备一次调用的完整输入。每条动态命令消费一个由 `input_type` 指定的输入帧，固定模式不占输入位置；程序末尾自动返回。Debug 编译记录输入类型标记、对应命令下标以及源和程序位置。调用写入前检查入口、提交方式、重复调用、数量、具体类型与顺序；数组长度不参与类型匹配。失败以 `program_input_error` 报告，Release 移除诊断元数据与检查。输入类型列表从命令的 `input_type` 自动生成，仅固定命令不参与；字段相符的事件可显式别名复用。动态适配器提交 `any_command_input` 序列，由 C++ 包装提供同样的检查，无需脚本自行处理元数据。

编译上下文不公开程序容器、入口数值或内部连接指令。详见[固定程序模型](fixed_program.md)。

### 编译示例

接着前面的名称依赖和标签声明，编译后的 definition 可以保存依赖 ID 与本次集合的查询结果：

```cpp
struct definition_type
{
    definition_id<support_view> bonus_support;
    std::optional<tag_id> food_tag;
    std::vector<definition_id<card_definition>> food_cards;
};

definition_type compile(definition_compile_context& context) const
{
    return {
        .bonus_support = context.resolve_id<support_view>(bonus_support_name),
        .food_tag = context.find_tag(food_tag_name),
        .food_cards = context.find_ids_by_tag<card_definition>(food_filter)
    };
}
```

需要响应程序时，definition 同样直接保存 `add_program` 返回的入口：

```cpp
struct definition_type
{
    std::uint32_t maximum_count;
    program_entry absorption;
};

definition_type compile(definition_compile_context& context) const
{
    return {
        .maximum_count = maximum_count,
        .absorption = context.add_program(std::tuple{
            modify_combat_status_state{}
        })
    };
}
```

`definition_id<TCategory>` 和 `tag_id` 只在产生它们的 `definition_library` 内有意义。definition id 标识不可变规则定义，不是 `support_id`、`character_id` 等一局对局中的实体 id。

## Handler

当前使用入口见[定义源协议的事件响应](../reference/definition/source_protocol.md#事件响应)。下文保留静态函数选择和动态 adapter 兼容的实现理由。

handler 在对局运行时响应事件。此时 source 对象不再参与；handler 读取 `compile(...)` 返回的 definition，并可以修改当前事件或选择一段已经编译的响应程序。

### `handle`

handler 的签名为：

```cpp
static program_entry handle(
    const TDefinition& definition, const TEntityView& self, TEvent& event,
    handle_context& context);
```

`TDefinition` 必须为本源 `compile` 的返回类型。`self` 是当前响应者，`event` 是本次事件，`context.table()` 只读，通过 `context.random()` 取得随机值，通过 `context.invoke(...)` 提交后续效果。返回类型必须为 `program_entry`；无后续效果时返回空入口，需要后续操作时一次性提交全部输入并立即返回 `invoke` 的结果。

静态源没有匹配调用表示不支持这项响应；有调用而返回类型错误时不能静默退化为无响应。普通事件与费用事件使用相同签名。普通响应通过 `context.invoke(entry, ...)` 提交，费用响应通过 `context.invoke(substack_t{}, entry, ...)` 仅收集输入与入口。写入方式由重载在编译期选择；Debug 保存预期方式以诊断错误重载，Release 不保存模式字段，也不检查误用。

计数护盾响应示例：

```cpp
static program_entry handle(
    const definition_type& definition, const combat_status_view& self, damage_effect& event,
    handle_context& context)
{
    if(event.target.player_id != self.player().id()
        || event.flags.contains(damage_flag_bits::ignore_shield))
    {
        return {};
    }
    const auto absorbed = std::min({ event.value, self.state().count, definition.maximum_count });
    if(absorbed == 0)
    {
        return {};
    }
    event.value -= absorbed;
    return context.invoke(definition.absorption, modify_combat_status_state_input{
        .status = self.id(),
        .count = -static_cast<std::int64_t>(absorbed)
    });
}
```

伤害调整发生在响应内，扣层发生在所选程序中；负增量应用于命令执行时的当前层数，`round_usages` 的零增量保留每回合剩余次数。修改先写入状态再通知自身，定义可在状态变化响应中决定零层数是否删除。两者之间不保留一个供命令任意读取的外层事件 Context。

角色初始化现使用 [character_initial_state](../reference/definition/queries/character_initial_state.md)；卡牌初始费用与目标检查也改用查询，不再为只返回数据的操作制造事件及空入口。事件字段和响应时序由 reference 说明，完整内部映射和帧结构留在[事件分派](event_dispatch.md)与[栈布局备忘](stack_layout.md)。

handler 使用静态函数，是因为运行时持有编译后的 definition，而不保留原 source。动态 adapter 需要的 Lua 状态引用、回调索引或其他稳定句柄应由 `compile(...)` 放进 definition，再由静态 handler 读取。

### 动态源的 `can_handle`

```cpp
static constexpr bool is_dynamic = true;

template<class TEntityView, class TEvent>
bool can_handle() const;
```

未声明 `is_dynamic` 或其值为 `false` 时，source 为静态源，只按静态 `handle` 是否存在判断响应能力，不调用源的 `can_handle`。这样普通 C++ 定义可以直接写自己需要的响应重载。

`is_dynamic` 为 `true` 的源必须为所属类别的各个实体 view 与可订阅事件组合提供返回 `bool` 的 const 成员 `can_handle` 和返回 `program_entry` 的静态 `handle`。动态历史摘要使用单个事件模板参数的 `can_handle<Event>()`，并完整提供返回 `void` 的摘要 `handle`。所有接口在构造 `definition_source_view` 时进行 C++ 编译检查，包括能力判断返回 `false` 的分支；缺失接口或返回类型错误不再推迟到编译定义库时抛出异常。

能力判断返回 `false` 时不安装 handler，返回 `true` 时安装对应的静态 `handle`。不支持的分支不会被正常分派调用，因此其函数体无须产生有效结果；定义源若绕过能力选择自行调用该分支，属于未定义行为。

动态 adapter 可以提供覆盖全部事件的通用 handler 模板，并根据脚本实际注册的回调返回能力判断结果。该判断只在编译定义库时发生，最终仍保存事件对应的擦除函数指针，不增加对局运行时的字符串查询、事件类型分支或脚本能力检查。定义库公开的 `can_handle` 仍检查已保存的入口是否为空。

`can_handle` 表示“存在这一类响应”，不保证 handler 每次调用都会提交后续效果。card 和 card status 可能为不同区域 view 提供不同响应，所以接口同时区分 view 与 event。

## 查询与结果保存

查询通过 `TSource::query(const definition_type&, const Q&)` 返回 `Q::result_t`，`definition_type` 是本源 `compile` 的返回类型。每个类别使用一份 `supported_queries<Category>`；类型为空的判据仅为 `std::is_empty_v<Q>`，嵌套 `result_t` 不影响这个判据。

空查询在具体 definition 编译完成后调用一次，保存结果；非空查询保存对应的擦除函数，在收到参数时以 `std::any_cast` 取得 definition 后调用源的静态 query。运行期不需要查询种类的枚举或字符串查找。空查询按具体定义条目保存，不能按 C++ 源类型共享，因为同一源类型的不同实例可以有不同配置。

静态源没有匹配的 query 时使用未限定的 `query_default(parameters)`，由 ADL 找到默认方法；静态源不调用 `can_query`。动态源须为所属类别的全部查询提供 `template<class Q> bool can_query() const` 和返回 `Q::result_t` 的静态 `query`。二者均在构造 source view 时检查，不能用 `can_query` 返回 `false` 代替缺少接口。返回 `true` 时使用源查询，返回 `false` 时使用默认查询且不调用源查询；定义源自行调用声明不支持的查询分支属于未定义行为。源函数和默认方法都检查准确返回类型。

动态能力由具体源对象决定，不能写进按 C++ 源类型共享的 RTTI 结果。在任何 `layout(...)` 或 `compile(...)` 调用前，source view 为每个查询取得自定义函数指针；静态源没有自定义实现或动态源未启用时返回空指针。非空查询直接把这个结果写入最终库的查询表，编译期间一直保留空指针，以便 `has_query` 区分自定义实现和默认实现。全部 definition 编译完成后才用默认函数补齐这些空项；不需要另一份能力表，也不依靠跨 DLL 的函数地址比较来识别默认实现。

空查询的自定义函数指针仅暂存在本次编译条目中，使整个编译期间的 `has_query` 判断保持一致。每项 source 的 `compile(...)` 返回 definition 后，选择该自定义函数或默认函数执行一次，把结果写入最终库；临时函数指针随编译条目一起释放。各类别只保存自己支持的查询内容，不增加对局运行期查询能力标志，也不在每次查询中判断是否走默认实现。

角色初始状态、初始费用采用值结果。若以后增加包含指针或视图的结果，缓存只保存该对象本身，不自动拥有目标数据；库复制后仍需遵守其借用关系。查询结果不在 source view 的按源类型共享 RTTI 中保存，源编译上下文解析出的 ID 可正常参与结果计算。

## 动态定义源

Lua 等动态来源通过声明 `is_dynamic = true` 的 C++ adapter 接入。adapter 为所属类别的全部事件和查询提供完整的 `handle`、`query`、`can_handle`、`can_query`，可以用泛型函数覆盖；名称、标签、依赖和编译接口仍与静态 source 一致。脚本侧可以只提供实际支持的回调集合，由能力判断选择，不必复制 C++ 模板协议。adapter 可以从脚本元数据返回名称、标签和依赖 range，在 `compile(...)` 中解析依赖并加入脚本提供的程序，再把运行时回调所需的稳定句柄放进 definition。

adapter 把定义的固定命令序列交给 `add_program`。每个程序所需输入对象的数量、类型和顺序由命令序列确定；响应时计算输入值与各数组的内容。C++ 调用逐项提交专用输入对象，动态 adapter 使用 `span<const any_command_input>` 提交同样的对象序列。C++ 包装承担复制和 debug 匹配检查，脚本不需要理解字节布局。命令实现不因外层事件和实体类别组合而复制。

## 注册与生命周期

对应公开接口为 [add](../reference/definition/definition_source_library/add.md)、[compile](../reference/executor/compile.md) 和 [definition_selection](../reference/executor/definition_selection.md)。

**生命周期表述的细化：**下面原记录要求 source 覆盖源库及其编译库的全部使用期，是把源对象与它可能拥有的字符串一并保活的保守约束。当前源码中，源库的 `definition_source_view::source_` 非拥有地指向源对象；编译后的库保存 definition 数据、字符视图和不捕获 source 的静态 handler 函数指针，不再保存这个源对象指针。因此需要分别保证：源库使用期间 source 有效；所有仍借用的名称/标签字符在相应库或映射使用期间有效；definition 若另存 Lua 状态、回调句柄或其他非拥有对象，其目标也必须存活。字符属于 source 自身时，source 当然仍要覆盖字符使用期。不能因为运行期不调用 source 就把尚被借用的数据销毁。

注册接口与约束：

```cpp
template<class TSource>
std::expected<void, std::vector<source_add_error>> definition_source_library::add(const TSource& source);
```

`add` 保存 source 的非拥有引用。有值的 `expected` 表示添加成功；失败则保存名称冲突与缺失依赖的结构化诊断列表。source 对象必须在保存它的 `definition_source_library` 使用期间保持存活；编译库、ID 映射以及编译后配置借用的名称、标签或其他对象须分别覆盖对应使用期。

同时添加多个 source 的重载是原子的：它允许同一批 source 互相依赖，检查时收集全部名称冲突与缺失依赖，失败时整批都不加入。登记和合并都会去重同类别同名、同对象且同类型的项。冲突原因先判断类型不同，再判断同类型但对象不同。动态源也以 C++ 源对象及类型为身份，不自动识别不同对象是否引用同一脚本定义。源库之间合并返回 `expected<void, vector<source_conflict>>`，只检查名称冲突，不重新读取或验证依赖。

批量诊断先按参数顺序记录冲突，再按输入顺序、每个源的类别与依赖声明顺序记录缺失依赖；同一源和重复依赖诊断去重，已有冲突的名称不再被当作缺失依赖。合并按类别和对方库的登记顺序记录冲突。所有输入索引从零开始；冲突第一位置为空表示接收库，第二位置为空表示被合并库。诊断保留分类、名称和位置等结构化数据；`error_string` 对两种错误列表提供文本输出，保持列表顺序，每条一行且末尾无换行。

源库是普通定义源集合，默认构造后通过 `add` 登记 source，不再提供带源参数的构造。四个反应定义由独立的 `basic_definition_sources` 配置：草原核与激化领域使用 combat status source view，燃烧烈焰使用 summon source view，冻结使用 attachment source view。`compile` 显式接收此配置，临时并入四个基础源而不修改原源库；所需其他依赖须由源库或四个基础源满足。绑定来自调用方配置的源对象，不通过固定名称、标签或版本字符串识别。普通 source 的按名依赖仍在 `add` 时验证；基础源冲突、依赖缺失和选择错误通过编译结果的结构化诊断报告。

`make_definition_source_library(sources...)` 提供创建并登记的工厂：默认构造一个库，执行一次批量 `add`，然后返回 `expected<definition_source_library, vector<source_add_error>>`。不传源时返回空库，登记验证失败时返回全部诊断，不因此抛出异常；合并已有库仍使用 `add(library)`。

单项定义编译通过 context 的 `dendro_core_id()`、`catalyzing_field_id()`、`burning_flame_id()`、`frozen_id()` 引用本次配置的基础定义，无须声明具体版本的名称依赖。context 与最终 definition library 使用同一组已解析 ID。

整库编译可以使用源库中的全部定义，也可以通过 `definition_selection` 按类别指定需要的 definition。四个默认反应定义始终属于选择根，与显式选中的定义一起求依赖闭包。源库利用登记时保留的声明信息，在私有准备算法中完成选择、依赖闭包和 ID 分配；executor 中的整库编译通过已有的 `definition_library` 友元关系使用它，再通过 source view 完成最终编译。初始化程序、回合程序与 `compile_mode` 必须在同一次编译中提供；所有响应程序继承该编译模式。编译后的库不能通过合并增补定义；改变定义集合后需要重新编译。

```cpp
auto result = compile(source_library, basics, initialization_program, round_program, givm::compile_mode::normal);
if(not result)
{
    std::println("{}", error_string(result.error()));
    return 1;
}
auto [library, id_map] = std::move(*result);
```

`initialization_program` 只执行一次；随后 `round_program` 会反复执行，直到游戏结束。初始化完成和每轮回合程序完成时自动递增回合数，检查牌桌参数 `max_rounds`，未超限则清空骰子并开始下一轮命令。观察模式在递增后、上限检查前报告 `round_started`；回合程序中的 `start_round` 仅负责显式广播规则通知，应位于投骰命令之后。空回合程序也会自动推进至超限终局；普通响应子程序不推进回合。两者都由不消费响应输入的公开命令值组成；支持固定参数和消费输入两种方式的命令须选择固定参数。`compile(...)` 不提供省略这两段程序的重载。

```cpp
using definition_selection = std::array<std::span<const std::string_view>, definition_types::size()>;
```

需要只编译部分定义时，使用接受 `const definition_selection& selection` 的重载。`selection` 按 definition 类别保存名称序列；每个选中的 definition、四个默认反应定义及它们的传递依赖都会进入编译结果。

[`compile` 的返回值](../reference/executor/compile.md#返回值)为 `expected`：成功值为 `definition_compile_result`，其中 `library` 是编译后的游戏规则，`id_map` 是同一次编译使用的名称映射，供上层在对局开始前把名称形式的牌组或其他输入链接为 issued ID。检查成功后，成功值可以按该顺序结构化绑定；对局运行时只需要 `library`。失败值为 `vector<compile_error>`，每项包含发生位置和一层具体错误 variant，不返回部分 ID 映射。

命令的参数错误独立定义为 `givm::xxx_error`，命令内的 `error_type` 只保留别名。该错误及其文本格式化与公开命令一起位于 definition 的对应命令文件，不需要 executor；整库错误的原因 variant、位置类型与总格式化位于 `executor/compile_error.hpp`。两个 `commands.hpp` 都只负责聚合包含。命令和输入类型列表由 `definition/any_command.hpp` 集中保存，不让错误格式化反向依赖命令实现。

编译上下文只保存当前阶段 `stage_` 和可选源 `source_`，不长期保存程序类别、程序编号和命令下标全为空的完整 `compile_location`。普通上下文诊断在产生时组合位置；`add_program` 检查命令时单独构造程序位置。`program_count_` 仍然保留，用于记录同一个定义源内第几次调用 `add_program`：响应程序的编号从零开始，错误中据此区分该源登记的不同程序。这个计数不参与游戏运行期。

依赖其他定义的效果应在定义源的 `compile(context)` 中构造，通过 `context.resolve_id` 解析已声明依赖，或通过基础定义查询取得本次编译选定的反应定义 ID。上层牌组链接使用编译结果中的 `id_map`。不再提供独立的映射构建入口，也不先编译一份临时库来预取另一轮编译的 ID。

名称、标签和依赖声明在登记后保持不变。ID 准备和最终编译都复用源库登记时缓存的声明数据，不再通过 source view 重读这些声明。编译阶段仍通过 source view 获取响应与查询函数、调用历史摘要布局和 source 的 `compile(...)`。编译器按配套 ID 装配定义，不能把源库的遍历顺序直接当成 ID 顺序。

## 编译库与持久化

**上层格式的设计记录。**以下描述哪些信息适合作为可重建的稳定描述；当前核心没有因此提供任意 C++/Lua definition 的序列化器，也没有承诺 `table`、`executor` 的内存布局可直接作为存档格式。

`definition_library` 是定义源集和游戏流程规则共同编译出的不可变游戏规则，不是一局游戏的可变状态。table 与 executor 共同构成对局状态，都不保存 definition library 指针。执行器每次推进显式接收配套的定义库，table 仅保存实体的定义 ID。

definition library 通过 issued id 提供 definition view、名称、标签和事件分派查询；游戏入口和取指仅供内部执行器使用。编译后的具体 definition 对象由核心传给对应 handler；名称到 issued id 的查找由 `issued_id_map` 提供。其内部容器和程序布局不是公开接口。

需要持久化定义库构建信息时，稳定描述包括按类别记录的 definition source 完整名称，以及初始化程序和回合程序中的公开命令，以及本次使用的编译模式。恢复时，上层注册表按“类别 + 完整名称”找到 source 并重新编译。同类别同名却实现不同属于拓展冲突，核心不尝试序列化或比较任意 C++、Lua 定义实现。

初始化程序和回合程序使用固定的公开命令集，上层格式可以为指令种类分配稳定编号并序列化其公开字段。游戏存档中的可变状态仍是与重建后定义库匹配的 table 和 executor。

## 背后的编译过程

**逻辑阶段与实际写入顺序。**以下步骤说明依赖关系，不要求每一步对应一次独立容器遍历。所有选中定义的声明与能力先准备好，历史摘要布局和 definition 编译随后执行。响应函数表直接保存在最终库中，不等各项 definition 编译后再安装；空查询函数则暂存在编译条目中，等待本源的 definition 产生后求值。尚未完成的库不对外发布；循环依赖能够成立，也依赖于解析时读取预先分配的 ID 和元数据，而非要求对方 definition 已构造。

一份规则库的构建包含以下工作：

1. 临时并入四个基础定义源，复用登记缓存中的定义类别、名称、标签和依赖声明；新登记的基础源按同样方式取得声明。
2. 从 `definition_selection` 指定的定义和四个默认反应定义求出依赖闭包，或选择全部定义，为选中的定义和标签建立 issued id 映射。
3. 建立局部 `definition_library`，按 ID 预填名称、标签位集、响应函数表、历史摘要响应列表和非空查询的自定义函数指针；缺少自定义查询时保留空指针。编译条目引用登记缓存和最终库，并暂存空查询的自定义函数指针。
4. 为全部历史摘要调用 `layout(...)` 并确定字段布局。此时上下文已经可以查看全部选中定义的声明和自定义能力。
5. 编译调用方提供的初始化程序和回合程序，补入回合推进及回跳连接。
6. 为每个选中的 source 建立 `definition_compile_context` 并调用一次 `compile(...)`。名称依赖解析使用已分配 ID；`add_program(...)` 当场检查并追加响应程序、补内部返回连接并返回入口。本源编译未产生错误时，执行其空查询并保存结果；否则跳过本源空查询，继续其他源的编译与诊断收集。
7. 全部 definition 完成后，用默认查询函数补齐非空查询表中的空项，并完成程序链接。
8. 无验证错误时，同时发布不可变的 `definition_library` 和本次编译使用的 `issued_id_map`；否则仅返回收集到的诊断列表。释放仅编译期间需要的条目。

尚未完成编译的 `definition_library` 不通过公开接口作为可运行规则暴露；定义源只能通过编译上下文查看已准备的元数据。程序段存放顺序、入口数值、内部连接指令和擦除存储也都不是定义源接口。definition source 与游戏流程只能提交核心公开命令描述；上层运行期间通过 execution_view 观察领域现场，而不是查看当前指令。

[开发备忘](../notes.md)
