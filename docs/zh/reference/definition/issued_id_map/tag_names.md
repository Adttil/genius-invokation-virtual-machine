[givm](../../../reference.md) / [定义](../../definition.md) / [issued_id_map](../issued_id_map.md) / **tag_names**

# givm::issued_id_map::tag_names

定义于头文件 `<givm/definition.hpp>`

```cpp
std::span<const std::string_view> tag_names() const;
```

取得全部可用标签的名称。

## 返回值

查看标签名称的只读范围；范围在映射对象存续且未被赋值替换期间有效。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{ "治疗", "料理" };
    for(const auto name : ids.tag_names())
    {
        std::println("可用标签: {}", name);
    }
}
```

输出

```text
可用标签: 治疗
可用标签: 料理
```
