[givm](../../../reference.md) / [通用工具](../../utils.md) / [子栈视图](../substack_view.md) / **top**

# 子栈视图的 top

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
template<class... T>
constexpr auto top() const noexcept;

template<frame_t First, frame_t... Rest>
constexpr auto top() const noexcept;
```

访问子栈顶部的一帧、该帧的类型后缀或连续多帧。类型描述、后缀和多帧规则与 [`frame_stack::top`](../frame_stack/top.md) 相同。

## 返回值

单个普通 [`frame_view`](../frame_view.md)，或按压入顺序排列的普通帧 view 元组。支持 `get`、`size` 和结构化绑定。

## 注意

栈中须有类型和顺序相符的数据。只读子栈只能取得只读元素；子栈 view 对象本身是否为 `const` 不改变其访问权限。

内部帧只包含固定字段和动态数组，不能包含 `substack_t`。返回的普通帧 view 不提供扩容后重新定位的保证；若子栈后续压栈导致扩容，应重新取得 view。

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{};
    auto [child] = stack.push(givm::substack());
    child.push(2, 4);
    child.push(6);
    auto [first, second] = child.top<givm::frame<int, int>, givm::frame<int>>();
    std::println("较早子帧: {}", first.get<0>());
    std::println("顶部子帧: {}", second.get<0>());
    std::println("顶部后缀: {}", child.top<int>().get<0>());
}
```

输出

```text
较早子帧: 2
顶部子帧: 6
顶部后缀: 6
```
