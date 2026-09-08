# Table 内部数据结构设计记录

本文记录 `card_table` 内部实体存储结构的目标、约束、候选方案、已否决方案，以及第一版实验实现的取舍。它讨论的是内部布局与生命周期，不规定卡牌或 status 的具体规则语义。

## 1. 问题边界

当前大多数实体按所属对象分别存放在 `std::vector` 中。这对角色、召唤物等数量少且容器数量固定的实体没有明显问题，但卡牌带来两个不同的问题：

1. 牌堆允许在任意位置插入、按牌堆位置访问和重排。直接用 `vector<card_data>` 表示牌堆时，中间插入会改变后续卡牌的地址和 ID。广播会先在栈上记录响应者实体 ID 快照、以后再推进响应，所以位置型 ID 可能使已排队的响应错误地落到另一张牌上。
2. 每张手牌和牌堆牌以后都可以拥有少量 card status。若每张牌各自持有一个 `vector<status_data>`，会产生大量独立的小分配。这里真正关心的是访问速度和缓存局部性；容器对象或尾端空位本身占多少字节只是次要因素。

本文将新实体简称为 **status**。它是附着于卡牌的效果状态，不是角色的 `attachment`，也不是玩家的 `combat_status`。

## 2. 设计约束

### 2.1 实体与上下文

实体是“数据 + 上下文”。同一份卡牌数据位于牌堆和位于手牌时，是两个不同的实体：

- 能响应的事件集合可能不同；
- 广播顺序可能不同；
- 旧的牌堆卡 ID 在抽入手牌后必须立即失效；
- 卡上的 status 也随卡牌上下文改变，旧上下文的 status ID 同样必须失效。

因此，强类型的 `deck_card_id` 与 `hand_card_id` 不能只在类型名上不同，查询时还必须验证其上下文。

### 2.2 裸数据层与实体访问层

table 模块分为两层：

- 裸数据层：`table_storage`、`player_data`、`card_data`、`status_data` 等只保存数据和内部容器，不提供面向用户的行为接口；
- 访问层：`card_table` 与 `xxx_entity<TStorage>` 共同把裸数据解释为“数据 + 完整上下文”的实体。

`xxx_entity<TStorage>` 是内部模板名，`xxx_view` 是只读别名，例如：

```cpp
template<class TStorage>
class character_entity;

using character_view = character_entity<const detail::table_storage>;
```

这样可以避免 `handle` 同时表示“事件处理”和“实体句柄”。事件响应仍使用 `handle` 语义，例如 `handle_fn_t` 和 `can_handle<TEvent>()`；实体访问对象则统一叫 entity/view。

### 2.3 table_accessor 与强耦合边界

`card_table`、所有 `xxx_entity<TStorage>` 和 `card_status_range` 本质上是同一组 table 内部机制。为了热路径效率，它们需要直接读写原始 storage，而不是通过公开构造函数或防御性包装层反复绕路。例如从玩家实体遍历手牌时，最有效的形式是直接把 `table_storage*`、父级 `player_data*` 和子级 `card_data*` 填入新的实体访问对象。

因此 table 模块使用 `detail::table_accessor` 作为单向桥：

- `make_uninitialized<TEntity>()` 构造一个尚未填充 storage 的实体访问对象；
- `storage_of(entity_or_table)` 打开 `card_table` 或实体访问对象的私有 `storage_`；
- 只有 `card_table`、实体访问对象和 range 被声明为 friend；
- 外部用户不能取得 `table_storage`，也不能随意构造一个有效实体。

这不是为了隐藏 table 家族内部的实现细节，而是为了把强耦合限制在一个有规律的内部边界中。public 接口仍按实体能力表达，private/detail 命名则应暴露真实实现，以便维护和优化。

`make_uninitialized` 不能写成 `return {};`，因为空花括号会 value-initialize 返回对象，并把其 storage 零初始化；这与“稍后立即填充 storage”的设计相反。直接写 `TEntity entity; return entity;` 虽表达默认初始化意图，但会依赖 NRVO 避免复制未初始化 storage。当前实现使用仅 `table_accessor` 可调用的 private tag 构造，直接构造返回对象本身；调用者必须立刻通过 `storage_of` 写入完整 storage，并且在写入前不得观察该对象。

### 2.4 生命周期与安全点

本项目不采用带 generation 的通用 slot map。执行过程中实体只会被标记删除或从所属链/顺序表中脱离，不复用其槽位。只有在 executor stack、日志和展示任务都不再保存旧 instruction/event/entity ID 的安全点才调用 `table.clean_up()`；终局时可先观察现场，再调用 `executor.clear()` 建立这一边界。

这意味着：

- 一次完整结算期间 ID 稳定；
- cleanup 可以重排槽位并使旧 ID 整体失效；
- 不需要为每个槽位长期保存 generation；
- cleanup 以前不能复用被删除槽位。

### 2.5 status ID 保存 owner

status 节点不应反复保存相同的 owner。owner 属于引用的上下文，应放进临时的 ID：

```cpp
struct hand_card_status_id {
    hand_card_id card;
    size_t index;
};

struct deck_card_status_id {
    deck_card_id card;
    size_t index;
};
```

ID 稍大没有实际问题：项目不会长期保存所有实体 ID。这样节点池只保存 status 自身数据和链接，查询时还能用 owner 检查手牌/牌堆上下文。

## 3. 牌堆与手牌的候选方案

### 3.1 方案 A：统一卡牌槽位池 + 两个顺序表

每名玩家保存一个卡牌槽位池，手牌和牌堆分别保存槽位索引顺序：

```cpp
enum class card_area { deleted, hand, deck };

struct card_slot {
    card_data data;
    card_area area;
};

std::vector<card_slot> card_slots;
std::vector<size_t> hand_order;
std::vector<size_t> deck_order;
```

`hand_card_id` 和 `deck_card_id` 都指向槽位，但有效性只需分别判断 `area == hand` 和 `area == deck`。区域标记不是在定义 ID 检查之外增加第二次检查，而是取代定义 ID 作为卡牌生命周期标记。抽牌时只修改两个顺序表与 `area`，无需移动 `card_data` 或 status 链首尾。

优点：

- 手牌与牌堆之间转移不搬动卡牌数据；
- 中间插牌和重排只移动小索引；
- 旧上下文 ID 会因 `area` 改变而自然失效；
- 所有卡牌统一管理，cleanup 逻辑一致。

缺点：

- 遍历手牌也变成 `order -> slot -> data` 的间接访问；
- 手牌本来不需要任意位置插入或按槽位保持稳定，这部分间接访问没有换来相应能力；
- 实现会同时改动手牌与牌堆的现有访问路径，第一轮实验范围较大。

该方案仍然值得基准测试。每名玩家只有几十张卡牌时，槽位池很可能整体常驻缓存，间接访问的代价未必明显。

### 3.2 方案 B：牌堆稳定槽位池 + 牌堆顺序表，手牌保持连续数组

牌堆保存两个数组：

```cpp
std::vector<card_data> deck_card_datas; // 稳定槽位，cleanup 前不移动
std::vector<size_t> deck_card_order;    // 牌堆位置 -> 槽位
std::vector<card_data> hand_card_datas; // 继续尾插、标记删除
```

牌堆中间插入只在 `deck_card_order` 中插入索引，新卡数据追加到槽位池尾部。`deck_card_id.index` 指向槽位而不是牌堆位置。抽牌时将数据及其 status 链首尾移入手牌尾部，原牌堆槽位立即标记删除；于是旧 `deck_card_id` 失效，并产生新的 `hand_card_id`。

优点：

- 修复牌堆 ID 稳定性；
- 保留手牌连续遍历，不为手牌引入多余间接访问；
- 牌堆的按位置访问、随机插入和重排只操作索引；
- 相比统一槽位池，更接近现有实现，适合作为第一轮实验。

缺点：

- 抽牌、替换手牌时仍需移动 `card_data`；
- 手牌与牌堆使用不同内部布局，部分模板代码不能完全共用；
- 转移时必须明确旧槽位失效、新实体建立，不能把它误写成同一实体继续存在。

第一版实现选择此方案。它优先修复已经存在的正确性问题，同时保留后续与方案 A 做真实性能比较的空间。

### 3.3 牌堆使用纯索引链表

链表可以稳定节点 ID并高效插入，但牌堆是唯一经常需要“第 N 张”“牌堆顶第 N 张”这类按序号访问的容器。为获得序号需线性遍历，而牌堆总共只有两个，没有聚合大量小容器的需求。因此 `稳定槽位 + vector<slot_index>` 更直接：任意位置插入仍只搬小索引，序号访问保持 O(1)。

## 4. status 容器的候选方案

### 4.1 方案 S1：扁平节点池 + 单向索引链表

所有卡牌共用一个 table 级节点池：

```cpp
struct status_slot {
    status_data data;
    size_t next;
};

std::vector<status_slot> status_slots;
```

每张卡只保存 `first_status` 与 `last_status`。status 通常只有零至四个，删除时可从 owner 所指卡牌的链首扫描前驱，然后立即断链并把 `definition_id` 标记无效。追加利用 `last_status` 保持 O(1)。

`status_data::definition_id` 已经承担有效/删除标记，所以不再让 `next` 兼职删除标记。节点也不保存 owner；owner 只存在于 ID 中。

优点：

- 实现简单；
- 所有 status 数据只有一次连续分配；
- 不再有每张牌一个小 vector 的独立分配；
- ID 可直接索引节点，删除后仍能用定义 ID 判无效；
- 存储层以后可在不改变实体语义的前提下换成分段池。

缺点：

- 同一张牌的节点只在连续追加、没有交错创建时才连续；
- 遍历每个节点都要读取 `next`，跨卡交错创建后局部性一般；
- 任意 status 的删除需要扫描很短的 owner 链寻找前驱。

第一版实现选择此方案，并采用“只在尾部追加，安全点原地 compact”的策略。

### 4.2 方案 S2：固定容量分段的索引链表

节点池的单元改为固定容纳 N 个 status 的段：

```cpp
template<size_t N>
struct status_segment {
    size_t next;
    status_data datas[N];
};

struct status_list {
    size_t first_segment;
    size_t last_segment;
    size_t tail_used;
};
```

`tail_used` 只在整张表处保存一次，不应每段重复保存。N=2 与 N=4 都值得测试：若绝大多数卡只有一至四个 status，一次加载一个段能减少逐节点追链，主要收益是缓存局部性，而不是少存几个 `next` 所节省的字节。

段的元数据与数据应优先保持同一个 AoS 块。把所有 `next` 放在一个数组、所有数据放在另一个数组，会把访问同一段需要的数据拆到两片内存中；虽然单个数组各自连续，却恶化了实际遍历路径的局部性。

删除可继续使用 `status_data::definition_id` 标记空洞。`alive_mask` 能减小标记空间，但每次访问增加位运算，未证明更快以前只应作为基准测试变量。

#### 单整数直接索引

若 ID 保存“段索引 + 段内索引”，访问需要两级地址计算。可将 ID 改为从池基址开始的相对字节偏移：

```cpp
address = pool_base + id.byte_offset;
```

这样段头仍与数据相邻，ID 仍为单整数，也无需为了统一索引而把段头填充到一个 status 大小。vector 重新分配只改变基址，不改变相对偏移；cleanup 本来就允许旧 ID 失效。

优点：

- 一张牌的常见少量 status 更可能在同一缓存线或相邻缓存线；
- 每 N 个数据只追一次段链接；
- 仍然把大量小容器合并进一个池。

缺点：

- 分配段、处理段内空洞、遍历尾段和 compact 都更复杂；
- 最后一段存在少量未使用槽位，但这不是主要问题；
- N 是实际工作负载相关参数，需要基准测试而不能仅凭结构大小决定。

该方案是扁平链表之后最值得尝试的优化方向。

## 5. cleanup 策略

### 5.1 不使用 free list

本设计不让“保留 free list”和“每次 compact 后无 free list”两种模式共存。只要 free list 是正常分配路径的一部分，即使它经常为空，每次添加也必须多一次判断与数据依赖。既然删除槽位在安全点前不能复用，第一版直接让新增永远尾插，cleanup 后仍然永远尾插。

### 5.2 保留 capacity 的原地填洞

cleanup 不默认分配一个大小刚好的新数组。曾经达到过的容量通常以后还会再次需要；丢弃容量会让后续扩容重新付出分配和搬移代价。`resize(live_count)` 只缩短 size，不调用 `shrink_to_fit`。

对单向索引链表，原地填洞并非简单搬一次节点就结束。可使用以下 O(n) 时间、O(1) 额外空间流程：

1. 从前向后寻找空洞，从后向前寻找仍存活的节点；
2. 把尾部存活节点搬到前部空洞；
3. 在旧源槽的 `next` 中临时编码 `moved_to(new_index)`；
4. 全部搬完后扫描保留下来的节点，通过旧源槽的转发表记重写每个 `next`；
5. 扫描所有卡牌，重写每条链的 `first_status` 与 `last_status`；
6. `resize(live_count)`，保留 capacity。

其复杂度为 O(槽位数 + 卡牌数)，但常数包括双向扫描、节点移动、全体链接重写以及对旧源槽的随机读取。它只消除空洞，不保证同一 owner 的节点重新聚集。若基准显示链遍历局部性比 cleanup 开销更重要，可以另测“按 owner 重建并暂时聚集节点”的版本，但不应默认接受其新分配与容量丢失。

牌堆稳定槽位池也可用相同的“尾部存活槽填前部空洞”思想；每搬一个槽，只需在牌堆顺序表中把旧槽位索引替换成新索引。

## 6. 已否决或暂不采用的设计

### 6.1 generation slot map

通用实时对象池常用 generation 防止槽位复用后旧 ID 指向新对象。本项目在结算期间不复用槽位，cleanup 又是明确的 ID 失效安全点，因此 generation 只增加 ID、槽位和检查成本，没有解决当前存在的问题。

### 6.2 每张卡一个 vector 或“稀疏组”

status 不一定稀有到只有少数卡拥有；很容易出现几乎每张手牌都有一两个 status。每卡一个 vector 会制造大量独立分配。所谓稀疏组若仍为每个非空 owner 建一个独立容器，只是在空卡上省掉 vector 对象，未解决大量小块内存和访问分散问题。

### 6.3 只池化 data、每张卡仍保存 vector<index>

这消除了 status 数据本身的小分配，却仍保留大量索引 vector 及其分配，整体接近“原来存数组地址的数组改成存数据索引的数组”。它没有同时解决“池化”和“合并大量小容器”两个目标，提升有限。

### 6.4 持久的通用 relocation map

当前实体之间需要维护的存储关系只有父子关系。cleanup 时为所有可能引用建立并长期保存通用 old-ID -> new-ID 映射属于伪需求。必要的重写应局限于 cleanup 内部，并只更新实际存在的父子链接或顺序索引。

### 6.5 free list 与 compact 混合

即使 free list 为空，它也会污染每次新增的热路径。若选择 free list，就应接受其始终存在并据实基准；若选择 compact，就应让新增无条件尾插。第一版选择后者，不做自适应混合。

### 6.6 每次重新分配恰好大小的新池

该方式能按 owner 重建出暂时连续的节点，但要分配新内存、搬移所有元素，并丢弃很可能再次需要的容量。除非基准证明重新分组的遍历收益明显超过这些成本，否则不作为默认 cleanup。

### 6.7 分段池的元数据/数据分离数组

它可以用单个段索引分别访问两个数组，但同一段的信息被拆成两块内存，实际操作往往同时需要段链接和数据。相比元数据紧邻数据的 AoS 段，它的局部性更可疑，暂不优先。

### 6.8 统一固定大小 cell

把段头填充为一个 status 单元大小，可让段头和数据共享单一 cell 索引，但会引入无意义填充。相对字节偏移同样提供单整数和一次基址加法，不需要这种空间换索引的做法。

### 6.9 牌堆位置直接作为实体 ID

中间插牌、洗牌或抽牌都会改变位置。只要响应者快照中保存了稍后才处理的 `deck_card_id`，位置 ID 就可能在执行前指向另一张牌，因此这是正确性问题而非单纯性能取舍。

### 6.10 共用卡牌槽位但不验证区域

牌堆牌和手牌牌是不同上下文的实体。若共用槽位却没有 `area` 一类上下文标记，抽牌后旧 `deck_card_id` 仍会命中数据，连带旧的 deck status ID 也无法失效。统一槽位方案只有在区域检查取代原生命周期检查时才成立。

## 7. 第一版实验实现与替换边界

第一版采用以下组合：

- 手牌：现有连续 `vector<card_data>`，尾插、标记删除、安全点清理；
- 牌堆：稳定 `vector<card_data>` 槽位池 + `vector<size_t>` 牌堆顺序；
- status：table 级扁平 `vector<status_slot>` + 每张卡的单向索引链首尾；
- 分配：一律尾插，无 free list；
- 清理：安全点原地填洞并保留 capacity；
- ID：牌堆卡 ID 指向槽位，status ID 同时保存 owner ID 与节点槽位。

status 使用独立的 `status_definition`，并按上下文生成 `hand_card_status_view` 与 `deck_card_status_view` 两套响应函数表。定义源边界相应提供可选的 `status_dependencies()` 与 `status_dependencies_by_tag()`。默认广播顺序为卡牌在前、该卡链中的 status 在后；实际规则需要其他顺序时仍可由具体指令显式采样并压入对应顺序的广播目标快照。

这一版的目标不是提前宣称最终性能最优，而是建立正确、简单、可测试的基线。后续至少应比较：

1. 混合卡牌布局与统一卡牌槽位布局的广播、抽牌、换牌和普通手牌遍历成本；
2. 扁平 status 链与 N=2/N=4 分段链在典型“每卡 0～4 个 status”负载下的遍历成本；
3. status 交错创建、频繁删除后的缓存行为；
4. 原地填洞 cleanup 与按 owner 重建 cleanup 的总成本和后续遍历收益。

在这些数据出现以前，应把第一版视为可替换实验基线，而不是冻结的 ABI。
