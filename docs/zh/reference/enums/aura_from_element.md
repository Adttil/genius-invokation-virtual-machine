[givm](../../reference.md) / [游戏用语](../enums.md) / **aura_from_element**

# givm::aura_from_element

定义于头文件 `<givm/enums/element_aura.hpp>`

```cpp
constexpr element_aura aura_from_element(element value) noexcept;
```

把一种元素表示为角色身上的单元素附着。

## 参数

|  |  |
| --- | --- |
| `value` | 要表示的元素 |

## 返回值

同名的单元素附着；`element::none` 对应 `element_aura::none`。

## 示例

```cpp
#include <print>

#include <givm/enums/element_aura.hpp>

int main()
{
    const auto aura = givm::aura_from_element(givm::element::hydro);
    std::println("是否水附着: {}", aura == givm::element_aura::hydro);
}
```

输出

```text
是否水附着: true
```
