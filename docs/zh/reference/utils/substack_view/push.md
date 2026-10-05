[givm](../../../reference.md) / [通用工具](../../utils.md) / [子栈视图](../substack_view.md) / **push**

# 子栈视图的 push

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
template<class... T>
constexpr auto push() const;

template<class... T>
    requires /* 动态数组在前，固定元素居中，至多一个子栈且位于最后 */
constexpr auto push(const T&... t) const;
```

在子栈顶部加入一帧。固定元素、动态数组和尾部子栈的初始化规则与 [`frame_stack::push`](../frame_stack/push.md) 相同。

## 返回值

新增帧的可写 view，支持 `get`、`size` 和结构化绑定，具体类型不作保证。

无参数重载只预留固定元素的空间，不初始化固定元素；尾部子栈初始化为空。动态数组必须使用带初始化参数的重载创建。

## 注意

仅允许可写且具备增删权限的子栈调用，其所属帧及全部祖先帧须分别位于各自栈顶。扩容可能使之前取得的普通帧 view、字段引用、指针和 span 失效；可重新定位的 view 仍须满足[有效期规则](../substack_view.md#注意)。

初值及初始化范围须在整个调用期间有效，不能依赖会被本次扩容搬迁的栈内引用或范围。可以先在栈外取得独立副本，再用该副本初始化新帧。

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{};
    auto [child] = stack.push(givm::substack());
    auto frame = child.push(givm::dynamic_array<int>(2u), 4);
    auto [values, total] = frame;
    values[0] = 1;
    values[1] = 3;
    std::println("子帧数组之和: {}", values[0] + values[1]);
    std::println("子帧固定值: {}", total);
}
```

输出

```text
子帧数组之和: 4
子帧固定值: 4
```
