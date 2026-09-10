[givm](../../reference.md) / [牌桌](../table.md) / **player_id**

# givm::player_id

定义于头文件 `<givm/table/entity_id.hpp>`

```cpp
struct player_id;
```

玩家在一张牌桌中的身份。使用此 ID 可以通过 [`card_table::operator[]`](card_table/operator_subscript.md) 再次取得相应实体。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `index` | `size_t` | 玩家编号，0 或 1 |

## 非成员函数

```cpp
friend constexpr bool operator==(player_id, player_id) = default;
```

比较各成员是否相等；比较不检查实体是否尚未移除。

## 注意

只接受 0 和 1 作为玩家编号。

## 示例

```cpp
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{ library };
    const givm::player_id id{ 1 };
    std::println("目标玩家: {}", table[id].id().index);
}
```

输出

```text
目标玩家: 1
```
