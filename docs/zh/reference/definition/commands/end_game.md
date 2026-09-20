[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **end_game**

# givm::end_game

定义于头文件 `<givm/definition.hpp>`

```cpp
struct end_game;
```

以指定胜负结果结束对局的命令，可用于游戏流程或事件响应中的终局判定。

## 成员类型

| | |
| --- | --- |
| `input_type` | `void`，表示不消费调用输入 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| [`result`](end_game/result.md) | [`game_result`](../../enums/game_result.md) | 本次结束对局的胜负结果 |

## 注意

执行时 `result` 必须是实际的终局结果，不能为 `game_result::no_result`；违反此前提属于未定义行为。

[`step`](../../executor/executor/step.md) 执行本命令后返回 `execution_state::finished`，通过终局视图的 [`result()`](../../executor/execution_view/finished/result.md) 取得胜负。终局不能继续执行；本命令之后的程序步骤不再执行。

程序中先于本命令执行的步骤仍可提前结束对局，此时结果由先发生的终局决定。

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
        std::tuple{ givm::end_game{ .result = givm::game_result::player_0_win } },
        std::tuple{}, givm::compile_mode::normal);
    givm::table table{};
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};

    execution.enter_entry(library);
    std::println("普通推进到终局: {}",
        execution.step(library, table, random) == givm::execution_state::finished);
    std::println("玩家 0 获胜: {}",
        execution.view_in<givm::execution_state::finished>().result()
            == givm::game_result::player_0_win);

    const auto [observed_library, observed_ids] = compile(
        sources,
        std::tuple{ givm::end_game{ .result = givm::game_result::player_0_win } },
        std::tuple{}, givm::compile_mode::observed);
    execution.enter_entry(observed_library);
    std::println("观察推进到终局: {}",
        execution.step(observed_library, table, random) == givm::execution_state::finished);
}
```

输出

```text
普通推进到终局: true
玩家 0 获胜: true
观察推进到终局: true
```

## 参阅

| | |
| --- | --- |
| [`program_entry`](../program_entry.md) | 响应程序入口 |
| [`execution_view<finished>`](../../executor/execution_view/finished.md) | 已结束对局的结果视图 |
