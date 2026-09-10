[givm](../../../reference.md) / [通用工具](../../utils.md) / [type_list](../type_list.md) / **each**

# givm::type_list::each

定义于头文件 `<givm/utils/type_list.hpp>`

```cpp
template<class F>
static constexpr void each(F&& fn);
```

按顺序对每种类型执行一次操作。

## 模板参数

|  |  |
| --- | --- |
| `F` | 可调用类型；须能通过左值 `fn.template operator()<T>()` 对序列中的每种类型 `T` 执行操作 |

## 参数

|  |  |
| --- | --- |
| `fn` | 对每种类型调用的操作 |

## 返回值

（无）

## 示例

```cpp
#include <print>

#include <givm/utils/type_list.hpp>

int main()
{
    int count = 0;
    givm::type_list<int, double, char>::each([&]<class T>() { ++count; });
    std::println("处理的类型数量: {}", count);
}
```

输出

```text
处理的类型数量: 3
```
