[开发备忘](../../notes.md) / [设计演变](../history.md) / **2026-09-05 固定程序设计稿**

# 固定程序重构设计（2026-09-05 历史稿）

来源：迁移前仓库 `D:\Mine\Code\genius-invokation` 的提交 `a4687db86d5e8de911430cce7dc595394a9c0688`，路径 `docs/fixed-program-design.md`。保留下方全部正文，只改页首和文内导航路径。这份较早稿件保存了后来的简化总览没有保留的实现方案、否决理由、伪代码及后续方向。

下文的“当前”“现已落地”均指当时设计稿的状态；旧接口名称、具体布局和“公共 ABI”的范围需要结合后来的调整阅读。其中，定义源和游戏流程只能使用核心给定公开指令的约定始终有效，模板未静态检查核心指令集合不改变这一约定。对照现有实现，应特别注意：

- `root_context`、省略 `context_type` 表示自由指令的规则已经更改；当前命令按具体命令值确定是否消费响应输入，同一操作的固定参数和消费输入方式由同一命令类型表达；编译后入口的输入数量、类型和顺序固定。
- `make_data`、`programs`、`program_item` 及 setter 补写属于旧定义源接口。当前定义源通过 `compile(definition_compile_context&)` 直接生成完整定义数据，见[定义编译记录](../definition_compilation.md)。
- 独立 `fixed_program` 类型和 `table.instruction(...)` 等旧命名不能照抄；当前定义库拥有程序存储，执行器从每次推进时显式传入的定义库取指。内部跳转/返回由执行器收束，详见[固定程序记录](../fixed_program.md)。
- 旧稿把完整帧作为跨入口 ABI 讨论；当前命令不依赖外层 Context，响应通过尾调用 invoke 提交入口和初始事件，费用预览缓存全部输入；旧 onpay_context 已移除。行动观察与输入由 execution_view 提供，executor 不公开原始栈。
- 出牌、技能的 action 矩阵与执行过程包含当时尚未接入的设计。当前出牌已通过 `begin_action` 提供，采用独立的 `card_effect` 原效果响应，具体接口及顺序见[费用预览与提交](../event_dispatch/payment_commit.md)。下文的旧出牌方案和技能设想不构成当前公开契约。
- 下文的三个特殊终局入口和固定终局前缀已经废弃，当前只保留空入口占位，根程序从内部位置 1 开始。公开 end_game 指令携带结果，定义源通过 add_program 编入程序，不再使用 program_entry 的结果工厂。指令槽尺寸、返回位置的数值关系和后续优化仍只是内部记录，不能反推为公开数值 ABI。当前推进统一返回 finished，栈顶仅追加结果值，旧现场被逻辑废弃；旧终局观察与继续执行约定也不再适用。

## 原稿正文

本文记录 definition 与 executor 从运行期动态子程序迁移到固定程序时形成、现已落地的核心设计。明确标为后续方向的内容仍不代表当前实现。

核心目标是：一次 definition 编译生成唯一的不可变 VM 程序，其中同时保存游戏主流程、普通响应程序和 onpay 程序；运行时不再拼接任意指令序列，只在这一个程序的地址空间中选择入口、跳转执行，并通过 stack 保存可暂停状态。

## 从动态子程序到固定程序

旧实现可以在运行时动态生成子程序，因此很多动态数据会直接塞进指令对象字段。例如根据当前 event 生成：

```cpp
set_element_aura{ .target = event.target, .aura = ... }
set_active_character{ .target = cost.target }
```

现在指令序列在编译阶段已经确定，运行时不能再通过“新建一条带字段值的指令”来携带 `target`、`source`、费用结果或玩家选择。因此这些运行期数据改由 stack 提供：事件对象、当前 handler id、行动输入、费用事件和 onpay 减费生效参数都成为固定栈布局的一部分。

这就是固定入口需要依赖特定栈布局 ABI 的原因：原本写进指令字段的动态参数，现在由调用者在进入入口前放到约定好的 stack frame 里。入口中的指令不猜参数来源，而是按自身所属的 ABI 读取。例如伤害响应入口读取普通广播 frame 中的 `damage_effect` 和 handler；onpay 入口读取 onpay frame 中的 handler 和减费生效参数；出牌本体入口读取 `card_will_be_played` 事件。

后续扩展也应沿用这一判断：凡是动态值来自当前 event、费用结果或玩家输入，就让固定指令读取相应 stack frame。若一段效果只是把多条 table 修改指令排在一起，则直接把它们声明为固定程序段。

## 基本原则

指令是干净的执行单元。一次指令从进入到彻底完成后，必须恢复进入时的 stack 形状。指令内部可以压临时 frame、推进广播、进入子流程或挂起；这些临时形状由该指令自身解释。只要指令完成时恢复原形状，后续指令就仍可按同一个上下文 ABI 读取 stack。

指令自身不保存运行期阶段。当前 re-entry 阶段放在 stack 顶部 frame 的 `stage_t` 后缀中。`stage_t` 不独立描述栈类型，它必须和当前指令类型共同解释。

栈顶总是以 `stage_t` 结尾，是为了让 executor 和当前指令在任意阻塞或重入点都能用同一接口找到阶段槽位。这个约定也让 activation frame、广播 frame、输入 frame 和指令临时 frame 都能作为“当前可重入状态”存在。`stage_t` 不是越细越好：只有同一条指令的不同重入点需要按不同栈顶形状解释时，才需要不同 stage。若两个阶段看到的栈顶形状相同，差异应优先放在该 frame 中的普通字段里，或直接继续执行，而不是增加新的 stage 值。

程序入口在运行时表现为唯一固定程序中的 offset，但编译层还必须知道入口要求的栈布局 ABI，例如普通广播 frame、onpay 调用 frame 或根流程。`program_entry<Context>` 用模板参数保存这项编译期约束；运行时不按名称或类型动态猜参数。

## 唯一程序与根流程

一次编译只产生一个由 `definition_library` 拥有的 `fixed_program`。所有 definition 的普通响应程序、onpay 程序和游戏流程程序都在构造中的同一个 `definition_library` 内直接追加到固定程序存储，最终共享同一个执行位置地址空间。概念布局可以是：

```text
fixed_program
  0: reserved null placeholder
  1: end_game marker for player_0_win
  2: end_game marker for player_1_win
  3: end_game marker for both_loss
  4: root_entry:
       root initialization segment
       round_entry:
         round segment
         internal jump to round_entry
  ...: definition response subprograms ...
       definition onpay subprograms ...
```

前四个位置是固定程序格式的一部分。位置 0 存放一个内部不可执行占位，使全零的 `program_entry<Context>` 自然表示 null；位置 1 到 3 存放三条无状态的内部 `end_game` 指令，位置本身分别表示三种终局结果。根程序紧接固定前缀，因此 `root_entry` 永远是位置 4；`definition_library` 不需要保存额外的运行期根入口字段，`root_entry()` 可以直接构造这个格式常量。根程序长度可以变化，各 definition 子程序从根程序之后继续顺序追加，其入口仍取追加前的 `size()`。

`program_entry<Context>` 的运行期载荷只是一个 `execution_position` 值，不包含程序指针、程序编号或其他 owner 信息。值 0 表示 null，值 1 到 3 表示不依赖 Context ABI 的静态终局入口，其余值是唯一程序中的普通 offset。Context 只约束普通入口；任意 Context 的响应都可以返回相应的终局入口。进入入口后 executor 的 `position`、activation 中的 `return_position` 和内部跳转目标都使用同一套值。`execution_position` 保持现有的 `detail::instruction_index`（即 `size_t`）别名，不引入另一层执行位置包装类型；裸数值和终局位置映射只存在于内部实现，定义扩展者通过 `program_entry<Context>` 的具名接口使用它们。

初始固定槽实现直接把前四个位置放进程序存储，后续程序入口仍取追加前的 `size()`，executor 以 `program[position]` 直接取指，不需要为普通 offset 做 `+1/-1` 编解码。把 null 放在 0 还使默认构造、全零初始化和频繁的空入口判断保持最直接；相比把特殊值放到整数上界，这个固定前缀避免了每次取指前的终局范围判断。旧动态队列现已删除，根流程和所有响应都使用这一地址空间。

根流程不是普通的可返回子程序。编译器把它组装为两段程序序列：只执行一次的初始化段和无限重复的回合段。程序序列既可以是异构具体指令组成的 tuple-like，也可以是具体指令或 `any_instruction_for<root_context>` 组成的 range；每个元素都按 `root_context` 检查。初始化段自然落入回合段，回合段末尾由编译器补入跳回 `round_entry` 的内部控制操作；两段之间不追加返回标志，根流程末尾也不会正常返回。

初期最直接的内部表示是独立的无条件回跳指令。以后可以把回跳目标融合到回合末尾指令的内部表示中，例如外部定义只描述不含执行位置的“结束回合”，组装时生成携带 `round_entry` 的内部形式。两种表示的控制流相同，也不要求向定义扩展者暴露裸跳转位置；无论采用哪种表示，回跳都必须由当前槽位中的指令执行，不允许 executor 在每步执行时把执行位置与另行记录的回合末尾位置比较。

回合循环本身不需要条件。角色全灭、达到最大回合数或其他终局条件由相应领域指令检查，并通过既有跳转能力把 `position` 设置为对应的具名终局位置。该跳转不清空 activation 或 stack；终局后这些 frame 不再被解释，外层观察完现场后通过专门的 clear/reset 流程统一清理。三条 `end_game` 和普通指令一样被取出并擦除分派，但永远返回阻塞且不推进 `position`，因此 executor 的热路径不需要预先判断当前执行位置是否终局。

`game_result` 增加值为 0 的 `no_result`，三个实际结果的值与三个终局位置一一对应。`executor::status()` 只根据当前 `position` 返回相应结果；null、尚未开始和普通执行位置都返回 `no_result`。终局结果不另存进 table、stack 或 executor 字段，`end_game` 指令本身也不携带结果。

根程序使用 `root_context` 作为入口 ABI 身份，但这不要求其中每条具体指令都声明 `context_type = root_context`。不读取外层根 frame 的流程指令仍然是 context-free，可以直接出现在原始指令 tuple/range 中，也可以预先由 `any_instruction_for<root_context>` 包装；只有确实读取根 ABI 的指令才声明该 Context。编译器补入的回跳和返回标志属于内部控制结构，不经过定义源可见的 wrapper。

## 子程序与返回帧

固定程序由 executor 按唯一程序中的执行位置执行。进入一段可返回的固定入口时，调用者压入 activation frame：

```cpp
frame<return_info, stage_t>
```

`return_info` 只保存唯一程序中的 `return_position` offset。固定程序不在运行期生成待清除的代码段，因此返回时不需要恢复指令队列 end，也没有执行来源字段。

`stage_t` 是该子程序入口的初始阶段，进入时为 0。入口内每条干净指令完成时都会把栈形状恢复到进入该指令前，因此同一段固定程序中的下一条指令仍能看到同一套上下文。

子程序末尾使用无状态的返回标志指令。返回标志读取栈顶 `return_info, stage_t`，断言 `stage_t == 0`，弹出 activation frame，并跳回 `return_position`。

根程序使用一个延迟建立的 `stage_t` 根 frame，并通过回跳持续运行；它只会跳入静态终局位置，不走普通返回标志。终局位置的 `end_game` 永不返回，因此遗留 activation 和其他 stack frame 不会被再次读取。运行期不应依赖普通指令槽内部存储形态；未来即使迁移到不定长指令槽，null 和三个静态终局位置仍属于固定程序地址格式的保留前缀。

## 栈布局 ABI

指令分为两类：

- context-free instruction：具体指令类型不声明 `context_type`，表示它不依赖外层事件或行动上下文，可以放入任意正常程序上下文。
- context-bound instruction：具体指令类型声明唯一的 `using context_type = Context`，表示它静态要求该栈布局 ABI。

不存在第三类运行期多态上下文指令，也不为单条具体指令声明 Context type list。若同一语义需要在多个 Context 中使用，应把指令做成以 Context 为模板参数的类型；每个具体实例仍只绑定一个 Context，共享实现放入普通 C++ helper。只有确实不读取任何外层上下文的指令才省略 `context_type`。

上下文不是随意压入的参数列表，而是由固定栈布局描述。文档中可以用“某事件的响应入口”或“某费用事件的 onpay 入口”描述入口种类，但这不要求代码里真的存在额外包装类型。真正的 ABI 是调用入口前已经在 stack 上准备好的 frame 形状。

普通响应入口依赖普通广播 frame。onpay 入口依赖 onpay 调用 frame。两者都是响应函数产生的入口，但它们看到的栈布局不同。onpay 不在原费用事件广播过程中执行，因此不能假装仍处于原 `CostEvent` frame 中。

指令若在执行中压入临时 frame 并挂起，仍由该指令自己解释当前栈形状。它可以在自己的协议里访问临时 frame 下方的上下文，也可以在第一阶段复制所需字段；这不是全局要求。全局要求只有一条：指令完成时恢复进入时的 stack 形状。

## 响应函数与响应程序

响应函数和响应子程序必须保持分离：

```text
响应函数：
  读 const table
  读/写 event
  判断是否触发
  返回固定入口
  不直接修改 table

响应程序：
  固定 VM 程序入口
  通过指令修改 table
  可触发新的广播
```

这个边界保留了当前事件响应的高效路径。很多判断、标签查询、数值比较和 event 修改不需要进入 VM；只有真正需要修改 table 的部分才进入固定程序。

source-facing handler 不接收 `schedule_fn`，而是返回一个强类型入口。入口类型带有 null 值，表示本次响应没有子程序需要执行；它也可以通过具名静态接口产生三个终局值。固定响应程序在编译阶段已经放入唯一程序，运行时动态参数通过栈协议提供，例如当前 event、当前 handler id、行动参数或费用事件中的减费生效参数。由于响应程序可以通过栈协议访问 event，它也可以修改 event。

普通事件和费用事件对返回入口的使用时机不同。普通事件广播必须在跳转前检查 null；null 表示不进入子程序、不产生 re-entry，直接继续遍历。三个终局值均为非 null 的真实指令位置，按普通入口进入后由对应 `end_game` 永久阻塞，不需要广播代码增加终局分支。费用事件广播不会立刻进入返回入口，而是把非 null 入口和事件中的减费生效参数记录到行动窗口的 onpay 矩阵，等玩家确认支付时再执行；终局值若出现在这里，也按同一个非 null 入口协议保存和进入。

费用计算也依赖这个分离。预览费用时只执行费用响应函数，得到最终费用和 onpay 入口矩阵；实际支付时执行 onpay 入口。这样费用预览和实际支付共用同一套响应函数，同时不会在预览阶段修改 table。

## 表达能力边界

固定程序限制的是“运行时不能临时生成新的程序形状”，不是限制响应函数和指令中的 C++ 计算能力。判断一种机制能否自然表达时，关键不是运行期是否出现了不同的类型、数值或目标，而是运行期状态是否要求从无界集合中生成不同长度、不同控制流形状的程序。

以下情况可以直接用固定程序表达：

- 指令类型或常量参数不同。例如“对敌方出战角色造成 2 点物理伤害”和“对敌方下一个角色造成 2 点物理伤害”本来就是不同指令值；擦除后仍只是固定程序中的两个不同槽位。
- 分支集合在编译时已知且数量较小。响应函数可以读取 table 和 event，再返回若干 `program_entry<Context>` 之一或 null。这已经是条件分支，不需要在 VM 中另造通用条件跳转。
- 结果只是运行期数值。响应函数可以直接修改 event，例如先发布 0 点伤害，再由持有层数的实体按当前层数增加伤害；手牌数、已损失生命、实际减费数等也属于同一类。
- 动态次数属于一个明确规则动作。诸如“对所有满足条件的角色各造成伤害”或“最多消耗 N 层吸收伤害”应由一条领域指令内部完成遍历或取最小值，而不是展开成运行期数量的 VM 指令。

需要警惕的是运行期状态决定一个无明显上限的程序分支集合，或要求生成任意长度的副作用序列。此时不应为每种数量预制入口，也不应恢复动态子程序，而应寻找规则上的最小整体操作，把循环、消耗或选择封装成一条 context-bound instruction。例如 `absorb_damage_by_count{}` 在 `damage_effect` 广播上下文中直接读取当前伤害事件和当前 handler，计算 `min(当前层数, 最大消耗层数, 尚未吸收伤害)`，修改事件并更新自身层数。除非它内部还要触发可挂起的子流程，否则不需要额外维护自己的 stage 或 frame。

“先锁定目标、结算后再附着元素”也不需要把目标捕获进运行期生成的指令。规则上可以先给目标附加一个实体或状态，让该实体在后续伤害事件中响应并附着元素。动态关系进入 table/event 协议，固定程序只表达规则动作。

因此分界可以概括为：有限且较小的控制分支由响应函数选择固定入口；任意运行期算术优先通过事件修改完成；具有无界次数但语义完整的规则动作由单条领域指令完成。只有无法归入这三类的新机制，才说明当前上下文 ABI 或指令粒度需要扩展。

终局入口也是响应函数可以选择的静态分支。若响应无需先修改 table，可以直接返回当前 Context 下的具名终局 `program_entry`；若某段固定程序必须先执行若干副作用再结束，则最后可以使用一条 context-free 的终局跳转指令。该指令只把执行位置改为固定前缀中的终局位置，不承担终局状态存储；当前规则没有这种程序段时不必提前公开它。

## 普通广播上下文

普通事件广播采样 handler 列表，并在 stack 上保存广播状态、当前 event 和当前 handler 槽位。固定程序方案下，广播 frame 形状为：

```cpp
auto&& [handlers, cursor, event, handler, stage] =
    stack.top<handler_id<E>[], stack_count_t, E, handler_id<E>, stage_t>();
```

各结构化绑定名称含义为：

- `handlers`：本次广播开始时采样得到的 handler 数组，顺序即规则响应顺序。
- `cursor`：下一个待处理 handler 在 `handlers` 中的索引。广播在调用响应函数前把当前项复制到 `handler` 并推进 cursor，因此响应入口返回后会自然从下一项继续。
- `E event`：本次广播的事件对象。响应函数和响应程序都可以按事件协议读写它。
- `handler`：当前 handler id，即调用响应函数前从 `handlers` 取出的副本。响应入口通过它知道“自己是谁”，不必依赖已经推进的 cursor 反查数组。
- `stage`：广播指令自身的 re-entry 阶段。

进入非空响应入口时，在上述广播 frame 之上再压入 activation frame：

```cpp
auto&& [broadcast, activation] = stack.top<
    frame<handler_id<E>[], stack_count_t, E, handler_id<E>, stage_t>,
    frame<return_info, stage_t>
>();
```

因此响应入口中的 context-bound 指令若要读取当前事件和 handler，应按这个多 frame 协议访问 activation frame 下方的广播 frame。实际布局可以改名或压缩，但必须满足：

- 广播指令能保存 handler 数组、游标和 event。
- 当前响应入口能静态读取 `E` 和当前 handler id。
- 返回响应入口后，广播仍能继续推进下一个 handler。
- `return_info, stage_t` 是 activation frame，返回指令能统一弹出。

响应函数先执行。若响应函数返回 null 入口，当前 handler 没有 VM 程序需要执行，广播直接继续。若返回非 null 入口，广播压入 activation frame 并跳转到该入口。响应程序结束后返回广播指令，广播继续。

普通响应程序依赖的是当前事件上下文。例如 `damage_effect` 响应程序可以读取伤害来源、目标、数值、类型和当前 handler。若响应程序中执行 `deal_damage` 等指令触发新广播，新广播拥有自己的事件上下文；旧响应程序上下文在该指令完成后仍由干净指令原则恢复可见。

## 费用事件与 onpay

费用事件包括主动出牌、使用技能和主动切人的费用计算。它们的目标是得到：

- 最终费用需求。
- 行动速度。
- 每个 handler 贡献的 onpay 入口。

费用响应函数读取并修改费用事件。费用事件除了最终费用需求和行动速度外，还包含一个事件族固定的减费生效参数。这个参数不是通用附加数据，而是费用事件协议的一部分，用来记录“本 handler 若在实际支付阶段生效，onpay 程序需要知道的最小信息”，例如本次实际减少了多少骰子。

费用计算广播在处理每个 handler 前清空该减费生效参数。典型减费响应：

```text
if 当前剩余费用为 0:
  返回 null 入口

根据 handler 状态和当前费用计算本次 reduction
修改费用事件中的剩余费用
写入费用事件中的减费生效参数
返回 onpay 入口
```

若前一个响应已经把费用降到 0，后续响应函数会看到当前费用为 0，于是返回 null 入口，也不会消耗层数或产生 onpay 入口。handler 的返回形式仍然统一为一个入口；费用计算广播只是额外把事件上的减费生效参数按 handler 索引记入行动窗口的 onpay 参数矩阵。

费用事件返回的入口不在费用广播过程中执行。它的语义是“若本次费用计算结果最终被玩家采用，支付阶段应执行的 onpay 入口”。

onpay 不是费用事件的响应程序。onpay 在玩家实际确认支付后执行，此时原费用广播已经结束。因此 onpay 不保存完整费用事件快照，而保存费用响应生成的最小提交信息，例如：

```cpp
struct cost_effect_argument
{
    elemental_dice_requirement reduced_dice;
};
```

onpay 执行时不读取原费用事件 frame，而是在进入 onpay 入口前压入一帧本次 onpay 调用上下文：

```cpp
auto&& [onpay_frame, activation] = stack.top<
    frame<handler_id<CostEvent>, cost_effect_argument<CostEvent>, stage_t>,
    frame<return_info, stage_t>
>();
```

`handler_id<CostEvent>` 是产生该 onpay 入口的 handler，`cost_effect_argument<CostEvent>` 是当时记录下来的减费生效参数。不要引入任意参数栈；也不要把完整 `cost_of_xxx` event 作为快照复制到每个 onpay 调用中。

## 玩家 Action 上下文

玩家 action 由 `begin_action` 准备行动窗口 frame 并挂起。外层只写栈顶输入槽，不插入指令。输入请求包含：

```cpp
struct action_request
{
    action_request_kind request_kind;
    action_kind action_kind;
    stack_count_t action_index;
};
```

`request_kind` 至少区分：

- `calculate_cost`：仅计算费用并填充对应费用事件槽位。
- `do_action_with_cost`：使用已有费用事件槽位执行。
- `do_action`：立即计算费用并执行。

支付骰子和目标合并为行动参数：

```cpp
struct action_argument
{
    dice_counts paid_dice;
    action_target target;
};
```

onpay 入口和减费生效参数一一对应，行动窗口中应保存为一个数组元素，避免把 entry 数组和参数数组拆开维护：

```cpp
template<class CostEvent>
struct onpay_item
{
    program_entry<onpay_context<CostEvent>> entry;
    cost_effect_argument<CostEvent> argument;
};
```

`entry` 为 null 表示该 handler 对该 action 没有 onpay。handler id 不保存在 `onpay_item` 中；矩阵列与 handler 数组对应，执行时按列从 handler 数组取当前 handler。

行动窗口的大 frame 应把所有变长数组集中放在前缀，并把外层输入槽放在尾部：

```cpp
auto&& [
    card_cost_handlers,
    skill_cost_handlers,
    switch_cost_handlers,
    card_costs,
    skill_costs,
    switch_costs,
    card_onpay_items,
    skill_onpay_items,
    switch_onpay_items,
    argument,
    request,
    stage
] = stack.top<
    handler_id<cost_of_card>[],
    handler_id<cost_of_skill>[],
    handler_id<cost_of_switch>[],
    cost_of_card[],
    cost_of_skill[],
    cost_of_switch[],
    onpay_item<cost_of_card>[],
    onpay_item<cost_of_skill>[],
    onpay_item<cost_of_switch>[],
    action_argument,
    action_request,
    stage_t
>();
```

各动态数组含义如下：

- `handler_id<cost_of_card>[]`：本行动窗口中计算出牌费用时使用的 handler 顺序。
- `handler_id<cost_of_skill>[]`：本行动窗口中计算技能费用时使用的 handler 顺序。
- `handler_id<cost_of_switch>[]`：本行动窗口中计算切人费用时使用的 handler 顺序。
- `card_costs`：每个可打出手牌对应一个 `cost_of_card` 事件，`action_index` 按该数组索引。
- `skill_costs`：每个可使用技能对应一个 `cost_of_skill` 事件，`action_index` 按该数组索引。
- `switch_costs`：每个可切换目标对应一个 `cost_of_switch` 事件，`action_index` 按该数组索引。
- `card_onpay_items`：按行主序保存出牌费用 onpay 矩阵，行数为 `card_costs.size()`，列数为 `card_cost_handlers.size()`。
- `skill_onpay_items`：按行主序保存技能费用 onpay 矩阵。
- `switch_onpay_items`：按行主序保存切人费用 onpay 矩阵。

这里不需要每个 onpay 矩阵槽位重复保存 handler id。矩阵列与对应 handler 数组一一对应：第 `column` 列的 handler 总是 `card_cost_handlers[column]`、`skill_cost_handlers[column]` 或 `switch_cost_handlers[column]`。

当前实现先完成主动切人这一种 action，因此现行 frame 只含 `switch_cost_handlers`、`switch_costs`、`switch_onpay_items` 和 onpay 遍历游标。出牌和技能接入后再把对应三组数组加入同一个行动窗口。

每个费用事件元素保存对应 action 的主体或目标、最终费用需求、行动速度和减费生效参数槽位。因此不再需要 `cache_of_card`、`cache_of_skill` 或 `cache_of_switch` 包装类型。数组索引就是 onpay 矩阵行索引。费用事件槽位是否有效由外层或当前行动窗口协议保证；栈上费用事件槽位不必自描述有效性。玩家 action 发生后，整个行动窗口中的候选费用事件槽位都会失效。

计算某个 action 费用时，`begin_action` 重置该费用事件和对应 onpay 行，然后按对应 `handler_id<cost_of_xxx>[]` 顺序调用响应函数。每个响应函数固定返回一个入口值；入口为 null 表示没有 onpay。若入口非 null，`begin_action` 把入口和费用事件中当前的减费生效参数写入当前 action 行、当前 handler 列的 `onpay_item`。

确认执行某个 action 后，`begin_action` 自己负责执行 onpay 和付款。onpay 只是 `begin_action` 内部的支付前置流程。

执行 onpay 时，`begin_action` 按列遍历选中 action 的 onpay 行。若 `items[column].entry` 为 null，则跳过。若非 null，则短暂压入：

```cpp
auto&& [handler, cost_argument, onpay_stage] =
    stack.top<handler_id<CostEvent>, cost_effect_argument<CostEvent>, stage_t>();
```

其中 `handler = cost_handlers[column]`，`cost_argument = items[column].argument`，然后进入 `items[column].entry`。该 onpay 入口返回后，`begin_action` 继续遍历下一列。所有 onpay 完成后，`begin_action` 再执行固定资源扣除，例如根据 `action_argument.paid_dice` 扣骰子。

付款完成后，`begin_action` 继续在同一个干净指令内执行 action 本体，尽量不再拆出额外的 action 执行指令。以后这个指令也可以改名为 `do_action` 之类更准确的名称；这里保留 `begin_action` 是为了对应当前实现。

主动切人完全可以由 `begin_action` 自己完成：根据选中的 `cost_of_switch` 得到目标角色，设置当前出战角色，并推进 `active_character_changed` 广播。切人不需要再进入一个只做 `set_active_character` 的固定入口。

主动出牌和使用技能类似。以出牌为例，`begin_action` 在完成 onpay 和付款后，根据选中的 `cost_of_card` 和 `action_argument.target` 构造“被打出”事件，并按普通广播栈布局压入 stack，使卡牌自己的固定入口也能按响应子程序的 ABI 读取事件和 handler：

```cpp
auto&& [handlers, cursor, event, handler, stage] =
    stack.top<handler_id<card_will_be_played>[], stack_count_t, card_will_be_played,
              handler_id<card_will_be_played>, stage_t>();
```

这里的 `handlers` 可以只包含被打出的那张牌，`handler` 即该牌的 id。`begin_action` 先调用这张牌定义的“被打出”固定入口，让卡牌效果本体在事件上下文中执行；该入口可以修改事件，例如取消效果或调整后续信息。卡牌入口返回后，`begin_action` 再对最终的 `card_played` 事件进行普通广播，通知其他 handler “牌已被打出”。

使用技能同理：`begin_action` 准备 `skill_will_be_used` 事件栈，调用该技能自己的固定入口；入口返回后再广播 `skill_used`。因此出牌和技能的主体 id、目标等动态信息都应放在事件里，而不是作为额外参数帧传给另一个执行指令。

行动速度在费用计算阶段确定，执行阶段只按已经确定的 `action_speed` 决定是否切换行动权。

## 现有机制状态

伤害流程已经端到端固定化。`deal_damage` 依次推进 `damage_calculation`、`damage_effect`、伤害附带的元素附着/反应和 `after_damage`；层数护盾 handler 返回固定入口，`absorb_damage_by_count` 通过普通广播栈布局读取事件与当前 handler。扣血完成后，`deal_damage` 自己检查双方角色状态并在需要时进入终局位置，不存在独立的 `check_game_finished` 指令。

`apply_element` 直接组织元素附着和反应广播。默认 aura 更新在该指令的固定状态机内完成；响应者若接管反应，则返回固定程序入口并通过上下文事件修改结果，不再按 event 动态生成带目标指令。

主动切人已经完成费用/onpay 固定程序闭环：费用预览按 handler 保存入口和 `cost_effect_argument`，确认后只提交选中 action 的 onpay 行，再由 `begin_action` 扣骰、广播资源变化、设置 `cost_of_switch.target` 指向的出战角色并推进切人广播。所有费用 handler 都使用五参数固定入口协议。

出牌和技能后续也应收进 `begin_action` 的 action 执行流程。`begin_action` 压好“被打出”或“被使用”事件栈，调用牌或技能自己的固定入口，然后再广播“已打出”或“已使用”事件。

回合主流程已经编译进 `definition_library` 拥有的同一个固定程序。初始化段只执行一次，随后进入无限重复的回合段；编译器在回合段末尾补入一条无条件 `jump_instruction`。当前七圣召唤式回合段由 `start_round`、`start_dice_roll_phase`、`start_battle`、`begin_action`、`end_round` 和两条 `draw_cards` 组成，回合上限、骰子数、重投次数与抽牌数直接由这些固定指令字段配置。终局条件由相应领域指令把执行位置切到固定前缀中的结果位置。

## Definition 与 Executor 边界

固定程序重构同时重切 definition 和 executor 两个模块的职责：definition 负责定义源、依赖解析、程序声明、程序擦除表示和不可变程序；executor 负责 `execution_context`、stack、执行位置、具体指令类型和执行循环。需要前移的执行前置概念直接放入 definition 模块，而不是引入新的跨模块层。

### Definition 侧

definition 模块需要拥有以下类型的完整定义：

- `program_entry<Context>`。它是可空、带编译期上下文类型的入口值，会被写入源自己的 data，并由 handler 运行时返回。
- `any_instruction_for<Context>`。这是把异构动态来源整理为同构序列时使用的、带编译期 Context 的擦除值；它在从具体指令构造时完成静态上下文检查。native source 也可以直接返回原始具体指令序列，由编译器在逐元素追加时经过同一构造边界。
- 当前 `executor::detail::any_instruction` 前移后的内部 `any_instruction`。第一阶段它仍是固定大小的拥有型擦除值，只保存执行所需 RTTI；类型别名 `context_type = void` 表示静态 Context 已丢失，不增加逐对象 Context 元数据或运行期查询。
- 唯一的不可变程序容器和程序构建结果。`definition_library` 同时拥有根流程和所有 definition 程序段；强类型根入口由固定位置直接构造。
- 程序声明元素。它类似依赖声明元素，保存“一次遍历即可取得的指令序列”和“如何把入口写入 data”。

事件和 onpay 上下文类型在 definition core 中主要作为类型身份使用，例如 `handler_fn_t<View, Event>`、`handler_id<Event>`、订阅事件列表、`program_entry<Context>` 和 `any_instruction_for<Context>` 的模板参数；这里保留前置声明和类型列表即可，不要求事件完整定义。

具体 executor 指令类型不需要前移到 definition。`any_instruction_for<Context>` 的构造函数模板只需在看见具体指令类型的地方检查其可用 Context，并构造内部 `any_instruction`；具体指令类型及其 `execute` 实现仍属于 executor 指令集。内部 RTTI 的执行函数指针可以只依赖 `execution_context`、`card_table` 和 `random_fn` 的前置声明。

### Executor 侧

executor 模块负责定义 `execution_context`、stack、执行位置、执行循环和具体指令实现。固定程序后，executor 不再拥有“运行时随手拼出一段子程序”的核心职责，而是执行 `definition_library` 中唯一的不可变程序，通过执行位置偏移量进入其中的根入口或响应入口，并用擦除指令 RTTI 中的执行函数指针运行当前指令。最终模型不需要在多个固定程序之间切换。

具体指令仍然是强类型 C++ 类型。native source 可以返回异构具体指令 tuple-like 或同构具体指令 range；脚本 adapter 通常返回 `any_instruction_for<Context>` 的同构 range。`definition_library` 逐个访问元素，并以构造 `any_instruction_for<Context>` 的方式统一完成 Context 检查和擦除，再立即追加内部表示。executor 只解释编译后的指令序列，不解释定义源接口，也不维护另一套操作码注册表。

### 程序上下文与入口类型

`Context` 是编译期 ABI 身份，不是运行期标签。普通响应程序以事件类型本身作为上下文，例如 `program_entry<damage_effect>`；onpay 具有不同的栈布局，因此使用单独的类型身份：

```cpp
template<class CostEvent>
struct onpay_context;

program_entry<damage_effect> damage_entry;
program_entry<onpay_context<cost_of_card>> card_onpay_entry;
```

Context 不限于事件类型；固定主流程使用 `root_context` 作为入口 ABI 身份。根流程中不读取外层 frame 的领域指令仍然是 context-free；Context 描述真实栈依赖，不承担“通常应在哪里调用”的权限分类。回合尾部回跳和子程序返回标志由编译器内部生成，不暴露给定义源构造，也不需要作为 source-facing 指令参与 Context 检查。

`program_entry<Context>` 的运行期载荷只是一个 `execution_position` 整数；模板参数不增加运行期字段，也不需要保存程序 owner。默认值 0 是 null，普通值表示唯一不可变程序中的 Context 入口，三个具名终局值表示不读取任何 Context ABI 的静态终局位置。不同 Context 的普通入口不能隐式互换；各 Context 类型可以分别构造数值相同的终局入口。普通事件 handler 返回 `program_entry<Event>`，费用事件 handler 返回 `program_entry<onpay_context<CostEvent>>`。这能在定义源包装层阻止把费用广播入口误当作 onpay 入口，或把某事件程序返回到另一种广播 frame，同时不妨碍任意响应直接终止对局。

具体指令与 Context 的兼容关系直接从指令类型推导：

```cpp
template<class Instruction, class Context>
concept instruction_compatible_with =
    not requires
    {
        typename std::remove_cvref_t<Instruction>::context_type;
    }
    or std::same_as<
        std::remove_cvref_t<
            typename std::remove_cvref_t<Instruction>::context_type
        >,
        std::remove_cvref_t<Context>
    >;
```

`any_instruction_for<Context>` 是只有一个内部擦除值成员的拥有型 wrapper。它不公开继承 `any_instruction`，避免调用者通过基类引用覆盖内容并破坏 Context 不变量，也避免把扩展接口继承关系绑定到未来的内部存储形式。RTTI 和非持有 view 都先于拥有型内部表示定义：

```cpp
namespace detail
{
    using instruction_execute_fn = bool (*)(
        const void*, card_table&, execution_context&, random_fn&
    );

    struct instruction_rtti
    {
        instruction_execute_fn execute;
    };

    template<class Instruction>
    inline constexpr instruction_rtti instruction_rtti_of{
        /* 直接生成该指令的 execute 包装函数 */
    };

    class any_instruction_view
    {
    public:
        using context_type = void;

        explicit any_instruction_view(
            const instruction_rtti* rtti,
            const unsigned char* storage
        ) noexcept
            : rtti_{ rtti }, storage_{ storage }
        {}

        // is、as、type_index 和 execute 直接使用上述两个指针。
    };

    class any_instruction
    {
    public:
        using context_type = void;

        template<class Instruction>
        explicit any_instruction(const Instruction& instruction) noexcept
        {
            // 内部可信路径只复制 payload 并绑定 RTTI，不重复公开边界检查。
        }

        template<class Context>
        explicit any_instruction(
            const any_instruction_for<Context>& instruction
        ) noexcept
            : any_instruction{
                static_cast<const any_instruction&>(instruction)
            }
        {}

        operator any_instruction_view() const noexcept
        {
            return any_instruction_view{ rtti_, storage_ };
        }

        bool execute(
            card_table& table,
            execution_context& context,
            random_fn& random
        ) const
        {
            return rtti_->execute(storage_, table, context, random);
        }
    };
}

template<class Instruction>
inline constexpr auto instruction_type_index =
    &detail::instruction_rtti_of<Instruction>;

template<class Context>
class any_instruction_for
{
public:
    using context_type = Context;

    template<instruction_compatible_with<Context> Instruction>
    explicit any_instruction_for(const Instruction& instruction)
        : instruction_{ instruction }
    {
        static_assert(sizeof(Instruction) <= detail::instruction_storage_size);
        static_assert(alignof(Instruction) <= alignof(std::max_align_t));
        static_assert(std::is_implicit_lifetime_v<Instruction>);
        static_assert(std::is_trivially_copyable_v<Instruction>);
        static_assert(std::is_trivially_destructible_v<Instruction>);
    }

    bool execute(
        card_table& table,
        execution_context& context,
        random_fn& random
    ) const
    {
        return instruction_.execute(table, context, random);
    }

    explicit operator const detail::any_instruction&() const noexcept
    {
        return instruction_;
    }

private:
    detail::any_instruction instruction_;
};
```

具体指令的大小、对齐、隐式生命周期和平凡复制/析构要求在公开的 `any_instruction_for<Context>` 构造边界检查。内部 `any_instruction` 的无约束模板构造只服务于已经检查过的降级路径和编译器生成的可信内部指令；以后替换内部存储类型时，不会连同 source-facing 指令契约一起删掉。`execute(...) const` 是否成立仍会在为具体类型实例化 RTTI 包装函数时自然检查，不额外建立只为筛选候选重载的 concept。

wrapper 提供普通指令具有的 `context_type` 和 `execute(...) const`，因而保持鸭子类型；它的通用构造接收 `const Instruction&`，让同类型 wrapper 始终优先使用默认复制/移动构造。`any_instruction` 仍保留更特化的 wrapper 构造重载，借助公开显式转换直接复制内部擦除值，避免把 wrapper 再擦除一层。这个构造完整定义在类内，转换代换依赖模板参数，不需要等待 wrapper 完整定义，也不需要 friend。

内部 `any_instruction` 和非持有 view 都声明 `context_type = void`。它表示原有静态 Context 已经未知，因而与任何实际程序 Context 都不匹配；`instruction_compatible_with` 不需要识别某个特殊具体类型。没有 `context_type` 的普通指令仍表示 Context 无关；声明了 Context 的普通指令和 wrapper 则只匹配同一 Context。

`program_item` 本身不再重复携带 Context。它由哪个 `programs(std::type_identity<Context>, ...)` 重载返回，就属于哪个 Context。编译器逐个用程序段元素构造 `any_instruction_for<Context>`：原始具体指令在这里接受静态 Context 检查，已经擦除为同 Context wrapper 的元素则走普通复制或移动。内部 `any_instruction` 不再重复检查，也不保存 Context。

### 定义源程序声明

data 的形制不受限制。data 是源自己的实现细节，编译器只通过 source 声明的 setter 写入依赖和程序入口；handler 仍然直接拿到具体 `data_type const&` 使用其中字段。这样既保持专用 data 的性能，也让响应函数写法直观。

程序声明应模仿当前依赖声明，而不是给程序命名。它是按 Context 探测的可选 source 接口：

```cpp
auto programs(std::type_identity<Context>, const data_type& data) const;
```

这里的成员、静态、const 性和具体返回类型不需要规定死，编译层像现有依赖接口一样用约束探测并保留普通重载解析。关键契约是：调用发生在依赖 dense id 已写入 data 之后，传入 `const data_type&`；定义源可以据此把依赖 id 直接存入指令。程序不再携带待编译的名称字符串，也不需要第二套名称到指令参数的映射。

`programs(..., data)` 可以读取 `make_data()` 留下的源配置、脚本状态和已经解析的 id，但不能依赖 data 中正在由编译器逐项写回的其他 `program_entry`。入口字段在程序阶段开始时都应视为 null；这个限制避免程序声明顺序影响编译结果，也使所有 Context 的程序可以按任意稳定顺序写入。若以后确实需要固定程序之间的显式调用，应单独设计编译期引用或重定位，不借用半完成 data 隐式传递入口。

`programs(...)` 的外层结果可以是 tuple-like 或只允许顺序遍历一次的 range。每个 `program_item` 包含：

1. 一段 tuple-like 或可单遍遍历的 range。tuple-like 的元素可以是不同的原始具体指令；range 可以保存同一种具体指令，也可以保存 `any_instruction_for<Context>` 以表达运行期异构指令。两种形态也允许直接包含同 Context wrapper，只要每个元素都能构造 `any_instruction_for<Context>` 即可。
2. 一个把编译后的 `program_entry<Context>` 写入 data 的 setter。

setter 和依赖 setter 一样可以是成员对象指针，也可以是任意合适的小型调用对象，不擦除成预先规定的函数类型。下面是概念示例，指令名称不构成最终 API 承诺：

```cpp
auto programs(std::type_identity<damage_effect>, const data_type& data) const
{
    return std::tuple{
        program_item{
            std::tuple{
                increase_damage_from{ data.related_definition },
                consume_self_count{}
            },
            &data_type::damage_entry
        }
    };
}
```

编译器只遍历每段序列一次，把每个元素转换成带 Context 保证的 wrapper 后立即复制内部表示到最终不可变程序，不强迫先构造中间 `vector<any_instruction>`。native source 可以直接返回 `std::tuple{instruction_a{}, instruction_b{}}` 或具体指令的数组、view/generator；需要运行期异构性的 adapter 可以自行选择 `std::vector<any_instruction_for<Context>>`。即使源数据来自编译期常量，指令仍必须在最终程序中拥有存储，因此这里不要求通过模板参数或规避 ODR-use 来制造“零复制”形式；接口只保证不会额外制造一次不必要的程序段搬运。

返回 tuple-like/range 而不是接收 recorder/emitter，还限制了定义源的权限：它只能描述程序内容，不能观察最终写入位置、回退编译器状态或直接修改目标程序。source 实例仍由 `definition_source_view` 保存并通过擦除表回调，因此持有 Lua state 等状态的 adapter 可以参与生成；这个状态也可以由 `make_data()` 放入 data，再由 `programs(..., data)` 读取。

Lua 等动态 adapter 同样按 Context 提供重载，并产生同构序列：

```cpp
std::vector<any_instruction_for<E>>
programs(std::type_identity<E>, const data_type& data) const;
```

脚本 opcode 可以在 definition 编译时通过 variant visitor 或 Context 专用工厂映射到具体 C++ 指令，再构造 `any_instruction_for<E>`。visitor 对兼容的具体类型实例化 wrapper 构造；不兼容的分支通过 `if constexpr` 报 Lua definition error。这样 C++ adapter 不能产出类型错误的 wrapper，而脚本文本中的非法组合也会在 definition library 发布前被拒绝。Lua 不需要 tuple，也不需要把 Context 元数据保留到最终固定程序。

### data 的编译阶段

data 确实经历三个逻辑阶段，但不需要把它们变成三个公开状态类型：

1. `make_data()` 产生初始 data，此时可以只设置 Lua state 指针或定义源私有配置。
2. 编译器通过已经完整生成的 `issued_id_map` 解析名称、标签和按标签依赖，把所有 dense id 写入 data。
3. 编译器把该 data 以 `const data_type&` 传给各 Context 的 `programs(...)`，直接把序列写入最终固定程序，再通过各自 setter 把 `program_entry<Context>` 写回 data。

只有完成第三阶段的 data 才会被放入 `definition_library` bucket，并在运行时作为 `const definition_data&` 暴露。前两个中间状态只是 `compile()` 内部局部对象，既不能进入 table，也不能被 handler 观察。这样避免为任意 source 自定义的 `data_type` 增加 typestate 包装，也不需要把依赖字段和程序入口拆成另一套平行存储。

上述三阶段只描述单个 source data。整个 library 的编译先生成完整 `issued_id_map`，再构造唯一的 `definition_library`。固定程序构造时先写入位置 0 的不可执行占位和位置 1 到 3 的三条内部 `end_game`，紧接着从固定位置 4 写入根初始化段和回合段并补入回合尾部回跳。此后所有定义程序直接追加到同一存储，并在各自 data 完成第三阶段后发布到 bucket；最终才发布整个 `definition_library`。根入口由固定位置构造，不记录随编译顺序变化的 `root_entry` 状态。

这里“把 data 放入 bucket”仅指在单个定义编译完成后，把完整 data 移入编译结果；不是向 data 追加字段或字节。整个 `definition_library` 仍在局部构造完成后一次性返回，若任一定义程序或根流程生成抛出异常，不会发布半完成的 data、入口或程序。

固定程序 handler 读取完整 data 中已有的强类型入口并返回它。费用 handler 若返回非 null onpay 入口，费用广播从费用 event 读取 `cost_effect_argument` 并与入口一起写入 onpay 矩阵。当前七圣召唤规则下，onpay 只需要读取自己实际减少的费用；以后若出现新的必要信息，优先扩充 `cost_effect_argument`，不允许 onpay 任意向下读取已经结束的费用广播或更深的 action frame。

### 指令擦除与存储

第一阶段直接复用现有固定 64 字节存储约束。`any_instruction_for<Context>` 只组合一个内部拥有型擦除值，Context 是纯编译期参数，不增加 Context key、查询函数或其他逐对象数据。具体指令继续要求可平凡复制、可平凡析构并满足大小和对齐限制，这些要求由 wrapper 构造检查；内部 RTTI 不需要 `copy_payload` 或析构函数指针，payload 通过 `memcpy` 写入，容器复制也使用平凡复制语义。

`instruction_rtti` 和每个具体类型的 `instruction_rtti_of<T>` 位于 `detail` 命名空间，不再嵌套在拥有型擦除对象中。公开自由变量 `instruction_type_index<T>` 直接引用同一个 RTTI 地址，外层无需知道 view 的具体类型即可取得索引。RTTI 的 execute 包装函数接收 `const void*` payload；当前非持有 view 自身保存 `const instruction_rtti*` 和 `const unsigned char*`，其 `is`、`as`、`type_index` 与 `execute` 直接使用这两个指针。拥有型 `any_instruction` 只保存 RTTI 与 payload，不提供平行的类型查询或取址接口；它保留直接 `execute`，`any_instruction_for<Context>` 执行时无需先转换成 view。到 view 的转换只用于 table 暴露非持有观察结果。

Context 安全只建立在一个方向的边界上：程序段中的每个具体指令必须能构造兼容的 `any_instruction_for<Context>`，同 Context wrapper 可以复制或移动，再由 `any_instruction` 的专用构造重载降级复制其中已经擦除的表示；未分型的内部表示不能反向升级为某个 Context wrapper。`definition_library` 的私有编译逻辑统一经过这个边界，降级后不再检查 Context，executor 热路径也不承担兼容性分支。

固定程序是不可变且可共享的，RTTI 包装函数和 view 的 `execute(...)` 都只读取 payload；具体指令也统一通过 `execute(...) const` 调用。

具体指令的 `execute(...)` 也统一为 `const`。当前需要重入的指令已经把 stage 和运行期数据放在 stack/table 中，未发现必须修改指令对象本身的语义；现有非 const 签名属于动态程序时期遗留约束。若将来某条指令真的需要自修改，它应把可恢复状态移入 stack，而不是破坏固定程序共享性。

`detail` 下“每个具体类型一个静态 RTTI + 自动生成 execute 包装函数”的机制就是所需分派连接。RTTI 只需保存 execute 函数指针；包装函数在具体指令可见处实例化并最终调用 executor 模块中的 `TInstruction::execute`。不保存 `context_type`，不提供 `usable_in<Context>()`，也不手写 descriptor 表或同步“定义源可生成的指令列表”和 executor 分派列表。

这里也不区分 op 和 instruction。程序序列中的元素就是 VM 的最小领域操作；共享逻辑放在可内联的 C++ helper 中。把一个语义操作人为降低成多条可复用小指令只会增加擦除槽位、dispatch 次数和 stack 协议，不作为本次架构的一层。

编译器在每段可返回子程序末尾自动追加内部返回标志，定义源不需要看到该指令。根流程单独组装：初始化段和回合段之间没有返回标志，回合段末尾追加跳到 `round_entry` 的内部控制操作。位置 0 的占位、位置 1 到 3 的 `end_game`、返回标志和根回跳都由内部组装路径生成，不接受定义源提供的裸执行位置。未来可以把回跳融合进回合末尾指令，也可以把内部 `any_instruction` 的固定槽替换为变长紧密存储；`any_instruction_for<Context>` 的构造语义、`program_entry<Context>`、source sequence 和唯一执行位置地址空间都不依赖这些内部布局优化。

## 迁移落地状态

固定程序迁移已经完成以下基础工作：

- Catch2 单测与完整最小对局烟测已经建立。
- 所有具体指令和内部擦除调用均为 const 执行。
- `any_instruction` 位于 definition 可见边界，`any_instruction_for<Context>` 在构造时静态检查上下文。
- `definition_library` 拥有唯一 `fixed_program`，其中包含固定前缀、根流程、definition 程序段、内部返回和回跳指令。
- 定义源通过 `programs(std::type_identity<Context>, const data_type&)` 提供 tuple-like 或 range 程序段，编译器直接追加到最终程序。
- 普通广播和费用/onpay 均使用 handler 返回的强类型固定入口。
- 伤害吸收、完整伤害/元素链、主动切人以及最小游戏根流程已经迁移。
- `schedule_fn`、六参数 handler、运行期候选分支、动态指令队列、队列恢复位置和执行来源标签均已删除。
- executor 只保存执行位置与 stack；外部观察通过 `table.instruction(executor.position())`，正常执行通过 `executor.execute_next(table, random)`。

后续若继续优化，应作为独立工作评估回跳融合、变长指令存储、程序去重和调试元数据，不再恢复运行期程序生成能力。

Catch2 放在行为迁移之前，因为测试框架本身是低风险、独立的构建改动，并能给后续每个小提交提供回归边界；不在这一步顺便建立庞大的规则测试体系。单测与头文件的一一映射表示测试所有权，不强制测试只能 include 同名头：当前向模板需要后续具体类型才能实例化时，可以直接引入有限的相关头文件，但不应无理由改用聚合全头文件。

仍可推迟到实现时决定的只是局部命名与优化，例如回跳是否与回合末尾指令融合、固定程序容器的分块策略和调试信息格式。已确定的架构边界是：一次 definition 编译只发布一个包含全部 definition 程序与根流程的固定程序；位置 0 是 null 占位，位置 1 到 3 是结果由位置编码且永不返回的终局指令，根入口静态固定在位置 4 且不另存字段，definition 程序排在根程序之后；`execution_position` 继续是现有整数别名；`program_entry<Context>` 可以表示 null、普通 Context 入口或 Context 无关的终局入口；定义源程序段可以是异构原始指令 tuple-like，也可以是具体指令或 `any_instruction_for<Context>` 的单遍 range；每个元素在追加边界构造同 Context wrapper 并完成静态检查；内部表示不再保存 Context；data 只在完整后发布；运行期参数进入明确栈 ABI；handler 返回入口；table 修改只由固定程序中的指令完成；根回合段无限回跳并在终局时保留 stack、只把执行位置跳到相应 `end_game`。

[返回架构总览](../architecture.md)
