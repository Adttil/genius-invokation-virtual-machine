[givm](../../reference.md) / [枚举值](../enums.md) / **element_aura**

# givm::element_aura

定义于头文件 `<givm/enums/element_aura.hpp>`

```cpp
enum class element_aura : unsigned char
{
    none,
    cryo,
    hydro,
    pyro,
    electro,
    anemo,
    geo,
    dendro,
    cryo_dendro,
    dendro_cryo
};
```

角色身上保留的元素附着。冰与草共存时，还区分二者的先后次序。

## 枚举值

|  |  |
| --- | --- |
| `none` | 无附着 |
| `cryo` | 冰附着 |
| `hydro` | 水附着 |
| `pyro` | 火附着 |
| `electro` | 雷附着 |
| `anemo` | 保留值，正常附着流程不会产生风附着 |
| `geo` | 保留值，正常附着流程不会产生岩附着 |
| `dendro` | 草附着 |
| `cryo_dendro` | 冰在先、草在后的共存附着 |
| `dendro_cryo` | 草在先、冰在后的共存附着 |

## 示例

```cpp
#include <print>

#include <givm/enums/elemental_reaction.hpp>

int main()
{
    const auto aura = givm::aura_without_reaction(givm::element_aura::cryo, givm::element::dendro);
    std::println("加入草后形成冰草共存: {}", aura == givm::element_aura::cryo_dendro);
    const auto reaction = givm::reaction_from_aura(aura, givm::element::pyro);
    std::println("随后遇火优先发生融化: {}", reaction == givm::elemental_reaction::melt);
}
```

输出

```text
加入草后形成冰草共存: true
随后遇火优先发生融化: true
```

## 参阅

|  |  |
| --- | --- |
| [`aura_from_element`](aura_from_element.md) | 取得单元素附着 |
| [`primary_element_from_aura`](primary_element_from_aura.md) | 取得优先参与反应的附着元素 |
| [`aura_without_reaction`](aura_without_reaction.md) | 取得不发生反应时的附着 |
| [`reaction_from_aura`](reaction_from_aura.md) | 判断附着与新元素的反应 |
| [`aura_after_reaction`](aura_after_reaction.md) | 取得反应后的附着 |
