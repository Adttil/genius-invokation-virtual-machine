[givm](../../../reference.md) / [通用工具](../../utils.md) / [frame_stack](../frame_stack.md) / **swap**

# givm::swap

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
constexpr void swap(frame_stack& left, frame_stack& right) noexcept;
```

交换两个栈的数据和容量。

## 参数

|  |  |
| --- | --- |
| `left` | 第一个栈 |
| `right` | 第二个栈 |

## 返回值

（无）

## 注意

交换后，既有 view 和引用仍指向原数据，但该数据归另一栈所有。

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack left{};
    givm::frame_stack right{};
    left.push(3);
    right.push(8);
    swap(left, right);
    std::println("交换后左栈: {}", left.top<int>().get<0>());
    std::println("交换后右栈: {}", right.top<int>().get<0>());
}
```

输出

```text
交换后左栈: 8
交换后右栈: 3
```
