[givm](../../reference.md) / [通用工具](../utils.md) / **get**

# get（帧 view）

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
template<size_t I>
constexpr decltype(auto) get(/* 帧 view */ self) noexcept;
```

访问这组数据中指定位置的元素。

## 模板参数

|  |  |
| --- | --- |
| `I` | 从零开始的位置，须小于 view 的元素个数 |

## 参数

|  |  |
| --- | --- |
| `self` | 要访问的帧 view |

## 返回值

固定元素的引用、动态数组的 `std::span`，或[子栈视图](substack_view.md)。元素的可修改性由所属栈的 const 属性决定；子栈能否增删内部帧还取决于 [`top`](frame_stack/top.md) 的访问方式。

## 注意

各类帧 view 均提供通过实参相关查找调用的 `get<I>`。调用者只需指定位置 `I`，无需命名具体 view 类型。

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{};
    auto view = stack.push(3, 5);
    get<1>(view) = 8;
    std::println("修改后的第二个值: {}", view.get<1>());
}
```

输出

```text
修改后的第二个值: 8
```
