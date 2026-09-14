[givm](../../../reference.md) / [牌桌](../../table.md) / [card_table](../card_table.md) / **parameters**

# givm::card_table::parameters

定义于头文件 `<givm/table.hpp>`

```cpp
template<class Self>
constexpr auto& parameters(this Self& self) noexcept;
```

访问这场对局的容量限制等参数。

## 模板参数

|  |  |
| --- | --- |
| `Self` | 由牌桌对象推导并保留其 const 限定的类型 |

## 参数

|  |  |
| --- | --- |
| `self` | 要访问的牌桌左值 |

## 返回值

牌桌中的 [`game_parameters`](../game_parameters.md) 引用；只读牌桌返回只读引用。

## 示例

```cpp
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{};
    table.parameters().hand_limit = 12;
    std::println("手牌上限: {}", table.parameters().hand_limit);
}
```

输出

```text
手牌上限: 12
```
