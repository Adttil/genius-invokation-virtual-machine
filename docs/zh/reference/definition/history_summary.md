[givm](../../reference.md) / [定义](../definition.md) / **历史摘要**

# 历史摘要

定义于头文件 `<givm/definition.hpp>`；编译与牌桌接口完整使用时包含 `<givm/givm.hpp>`。

历史摘要是按定义选择的对局记录，例如双方本回合被击倒的次数、整局已经使用的某类卡牌数量。它保存效果实际需要的累计值，不保存全部历史事件，也不属于角色附属、出战状态或其他场上实体。每个牌桌为每项已编译的摘要保留一份独立状态；双方记录可以是摘要中的两个字段，或一个长度为 2 的数组。

需要历史的定义通过 `history_summary_dependencies()` 声明名称依赖。摘要和其他定义一样参与[定义选择与依赖闭包](source_protocol.md#可选的分类与依赖)：只编译选定的定义及其依赖时，未被选中或依赖的摘要不参与对局。后续生成的卡牌可以读取从开局就记录的结果，不要求卡牌在事件发生时已经在手牌中。

## 类

| | |
| --- | --- |
| `history_summary_definition` | 历史摘要的定义类别 |
| `history_field_descriptor` | 字段名称、数值类型、是否为数组及元素数量的描述 |
| `history_field_key<T>` | 摘要更新函数访问自身字段的键 |
| `history_value_key<T>` | 效果或上层代码读取牌桌历史字段的键 |
| `dynamic_history_field` | 尚未选择 C++ 数值类型的字段描述 |
| `history_summary_state` | 一项历史摘要的可写状态视图 |

## 类型别名

| | |
| --- | --- |
| `history_summary_layout` | `std::vector<history_field_descriptor>`，一项摘要的字段描述列表 |

## 函数

| | |
| --- | --- |
| `history_field<T>(name)` | 描述一个标量字段 |
| `history_array<T>(name, count)` | 描述一个指定长度的数组字段 |

`T` 可为 `bool`、`std::int8_t`、`std::uint8_t`、`std::int16_t`、`std::uint16_t`、`std::int32_t`、`std::uint32_t`、`std::int64_t`、`std::uint64_t`、`float` 或 `double`。数组长度可为零，在编译定义库时确定，对局中不改变。读取数组字段的键以 `T[]` 为模板参数；描述数组时仍使用 `history_array<T>`。

## 枚举

`history_value_type` 表示动态字段描述中的数值类型。

| | |
| --- | --- |
| `boolean` | `bool` |
| `i8`、`u8` | `std::int8_t`、`std::uint8_t` |
| `i16`、`u16` | `std::int16_t`、`std::uint16_t` |
| `i32`、`u32` | `std::int32_t`、`std::uint32_t` |
| `i64`、`u64` | `std::int64_t`、`std::uint64_t` |
| `f32`、`f64` | `float`、`double` |

## 定义源协议

摘要源沿用普通[定义源协议](source_protocol.md)的名称、标签、依赖与 `compile`，定义类别为 `history_summary_definition`。此外必须提供字段描述：

```cpp
givm::history_summary_layout layout(const givm::definition_compile_context& context) const;
```

编译先确定最终定义集合及其 ID，再取得所有摘要的 `layout`，最后调用各定义的 `compile`。因此 `layout` 可以通过 `context.definition_count<Category>()` 按最终定义数量决定数组长度，但此时不能查询历史字段。字段名称须在摘要内唯一，标量字段的 `count` 须为 1。

在摘要自身的 `compile` 中，`context.history_field<T>(name)` 取得 `history_field_key<T>`。其他定义先声明摘要依赖，再通过 `context.resolve_history_field<T>(summary, name)` 取得 `history_value_key<T>`。这些键可以保存在 `compile` 返回的定义数据中，不需要运行时按名称查找。

摘要通过事件响应更新自己的字段，其中 `D` 是本源 `compile` 的返回类型：

```cpp
static void handle(const D& definition, givm::history_summary_state state, const Event& event,
    const givm::table& table, const givm::definition_library& library);
```

`Event` 须属于 `subscribed_events<history_summary_definition>`。其中 [`history_summary_initialization`](events/history_summary_initialization.md) 仅供摘要初始化：[`executor::start`](../executor/executor/start.md) 准备好状态空间后同步发送一次，返回前完成全部初始化响应。通常先完成双方 [`load_deck`](../executor/load_deck.md)，让该响应可以读取完整初始牌桌。字段不保证清零，源须在读取前写入有效值；可接受后续首次写入的字段不必在此初始化。摘要之间不得依赖初始化先后；需要共同初始化的数据应放在同一摘要中。

其余可订阅事件均为通知类事件，例如 [`round_started`](events/round_started.md)、[`character_defeated`](events/character_defeated.md)、[`skill_used`](events/skill_used.md)、[`card_played`](events/card_played.md)。报价、参数检查、伤害计算与濒死等可修改或尚未确认结果的时机不用于摘要更新。摘要不提供普通查询。

静态摘要源通过是否存在相应 `handle` 决定订阅。动态源声明 `static constexpr bool is_dynamic = true;`，并以 `template<class Event> bool can_handle() const` 选择实际提供的响应；此处没有实体 view 模板参数。动态摘要须为全部可订阅事件提供 `can_handle` 和签名正确的静态 `handle`，包括返回 `false` 的事件；构造 `definition_source_view` 时检查完整性，缺少接口或返回类型错误属于 C++ 编译错误。返回 `false` 时不会调用该响应；定义源若自行直接调用该不支持的分支，属于未定义行为。适配器需要的脚本状态可以保存在 `D` 中。

## 更新与读取

普通实体对该通知的全部响应及其效果完成后，最后更新订阅该通知的摘要。因此普通响应读取的摘要尚未包含当前通知；若其效果触发并完成了嵌套通知，则可以读到嵌套通知的记录。因输入请求而暂停时，本通知的摘要尚未更新；恢复后完成全部响应才更新一次。只有实际发生并完成普通响应的通知才更新摘要；例如角色成功复活或击倒导致立即终局时，不发送 `character_defeated`，因此不增加其计数。

不同摘要不应依赖彼此对同一事件的更新先后；需要联合更新的数据应放在同一摘要中。

“本回合”记录应区分记录所属回合与当前 `table.state().round_number`。自动回合推进先增加回合数，`round_started` 通知则在投骰之后；若条件在投骰期间也要读取本回合记录，不能只依靠该通知清零。可以同时记录回合编号，更新时发现编号改变便重置计数，读取时将旧回合的记录视为零。

摘要函数的事件、牌桌和定义库参数均只读，返回值为 `void`；函数不取得随机源，也不提交效果程序。通过 `state[key]` 只能修改当前摘要的字段：标量返回 `T&`，数组返回 `std::span<T>`。

向历史摘要发送通知或初始化事件时，所有响应均在当前调用中同步完成，不产生输入等待或可观察的执行状态。

效果通过只读牌桌的 `table[key]` 读取 `history_value_key<T>`：标量返回 `const T&`，数组返回 `std::span<const T>`。上层可以通过 `library.history_field<T>(summary_id, name)` 取得相同的读取键。键须用于产生它的定义库及配套牌桌；不要使用默认构造或不匹配的键。复制牌桌会独立复制摘要状态，修改副本不影响原牌桌。

两种键都提供返回 `std::size_t` 的 `offset()` 和 `count()`。`history_field_key<T>::offset()` 是字段在本摘要中的字节偏移，`history_value_key<T>::offset()` 是字段在牌桌历史区中的字节偏移；二者不能互换。`count()` 是元素数量，标量恒为 1，数组长度随键保存。

## 动态字段描述

Lua 等适配器可以直接生成 `history_field_descriptor{ name, type, is_array, count }`，不必让脚本使用 C++ 类型模板。未指定模板参数的 `context.history_field(name)`、`context.resolve_history_field(summary, name)` 和 `library.history_field(summary_id, name)` 返回 `dynamic_history_field`。

适配器根据其 `type` 和 `is_array` 选择 C++ 类型：自身字段使用 `as<T>()` 转为 `history_field_key<T>`，供牌桌读取的字段使用 `as_value<T>()` 转为 `history_value_key<T>`。数组使用 `T[]`，标量使用 `T`；转换必须与描述相符，并按原查询接口选择对应的键种类。

## 异常

| | |
| --- | --- |
| `std::invalid_argument` | 字段描述无效或重名、字段不存在、字段类型或数组形态不匹配、在 `layout` 阶段查询字段，或查询未声明依赖的摘要 |
| `std::length_error` | 编译历史摘要时，字段数组或整体布局所需的长度溢出 |

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct defeats_source
{
    using definition_category = givm::history_summary_definition;

    std::string_view name() const { return "角色击倒累计"; }

    givm::history_summary_layout layout(const givm::definition_compile_context&) const
    {
        return { givm::history_array<std::uint32_t>("次数", 2) };
    }

    givm::history_field_key<std::uint32_t[]> compile(givm::definition_compile_context& context) const
    {
        return context.history_field<std::uint32_t[]>("次数");
    }

    static void handle(const givm::history_field_key<std::uint32_t[]>& counts,
        givm::history_summary_state state, const givm::history_summary_initialization&,
        const givm::table&, const givm::definition_library&)
    {
        for(auto& count : state[counts]) count = 0;
    }

    static void handle(const givm::history_field_key<std::uint32_t[]>& counts,
        givm::history_summary_state state, const givm::character_defeated& event,
        const givm::table&, const givm::definition_library&)
    {
        ++state[counts][event.target.player_id.index];
    }
};

int main()
{
    const defeats_source summary{};
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    givm::definition_source_library sources{};
    if(not sources.add(summary)) return 1;
    const auto [library, ids] = compile(sources, basics, std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    givm::table table;
    load_deck(table, library, {}, {});
    givm::executor execution;
    execution.start(library, table);
    const auto id = ids.get_id<givm::history_summary_definition>(summary.name());
    const auto counts = library.history_field<std::uint32_t[]>(id, "次数");
    std::println("玩家0累计被击倒次数: {}", table[counts][0]);
}
```

输出

```text
玩家0累计被击倒次数: 0
```
