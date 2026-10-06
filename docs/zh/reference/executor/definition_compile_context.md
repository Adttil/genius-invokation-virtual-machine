[givm](../../reference.md) / [执行](../executor.md) / **definition_compile_context**

# givm::definition_compile_context

定义于头文件 `<givm/definition_source.hpp>`

```cpp
class definition_compile_context;
```

编写实体定义时使用的编译环境。它提供本次规则集合的定义信息，解析必须存在的名称依赖，并登记响应事件时要执行的效果。

## 成员类型

| | |
| --- | --- |
| [`definition_view`](definition_compile_context/definition_view.md) | 本次编译集合中一项定义的元数据视图 |

## 成员函数

|  |  |
| --- | --- |
| [`resolve_id`](definition_compile_context/resolve_id.md) | 按名称取得已声明依赖的定义 ID |
| [`definitions`](definition_compile_context/definitions.md) | 遍历本次集合中指定类别的定义 |
| [`find_definition`](definition_compile_context/find_definition.md) | 按名称查找本次集合中的定义 |
| [`operator[]`](definition_compile_context/operator_at.md) | 按 ID 取得定义的元数据视图 |
| [`find_tag`](definition_compile_context/find_tag.md) | 查找本次集合中的标签 ID |
| [`find_ids_by_tag`](definition_compile_context/find_ids_by_tag.md) | 筛选本次集合中满足标签条件的定义 |
| [`add_normal_effect`](definition_compile_context/add_normal_effect.md) | 登记普通效果 |
| [`add_immediate_effect`](definition_compile_context/add_immediate_effect.md) | 登记立即效果 |
| [`add_preview_effect`](definition_compile_context/add_preview_effect.md) | 登记预览效果 |
| [`add_effect`](definition_compile_context/add_effect.md) | 登记一段效果并取得入口 |
| [`definition_count<Category>()`](../definition/history_summary.md#定义源协议) | 取得最终编译集合内指定类别的定义数量 |
| [`history_field<T>(name)`](../definition/history_summary.md#定义源协议) | 取得当前摘要自身字段的访问键 |
| [`resolve_history_field<T>(summary, name)`](../definition/history_summary.md#定义源协议) | 取得已声明依赖的摘要字段读取键 |

| [`default_reaction_id`](definition_compile_context/default_reaction_id.md) | 取得一个槽位的默认反应定义 ID |

## 注意

由 [`givm::compile`](compile.md) 在调用定义源的 `compile` 时提供，只在本次编译调用中使用。`resolve_id` 解析的硬依赖须通过[定义源协议](../definition/source_protocol.md)提前声明；遍历、查找和标签筛选不需要声明依赖，也不扩充本次集合。默认反应 ID 查询直接使用本次 [`reaction_definition_names`](../definition/reaction_definition_names.md) 配置，无须声明具体名称依赖。

本次集合在任何定义的 `compile` 或历史摘要的 `layout` 开始前已经确定。所有定义的名称、标签和能力信息均可查询，无须等待被查询定义完成编译。元数据视图不提供执行响应或查询结果的接口，且只能在本次编译期间使用；可以保存取得的 ID，不能把视图或其借用的范围保存到对局运行期。

历史摘要的 `layout` 也接收只读编译上下文，此时可以查询最终定义数量，但不能取得字段键。所有摘要字段描述完成后才调用定义的 `compile`；此时 `history_field<T>` 返回摘要自身使用的 `history_field_key<T>`，`resolve_history_field<T>` 返回通过牌桌读取的 `history_value_key<T>`。数组字段使用 `T[]`。省略模板参数时，前者返回 `dynamic_history_field`，后者返回 `dynamic_history_value`，均为各强类型键的 variant。字段访问阶段、名称、类型、数组形态或声明依赖不符时记录结构化诊断，最终由 [`compile`](compile.md) 返回失败；本次取得的无效键不能用于访问状态。

## 示例

```cpp
#include <utility>
#include <array>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct support_source
{
    using definition_category = givm::support_view;
    std::string_view name() const { return "协助者"; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

struct card_source
{
    using definition_category = givm::card_definition;
    std::string_view name() const { return "召唤卡"; }
    auto support_dependencies() const
    { return std::array<std::string_view, 1>{ "协助者" }; }

    givm::definition_id<givm::support_view> compile(givm::definition_compile_context& context) const
    {
        const auto support = context.resolve_id<givm::support_view>("协助者");
        std::println("已找到依赖的支援: {}", support.is_valid());
        return support;
    }
};

int main()
{
    const card_source card{};
    const support_source support{};
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    if(not sources.add(card, support)) return 1;
    auto library_result = compile(
        sources, basics,
        std::tuple{}, std::tuple{ givm::start_round{}, givm::settle{} }, givm::compile_mode::normal
    );
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
}
```

输出

```text
已找到依赖的支援: true
```
