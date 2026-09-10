[givm](../../../reference.md) / [通用工具](../../utils.md) / [type_list](../type_list.md) / **all**

# givm::type_list::all

定义于头文件 `<givm/utils/type_list.hpp>`

```cpp
template<class F>
static constexpr bool all(F&& fn);
```

检查序列中的每种类型是否都满足给定条件。

## 模板参数

|  |  |
| --- | --- |
| `F` | 可调用类型；须能通过左值 `fn.template operator()<T>()` 检查序列中的每种类型 `T`，且结果可用作布尔条件 |

## 参数

|  |  |
| --- | --- |
| `fn` | 以 `fn.template operator()<T>()` 检查每种类型的条件 |

## 返回值

全部为真时返回 `true`；遇到第一个假值停止。空序列返回 `true`。

## 示例

```cpp
#include <print>
#include <type_traits>

#include <givm/utils/type_list.hpp>

int main()
{
    const auto integral = []<class T>() { return std::is_integral_v<T>; };
    std::println("全是整数类型: {}", givm::type_list<int, unsigned>::all(integral));
    std::println("包含浮点类型时: {}", givm::type_list<int, double>::all(integral));
}
```

输出

```text
全是整数类型: true
包含浮点类型时: false
```
