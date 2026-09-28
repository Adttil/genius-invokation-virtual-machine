[givm](../../../reference.md) / [定义](../../definition.md) / [查询](../queries.md) / **combat_status_state_limit**

# givm::combat_status_state_limit

定义于头文件 `<givm/definition.hpp>`

给出出战状态各项状态的默认上限，例如普通增层采用的层数上限或可用次数上限。

```cpp
struct combat_status_state_limit
{
    using result_t = combat_status_state;
};
```

## 返回值

[combat_status_state](../../table/combat_status_state.md)，其中每个字段表示新建和普通状态修改所采用的默认上限，并非实体状态必须始终满足的约束。生成、直接添加、设置状态和按增量修改状态默认使用这些上限。

本查询没有参数，结果在编译定义库时保存，运行时直接读取。定义源未提供查询时，[query_default](../query_default.md) 返回两个字段均为 `UINT32_MAX` 的状态。

生成或直接添加时，命令输入 `state` 的各字段默认同样为 `UINT32_MAX`；它们经过普通的上限裁剪取得初值。创建时按此上限裁剪，调用方仍可显式请求更小的初值。

[set_combat_status_state](../commands/set_combat_status_state.md) 和 [modify_combat_status_state](../commands/modify_combat_status_state.md) 可通过命令的 `ignore_limit` 选项突破此上限。普通修改不会仅因当前值已超限而将它压回上限；赋值仍允许明确要求较小的值。
