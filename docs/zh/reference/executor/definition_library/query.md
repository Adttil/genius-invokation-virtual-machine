[givm](../../../reference.md) / [执行](../../executor.md) / [definition_library](../definition_library.md) / **query**

# givm::definition_library::query

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TDefinitionType, class TQuery>
TQuery::result_t query(definition_id<TDefinitionType> id, const TQuery& parameters) const;
```

取得指定定义的规则信息或检查结果。角色初始状态、卡牌初始费用等无参查询读取定义库中已保存的结果；目标检查等查询使用当前提供的参数。

## 模板参数

| | |
| --- | --- |
| `TDefinitionType` | 由 ID 推导的定义类别。 |
| `TQuery` | 该类别的 [`supported_queries`](../../definition/supported_queries.md) 中的查询类型。 |

## 参数

| | |
| --- | --- |
| `id` | 本定义库中的有效定义 ID。 |
| `parameters` | 查询所需的参数，须满足相应[查询类型](../../definition/queries.md)的前提。 |

## 返回值

`TQuery::result_t` 类型的查询结果。未提供源查询时使用 [`query_default`](../../definition/query_default.md) 的结果。

## 注意

查询参数中的实体与牌桌须使用本定义库；自身实体须采用 `id` 指定的定义。查询不提交行动，不执行命令，不接收随机源。

对于 `std::is_empty_v<TQuery>` 为 `true` 的查询，每次编译定义库时求值一次，此处只返回已保存的结果。其他查询在本次调用中求值；查询抛出的异常会传递给调用方。

## 示例

```cpp
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct character_source
{
    using definition_category = givm::character_view;

    std::string_view name() const { return "角色"; }
    int compile(givm::definition_compile_context&) const { return 10; }

    static givm::character_state query(const int& health, const givm::character_initial_state&)
    {
        return { .max_health = static_cast<std::uint32_t>(health), .health = static_cast<std::uint32_t>(health) };
    }
};

int main()
{
    const character_source source{};
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    sources.add(source);
    const auto [library, ids] = compile(
        sources, std::tuple{}, std::tuple{ givm::start_round{} }, givm::compile_mode::normal);
    const auto id = ids.get_id<givm::character_view>("角色");
    const auto state = library.query(id, givm::character_initial_state{});
    std::println("初始生命: {}", state.health);
}
```

输出

```text
初始生命: 10
```
