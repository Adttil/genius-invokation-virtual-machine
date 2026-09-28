[givm](../../reference.md) / [执行](../executor.md) / **compile_stage**

# givm::compile_stage

定义于头文件 `<givm/executor.hpp>`

```cpp
enum class compile_stage
{
    source_selection,
    history_layout,
    definition,
    program
};
```

编译错误发生的阶段。

## 枚举项

| | |
| --- | --- |
| `source_selection` | 合并基础定义、选择定义及准备依赖闭包 |
| `history_layout` | 准备历史摘要的字段布局 |
| `definition` | 执行定义源的编译与静态查询 |
| `program` | 验证和编译命令序列 |
