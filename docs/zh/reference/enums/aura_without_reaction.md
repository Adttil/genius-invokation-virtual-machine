[givm](../../reference.md) / [游戏用语](../enums.md) / **aura_without_reaction**

# givm::aura_without_reaction

定义于头文件 `<givm/enums/element_aura.hpp>`

```cpp
constexpr element_aura aura_without_reaction(element_aura current, element incoming) noexcept;
```

确定不发生元素反应时，新元素与已有附着形成的附着状态。

## 参数

|  |  |
| --- | --- |
| `current` | 已有附着 |
| `incoming` | 新施加的元素 |

## 返回值

冰附着遇草、草附着遇冰时，返回相应顺序的共存附着；其他情况返回新元素对应的附着。

## 示例

```cpp
#include <print>

#include <givm/enums/element_aura.hpp>

int main()
{
    const auto aura = givm::aura_without_reaction(givm::element_aura::cryo, givm::element::dendro);
    std::println("形成冰草共存: {}", aura == givm::element_aura::cryo_dendro);
}
```

输出

```text
形成冰草共存: true
```
