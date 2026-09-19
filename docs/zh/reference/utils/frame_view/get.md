[givm](../../../reference.md) / [通用工具](../../utils.md) / [frame_view](../frame_view.md) / **get**

# givm::frame_view::get

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
template<size_t I>
constexpr decltype(auto) get() const noexcept;
```

访问这组数据中指定位置的元素。

## 模板参数

|  |  |
| --- | --- |
| `I` | 从零开始的位置，须小于 view 的元素个数 |

## 返回值

固定元素的引用、动态数组的 `std::span`，或[子栈视图](../substack_view.md)。元素的可修改性由所属栈的 const 属性决定；子栈能否增删内部帧还取决于 [`top`](../frame_stack/top.md) 的访问方式。

取得的引用或 span 仍可能因存储扩容失效，即使帧 view 本身允许在子栈增长后继续使用，也需要重新取得这些引用或 span。

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{};
    auto view = stack.push(3, 5);
    view.get<1>() = 8;
    std::println("修改后的第二个值: {}", view.get<1>());
}
```

输出

```text
修改后的第二个值: 8
```
