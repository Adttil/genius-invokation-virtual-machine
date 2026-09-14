# 牌组链接、装载与初始化的分工

本篇保留牌组准备拆成多个阶段的理由、阶段之间的拥有关系和匹配约束，供修改对局创建流程时查阅。当前接口分别见 [link_deck](../reference/definition/link_deck.md)、[linked_deck](../reference/table/linked_deck.md) 和 [card_table::load_deck](../reference/table/card_table/load_deck.md)；以下保留跨接口组合时容易遗漏的条件，不另设一套牌组 API。

对照基线为 `b3d6c50`：链接对象与链接函数已经位于 definition 模块的 `include/givm/definition/deck.hpp`，装载仍是 table 的操作，洗牌和角色初始化仍由 executor 的公开指令完成。这项职责划分解释了为何仓库不需要第四个独立 deck 核心模块。

牌组是每局游戏的输入，不是游戏规则程序的一部分。上层可以用文件、网络消息或其他拥有式格式按 definition 名称保存牌组；核心不规定这种持久化对象的具体类型，也不取得名称字符串的所有权。

核心把进入对局前的准备分为链接和装载两个阶段。链接把名称转换为当前规则库的 issued ID；装载把已经链接的牌组写入 table。随机洗牌和角色初始化仍由初始化程序中的公开指令执行。

## 编译结果

准确的成员与绑定顺序见 [definition_compile_result](../reference/definition/definition_compile_result.md)；下文的非拥有关系在[定义编译备忘](definition_compilation.md#注册与生命周期)中分别说明源对象和字符存储。

`definition_source_library::compile(...)` 返回 `definition_compile_result`。其 `library` 成员是编译后的 `definition_library`，`id_map` 成员是同一次编译产生的 `issued_id_map`：

```cpp
auto [library, id_map] = source_library.compile(...);
```

两个成员对应同一个定义集合和 issued ID 分配结果；返回值也可以按该顺序结构化绑定。

`id_map` 用于链接牌组或其他对局前输入，不需要保存在 table 或 executor 中。table 只保存定义 ID，table 与 executor 均不持有 `library`；执行时由调用方显式传入，handler 通过定义 ID 比较识别已申请的依赖。进入对局后不再进行名称查询。

`id_map` 自身保存非拥有的名称和标签字符串视图，其底层字符在映射使用期间仍须有效。`linked_deck` 则只保存解析后的 ID，不延长这些字符串或 `id_map` 的生命周期；source 的生命周期约定见 [定义源](definition_compilation.md)。

## 链接牌组

[link_deck](../reference/definition/link_deck.md) 承接调用形式、名称解析错误和非牌组合法性检查；本节完整保留选择闭包、顺序、生命周期与库匹配等跨步骤约束。

```cpp
template<class TCardNames, class TCharacterNames>
linked_deck link_deck(const issued_id_map& id_map, TCardNames&& card_names, TCharacterNames&& character_names);
```

`card_names` 和 `character_names` 分别是卡牌 definition 名称与角色 definition 名称的可遍历序列。函数在返回前顺序消费两个序列，不保存序列、元素或字符串视图的引用。

每个名称必须存在于 `id_map` 的对应定义类别中。缺少卡牌或角色 definition 时抛出 `std::invalid_argument`。链接只验证名称可解析，不负责检查牌组数量、同名数量、角色与卡牌组合等特定游戏模式的合法性。

若规则库只从指定根名称编译依赖闭包，上层必须在编译前把双方牌组中的卡牌和角色名称加入相应类别的根名称集合。也可以编译源库中的全部定义。没有被选入本次规则库的名称不能在之后的牌组链接中使用。

返回的 `linked_deck` 只拥有定义 ID：

```cpp
std::vector<definition_id<card_definition>> linked_deck::cards;
```

`cards` 按提供顺序保存牌堆中的卡牌。装载后第一个元素位于牌堆底部，最后一个元素位于牌堆顶部；通常初始化程序会在首次抽牌前洗牌。

```cpp
std::vector<definition_id<character_view>> linked_deck::characters;
```

`characters` 按提供顺序保存角色槽位。该顺序决定之后使用 `character_id.index` 访问角色时的 index。

`linked_deck` 必须与其 issued ID 分配结果相匹配的定义库配合使用。相同的选中定义集合及标签声明产生一致的 issued ID；仅仅注册了同一组 source，却采用不同的 `definition_selection`，不保证相同 ID。`linked_deck` 不携带规则库身份，`load_deck` 也不检查这种匹配关系，调用方应直接使用同一次编译返回的 `id_map` 完成链接。

## 装载牌组

当前函数另带 `constexpr`，见 [card_table::load_deck](../reference/table/card_table/load_deck.md)；下面省略该修饰的声明只展示原记录中的阶段边界。

```cpp
void card_table::load_deck(player_id player, const linked_deck& deck);
```

`player` 指定接收牌组的玩家。该玩家的牌堆和角色区必须为空。

函数按 `deck` 中的顺序创建使用默认 `card_state{}` 的牌堆实体和使用默认 `character_state{}` 的角色实体。它不使用随机数、不广播事件、不调用 definition handler，也不进入 executor 程序。

装载完成后的 table 是一份尚未开始执行游戏流程的初始状态。它可以在进入游戏主入口前复制，用于从同一未经随机化的牌组状态开始多场独立对局。

## 初始化程序

角色初始化事件的行为见 [character_initialization](../reference/executor/events/character_initialization.md)。它由每个角色自己的定义填写状态；当前 `initialize_characters` 和 `enter_character` 均不执行这个响应返回的程序入口。

需要随机性或 definition 逻辑的准备步骤属于游戏规则，应由初始化程序表达：

1. [`shuffle_deck`](../reference/executor/instructions/shuffle_deck.md) 随机重排指定玩家的牌堆。
2. [`initialize_characters`](../reference/executor/instructions/initialize_characters.md) 调用指定玩家每个角色自己的 `character_initialization` handler。
3. 初始化程序继续执行初始抽牌、换牌和选择出战角色等规则。

具体顺序由编译调用方提供的初始化程序决定。`load_deck` 不隐式补做这些步骤。

[开发备忘](../notes.md)
