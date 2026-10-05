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

单个帧 view，或按压入顺序排列的帧 view 元组。支持 `get`、`size` 和结构化绑定，具体类型不作保证。

## 注意

栈中须有类型和顺序相符的数据。只读子栈只能取得只读元素；子栈 view 对象本身是否为 `const` 不改变其访问权限。

内部帧可以包含尾部 `substack_t`。含子栈的访问形态按 [`frame_stack::top`](../frame_stack/top.md#子栈与-view-的有效期) 的规则重新定位；增删下一层帧还要求全部祖先帧分别位于各自栈顶。普通帧 view 不提供扩容后重新定位的保证，扩容后应重新取得。

外层增删兄弟帧会使旧的内层栈顶 view 失效；重新满足栈顶关系后仍须重新调用 `top`。

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
