[givm](../../../reference.md) / [定义](../../definition.md) / [definition_source_library](../definition_source_library.md) / **(构造函数)**

# givm::definition_source_library::definition_source_library

定义于头文件 `<givm/definition.hpp>`

```cpp
definition_source_library();
```

建立空的定义源集合。随后通过 [`add`](add.md) 登记定义源或合并其他源库，并检查返回的结构化诊断。

需要在创建时登记一批源，可以使用 [`make_definition_source_library`](../make_definition_source_library.md) 同时取得源库或全部登记诊断。

## 返回值

(无)

## 注意

构造函数不登记任何定义。元素反应采用的四个基础定义仍由编译时的 [`basic_definition_sources`](../basic_definition_sources.md) 另行指定。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    std::println("已登记恢复药剂: {}", sources.has<givm::card_definition>("恢复药剂"));
}
```

输出

```text
已登记恢复药剂: false
```
