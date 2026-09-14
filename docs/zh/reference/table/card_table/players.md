[givm](../../../reference.md) / [牌桌](../../table.md) / [card_table](../card_table.md) / **players**

# givm::card_table::players

定义于头文件 `<givm/table.hpp>`

```cpp
template<class Self>
constexpr auto players(this Self& self);
```

遍历参加这场对局的双方玩家。

## 模板参数

|  |  |
| --- | --- |
| `Self` | 由牌桌对象推导并保留其 const 限定的类型 |

## 参数

|  |  |
| --- | --- |
| `self` | 要访问的牌桌左值 |

## 返回值

依次产生玩家 0 和玩家 1 的 [`player_entity`](../player_entity.md) 的范围。对只读牌桌取得的实体只允许读取。

## 注意

范围及其中的实体访问对象依赖牌桌的存活。

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
    for(const auto player : table.players())
    {
        std::println("玩家: {}", player.id().index);
    }
}
```

输出

```text
玩家: 0
玩家: 1
```
