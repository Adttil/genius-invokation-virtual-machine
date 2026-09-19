[givm](../../../reference.md) / [通用工具](../../utils.md) / [frame_stack](../frame_stack.md) / **push**

# givm::frame_stack::push

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
template<class... T>
constexpr auto push();

template<class... T>
    requires /* 动态数组在前，固定元素居中，可选子栈在最后 */
constexpr auto push(const T&... t);
```

在栈顶加入一帧，随后可以通过返回的 view 读取或填写其中的值。

## 模板参数

|  |  |
| --- | --- |
| `T...` | 非空的元素类型序列，或用于初始化各元素的实参类型；值须可平凡复制且满足栈的对齐要求 |

## 参数

|  |  |
| --- | --- |
| `t...` | 各元素的初值；动态数组通过 `dynamic_array<T>` 指定，尾部子栈通过 `substack()` 指定 |

## 返回值

新帧的可写帧 view，支持 [`get`](../get.md) 和结构化绑定；具体 view 类型不作保证。无参数重载不初始化固定元素，但会把尾部 `substack_t` 元素初始化为空子栈；带参数重载复制给定初值，并把 `substack()` 对应的元素初始化为空子栈。

## 注意

无参数重载可以创建固定元素和尾部子栈，例如 `push<int, substack_t>()`，但不能创建动态数组。未初始化的固定元素和按数量创建的动态数组元素都应先写入再读取。

动态数组必须位于所有固定元素之前；子栈最多一个，必须位于所有其他元素之后。可以没有动态数组或固定元素。

初值及初始化范围须在整个调用期间有效，不能依赖会被本次扩容搬迁的栈内引用或范围。需要复制已有栈数据时，应先在栈外取得独立副本，再用该副本初始化新帧。

空间不足时栈会增长。新帧包含子栈时，返回的 view 支持在该子栈增长后重新取得元素，规则与 [`top`](top.md) 相同；已经取得的引用或 span 仍可能失效。普通帧 view 不提供这项重新定位保证。

## 示例

```cpp
#include <print>
#include <array>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{};
    stack.push<int>().get<0>() = 6;
    std::println("填写的值: {}", stack.top<int>().get<0>());
    const std::array values{ 2, 4, 6 };
    auto [array, count] = stack.push(givm::dynamic_array<int>(values), 3);
    std::println("数组元素数: {}", array.size());
    std::println("最后一个元素: {}", array.back());
    std::println("随帧保存的数量: {}", count);
}
```

输出

```text
填写的值: 6
数组元素数: 3
最后一个元素: 6
随帧保存的数量: 3
```

## 参阅

|  |  |
| --- | --- |
| [`dynamic_array`](../dynamic_array.md) | 动态数组初始化参数的构造工具，是一个可调用常量 |
| [`substack`](../substack.md) | 空子栈的初始化参数 |
| [`top`](top.md) | 访问已有数据 |
