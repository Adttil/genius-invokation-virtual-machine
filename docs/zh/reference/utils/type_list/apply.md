[givm](../../../reference.md) / [通用工具](../../utils.md) / [type_list](../type_list.md) / **apply**

# givm::type_list::apply

定义于头文件 `<givm/utils/type_list.hpp>`

```cpp
template<template<class...> class U>
using apply = U<T...>;
```

以本序列作为类型实参的 `U` 特化。

## 模板参数

|  |  |
| --- | --- |
| `U` | 能接受相应类型实参的模板 |

## 示例

```cpp
#include <print>
#include <tuple>

#include <givm/utils/type_list.hpp>

int main()
{
    using record = givm::type_list<int, double>::apply<std::tuple>;
    record value{ 7, 2.5 };
    std::println("记录中的整数: {}", std::get<0>(value));
}
```

输出

```text
记录中的整数: 7
```
