[givm](../../../reference.md) / [定义](../../definition.md) / [issued_id](../issued_id.md) / **set_invalid**

# givm::issued_id::set_invalid

定义于头文件 `<givm/definition.hpp>`

```cpp
constexpr void set_invalid() noexcept;
```

清除当前 ID，使对象重新表示尚未选择定义或标签。

## 返回值

（无）

## 注意

不删除定义库或 ID 映射中的项目，也不影响其他 ID 对象。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{ "治疗" };
    auto card = ids.add<givm::card_definition>("恢复药剂", { "治疗" });
    std::println("已取得卡牌定义 ID: {}", card.is_valid());
    card.set_invalid();
    std::println("清除后有效: {}", static_cast<bool>(card));
}
```

输出

```text
已取得卡牌定义 ID: true
清除后有效: false
```
