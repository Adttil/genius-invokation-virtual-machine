[givm](../../reference.md) / [枚举值](../enums.md) / **primary_element_from_aura**

# givm::primary_element_from_aura

定义于头文件 `<givm/enums/element_aura.hpp>`

```cpp
constexpr element primary_element_from_aura(element_aura aura) noexcept;
```

取得附着中优先用于判断反应的元素。冰草共存时始终返回冰。

## 参数

|  |  |
| --- | --- |
| `aura` | 已有的元素附着 |

## 返回值

单元素附着对应的元素；冰草共存时返回 `element::cryo`；无附着时返回 `element::none`。

## 示例

```cpp
#include <print>

#include <givm/enums/element_aura.hpp>

int main()
{
    const auto element = givm::primary_element_from_aura(givm::element_aura::cryo_dendro);
    std::println("冰草优先冰: {}", element == givm::element::cryo);
}
```

输出

```text
冰草优先冰: true
```

## 参阅

|  |  |
| --- | --- |
| [`element_aura`](element_aura.md) | 角色身上保留的元素附着 |
| [`element`](element.md) | 元素种类 |
