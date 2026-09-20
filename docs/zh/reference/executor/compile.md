[givm](../../reference.md) / [执行](../executor.md) / **compile**

# givm::compile

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TInitializationSequence, class TRoundSequence>
auto compile(
    const definition_source_library& sources,
    TInitializationSequence&& initialization_program,
    TRoundSequence&& round_program,
    compile_mode mode
); // (1)

template<class TInitializationSequence, class TRoundSequence>
auto compile(
    const definition_source_library& sources,
    const definition_selection& selection,
    TInitializationSequence&& initialization_program,
    TRoundSequence&& round_program,
    compile_mode mode
); // (2)
```

准备一场对局要使用的实体定义和对局流程。初始化部分只进行一次，回合部分随后反复进行，直到流程主动暂停或结束对局。

(1) 使用全部已登记定义。(2) 从指定定义出发，自动包含直接和间接依赖的定义；未选中的定义不会编译。

## 模板参数

|  |  |
| --- | --- |
| `TInitializationSequence` | 初始化命令序列，可为 tuple-like 对象或可遍历范围 |
| `TRoundSequence` | 每回合的命令序列，可为 tuple-like 对象或可遍历范围 |

## 参数

|  |  |
| --- | --- |
| `sources` | 已登记本场可用定义的源库 |
| `selection` | 各类别首先选择的定义名称 |
| `initialization_program` | 对局开始时依次执行的命令 |
| `round_program` | 每回合依次执行的命令 |
| `mode` | [`compile_mode`](compile_mode.md)，决定是否提供额外观察现场 |

## 返回值

返回一个类型未指定的对象，包含以下公开成员，按表中顺序支持结构化绑定：

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `library` | [`definition_library`](definition_library.md) | 编译后的实体定义与对局流程 |
| `id_map` | [`issued_id_map`](../definition/issued_id_map.md) | 同一次编译产生的定义及标签名称映射 |

两者对应同一个定义集合和 ID 分配结果。调用方可以通过 `auto` 保存结果，或使用 `auto [library, id_map] = compile(...);` 分别取得两者。

## 异常

|  |  |
| --- | --- |
| `std::invalid_argument` | 选择了未知定义，或定义源编译时查询了未声明的依赖；未定义 `NDEBUG` 时，初始化或回合流程需要响应输入也会抛出 |

## 注意

两段流程只能使用[核心给定的命令](../definition/commands.md)，也可用 [`any_command`](../definition/any_command.md) 保存。命令的 `input_type` 须为 `void`，不消费响应输入。回合流程必须能够暂停或结束，避免空流程无限运行。定义源的编译操作抛出的异常继续向调用者传播。

`mode` 必须显式指定。两种模式返回相同的 `definition_library` 类型，并通过同一个 `executor::step` 推进；普通模式仍保留输入请求与终局，观察模式额外报告领域观察现场。模式同时应用于初始化、回合流程和定义源登记的所有响应程序。

编译返回后，初始化和回合流程的输入序列及其中的命令对象可以销毁，不影响定义库的使用。定义源编译所得配置数据随定义库保持有效；源名称、标签以及配置数据借用的对象仍须由调用方保证存活。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{ givm::start_round{ .max_rounds = 2 } }, givm::compile_mode::normal
    );
    givm::table table{};
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    execution.enter_entry(library);
    execution.step(library, table, random);
    std::println("终局时的回合数: {}", table.state().round_number);
    std::println("以双败结束: {}", execution.view_in<givm::execution_state::finished>().result() == givm::game_result::both_loss);
}
```

输出

```text
终局时的回合数: 3
以双败结束: true
```

## 参阅

|  |  |
| --- | --- |
| [`definition_source_library::make_issued_id_map`](../definition/definition_source_library/make_issued_id_map.md) | 在编译前取得配套 ID |
| [`executor::enter_entry`](executor/enter_entry.md) | 开始执行一场游戏 |
