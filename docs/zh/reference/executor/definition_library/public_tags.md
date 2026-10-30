# givm::definition_library::public_tags

定义于头文件 `<givm/runtime.hpp>`

```cpp
tag_mask_view public_tags(hidden_hand_card_view card) const noexcept;
template<definition_category Category>
tag_mask_view public_tags(definition_id<Category> id) const noexcept;
```

读取隐藏手牌或指定定义的公开标签前缀。返回 [`tag_mask_view`](../../definition/tag_mask_view.md)，不复制标签，也不向调用方提供隐藏牌的定义ID。ID及实体必须属于本定义库配套的牌桌。
