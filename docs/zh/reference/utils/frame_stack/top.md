[givm](../../../reference.md) / [通用工具](../../utils.md) / [frame_stack](../frame_stack.md) / **top**

# givm::frame_stack::top

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
template<class... T, class Self>
constexpr auto top(this Self& self) noexcept;

template<frame_t First, frame_t... Rest, class Self>
constexpr auto top(this Self& self) noexcept;
```

访问栈顶部的一帧、该帧的类型后缀，或连续的多帧。后缀可以包含固定元素以及尾部子栈。

## 模板参数

|  |  |
| --- | --- |
| `T...` | 访问的一帧或后缀的类型序列；子栈用 `substack_t` 表示 |
| `First`、`Rest...` | 多帧的 [`frame`](../frame.md) 描述，按栈中从较早压入到较晚压入的顺序排列 |
| `Self` | 栈对象类型，决定是否允许修改元素 |

## 参数

|  |  |
| --- | --- |
| `self` | 要访问的栈 |

## 返回值

类型参数形式返回一个帧 view；多帧形式返回按给定顺序排列的 view 元组。各 view 支持 [`get`](../get.md)、`size` 和结构化绑定，具体类型不作保证。

## 注意

栈中须有对应数据，类型、顺序和所读范围须与原数据相符。只读栈返回的 view 不能修改元素。取得 view 不会复制数据，也不会弹出数据。

包含动态数组时，必须描述整个帧。帧有尾部子栈时，后缀访问也必须包含 `substack_t`，不能自动跳过子栈读取前面的固定元素。

### 子栈与 view 的有效期

| 访问方式 | 子栈变化后访问帧元素 | 增删子栈中的帧 |
| --- | --- | --- |
| 单帧或后缀包含子栈 | 可继续通过该 view 重新取得元素 | 可写栈允许 |
| 多帧访问，最后一帧包含子栈 | 所有返回的 view 都可继续重新取得元素 | 只有最后一帧的子栈允许 |
| 其他情况 | 沿用普通 view 的失效规则 | 不允许修改非栈顶帧的子栈长度 |

上述保证以对应帧仍存在且仍满足栈顶关系为前提。后续压入其他父帧、弹出父帧，或移动、赋值、交换所属栈之后，应重新取得 view。栈顶条件限制子栈内部帧的增删，不限制通过较早帧的 view 读取已有内容。

不能增删子帧不影响按 const 属性修改已有内容。已由 `get` 或结构化绑定取得的引用、指针和 span 仍可能因扩容失效；需要在修改子栈后再次通过帧 view 取得。

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
