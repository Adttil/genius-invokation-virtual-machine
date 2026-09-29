[givm](../../reference.md) / [定义](../definition.md) / **make_definition_source_library**

# givm::make_definition_source_library

定义于头文件 `<givm/source_library.hpp>`

```cpp
template<class... TSources>
inline std::expected<definition_source_library, std::vector<source_add_error>>
make_definition_source_library(const TSources&... sources);
```

创建定义源库并批量登记所给定义源，返回完整的源库或全部登记诊断，便于在创建时处理名称冲突和缺失依赖。

## 模板参数

| | |
| --- | --- |
| `TSources...` | 符合[定义源协议](source_protocol.md)的类型，不接受 `definition_source_library` |

## 参数

| | |
| --- | --- |
| `sources...` | 本次登记的源，可互相满足按名称声明的依赖；允许为空 |

## 返回值

成功时，`expected` 包含已登记全部源的 [`definition_source_library`](definition_source_library.md)；不传入源时得到空库。

失败时，`error()` 包含 [`source_add_error`](source_add_error.md) 列表，诊断内容和顺序与向空库执行一次批量 [`add`](definition_source_library/add.md) 相同。名称冲突和缺失依赖通过返回值报告，不因此抛出验证异常。

## 注意

函数使用与 `add` 相同的去重、依赖和生命周期规则，不拥有源对象。源对象及其名称、标签等借用存储必须覆盖对应使用期。

本函数接收定义源，用于创建库；合并已有源库使用 [`definition_source_library::add`](definition_source_library/add.md)。默认反应的 [`basic_definition_sources`](basic_definition_sources.md) 仍在准备 ID 映射和编译时单独提供。

## 示例

```cpp
#include <print>
#include <string_view>

#include <givm/source_library.hpp>

struct card_source
{
    using definition_category = givm::card_definition;

    std::string_view name() const { return "恢复药剂"; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

int main()
{
    const card_source potion{};
    auto result = givm::make_definition_source_library(potion);
    if(not result) return 1;
    std::println("创建时登记恢复药剂: {}", result->has<givm::card_definition>("恢复药剂"));

    auto empty = givm::make_definition_source_library();
    if(not empty) return 1;
    std::println("空库包含恢复药剂: {}", empty->has<givm::card_definition>("恢复药剂"));
}
```

输出

```text
创建时登记恢复药剂: true
空库包含恢复药剂: false
```

## 参阅

| | |
| --- | --- |
| [`definition_source_library::add`](definition_source_library/add.md) | 向已有库登记定义源或合并源库 |
| [`error_string`](error_string.md) | 将诊断列表转换为可读文本 |
