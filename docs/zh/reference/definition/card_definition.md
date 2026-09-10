[givm](../../reference.md) / [定义](../definition.md) / **card_definition**

# givm::card_definition

定义于头文件 `<givm/definition.hpp>`

```cpp
struct card_definition {};
```

卡牌定义的类别标记。同一张卡牌进入手牌或仍在牌堆时共享这一类定义，但可以针对所在区域提供不同的事件响应。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{};
    const auto id = ids.add<givm::card_definition>("恢复药剂", {});
    std::println("已取得卡牌定义: {}", id.is_valid());
}
```

输出

```text
已取得卡牌定义: true
```

## 参阅

|  |  |
| --- | --- |
| [`views_of_definition`](views_of_definition.md) | 定义对应的实体形态 |
