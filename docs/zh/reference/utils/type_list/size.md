[givm](../../../reference.md) / [通用工具](../../utils.md) / [type_list](../type_list.md) / **size**

# givm::type_list::size

定义于头文件 `<givm/utils/type_list.hpp>`

```cpp
static consteval size_t size() noexcept;
```

取得类型序列中的类型数量。

## 返回值

类型数量；空序列为零。

## 示例

```cpp
#include <print>

#include <givm/utils/type_list.hpp>

int main()
{
    std::println("类型数量: {}", givm::type_list<int, double>::size());
    std::println("空序列数量: {}", givm::type_list<>::size());
}
```

输出

```text
类型数量: 2
空序列数量: 0
```
