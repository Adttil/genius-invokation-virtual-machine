[givm](../../reference.md) / [牌桌](../table.md) / **tag_id**

# givm::tag_id

定义于头文件 `<givm/table.hpp>`

```cpp
using tag_id = issued_id<std::string_view>;
```

一个分类标签的身份标识，例如卡牌的用途或角色的元素分类。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{ "治疗" };
    const givm::tag_id tag = ids.get_tag_id("治疗");
    std::println("标签名称: {}", ids.tag_name(tag));
}
```

输出

```text
标签名称: 治疗
```

## 参阅

|  |  |
| --- | --- |
| [`issued_id`](issued_id.md) | 按类别区分的已发行 ID |
