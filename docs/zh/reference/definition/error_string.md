[givm](../../reference.md) / [定义](../definition.md) / **error_string**

# givm::error_string

源准备诊断的重载 (1)—(3) 定义于头文件 `<givm/source_library.hpp>`；全部重载也可通过 `<givm/definition.hpp>` 引入。

```cpp
std::string error_string(const std::vector<source_add_error>& errors); // (1)
std::string error_string(const std::vector<source_conflict>& errors); // (2)
std::string error_string(const std::vector<source_preparation_error>& errors); // (3)
inline std::string error_string(const std::vector<deck_link_error>& errors); // (4)

inline std::string error_string(const draw_cards_error& error); // (5)
inline std::string error_string(start_round_error error); // (6)
```

将定义源、牌组链接或单个命令的结构化诊断转换为可读文本，供日志和错误提示使用。

(1) 格式化登记源时的冲突与缺失依赖。(2) 格式化合并源库时的名称冲突。(3) 格式化基础源合并与定义选择时的诊断。(4) 格式化牌组链接时缺失的卡牌或角色名称与位置。

(5)、(6) 为单个命令错误的重载示例。每个命令都有独立的非模板重载：结构体错误以 `const 命令名_error&` 接收，空枚举错误按值接收。这里分别列出 [`draw_cards_error`](commands/draw_cards.md#编译检查) 与 [`start_round_error`](commands/start_round.md#编译检查)，其余类型见对应[命令页面](commands.md)。

## 参数

| | |
| --- | --- |
| `errors` | [`add`](definition_source_library/add.md)、[`make_definition_source_library`](make_definition_source_library.md) 或 [`link_deck`](link_deck.md) 返回的诊断列表，也可格式化从编译诊断中提取的源准备错误 |
| `error` | [`compile`](../executor/compile.md) 诊断中一条命令的具体参数错误，与该命令的 `error_type` 别名为同一类型 |

## 返回值

按列表原顺序逐条格式化的字符串，每条诊断占一行，行间使用换行符，末尾没有换行符。空列表得到空字符串。

单命令错误返回命令名称、具体错误原因和相关字段数值，不追加换行，也不包含源或程序位置。没有错误项的空枚举重载返回空字符串；对应命令的检查不会产生错误值。

冲突文本包含定义类别名称、定义名称、对象不同或类型不同的原因，以及双方来自输入参数还是已有库。缺失依赖文本包含源定义、参数索引及缺失依赖的类别和名称。选择错误包含缺失定义，若由另一项定义要求还会列出该来源。输入参数索引从零开始，与结构化诊断保持一致。

定义类别使用 [`definition_types`](definition_types.md) 中的 C++ 类型名称，如 `card_definition`、`support_view`。冲突原因写作 `different_object` 或 `different_type`；参数位置写作 `input[N]`，接收库与被合并库分别写作 `receiver library` 和 `incoming library`。完整调用与输出示例见 [`add`](definition_source_library/add.md#示例)。

定义名称使用双引号包围，其中的反斜线、双引号、换行符、回车符和制表符分别转义为 `\\`、`\"`、`\n`、`\r` 和 `\t`，使每条诊断保持为一行。

## 注意

本函数只生成文本，不排序或去重诊断，也不改变源库。程序需要按错误类型或位置分支时，应读取原诊断字段。

## 参阅

| | |
| --- | --- |
| [`source_add_error`](source_add_error.md) | 登记源时的诊断 variant |
| [`source_conflict`](source_conflict.md) | 同类别同名源的冲突诊断 |
| [编译诊断的 `error_string`](../executor/error_string.md) | 格式化附有源、程序和命令位置的编译诊断 |
