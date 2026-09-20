[givm](../../../reference.md) / [定义](../../definition.md) / [查询](../queries.md) / **skill_initial_cost**

# givm::skill_initial_cost

定义于头文件 `<givm/definition.hpp>`

```cpp
struct skill_initial_cost
{
    using result_t = action_cost_requirement;
};
```

取得技能自身的骰子、充能费用与行动速度。使用技能费用预览以此为基础，再通过 [`cost_of_skill`](../events/cost_of_skill.md) 响应处理本场对局中的增减费与速度调整。

## 成员类型

| | |
| --- | --- |
| `result_t` | [`action_cost_requirement`](../../table/action_cost_requirement.md) |

## 注意

本查询没有参数，结果在编译定义库时按具体定义保存；每次报价直接读取，不重新调用定义源的查询函数，也不依赖所选目标。

结果的 `energy_tag` 指定费用需要的充能类型。需要替代充能时，定义源通过 `tag_dependencies()` 声明资源标签，在 `compile` 中用 [`resolve_tag`](../../executor/definition_compile_context/resolve_tag.md) 取得 ID，再写入本查询结果。

未提供源查询时，[默认方法](../query_default.md)返回零骰子、零充能费用与 `action_speed::combat`。
