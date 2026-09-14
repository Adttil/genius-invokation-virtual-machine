[givm](../../../reference.md) / [牌桌](../../table.md) / [issued_id](../issued_id.md) / **operator bool**

# givm::issued_id::operator bool

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr explicit operator bool() const noexcept;
```

检查是否已取得定义或标签的 ID。

## 返回值

持有 ID 时返回 `true`，否则返回 `false`。

## 注意

本函数不验证 ID 与某个定义库的对应关系。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{ "治疗" };
    auto selected = ids.get_tag_id("治疗");
    std::println("已选择标签: {}", static_cast<bool>(selected));
    selected.set_invalid();
    std::println("清除后已选择: {}", static_cast<bool>(selected));
}
```

输出

```text
已选择标签: true
清除后已选择: false
```
