[givm](../../reference.md) / [定义](../definition.md) / **instruction_type_index**

# givm::instruction_type_index

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TInstruction>
inline constexpr auto instruction_type_index = &detail::instruction_rtti_of<TInstruction>;
```

一类具体指令的类型标识，用来识别定义库中读取到的指令属于哪种操作。

## 模板参数

|  |  |
| --- | --- |
| `TInstruction` | 要标识的具体指令类型 |

## 注意

只将该值用于类型比较；不要解引用、持久保存或依赖它的数值。通常也可以直接使用指令视图的 `is<TInstruction>()`。

## 示例

```cpp
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, ids] = sources.compile(
        std::tuple{}, std::tuple{ givm::start_round{} }
    );
    const auto instruction = library.instruction(library.entry());
    std::println("是开始回合指令: {}", instruction.type_index() == givm::instruction_type_index<givm::start_round>);
}
```

输出

```text
是开始回合指令: true
```
