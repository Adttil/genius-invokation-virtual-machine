[givm](../../../reference.md) / [牌桌](../../table.md) / [table](../table.md) / **operator[]**

# givm::table::operator[]

定义于头文件 `<givm/table.hpp>`

```cpp
template<class T>
decltype(auto) operator[](history_value_key<T> key) const noexcept;

constexpr auto operator[](player_id player_id) const;

constexpr auto operator[](reaction_id reaction_id) const;

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

取得 ID 指定的玩家或实体，或通过历史字段键读取牌桌中保存的对局记录。

## 参数

|  |  |
| --- | --- |
| 各实体 ID | 属于这张牌桌的相应类型 ID；玩家 ID 的 index 为 0 或 1 |
| `key` | 由配套定义库或编译上下文取得的有效 [`history_value_key<T>`](../../definition/history_summary.md#类)，指定字段类型及位置；数组字段使用 `T[]` |

## 返回值

| 参数 | 返回值 |
| --- | --- |
| 实体 ID | 与 ID 对应的只读实体视图 |
| `history_value_key<T>`，`T` 为标量类型 | `const T&`，引用该历史字段 |
| `history_value_key<T[]>` | `std::span<const T>`，访问该历史数组的全部元素 |

牌桌本身的 const 限定不改变返回类型。

## 注意

ID 必须仍能定位其所属实体；本函数不检查越界或失效的 ID。实体删除后、清理前，原有 ID 仍可用于取得视图并读取保留的信息；转移后的旧区域 ID 不享有此保证。实体访问对象的存活条件和 ID 的保存期限见[实体的身份与访问](../entity_access.md)。

历史字段需在 [`executor::start`](../../executor/executor/start.md) 初始化历史存储后读取。标量引用和数组 span 借用牌桌的历史存储；该存储被销毁或重新初始化后不能继续使用原有引用和 span。字段键的取得与历史摘要的更新方式见[历史摘要](../../definition/history_summary.md#更新与读取)。


## 示例

```cpp
#include <print>

#include <givm/table.hpp>

int main()
{
    givm::table table{};
    const givm::player_view player = table[givm::player_id{ 0 }];
    std::println("玩家 {} 的骰子数: {}", player.id().index(), player.state().dice.total());
}
```

输出

```text
玩家 0 的骰子数: 0
```
