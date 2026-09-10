[givm](../../reference.md) / [枚举值](../enums.md) / **reaction_between**

# givm::reaction_between

定义于头文件 `<givm/enums/elemental_reaction.hpp>`

```cpp
constexpr elemental_reaction reaction_between(element aura, element incoming) noexcept;
```

判断两种元素相遇所产生的元素反应。

## 参数

|  |  |
| --- | --- |
| `aura` | 已有元素 |
| `incoming` | 新施加的元素 |

## 返回值

下表列出的反应。任一元素为 `none`、两者相同或没有对应反应时返回 `elemental_reaction::none`。

## 反应对应关系

| 元素组合（顺序可交换） | 反应 |
| --- | --- |
| 冰与火 | 融化 |
| 水与火 | 蒸发 |
| 火与雷 | 超载 |
| 冰与雷 | 超导 |
| 水与雷 | 感电 |
| 冰与水 | 冻结 |
| 火与草 | 燃烧 |
| 水与草 | 绽放 |
| 雷与草 | 激化 |
| 风与冰、水、火或雷 | 扩散 |
| 岩与冰、水、火或雷 | 结晶 |

## 示例

```cpp
#include <print>

#include <givm/enums/elemental_reaction.hpp>

int main()
{
    const auto reaction = givm::reaction_between(givm::element::cryo, givm::element::pyro);
    const auto reverse = givm::reaction_between(givm::element::pyro, givm::element::cryo);
    std::println("是否融化: {}", reaction == givm::elemental_reaction::melt);
    std::println("交换元素后反应相同: {}", reaction == reverse);
}
```

输出

```text
是否融化: true
交换元素后反应相同: true
```

## 参阅

|  |  |
| --- | --- |
| [`element`](element.md) | 元素种类 |
| [`elemental_reaction`](elemental_reaction.md) | 元素相遇时产生的反应种类 |
