[givm](../../../reference.md) / [通用工具](../../utils.md) / [type_list](../type_list.md) / **get**

# givm::type_list::get

定义于头文件 `<givm/utils/type_list.hpp>`

```cpp
static consteval std::type_identity<T> get(std::integral_constant<size_t, I>);
```

按位置取得一个类型标记，供编译期类型推导使用。

序列中的每一项都有一个上述重载，其中 `I` 是该项从零开始的位置，`T` 是该位置上的类型。

## 参数

|  |  |
| --- | --- |
| 位置标记实参 | `std::integral_constant<size_t, I>`，其中 `I` 为有效位置 |

## 返回值

该位置类型的 `std::type_identity`。

## 示例

```cpp
#include <print>

#include <givm/utils/type_list.hpp>

int main()
{
    using values = givm::type_list<int, double>;
    using selected = decltype(values::get(std::integral_constant<std::size_t, 0>{}))::type;
    selected value = 7;
    std::println("所选类型的值: {}", value);
}
```

输出

```text
所选类型的值: 7
```
