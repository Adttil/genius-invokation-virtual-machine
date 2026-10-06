[givm](../../reference.md) / [牌桌](../table.md) / **reaction_id**

# givm::reaction_id

定义于头文件 `<givm/table.hpp>`。

反应映射中一个槽位的身份，由映射所属玩家及基础反应槽位组成。它不表示某一次反应，也不随角色死亡而失效。

```cpp
struct reaction_id
{
    player_id player_id{};
    elemental_reaction slot = elemental_reaction::none;

    constexpr explicit operator bool() const noexcept;
    friend constexpr bool operator==(reaction_id, reaction_id) = default;
};
```

`operator bool` 只检查 slot 是否为 none；不检查玩家范围或定义是否已经装载。访问牌桌时，玩家必须有效，槽位必须属于该玩家的反应映射。

通过 `table[id]` 取得 [`reaction_view`](reaction_view.md)，读取该玩家此槽位的实际反应定义。反应发生在玩家 P 的角色上时，映射所属玩家是 P 的对手；引发来源由反应事件另外保留。
