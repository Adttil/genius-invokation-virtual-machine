[givm](../../../reference.md) / [定义](../../definition.md) / [查询](../queries.md) / **support_state_limit**

# givm::support_state_limit

定义于头文件 `<givm/definition.hpp>`

给出支援各项状态的上限，例如最多保留的层数或可用次数。

```cpp
struct support_state_limit
{
    using result_t = support_state;
};
```

## 返回值

[support_state](../../table/support_state.md)，其中每个字段表示该字段允许保存的上限。添加、设置状态和按增量修改状态的命令均使用这些上限。

本查询没有参数，结果在编译定义库时保存，运行时直接读取。定义源未提供查询时，[query_default](../query_default.md) 返回两个字段均为 `UINT32_MAX` 的状态。

添加时，命令输入 `state` 的各字段默认同样为 `UINT32_MAX`；它们经过普通的上限裁剪取得初值。查询限定上限，调用方仍可显式请求更小的初值。
