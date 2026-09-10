[givm](../../../reference.md) / [通用工具](../../utils.md) / [type_list](../type_list.md) / **type_at**

# givm::type_list::type_at

定义于头文件 `<givm/utils/type_list.hpp>`

```cpp
template<size_t J>
using type_at = /* 第 J 个类型 */;
```

序列中指定位置的类型。

## 模板参数

|  |  |
| --- | --- |
| `J` | 从零开始的位置，须小于类型数量 |

## 示例

```cpp
#include <print>

#include <givm/utils/type_list.hpp>

int main()
{
    using selected = givm::type_list<int, double>::type_at<0>;
    selected value = 7;
    std::println("所选类型的值: {}", value);
}
```

输出

```text
所选类型的值: 7
```
