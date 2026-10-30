# givm::perspective_player_view::operator[]

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr auto operator[](hand_card_id card) const;
```

按ID读取手牌，返回类型遵循 `hand_cards()` 的规则。ID必须属于本玩家；Debug断言该前提，Release不检查。
