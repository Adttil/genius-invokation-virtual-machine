[givm](../../../reference.md) / [牌桌](../../table.md) / [table](../table.md) / **operator[]**

# givm::table::operator[]

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr auto operator[](player_id player_id) const;

constexpr auto operator[](support_id support_id) const;

constexpr auto operator[](summon_id summon_id) const;

constexpr auto operator[](combat_status_id combat_status_id) const;

constexpr auto operator[](hand_card_id hand_card_id) const;

constexpr auto operator[](deck_card_id deck_card_id) const;

constexpr auto operator[](hand_card_status_id status_id) const;

constexpr auto operator[](deck_card_status_id status_id) const;

constexpr auto operator[](character_id character_id) const;

constexpr auto operator[](skill_id skill_id) const;

constexpr auto operator[](attachment_id attachment_id) const;
```

取得 ID 指定的玩家或场上实体。

## 参数

|  |  |
| --- | --- |
| 各实体 ID | 属于这张牌桌的相应类型 ID；玩家 ID 的 index 为 0 或 1 |

## 返回值

与 ID 对应的只读实体视图；牌桌本身的 const 限定不改变返回类型。

## 注意

ID 必须仍能定位其所属实体；本函数不检查越界或失效的 ID。实体访问对象的有效性和 ID 的保存期限见[实体的身份与访问](../entity_access.md)。


## 示例

```cpp
#include <print>

#include <givm/table.hpp>

int main()
{
    givm::table table{};
    const givm::player_view player = table[givm::player_id{ 0 }];
    std::println("玩家 {} 的骰子数: {}", player.id().index, player.state().dice.total());
}
```

输出

```text
玩家 0 的骰子数: 0
```
