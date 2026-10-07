[givm](../../../reference.md) / [执行](../../executor.md) / [definition_compile_context](../definition_compile_context.md) / **add_effect**

# givm::definition_compile_context::add_effect

定义于头文件 `<givm/definition_source.hpp>`

```cpp
template<event_category Category>
effect<Category> add_effect(std::span<const any_command> commands); // (1)

template<event_category Category, class TCommands>
    requires /* 命令序列，且不能隐式转换为 span<const any_command> */
effect<Category> add_effect(TCommands&& commands); // (2)

template<event_category Category, class... TCommands>
    requires (std::constructible_from<any_command, TCommands> && ...)
effect<Category> add_effect(TCommands&&... commands); // (3)
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

返回登记效果的 [`effect<Category>`](../../definition/effect.md)。命令参数验证失败时，将诊断加入本次 [`compile`](../compile.md) 的错误列表；诊断包含当前源、响应程序编号和命令下标。只有整库编译成功后，返回入口才可用于执行。

## 注意

返回入口只用于本次编译产生的定义库。效果正常完成后回到发起它的结算；若执行期间结束对局，则不再返回原结算。本函数只登记效果，不立即执行。每个动态命令需要一个专用的 [命令输入](../../definition/command_inputs.md) 对象，固定模式不占输入位置。编译后，该入口要求的输入对象数量、类型和顺序固定，对象中的数组长度可以在响应时决定；响应提交入口时须同时提供匹配输入。所登记效果沿用最终 `compile` 调用选择的编译模式。

命令序列在第一条 [`return_response`](../../definition/commands/return_response.md) 处结束；之后的命令不再检查、编译或计入输入要求。没有显式返回时补固定返回 `return_response::null`，结束该实体的响应链。

`normal` 和 `preview` 效果在返回前自动完成末段收尾与结算、退出自己的结算域并恢复外层本方。`immediate` 效果不结算，其记录归入外层段；编译时禁止它包含 `end_segment` 或 `settle`。普通和预览效果可以交给 [`defer_program`](../../definition/commands/defer_program.md)，延迟执行忽略返回编号；立即效果不能用于延迟调用。

具名接口 `add_normal_effect`、`add_immediate_effect`、`add_preview_effect` 分别选择对应类别。它们同样支持 span、命令序列及不定参数。

命令及其中借用的数据须在本次调用期间保持有效。返回前完成编译，不保留传入序列或命令对象；调用后可以销毁它们。

## 示例

```cpp
#include <cstdint>
#include <utility>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/basic_definitions.hpp>
#include <givm/compile.hpp>
#include <givm/definition_source.hpp>

struct support_source
{
    static constexpr auto category = givm::definition_category::support;

    std::string_view name() const { return "洗牌助手"; }

    givm::normal_effect compile(givm::definition_compile_context& context) const
    {
        auto entry = context.add_normal_effect(
            std::tuple{ givm::shuffle_deck{ .player = givm::player_id{ 0 } } }
        );
        std::println("已登记回合结束效果: {}", static_cast<bool>(entry));
        return entry;
    }

    static givm::normal_effect handle(
        const givm::normal_effect& entry,
        givm::round_ended&,
        givm::handle_context<givm::support_view>& context, std::uint32_t = 0)
    {
        return context.invoke(entry);
    }
};

int main()
{
    const support_source source{};
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    if(not sources.add(source)) return 1;
    auto library_result = compile(
        sources, basics,
        std::tuple{}, std::tuple{ givm::start_round{}, givm::settle{} }, givm::compile_mode::normal
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
