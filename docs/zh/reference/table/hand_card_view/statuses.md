[givm](../../../reference.md) / [牌桌](../../table.md) / [hand_card_view](../hand_card_view.md) / **statuses**

# givm::hand_card_view::statuses

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr auto statuses() const;
```

遍历附着在这张卡牌上的状态，按添加顺序取得尚未移除的状态。

## 返回值

产生 [`hand_card_status_view`](../hand_card_status_view.md) 的范围。

## 注意

卡牌须曾存在于此处，且未转移、未清理；卡牌删除后仍可取得范围，范围只包含尚未移除的状态。移除或增加状态后应重新取得范围；访问对象的存活条件见[实体的身份与访问](../entity_access.md)。
