[givm](../../../reference.md) / [定义](../../definition.md) / [issued_id_map](../issued_id_map.md) / **get_tag_id**

# givm::issued_id_map::get_tag_id

定义于头文件 `<givm/definition.hpp>`

```cpp
tag_id get_tag_id(std::string_view name) const;
```

取得一个分类标签的 ID。

## 参数

|  |  |
| --- | --- |
| `name` | 已经登记的标签名称 |

## 返回值

名称对应的标签 ID。

## 注意

名称必须存在；不确定时先使用 [`has_tag`](has_tag.md) 检查。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{ "治疗" };
    const auto tag = ids.get_tag_id("治疗");
    std::println("取得的标签: {}", ids.tag_name(tag));
}
```

输出

```text
取得的标签: 治疗
```
