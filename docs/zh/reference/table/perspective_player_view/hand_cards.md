# givm::perspective_player_view::hand_cards

定义于头文件 `<givm/table.hpp>`

```cpp
template<bool SkipErased = true>
constexpr auto hand_cards() const;
```

本人及观众取得普通 `hand_card_view`，对手取得 `hidden_hand_card_view`。默认跳过已移除的手牌，不复制集合。
