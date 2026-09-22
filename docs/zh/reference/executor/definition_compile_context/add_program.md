[givm](../../../reference.md) / [执行](../../executor.md) / [definition_compile_context](../definition_compile_context.md) / **add_program**

# givm::definition_compile_context::add_program

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TCommands>
program_entry add_program(TCommands&& commands);
```

登记响应事件时需要依次执行的一段效果，并取得可在以后响应时提交的入口。

## 模板参数

|  |  |
| --- | --- |
| `TCommands` | 命令序列，可为 tuple-like 对象或可遍历范围 |

## 参数

|  |  |
| --- | --- |
| `commands` | 按顺序执行的[核心命令](../../definition/commands.md)，也可用 [`any_command`](../../definition/any_command.md) 保存 |

## 返回值

登记效果的非空 [`program_entry`](../../definition/program_entry.md)。

## 注意

返回入口只用于本次编译产生的定义库。效果正常完成后回到发起它的结算；若执行期间结束对局，则不再返回原结算。本函数只登记效果，不立即执行。所需输入由具体命令值按执行顺序确定，不消费响应输入的命令不占输入位置。编译后，该入口要求的输入数量、类型和顺序固定；响应提交入口时须同时提供匹配输入。所登记效果沿用最终 `compile` 调用选择的编译模式。

## 示例

```cpp
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
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{ givm::start_round{} }, givm::compile_mode::normal
    );
}
```

输出

```text
已登记回合结束效果: true
```
