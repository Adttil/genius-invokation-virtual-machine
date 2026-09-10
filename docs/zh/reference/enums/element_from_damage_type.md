[givm](../../reference.md) / [游戏用语](../enums.md) / **element_from_damage_type**

# givm::element_from_damage_type

定义于头文件 `<givm/enums/damage_type.hpp>`

```cpp
constexpr element element_from_damage_type(damage_type type) noexcept;
```

取得伤害所携带的元素。物理、穿透和真实伤害不携带元素。

## 参数

|  |  |
| --- | --- |
| `type` | 伤害类别 |

## 返回值

对应元素；无元素伤害返回 `element::none`。

## 示例

```cpp
#include <print>

#include <givm/enums/damage_type.hpp>

int main()
{
    const auto value = givm::element_from_damage_type(givm::damage_type::physical);
    std::println("物理伤害不带元素: {}", value == givm::element::none);
}
```

输出

```text
物理伤害不带元素: true
```
