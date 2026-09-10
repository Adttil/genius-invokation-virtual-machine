[givm](../../reference.md) / [定义](../definition.md) / **tag_id**

# givm::tag_id

定义于头文件 `<givm/definition.hpp>`

```cpp
using tag_id = issued_id<std::string_view>;
```

标识定义的一个分类标签，例如卡牌的用途或角色的元素分类。

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
| [`issued_id`](issued_id.md) | ID 的共同操作 |
