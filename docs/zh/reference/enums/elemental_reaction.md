[givm](../../reference.md) / [枚举值](../enums.md) / **elemental_reaction**

# givm::elemental_reaction

定义于头文件 `<givm/enums/elemental_reaction.hpp>`

```cpp
enum class elemental_reaction : std::uint8_t
{
    none,
    melt,
    vaporize,
    overloaded,
    superconduct,
    electro_charged,
    frozen,
    burning,
    bloom,
    quicken,
    swirl_cryo,
    swirl_hydro,
    swirl_pyro,
    swirl_electro,
    crystallize_cryo,
    crystallize_hydro,
    crystallize_pyro,
    crystallize_electro
};
```

元素组合触发的固定反应槽位，共 17 个；不是本局采用的具体反应定义。普通感电和月感电共用 `electro_charged` 槽位，实际效果由玩家的反应定义表决定。扩散和结晶按元素分别占用槽位，允许只替换其中一种。扩散和结晶分别要求风和岩为新施加的元素，二者本身不会留下元素附着。

## 枚举值

|  |  |
| --- | --- |
| `none` | 无反应 |
| `melt` | 融化 |
| `vaporize` | 蒸发 |
| `overloaded` | 超载 |
| `superconduct` | 超导 |
| `electro_charged` | 感电 |
| `frozen` | 冻结 |
| `swirl_cryo/hydro/pyro/electro` | 对应元素的扩散槽位 |
| `crystallize_cryo/hydro/pyro/electro` | 对应元素的结晶槽位 |
| `burning` | 燃烧 |
| `bloom` | 绽放 |
| `quicken` | 激化 |

## 示例

```cpp
#include <print>

#include <givm/enums/elemental_reaction.hpp>

int main()
{
    const auto melt = givm::reaction_between(givm::element::cryo, givm::element::pyro);
    const auto coexist = givm::reaction_between(givm::element::cryo, givm::element::dendro);
    std::println("冰与火的反应是融化: {}", melt == givm::elemental_reaction::melt);
    std::println("冰与草不发生反应: {}", coexist == givm::elemental_reaction::none);
}
```

输出

```text
冰与火的反应是融化: true
冰与草不发生反应: true
```

## 参阅

|  |  |
| --- | --- |
| [`reaction_between`](reaction_between.md) | 判断两个元素的反应 |
| [`reaction_from_aura`](reaction_from_aura.md) | 判断附着与新元素的反应 |
| [`aura_after_reaction`](aura_after_reaction.md) | 取得反应后的附着 |
