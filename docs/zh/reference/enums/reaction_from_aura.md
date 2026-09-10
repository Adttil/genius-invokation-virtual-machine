[givm](../../reference.md) / [枚举值](../enums.md) / **reaction_from_aura**

# givm::reaction_from_aura

定义于头文件 `<givm/enums/elemental_reaction.hpp>`

```cpp
constexpr elemental_reaction reaction_from_aura(element_aura aura, element incoming) noexcept;
```

判断角色已有附着与新元素产生的反应。多元素共存时，由优先元素参与判断。

## 参数

|  |  |
| --- | --- |
| `aura` | 已有附着 |
| `incoming` | 新施加的元素 |

## 返回值

[`primary_element_from_aura`](primary_element_from_aura.md) 取得的元素与 `incoming` 之间的反应。

## 示例

```cpp
#include <print>

#include <givm/enums/elemental_reaction.hpp>

int main()
{
    const auto reaction = givm::reaction_from_aura(givm::element_aura::cryo_dendro, givm::element::pyro);
    std::println("冰在先时发生融化: {}", reaction == givm::elemental_reaction::melt);
}
```

输出

```text
冰在先时发生融化: true
```

## 参阅

|  |  |
| --- | --- |
| [`element_aura`](element_aura.md) | 角色身上保留的元素附着 |
| [`element`](element.md) | 元素种类 |
| [`elemental_reaction`](elemental_reaction.md) | 元素相遇时产生的反应种类 |
