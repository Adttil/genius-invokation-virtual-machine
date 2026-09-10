[givm](../../../reference.md) / [游戏用语](../../enums.md) / [elemental_dice_cost](../elemental_dice_cost.md) / **operator[]**

# givm::elemental_dice_cost::operator[]

定义于头文件 `<givm/enums/elemental_dice.hpp>`

```cpp
template<class Self>
constexpr auto& operator[](this Self& self, elemental_dice dice) noexcept;
```

读取或修改某种骰子的费用数量。

## 模板参数

|  |  |
| --- | --- |
| `Self` | 调用对象类型，保留其 const 限定 |

## 参数

|  |  |
| --- | --- |
| `self` | 要访问的费用 |
| `dice` | 骰子种类 |

## 返回值

对应数量的 `unsigned char` 引用，保留调用对象的 const 限定。

## 示例

```cpp
#include <print>

#include <givm/enums/elemental_dice.hpp>

int main()
{
    givm::elemental_dice_cost fire{ givm::elemental_dice::pyro, 3 };
    givm::elemental_dice_cost water{ givm::elemental_dice::hydro, 1 };
    fire[givm::elemental_dice::pyro] = 2;
    std::println("调整后的火骰费用: {}", fire[givm::elemental_dice::pyro]);
}
```

输出

```text
调整后的火骰费用: 2
```
