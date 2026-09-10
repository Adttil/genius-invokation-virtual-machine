[givm](../../reference.md) / [通用工具](../utils.md) / **frame_view**

# givm::frame_view

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
template<bool IsMutable, class... T>
class frame_view;
```

对一组栈上数据的访问对象。可以用结构化绑定分别取得各个值，而无需复制整组数据。

## 模板参数

|  |  |
| --- | --- |
| `IsMutable` | 是否允许通过 view 修改元素 |
| `T...` | 按顺序访问的元素类型；动态数组用 `T[]` 表示并位于固定元素之前 |

## 成员类型

|  |  |
| --- | --- |
| `byte_type` | 字节类型；`IsMutable` 为 true 时是 `unsigned char`，否则是 `const unsigned char` |

## 成员函数

|  |  |
| --- | --- |
| [`size`](frame_view/size.md) | 取得元素个数 |
| [`get`](frame_view/get.md) | 访问指定元素 |

## 非成员函数

|  |  |
| --- | --- |
| [`get`](get.md) | 通过实参相关查找访问元素 |

## 注意

view 由 [`frame_stack`](frame_stack.md) 提供，不拥有数据。栈重新分配空间或相应数据被弹出、清空后，view 不可继续使用。view 对象本身是否为 `const` 不决定元素能否修改，权限由 `IsMutable` 决定。

支持 `std::tuple_size`、`std::tuple_element` 和结构化绑定。动态数组元素作为 `std::span` 访问。

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{};
    auto view = stack.push(3, 5);
    auto [left, right] = view;
    left += right;
    std::println("修改后的首值: {}", stack.top<int, int>().get<0>());
    std::println("元素个数: {}", view.size());
}
```

输出

```text
修改后的首值: 8
元素个数: 2
```
