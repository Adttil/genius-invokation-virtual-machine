# 牌组链接与装载

牌组是每局游戏的输入，不是游戏规则程序的一部分。上层可以用文件、网络消息或其他拥有式格式按 definition 名称保存牌组；核心不规定这种持久化对象的具体类型，也不取得名称字符串的所有权。

核心把进入对局前的准备分为链接和装载两个阶段。链接把名称转换为当前规则库的 issued ID；装载把已经链接的牌组写入 table。随机洗牌和角色初始化仍由初始化程序中的公开指令执行。

## 编译结果

`definition_source_library::compile(...)` 返回 `definition_compile_result`。其 `library` 成员是编译后的 `definition_library`，`id_map` 成员是同一次编译产生的 `issued_id_map`：

```cpp
auto [library, id_map] = source_library.compile(...);
```

两个成员对应同一个定义集合和 issued ID 分配结果；返回值也可以按该顺序结构化绑定。

`id_map` 用于链接牌组或其他对局前输入，不需要保存在 table 或 executor 中。table 只引用 `library`；进入对局后不再进行名称查询。

## 链接牌组

```cpp
template<class TCardNames, class TCharacterNames>
linked_deck link_deck(const issued_id_map& id_map, TCardNames&& card_names, TCharacterNames&& character_names);
```

`card_names` 和 `character_names` 分别是卡牌 definition 名称与角色 definition 名称的可遍历序列。函数在返回前顺序消费两个序列，不保存序列、元素或字符串视图的引用。

每个名称必须存在于 `id_map` 的对应定义类别中。缺少卡牌或角色 definition 时抛出 `std::invalid_argument`。链接只验证名称可解析，不负责检查牌组数量、同名数量、角色与卡牌组合等特定游戏模式的合法性。

若规则库只从指定根名称编译依赖闭包，上层必须在编译前把双方牌组中的卡牌和角色名称加入相应类别的根名称集合。也可以编译源库中的全部定义。没有被选入本次规则库的名称不能在之后的牌组链接中使用。

返回的 `linked_deck` 只拥有 issued ID：

```cpp
std::vector<definition_id<card_definition>> linked_deck::cards;
```

`cards` 按提供顺序保存牌堆中的卡牌。装载后第一个元素位于牌堆底部，最后一个元素位于牌堆顶部；通常初始化程序会在首次抽牌前洗牌。

```cpp
std::vector<definition_id<character_view>> linked_deck::characters;
```

`characters` 按提供顺序保存角色槽位。该顺序决定之后使用 `character_id.index` 访问角色时的 index。

`linked_deck` 只能与产生其 issued ID 的定义库配合使用。同一定义源集合保证产生一致的 issued ID；把其他定义库的链接结果装入 table 属于调用方错误。

## 装载牌组

```cpp
void card_table::load_deck(player_id player, const linked_deck& deck);
```

`player` 指定接收牌组的玩家。该玩家的牌堆和角色区必须为空。

函数按 `deck` 中的顺序创建使用默认 `card_state{}` 的牌堆实体和使用默认 `character_state{}` 的角色实体。它不使用随机数、不广播事件、不调用 definition handler，也不进入 executor 程序。

装载完成后的 table 是一份尚未开始执行游戏流程的初始状态。它可以在进入游戏主入口前复制，用于从同一未经随机化的牌组状态开始多场独立对局。

## 初始化程序

需要随机性或 definition 逻辑的准备步骤属于游戏规则，应由初始化程序表达：

1. [`shuffle_deck`](instructions/shuffle_deck.md) 随机重排指定玩家的牌堆。
2. [`initialize_characters`](instructions/initialize_characters.md) 调用指定玩家每个角色自己的 `character_initialization` handler。
3. 初始化程序继续执行初始抽牌、换牌和选择出战角色等规则。

具体顺序由编译调用方提供的初始化程序决定。`load_deck` 不隐式补做这些步骤。

[返回文档入口](README.md)
