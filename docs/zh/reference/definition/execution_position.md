[givm](../../reference.md) / [定义](../definition.md) / **execution_position**

# givm::execution_position

定义于头文件 `<givm/definition.hpp>`

```cpp
using execution_position = std::size_t;
```

对局流程中当前将要执行的指令位置。调用方通常从执行器取得它，再通过配套定义库查看该位置上的操作。

## 注意

位置与 [`program_entry`](program_entry.md) 的用途不同：前者用于观察执行进度，后者用于选择要进入的效果。不要从入口推测执行位置，也不要跨定义库使用位置。

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
    givm::executor execution{};
    execution.enter_entry(library);
    const givm::execution_position position = execution.position();
    std::println("接下来开始回合: {}", library.instruction(position).is<givm::start_round>());
}
```

输出

```text
接下来开始回合: true
```
