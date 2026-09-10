[givm](../../../reference.md) / [通用工具](../../utils.md) / [frame_view](../frame_view.md) / **size**

# givm::frame_view::size

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
static consteval size_t size() noexcept;
```

取得这组数据中元素的个数。动态数组作为一个元素计数。

## 返回值

模板参数 `T...` 的数量。

## 示例

```cpp
#include <print>
#include <array>

#include <givm/utils/stack.hpp>

int main()
{
    const std::array initial{ 2, 4, 6 };
    givm::frame_stack stack{};
    auto view = stack.push(givm::dynamic_array<int>(initial), 9);
    std::println("帧元素数: {}", view.size());
    std::println("数组内部元素数: {}", view.get<0>().size());
}
```

输出

```text
帧元素数: 2
数组内部元素数: 3
```
