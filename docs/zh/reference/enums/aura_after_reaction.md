[givm](../../reference.md) / [游戏用语](../enums.md) / **aura_after_reaction**

# givm::aura_after_reaction

定义于头文件 `<givm/enums/elemental_reaction.hpp>`

```cpp
constexpr element_aura aura_after_reaction(element_aura aura, element incoming, elemental_reaction reaction) noexcept;
```

取得一次元素反应结算后保留的附着。

## 参数

|  |  |
| --- | --- |
| `aura` | 反应前的附着 |
| `incoming` | 新施加的元素 |
| `reaction` | 本次反应 |

## 返回值

`element_aura::none`，即清除附着。

## 示例

```cpp
#include <print>

#include <givm/enums/elemental_reaction.hpp>

int main()
{
    const auto aura = givm::aura_after_reaction(givm::element_aura::cryo, givm::element::pyro, givm::elemental_reaction::melt);
    std::println("融化后无附着: {}", aura == givm::element_aura::none);
}
```

输出

```text
融化后无附着: true
```
