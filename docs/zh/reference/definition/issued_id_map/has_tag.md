[givm](../../../reference.md) / [定义](../../definition.md) / [issued_id_map](../issued_id_map.md) / **has_tag**

# givm::issued_id_map::has_tag

定义于头文件 `<givm/definition.hpp>`

```cpp
bool has_tag(std::string_view name) const;
```

检查标签是否已登记。

## 参数

|  |  |
| --- | --- |
| `name` | 标签名称 |

## 返回值

已登记时返回 `true`，否则返回 `false`。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{ "治疗" };
    std::println("存在治疗标签: {}", ids.has_tag("治疗"));
    std::println("存在料理标签: {}", ids.has_tag("料理"));
}
```

输出

```text
存在治疗标签: true
存在料理标签: false
```
