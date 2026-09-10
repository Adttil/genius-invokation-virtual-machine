[givm](../../../reference.md) / [牌桌](../../table.md) / [card_table](../card_table.md) / **operator[]**

# givm::card_table::operator[]

定义于头文件 `<givm/table.hpp>`

```cpp
template<class Self>
constexpr auto operator[](this Self& self, player_id player_id);

template<class Self>
constexpr auto operator[](this Self& self, support_id support_id);

template<class Self>
constexpr auto operator[](this Self& self, summon_id summon_id);

template<class Self>
constexpr auto operator[](this Self& self, combat_status_id combat_status_id);

template<class Self>
constexpr auto operator[](this Self& self, hand_card_id hand_card_id);

template<class Self>
constexpr auto operator[](this Self& self, deck_card_id deck_card_id);

template<class Self>
constexpr auto operator[](this Self& self, hand_card_status_id status_id);

template<class Self>
constexpr auto operator[](this Self& self, deck_card_status_id status_id);

template<class Self>
constexpr auto operator[](this Self& self, character_id character_id);

template<class Self>
constexpr auto operator[](this Self& self, skill_id skill_id);

template<class Self>
constexpr auto operator[](this Self& self, attachment_id attachment_id);
```

取得 ID 指定的玩家或场上实体。

## 模板参数

|  |  |
| --- | --- |
| `Self` | 由牌桌对象推导，保留其 const 限定 |

## 参数

|  |  |
| --- | --- |
| `self` | 要访问的牌桌左值 |
| 各实体 ID | 属于这张牌桌的相应类型 ID；玩家 ID 的 index 为 0 或 1 |

## 返回值

与 ID 对应的实体访问对象。只读牌桌返回对应的只读视图。

## 注意

ID 必须仍能定位其所属实体；本函数不检查越界或失效的 ID。实体访问对象的有效性和 ID 的保存期限见[实体的身份与访问](../entity_access.md)。

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
    const givm::player_id first{ 0 };
    table[first].state().dice[givm::elemental_dice::pyro] = 2;
    std::println("玩家 {} 的火骰: {}", first.index, table[first].state().dice[givm::elemental_dice::pyro]);
}
```

输出

```text
玩家 0 的火骰: 2
```
