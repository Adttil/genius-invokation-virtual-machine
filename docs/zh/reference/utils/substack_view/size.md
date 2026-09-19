[givm](../../../reference.md) / [通用工具](../../utils.md) / [子栈视图](../substack_view.md) / **size**

# 子栈视图的 size

定义于头文件 `<givm/utils/stack.hpp>`

```cpp
constexpr size_t size() const noexcept;
```

取得子栈内部全部帧占用的字节数。返回值不是内部帧的数量，也不包含父帧自身占用的空间。

## 返回值

内部帧占用的字节数；空子栈返回零。

## 示例

```cpp
#include <print>

#include <givm/utils/stack.hpp>

int main()
{
    givm::frame_stack stack{};
    auto [child] = stack.push(givm::substack());
    std::println("初始字节数: {}", child.size());
    child.push(3);
    std::println("已有内容: {}", child.size() != 0);
    child.pop<int>();
    std::println("弹出后字节数: {}", child.size());
}
```

输出

```text
初始字节数: 0
已有内容: true
弹出后字节数: 0
```
