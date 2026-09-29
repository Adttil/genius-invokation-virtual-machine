[givm](../../../reference.md) / [执行](../../executor.md) / [definition_compile_context](../definition_compile_context.md) / **add_program**

# givm::definition_compile_context::add_program

定义于头文件 `<givm/executor.hpp>`

```cpp
program_entry add_program(std::span<const any_command> commands); // (1)

template<class TCommands>
    requires /* 命令序列，且不能隐式转换为 span<const any_command> */
program_entry add_program(TCommands&& commands); // (2)

template<class... TCommands>
    requires (std::constructible_from<any_command, TCommands> && ...)
program_entry add_program(TCommands&&... commands); // (3)
```

登记响应事件时需要依次执行的一段效果，并取得可在以后响应时提交的入口。

(1) 接收 [`any_command`](../../definition/any_command.md) 的连续序列。(2) 接收 tuple-like 对象或范围。(3) 依次接收零个或多个命令。

(2) 要求 `TCommands` 为 tuple-like 对象或输入范围，且不能隐式转换为 `span<const any_command>`；可转换的序列直接使用 (1)。

## 模板参数

|  |  |
| --- | --- |
| `TCommands` | (2) 中为 tuple-like 对象或可遍历范围；(3) 中为各命令的类型，每项须能构造 `any_command` |

## 参数

|  |  |
| --- | --- |
| `commands` | 按顺序执行的[核心命令](../../definition/commands.md)，也可用 [`any_command`](../../definition/any_command.md) 保存 |

## 返回值

返回登记效果的 [`program_entry`](../../definition/program_entry.md)。命令参数验证失败时，将诊断加入本次 [`compile`](../compile.md) 的错误列表；诊断包含当前源、响应程序编号和命令下标。只有整库编译成功后，返回入口才可用于执行。

## 注意

返回入口只用于本次编译产生的定义库。效果正常完成后回到发起它的结算；若执行期间结束对局，则不再返回原结算。本函数只登记效果，不立即执行。每个动态命令需要一个专用的 [命令输入](../../definition/command_inputs.md) 对象，固定模式不占输入位置。编译后，该入口要求的输入对象数量、类型和顺序固定，对象中的数组长度可以在响应时决定；响应提交入口时须同时提供匹配输入。所登记效果沿用最终 `compile` 调用选择的编译模式。

命令及其中借用的数据须在本次调用期间保持有效。返回前完成编译，不保留传入序列或命令对象；调用后可以销毁它们。

## 示例

```cpp
#include <utility>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct support_source
{
    using definition_category = givm::support_view;

    std::string_view name() const { return "洗牌助手"; }

    givm::program_entry compile(givm::definition_compile_context& context) const
    {
        auto entry = context.add_program(
            std::tuple{ givm::shuffle_deck{ .player = givm::player_id{ 0 } } }
        );
        std::println("已登记回合结束效果: {}", static_cast<bool>(entry));
        return entry;
    }

    static givm::program_entry handle(
        const givm::program_entry& entry,
        const givm::support_view&,
        givm::round_ended&,
        givm::handle_context& context)
    {
        return context.invoke(entry);
    }
};

int main()
{
    const support_source source{};
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    givm::definition_source_library sources{};
    if(not sources.add(source)) return 1;
    auto library_result = compile(
        sources, basics,
        std::tuple{}, std::tuple{ givm::start_round{} }, givm::compile_mode::normal
    );
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
}
```

输出

```text
已登记回合结束效果: true
```
