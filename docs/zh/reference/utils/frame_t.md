[givm](../../reference.md) / [通用工具](../utils.md) / **frame_t**

# givm::frame_t

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
template<class... T>
struct frame_t : type_list<T...> {};
```

一帧所包含的类型序列，可作为多帧访问操作的描述。

## 模板参数

|  |  |
| --- | --- |
| `T...` | 一帧的元素类型，按压入顺序排列 |

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{};
    stack.push(3);
    stack.push(5);
    auto [first, second] = stack.top<givm::frame_t<int>{}, givm::frame_t<int>{}>();
    std::println("两帧之和: {}", first.get<0>() + second.get<0>());
}
```

输出

```text
两帧之和: 8
```

## 参阅

|  |  |
| --- | --- |
| [`frame_stack::top`](frame_stack/top.md) | 访问连续多帧 |
| [`frame_stack::pop`](frame_stack/pop.md) | 移除连续多帧 |
