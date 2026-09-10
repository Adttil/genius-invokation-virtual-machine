[givm](../../../reference.md) / [通用工具](../../utils.md) / [frame_stack](../frame_stack.md) / **top**

# givm::frame_stack::top

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
template<class... T, class Self>
constexpr auto top(this Self& self) noexcept;

template<frame_t First, frame_t... Rest, class Self>
constexpr auto top(this Self& self) noexcept;
```

访问栈顶部的一帧、该帧的固定类型后缀，或连续的多帧。

## 模板参数

|  |  |
| --- | --- |
| `T...` | 访问的一帧或固定后缀的类型序列 |
| `First`、`Rest...` | 多帧的 [`frame`](../frame.md) 描述，按栈中从较早压入到较晚压入的顺序排列 |
| `Self` | 栈对象类型，决定是否允许修改元素 |

## 参数

|  |  |
| --- | --- |
| `self` | 要访问的栈 |

## 返回值

类型参数形式返回一个 [`frame_view`](../frame_view.md)；多帧形式返回按给定顺序排列的 view 元组。

## 注意

栈中须有对应数据，类型、顺序和所读范围须与原数据相符。只读栈返回的 view 不能修改元素。取得 view 不会复制数据，也不会弹出数据。

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{};
    stack.push(3, 5);
    std::println("顶部后缀: {}", stack.top<int>().get<0>());
    stack.push(8);
    auto [earlier, later] = stack.top<givm::frame<int, int>, givm::frame<int>>();
    std::println("较早帧的首值: {}", earlier.get<0>());
    std::println("较晚帧的值: {}", later.get<0>());
}
```

输出

```text
顶部后缀: 5
较早帧的首值: 3
较晚帧的值: 8
```
