[givm](../../reference.md) / [通用工具](../utils.md) / **frame**

# givm::frame

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
template<class... T>
inline constexpr frame_t<T...> frame{};
```

供多帧访问与弹出使用的帧描述常量。

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
    auto [first, second] = stack.top<givm::frame<int>, givm::frame<int>>();
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
