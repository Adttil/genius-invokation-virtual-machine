[givm](../../../reference.md) / [定义](../../definition.md) / [definition_library](../definition_library.md) / **instruction**

# givm::definition_library::instruction

定义于头文件 `<givm/definition.hpp>`

```cpp
constexpr decltype(auto) instruction(execution_position position) const noexcept;
```

查看某个执行位置上的指令。调用方可以用它识别执行器当前停在什么操作上，或者读取该指令配置的玩家、目标等参数。

## 参数

|  |  |
| --- | --- |
| `position` | 属于本定义库的有效执行位置，通常由 `executor::position()` 取得 |

## 返回值

该指令的只读视图。使用 `is<TInstruction>()` 检查指令类型，确认类型后通过 `as<TInstruction>()` 取得指令的常量引用；`type_index()` 可与 [`instruction_type_index<TInstruction>`](../instruction_type_index.md) 比较。

## 注意

视图及其返回的引用不能比定义库存续更久。`as<TInstruction>()` 要求类型匹配；不得用无效或其他定义库的执行位置查询。

## 示例

```cpp
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::shuffle_deck{ .player = givm::player_id{ 1 } } },
        std::tuple{ givm::start_round{} }
    );
    givm::executor execution{};
    execution.enter_entry(library);
    const auto instruction = library.instruction(execution.position());
    if(instruction.is<givm::shuffle_deck>())
    {
        std::println("准备为玩家 1 洗牌: {}", instruction.as<givm::shuffle_deck>().player == givm::player_id{ 1 });
    }
}
```

输出

```text
准备为玩家 1 洗牌: true
```
