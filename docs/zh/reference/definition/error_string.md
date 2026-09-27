[givm](../../reference.md) / [定义](../definition.md) / **error_string**

# givm::error_string

定义于头文件 `<givm/definition.hpp>`

```cpp
inline std::string error_string(const std::vector<source_add_error>& errors); // (1)
inline std::string error_string(const std::vector<source_conflict>& errors); // (2)
```

将定义源登记或合并的结构化诊断转换为可读文本，供日志和错误提示使用。

(1) 格式化登记源时的冲突与缺失依赖。(2) 格式化合并源库时的名称冲突。

## 参数

| | |
| --- | --- |
| `errors` | [`add`](definition_source_library/add.md) 或 [`make_definition_source_library`](make_definition_source_library.md) 返回的诊断列表 |

## 返回值

按列表原顺序逐条格式化的字符串，每条诊断占一行，行间使用换行符，末尾没有换行符。空列表得到空字符串。

冲突文本包含定义类别名称、定义名称、对象不同或类型不同的原因，以及双方来自输入参数还是已有库。缺失依赖文本包含源定义、参数索引及缺失依赖的类别和名称。输入参数索引从零开始，与结构化诊断保持一致。

定义类别使用 [`definition_types`](definition_types.md) 中的 C++ 类型名称，如 `card_definition`、`support_view`。冲突原因写作 `different_object` 或 `different_type`；参数位置写作 `input[N]`，接收库与被合并库分别写作 `receiver library` 和 `incoming library`。完整调用与输出示例见 [`add`](definition_source_library/add.md#示例)。

定义名称使用双引号包围，其中的反斜线、双引号、换行符、回车符和制表符分别转义为 `\\`、`\"`、`\n`、`\r` 和 `\t`，使每条诊断保持为一行。

## 注意

本函数只生成文本，不排序或去重诊断，也不改变源库。程序需要按错误类型或位置分支时，应读取原诊断字段。

## 参阅

| | |
| --- | --- |
| [`source_add_error`](source_add_error.md) | 登记源时的诊断 variant |
| [`source_conflict`](source_conflict.md) | 同类别同名源的冲突诊断 |
