[givm](../../reference.md) / [通用工具](../utils.md) / **type_list**

# givm::type_list

定义于头文件 `<givm/utils/type_list.hpp>`

```cpp
template<class... T>
struct type_list;
```

编译期的一组类型，用于把同一操作应用到多种定义、事件或其他类型。

## 模板参数

|  |  |
| --- | --- |
| `T...` | 按顺序列出的类型；可为空 |

## 成员类型

|  |  |
| --- | --- |
| [`type_at`](type_list/type_at.md) | 按位置取得类型 |
| [`apply`](type_list/apply.md) | 把类型序列应用到另一个模板 |

## 成员函数

|  |  |
| --- | --- |
| [`size`](type_list/size.md) | 取得类型数量 |
| [`index_of`](type_list/index_of.md) | 取得类型的位置 |
| [`get`](type_list/get.md) | 按位置取得类型标记 |
| [`all`](type_list/all.md) | 检查各类型是否均满足条件 |
| [`each`](type_list/each.md) | 对各类型执行操作 |

## 示例

```cpp
#include <print>
#include <tuple>

#include <givm/utils/type_list.hpp>

int main()
{
    using values = givm::type_list<int, double>;
    using tuple_type = values::apply<std::tuple>;
    tuple_type tuple{ 3, 2.5 };
    std::println("类型数量: {}", values::size());
    std::println("元组中的整数: {}", std::get<0>(tuple));
}
```

输出

```text
类型数量: 2
元组中的整数: 3
```
