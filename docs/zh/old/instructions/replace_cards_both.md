[返回](../events.md)

# replace_cards_both

准备并推进双方开局替换手牌流程。流程提供两个输入挂起点，分别等待双方写入栈顶 `selector` 输入槽；每份输入提交后，再次执行本指令时立即修改该玩家手牌与牌堆，但本同步流程内不广播 `card_drawn`。

## 字段

无字段。阶段由栈顶 frame 的 `stage_t` 后缀表达。

## 栈

本命令使用栈顶 `stage_t` 区分下一次进入时应如何解释栈顶；替换随机数与输入槽保存在 executor stack。

### `stage == prepare`

本阶段不要求特定栈顶布局。执行时读取双方当前手牌数，预发足够的随机数，并压入一个 frame：

```cpp
stack.top<std::uint32_t[], std::uint32_t, selector, stage_t>();
```

约定结构化绑定为：

```cpp
auto&& [random_pool, player0_random_count, input, stage] =
    stack.top<std::uint32_t[], std::uint32_t, selector, stage_t>();
```

`random_pool[0, player0_random_count)` 属于玩家 0；`random_pool[player0_random_count, random_pool.size())` 属于玩家 1。`input` 是 `stage_t` 前的外层输入槽，初始为玩家 0 的空选择。随后命令将栈顶 stage 改为 `first_selection` 并返回 `false`，当前执行位置仍指向本指令。

### `stage == first_selection`

进入本阶段时假定栈顶仍是：

```cpp
stack.top<std::uint32_t[], std::uint32_t, selector, stage_t>();
```

且外层已经按协议写好尾部 `input`。

1. 按 `input.player` 选择对应随机数区间。
2. 按 [`replace_cards`](replace_cards.md) 的牌区修改规则立即结算该玩家替换；每插回一张被替换手牌，消费该玩家随机区间中的下一个 `std::uint32_t` 来计算随机插入位置。
3. 将 `input` 重置为另一名玩家的空选择输入槽。
4. 将栈顶 stage 改为 `second_selection`。
5. 返回 `false`，当前执行位置仍指向本指令。

执行后栈顶布局不变：

```cpp
stack.top<std::uint32_t[], std::uint32_t, selector, stage_t>();
```

### `stage == second_selection`

进入本阶段时假定栈顶仍是：

```cpp
stack.top<std::uint32_t[], std::uint32_t, selector, stage_t>();
```

且外层已经按协议写好尾部 `input`。

1. 按 `input.player` 选择对应随机数区间。
2. 立即结算该玩家替换。
3. 弹出整个替换 frame；执行后栈顶恢复为本指令第一次执行前的布局。
4. 完成当前指令。

## 注意

- 本命令用于开局替换这类不需要抽牌广播的同步流程。
- 一方提交后会立刻看到自己的换牌结果，但同步流程中不广播 `card_drawn`，避免因提交顺序影响响应结算顺序。
- `stage_t` 前的完整 `selector` 是唯一外层输入槽；随机池、分界计数与 stage 由核心指令维护。
- 核心假定外层写入合法选择；合法性检查、候选项展示和错误报告属于上层。
