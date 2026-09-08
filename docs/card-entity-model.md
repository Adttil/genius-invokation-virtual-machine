# 牌与区域实体模型

保存牌实体的区域只有牌堆和手牌。“弃牌”是销毁牌实体的操作，不会把牌移动到一个弃牌区；“挑选”则是临时选择流程，不是牌区域。牌还可能携带附着状态。模型需要同时满足两个方向：

- 从事件响应角度，手牌、牌堆等区域的上下文不同。
- 从转移角度，牌在手牌和牌堆之间移动时，牌本身状态和牌上附着状态应原样保留。

牌实体数据同构、定义 id 同构，但运行期 view 和实体 id 必须按区域区分。

本文描述实体与转移语义，以及弃牌、挑选的设计边界。内部存储布局的取舍见 [Table 内部数据结构设计记录](table-storage-design.md)。

## Card Data

手牌和牌堆中的真实牌都使用同一种 `card_data` 结构：

- `definition_id<card_definition> definition_id`
- `card_state state`
- `size_t first_status`
- `size_t last_status`

card status 数据存放在 table 级 `status_slots` 池中，牌只保存链首尾。`status_data` 是区域无关的数据：

- `definition_id<status_definition> definition_id`
- `status_state state`（包含 `std::uint32_t count`）

因此，牌在牌堆和手牌之间移动时，不需要重建牌的状态，也不需要重映射牌上 status 的 definition id。移动的是完整 `card_data` 及其 status 链首尾，但其运行期实体 id 会从来源区域的 ID 域变为目标区域的 ID 域。

## 区域与 Entity/View

虽然 `card_data` 同构，但两个区域必须使用不同的 entity/view：

- `deck_card_entity`
- `hand_card_entity`

对应的实体 id 也必须是不同的强类型：

- `deck_card_id`
- `hand_card_id`

实体 id 保存玩家与区域内 index；entity/view 提供相应上下文的数据访问，并通过 `id()` 返回该 id。区域是实体身份的一部分，通过不同的强类型表达。

`hand_card_id.index` 标识手牌槽位；`deck_card_id.index` 标识牌堆槽位，而不是牌堆顺序中的位置。牌堆逻辑位置用于按顺序访问；插入或重排不改变 cleanup 前仍存活且未转移的牌堆实体的 ID。

ID 用于保存实体身份，entity/view 用于访问实体。需要在新增、删除或转移后继续访问实体时，按仍有效的 ID 重新取得 entity/view。ID 的有效期与 entity/view 的可用期不能混同。

两种实体仍指向相同结构的 `card_data`，并持有同一种 `definition_id<card_definition>`。这里共享的是定义 ID 类型，不是实体 ID 类型。

公共类型 `card_id` 是 `std::variant<hand_card_id, deck_card_id>`，供同步 event 描述广播当下的牌实体身份；实体离场后，同一个值可以作为历史 ID 随通知传递。若规则语义本身是“牌堆顶第 N 张”“某种定义的牌”等动态目标，后续固定程序仍应按位置、definition ID 或 tag ID 重新搜索。名称依赖在编译前声明、编译时解析，不要求对局运行期按名称查找。

## 同一张牌的转移

牌在同一 table 的区域之间转移时，保留完整 `card_data` 及其 status 链。例如 `take_top_deck_card()` 取出牌后，可以用 `add_hand_card(card_data)` 将其加入手牌。

转移语义：

1. 从源区域取出一个 `card_data`，源槽位立即标记为 invalid，源槽位中的 status 链首尾清空；牌堆还会移除对应的顺序项。
2. 将该 `card_data` 加入目标区域的新增槽位，取得目标区域的新实体 ID。
3. status 节点保留在同一 table 的池中，其 definition、状态和链顺序不变；从新牌实体访问时，使用新区域和新 owner 的 status ID。旧区域的牌和 status ID 从取出时起失效。
4. 源区域槽位保留到之后的 `table.clean_up()` safe point。

取出的 `card_data` 含有本 table 的 status 槽位索引，不是独立拥有这些节点的牌对象；不能据此把同一条 status 链挂到多张牌上，也不能直接转移到另一个 table。取出与重新加入之间不能调用 cleanup。

手牌槽位和牌堆槽位都使用无效标记，避免尚未结算的 event、stack frame 或外层观察记录所持 ID 因压缩而漂移。删除或转移会使对应旧 ID 立即成为 invalid，但不会复用它的槽位；`table.clean_up()` 才会压缩存储并使此前保存的所有槽位 ID 不再可用。牌堆位置不是实体 ID；任何跨指令的动态牌堆目标都必须重新搜索，而不能保存牌堆位置索引。

## 弃牌语义

弃牌不是区域转移。table 层的手牌和牌堆牌实体均提供 `erase()`，牌堆还提供 `discard_top_deck_card()`；这些操作同时删除牌上 status，不会自动广播弃牌事件。弃牌后不存在可通过“弃牌区 ID”继续访问的牌实体。

领域弃牌指令按规则在运行时确定目标：牌堆中的动态目标按位置、definition ID 或 tag ID 检索，不把一次对局中采样的 `deck_card_id` 固化到程序指令字段中。这不限制 table 层按该 ID 访问牌堆实体。

因此系统不需要也不应定义：

- `discard_card_id`
- `discard_card_entity`
- `discard_card_view`

真正的容器压缩仍留到 clean_up safe point。

## 挑选不是实体

“挑选”生成的候选牌通常不是来自牌堆或手牌，也不需要携带状态。它们应被建模为无状态候选项，而不是实体。

候选项可以只保存：

- `definition_id<card_definition>`
- 可选展示信息
- 可选来源信息

玩家选择后，系统才创建新的 `card_data` 并加入手牌或牌堆。

这避免了为临时候选牌分配实体 id，也避免它们参与事件响应、clean_up 和生命周期管理。

## 卡牌附着状态

卡牌本身可以携带附着状态。牌堆中的牌和手牌中的牌都可以有这些子实体，因此附着状态不只挂在手牌区域。

卡牌附着状态是 `card_data` 的子级实体列表：

- 它们随牌转移。
- 它们使用同一套 `definition_id<status_definition>`。
- 它们的状态不因区域转移而改变。
- 它们是否响应某事件，由当前区域 view 的事件订阅关系及编译库中为该 view 安装的 handler 决定。

如果某个 card status 只在手牌中响应事件，那么它的 handler 可以只注册在 `hand_card_status_view` 对应的事件接口上，而不是改变 status 的 definition id。

## 定义与上下文

同一张牌在牌堆和手牌中共享 `definition_id<card_definition>`，但定义库为两个上下文准备不同事件接口表：

- `hand_card_view` 的 handler 表。
- `deck_card_view` 的 handler 表。

这些 handler 读取同一个编译后的 card definition。区别在于各区域支持的事件组合以及调用时传入的 view 类型。

这能同时满足：

- 转移时不重映射 definition id。
- 响应事件时仍能区分手牌牌和牌堆牌。
- 热路径只按 issued id 查表，不需要 variant 包装 card handle。

## 与支援、召唤物等实体的关系

支援、召唤物、角色装备等场上实体和“对应卡牌”不是同一个运行时 data。原因是卡牌与场上实体的状态不同：

- 支援牌在手牌中通常只有卡牌状态。
- 支援区中的支援实体有计数器、持续时间或其他场上状态。
- 召唤物牌与场上召唤物也类似。

因此它们使用不同的 `data`。定义源接口按以下方式表达这种关系：

- 每个 source 通过唯一的 `definition_category` 指定一个类别，`compile(...)` 返回该类别的一项 definition；不提供单个 source 直接注册多个类别的接口。
- 例如卡牌 source 使用 `card_definition`，对应支援 source 使用 `support_view`，二者分别注册。
- `definition_id<card_definition>` 与 `definition_id<support_view>` 分属不同类别，可以使用相同名称。
- 卡牌 source 通过 `support_dependencies()` 声明支援名称，在 `compile(...)` 中用 `resolve_id<support_view>(...)` 取得关联的 ID；对局运行时直接使用编译结果。

这样可以让“牌”在牌区之间高效移动，同时不把“卡牌形态”和“场上实体形态”强行揉成一个 data。

[返回文档入口](README.md)
