[givm](../../reference.md) / [定义](../definition.md) / **views_of_definition**

# givm::views_of_definition

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TDefinition>
struct views_of_definition : type_list<TDefinition> {};

template<>
struct views_of_definition<card_definition>
    : type_list<hand_card_view, deck_card_view> {};

template<>
struct views_of_definition<status_definition>
    : type_list<hand_card_status_view, deck_card_status_view> {};
```

一类定义可以采用的实体形态。卡牌和卡牌状态分别覆盖手牌与牌堆中的形态；其他定义类别直接对应自身的只读 view。这决定了定义源可以为哪些实体形态编写事件响应。

## 模板参数

|  |  |
| --- | --- |
| `TDefinition` | 定义类别 |

## 注意

可用操作继承自 [`type_list`](../utils/type_list.md)。

## 示例

```cpp
#include <concepts>
#include <print>

#include <givm/givm.hpp>

int main()
{
    using card_views = givm::views_of_definition<givm::card_definition>;
    std::println("卡牌实体形态数量: {}", card_views::size());
    card_views::each([]<class TView>
    {
        std::println("可在手牌形态响应: {}", std::same_as<TView, givm::hand_card_view>);
    });
}
```

输出

```text
卡牌实体形态数量: 2
可在手牌形态响应: true
可在手牌形态响应: false
```
