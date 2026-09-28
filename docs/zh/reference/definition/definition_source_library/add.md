[givm](../../../reference.md) / [定义](../../definition.md) / [definition_source_library](../definition_source_library.md) / **add**

# givm::definition_source_library::add

定义于头文件 `<givm/definition.hpp>`

```cpp
std::expected<void, std::vector<source_add_error>> add(); // (1)
std::expected<void, std::vector<source_conflict>> add(const definition_source_library& library); // (2)

template<class TSource>
std::expected<void, std::vector<source_add_error>> add(const TSource& source); // (3)

template<class TFirstSource, class TSecondSource, class... TOtherSource>
std::expected<void, std::vector<source_add_error>> add(
    const TFirstSource& first, const TSecondSource& second, const TOtherSource&... others); // (4)
```

登记一批可供对局使用的定义源。一次调用中的源可以按名称相互依赖，适合一起加入某张卡牌及其生成的状态、召唤物等相关定义。

(1) 不添加内容。(2) 合并另一个源库。(3) 登记一个源。(4) 一起登记多个源。名称在各定义类别内必须唯一。

登记或合并时，同类别同名项若引用同一 C++ 源对象且类型相同，则跳过重复项；其他情况为名称冲突。动态定义源也按其 C++ 源对象及类型区分，不会自动识别不同对象是否引用同一脚本定义。

## 模板参数

|  |  |
| --- | --- |
| `TSource`、`TFirstSource`、`TSecondSource`、`TOtherSource...` | 符合[定义源协议](../source_protocol.md)的类型 |

## 参数

|  |  |
| --- | --- |
| `library` | 要合并的源库 |
| `source`、`first`、`second`、`others...` | 要登记的定义源对象 |

## 返回值

成功时返回有值的 `std::expected`；相同源去重、空批次以及将库合并到自身均视为成功。用 `has_value()` 或条件判断检查结果。

失败时，`error()` 返回本次发现的全部诊断，接收库保持不变：

- (2) 返回 [`source_conflict`](../source_conflict.md) 列表，仅检查名称冲突，不重新读取或验证依赖。
- (3)、(4) 返回 [`source_add_error`](../source_add_error.md) 列表，包含名称冲突与 [`source_missing_dependency`](../source_missing_dependency.md)。名称依赖须在已有库或本批输入中存在；同批源可以互相依赖。

名称冲突先区分类型：类型不同为 `source_conflict::reason::different_type`；类型相同但对象不同为 `different_object`。诊断中的定义名称由 [`definition_name`](../definition_name.md) 保存。

批量登记先按参数顺序收集冲突，再按输入顺序收集缺失依赖；每个源的依赖按定义类别索引及声明顺序检查。相同源与重复依赖不重复产生诊断；已有名称冲突的依赖名称不再额外报告为缺失。合并诊断按定义类别及对方库的登记顺序排列。

输入索引从零开始。`source_conflict::first_input_index` 为空表示接收库中的源；`second_input_index` 为空表示被合并库中的源。批量输入之间发生冲突时，两者均保存相应参数索引。

## 注意

本函数保存对源对象的非拥有引用，源对象不得提前销毁。单个源可声明对自身的名称依赖。编译中的标签筛选只查询已选择的定义，不构成登记依赖。

通过 [`error_string`](../error_string.md) 可将失败结果中的诊断列表转换为可读文本。希望创建库并同时登记源时，可使用 [`make_definition_source_library`](../make_definition_source_library.md)。

## 示例

```cpp
#include <array>
#include <print>
#include <string_view>

#include <givm/givm.hpp>

struct card_source
{
    using definition_category = givm::card_definition;

    std::string_view source_name;

    std::string_view name() const { return source_name; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

struct dependent_card_source
{
    using definition_category = givm::card_definition;

    std::string_view name() const { return "求助牌"; }
    auto support_dependencies() const
    { return std::array<std::string_view, 1>{ "失踪支援" }; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

int main()
{
    const card_source potion{ "恢复药剂" };
    const card_source food{ "恢复料理" };
    givm::definition_source_library sources{};
    std::println("批量登记成功: {}", sources.add(potion, food).has_value());
    std::println("重复登记成功: {}", sources.add(potion).has_value());

    const card_source another_potion{ "恢复药剂" };
    const dependent_card_source dependent{};
    const auto result = sources.add(another_potion, dependent);
    if(not result)
        std::println("{}", error_string(result.error()));
    std::println("失败后登记求助牌: {}", sources.has<givm::card_definition>("求助牌"));
}
```

输出

```text
批量登记成功: true
重复登记成功: true
source conflict (different_object): card_definition "恢复药剂"; first: receiver library; second: input[0]
missing dependency: card_definition "求助牌" (input[1]) requires support_view "失踪支援"
失败后登记求助牌: false
```
