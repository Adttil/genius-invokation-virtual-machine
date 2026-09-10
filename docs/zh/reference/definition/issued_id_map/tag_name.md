[givm](../../../reference.md) / [定义](../../definition.md) / [issued_id_map](../issued_id_map.md) / **tag_name**

# givm::issued_id_map::tag_name

定义于头文件 `<givm/definition.hpp>`

```cpp
std::string_view tag_name(tag_id id) const;
```

取得标签的名称，用于展示或与按名称编写的配置对应。

## 参数

|  |  |
| --- | --- |
| `id` | 由本映射发放的有效标签 ID |

## 返回值

对应的标签名称。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{ "治疗" };
    std::println("标签名称: {}", ids.tag_name(ids.get_tag_id("治疗")));
}
```

输出

```text
标签名称: 治疗
```
