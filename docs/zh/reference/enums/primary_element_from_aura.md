[givm](../../reference.md) / [枚举值](../enums.md) / **primary_element_from_aura**

# givm::primary_element_from_aura

定义于头文件 `<givm/enums/element_aura.hpp>`

```cpp
constexpr element primary_element_from_aura(element_aura aura) noexcept;
```

取得附着中优先用于判断反应的元素。冰草共存时，取名称中排在前面的元素。

## 参数

|  |  |
| --- | --- |
| `aura` | 已有的元素附着 |

## 返回值

单元素附着对应的元素；冰草共存时的首个元素；无附着时返回 `element::none`。

## 示例

```cpp
#include <print>

#include <givm/enums/element_aura.hpp>

int main()
{
    const auto first = givm::primary_element_from_aura(givm::element_aura::cryo_dendro);
    const auto second = givm::primary_element_from_aura(givm::element_aura::dendro_cryo);
    std::println("冰草优先冰: {}", first == givm::element::cryo);
    std::println("草冰优先草: {}", second == givm::element::dendro);
}
```

输出

```text
冰草优先冰: true
草冰优先草: true
```

## 参阅

|  |  |
| --- | --- |
| [`element_aura`](element_aura.md) | 角色身上保留的元素附着 |
| [`element`](element.md) | 元素种类 |
