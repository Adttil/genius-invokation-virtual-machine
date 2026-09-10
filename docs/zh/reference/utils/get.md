[givm](../../reference.md) / [通用工具](../utils.md) / **get**

# givm::get

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
template<size_t I>
constexpr decltype(auto) get(frame_view<IsMutable, T...> self) noexcept;
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

固定元素的引用，或动态数组的 `std::span`；可修改性由所属 [`frame_view`](frame_view.md) 的 `IsMutable` 决定。

## 注意

每个 [`frame_view<IsMutable, T...>`](frame_view.md) 实例提供上述友元函数模板，通过实参相关查找调用。声明中的 `IsMutable` 和 `T...` 来自该 view 类型，不是调用者需要指定的函数模板参数。

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
