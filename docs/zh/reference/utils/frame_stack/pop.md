[givm](../../../reference.md) / [通用工具](../../utils.md) / [frame_stack](../frame_stack.md) / **pop**

# givm::frame_stack::pop

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
template<class... T>
constexpr void pop() noexcept;

template<frame_t First, frame_t... Rest>
constexpr void pop() noexcept;

template<bool IsMutable, class... T, class... Rest>
constexpr void pop(frame_view<IsMutable, T...> first_frame, Rest... rest_frames) noexcept;
```

移除栈顶的一帧或连续多帧。也可以指定已有 view，移除该帧及其后压入的数据。

## 模板参数

|  |  |
| --- | --- |
| `T...` | 要移除的整帧类型序列 |
| `First`、`Rest...` | 要移除的连续帧，按压入顺序排列 |
| `IsMutable` | view 是否允许修改元素 |
| `Rest...`（view 重载） | 其余 view 实参的类型 |

## 参数

|  |  |
| --- | --- |
| `first_frame` | 本栈中仍有效的、最早要移除的整帧 view |
| `rest_frames...` | 其他帧的 view；移除范围由 `first_frame` 确定 |

## 返回值

（无）

## 注意

类型或 view 必须对应本栈中仍存在的整帧，不可把仅用于观察固定后缀的 view 当作整帧弹出。移除后不得继续访问被移除的数据。

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
