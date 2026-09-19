[givm](../../../reference.md) / [通用工具](../../utils.md) / [frame_stack](../frame_stack.md) / **pop**

# givm::frame_stack::pop

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
template<class... T>
constexpr void pop() noexcept;

template<frame_t First, frame_t... Rest>
constexpr void pop() noexcept;

template<class FirstView, class... RestViews>
    requires /* 实参为帧 view */
constexpr void pop(FirstView first_frame, RestViews... rest_frames) noexcept;
```

移除栈顶的一帧或连续多帧。也可以指定已有 view，移除该帧及其后压入的数据。

## 模板参数

|  |  |
| --- | --- |
| `T...` | 要移除的整帧类型序列 |
| `First`、`Rest...` | 要移除的连续帧，按压入顺序排列 |
| `FirstView`、`RestViews...` | 帧 view 实参的类型 |

## 参数

|  |  |
| --- | --- |
| `first_frame` | 本栈中仍有效的、最早要移除的整帧 view |
| `rest_frames...` | 其他帧的 view；移除范围由 `first_frame` 确定 |

## 返回值

（无）

## 注意

类型或 view 必须对应本栈中仍存在的整帧，不可把仅用于观察固定后缀的 view 当作整帧弹出。移除后不得继续访问被移除的数据。

整帧包含子栈时，使用 `substack_t` 描述该元素。弹出父帧会一并移除整个子栈及其内容，不必先逐个弹出内部帧。仅弹出子栈内部的帧，应使用[子栈视图的 `pop`](../substack_view/pop.md)。

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{};
    stack.reserve(128);
    stack.push(1);
    auto checkpoint = stack.push(2);
    stack.push(3);
    stack.pop(checkpoint);
    std::println("保留的值: {}", stack.top<int>().get<0>());
    stack.pop<int>();
    std::println("全部移除后为空: {}", stack.empty());
}
```

输出

```text
保留的值: 1
全部移除后为空: true
```
