[givm](../../../reference.md) / [定义](../../definition.md) / [definition_source_library](../definition_source_library.md) / **(构造函数)**

# givm::definition_source_library::definition_source_library

定义于头文件 `<givm/definition.hpp>`

```cpp
definition_source_library(); // (1)

template<class... TSources>
    requires (sizeof...(TSources) != 0)
        && (not std::same_as<std::remove_cvref_t<TSources>, definition_source_library> && ...)
explicit definition_source_library(const TSources&... sources); // (2)
```

建立可供编译的定义源集合。(1) 建立空集合。(2) 一次登记所给源。元素反应采用的四个基础定义由编译时的 [`basic_definition_sources`](../basic_definition_sources.md) 另行指定，不由构造参数位置决定。

## 模板参数

|  |  |
| --- | --- |
| `TSources...` | 符合[定义源协议](../source_protocol.md)的源类型，不包含 `definition_source_library` |

## 参数

|  |  |
| --- | --- |
| `sources...` | 一起登记的源，可相互满足按名称声明的依赖 |

## 返回值

（无）

## 异常

|  |  |
| --- | --- |
| `std::invalid_argument` | 本批源有同类别名称冲突，或按名称声明的依赖缺失 |

## 注意

本函数不拥有源对象，遵守与 [`add`](add.md) 相同的生命周期约定。一批源可相互依赖，登记后仍可继续用 `add` 增补其他内容。

构造函数只登记普通定义，不自动加入基础反应定义。需要合并另一个源库时使用 [`add`](add.md)。

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
