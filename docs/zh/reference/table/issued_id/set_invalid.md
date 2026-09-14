[givm](../../../reference.md) / [牌桌](../../table.md) / [issued_id](../issued_id.md) / **set_invalid**

# givm::issued_id::set_invalid

定义于头文件 `<givm/table.hpp>`

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
    auto selected = ids.get_tag_id("治疗");
    std::println("已取得标签 ID: {}", selected.is_valid());
    selected.set_invalid();
    std::println("清除后有效: {}", static_cast<bool>(selected));
}
```

输出

```text
已取得标签 ID: true
清除后有效: false
```
