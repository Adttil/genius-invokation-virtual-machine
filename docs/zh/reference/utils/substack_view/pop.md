[givm](../../../reference.md) / [通用工具](../../utils.md) / [子栈视图](../substack_view.md) / **pop**

# 子栈视图的 pop

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
template<class... T>
constexpr void pop() const noexcept;

template<frame_t First, frame_t... Rest>
constexpr void pop() const noexcept;

template<class FirstView, class... RestViews>
    requires /* 实参为帧 view */
constexpr void pop(FirstView first_frame, RestViews... rest_frames) const noexcept;
```

移除子栈顶部的一帧或连续多帧。参数规则与 [`frame_stack::pop`](../frame_stack/pop.md) 相同，类型或 view 必须描述完整帧。

## 返回值

（无）

## 注意

仅允许可写且具备增删权限的子栈调用，其所属帧须位于 `frame_stack` 栈顶。

弹出最后一个内部帧会得到空子栈，不会移除父帧中的子栈元素。

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{};
    auto [child] = stack.push(givm::substack());
    child.push(2);
    child.push(4);
    child.pop<int>();
    std::println("保留的子帧: {}", child.top<int>().get<0>());
    child.pop<int>();
    child.push(6);
    std::println("重新压入: {}", child.top<int>().get<0>());
}
```

输出

```text
保留的子帧: 2
重新压入: 6
```
