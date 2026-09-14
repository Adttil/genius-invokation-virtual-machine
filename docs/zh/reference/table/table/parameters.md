[givm](../../../reference.md) / [牌桌](../../table.md) / [table](../table.md) / **parameters**

# givm::table::parameters

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr const game_parameters& parameters() const noexcept;
```

访问这场对局的容量限制等参数。

## 返回值

牌桌中的 [`game_parameters`](../game_parameters.md) 的只读引用。


## 示例

```cpp
#include <print>

#include <givm/table.hpp>

int main()
{
    givm::table table{ givm::game_parameters{ .hand_limit = 12 } };
    std::println("手牌上限: {}", table.parameters().hand_limit);
}
```

输出

```text
手牌上限: 12
```
