[givm](../../../reference.md) / [枚举值](../../enums.md) / [dice_counts](../dice_counts.md) / **operator[]**

# givm::dice_counts::operator[]

定义于头文件 `<givm/enums/elemental_dice.hpp>`

```cpp
template<class Self>
constexpr auto& operator[](this Self& self, elemental_dice dice) noexcept;
```

读取或修改指定种类的骰子数量。

## 模板参数

|  |  |
| --- | --- |
| `Self` | 调用对象类型，保留其 const 限定 |

## 参数

|  |  |
| --- | --- |
| `self` | 要访问的骰子集合 |
| `dice` | 骰子种类 |

## 返回值

该种骰子数量的 `std::uint8_t` 引用，保留 `self` 的 const 限定。

## 示例

```cpp
#include <print>

#include <givm/enums/elemental_dice.hpp>

int main()
{
    givm::dice_counts dice{};
    dice[givm::elemental_dice::pyro] = 3;
    dice[givm::elemental_dice::omni] = 1;
    std::println("火骰数量: {}", dice[givm::elemental_dice::pyro]);
}
```

输出

```text
火骰数量: 3
```
