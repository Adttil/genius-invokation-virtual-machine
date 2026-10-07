[givm](../../reference.md) / [牌桌](../table.md) / **player_id**

# givm::player_id

定义于头文件 `<givm/table.hpp>`

```cpp
class player_id;
```

玩家在一张牌桌中的身份。使用此 ID 可以通过 [`table::operator[]`](table/operator_subscript.md) 再次取得相应实体。

## 成员函数

| 名称 | 说明 |
| --- | --- |
| [`(构造函数)`](player_id/constructor.md) | 默认构造保持平凡，未初始化的 ID 须先赋值 |
| [`operator=`](player_id/operator_assign.md) | 复制或移动同类 ID |
| [`value`](player_id/value.md) | 取得不含类别标志的完整编码字，供读取或保存 |
| [`index`](player_id/index.md) | 取得当前实体的索引 |
| [`operator==`](player_id/operator_equal.md) | 比较同类 ID 的身份，不检查实体是否在场，也不区分属于哪张牌桌 |

## 非成员函数

```cpp
friend constexpr bool operator==(player_id, player_id) = default;
```

比较编码的身份是否相等；比较不检查实体是否尚未移除。

## 注意

访问玩家时只接受 0 和 1。[`table_state::self_player`](table_state.md) 使用 `optional_player_id` 表示是否有本方。

## 示例

```cpp
#include <utility>
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    auto library_result = compile(sources, basics, std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, id_map] = std::move(*library_result);
    givm::table table{};
    const givm::player_id id{ 1 };
    std::println("目标玩家: {}", table[id].id().index());
}
```

输出

```text
目标玩家: 1
```
