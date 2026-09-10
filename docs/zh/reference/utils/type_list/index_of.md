[givm](../../../reference.md) / [通用工具](../../utils.md) / [type_list](../type_list.md) / **index_of**

# givm::type_list::index_of

定义于头文件 `<givm/utils/type_list.hpp>`

```cpp
template<class U>
    requires requires { index_of(std::type_identity<U>{}); }
static consteval size_t index_of() noexcept;

static consteval size_t index_of(std::type_identity<T>);
```

查找一种类型在序列中的位置。

序列中的每种类型 `T` 都提供一个接收 `std::type_identity<T>` 的重载。

## 模板参数

|  |  |
| --- | --- |
| `U` | 要查找的类型 |

## 参数

|  |  |
| --- | --- |
| 类型标记实参 | 也可用 `std::type_identity<T>` 指定要查找的类型 |

## 返回值

从零开始的位置。

## 注意

被查找类型必须存在且能唯一确定位置。

## 示例

```cpp
#include <print>

#include <givm/utils/type_list.hpp>

int main()
{
    using values = givm::type_list<int, double>;
    std::println("double 的位置: {}", values::index_of<double>());
}
```

输出

```text
double 的位置: 1
```
