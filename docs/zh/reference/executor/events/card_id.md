[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **card_id**

# givm::card_id

定义于头文件 `<givm/executor.hpp>`

```cpp
using card_id = std::variant<hand_card_id, deck_card_id>;
```

手牌或牌堆中的牌标识。通过访问当前保存的标识种类，可以区分这张牌所在的区域。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::card_id card{ givm::deck_card_id{} };
    std::println("是否为牌堆牌: {}", std::holds_alternative<givm::deck_card_id>(card));
}
```

输出

```text
是否为牌堆牌: true
```
