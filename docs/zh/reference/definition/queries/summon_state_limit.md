[givm](../../../reference.md) / [定义](../../definition.md) / [查询](../queries.md) / **summon_state_limit**

# givm::summon_state_limit

定义于头文件 `<givm/definition.hpp>`

给出召唤物各项状态的默认上限，包括效果量与可用次数。

```cpp
struct summon_state_limit
{
    using result_t = summon_state;
};
```

## 返回值

[summon_state](../../table/summon_state.md)，其中每个字段表示新建和普通状态修改所采用的默认上限，并非实体状态必须始终满足的约束。召唤、直接添加、设置状态和按增量修改状态默认使用这些上限。

本查询没有参数，结果在编译定义库时保存，运行时直接读取。定义源未提供查询时，[query_default](../query_default.md) 返回两个字段均为 `UINT32_MAX` 的状态。

召唤或直接添加时，命令输入 `state` 的各字段默认同样为 `UINT32_MAX`；它们经过普通的上限裁剪取得初值。创建时按此上限裁剪，调用方仍可显式请求更小的初值。

`usages` 的上限可以为零，创建召唤物也允许使用零次可用次数。

[set_summon_state](../commands/set_summon_state.md) 和 [modify_summon_state](../commands/modify_summon_state.md) 可通过命令的 `ignore_limit` 选项突破此上限。普通修改不会仅因当前值已超限而将它压回上限；赋值仍允许明确要求较小的值。
