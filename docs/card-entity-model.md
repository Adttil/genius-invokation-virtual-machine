# 牌与区域实体模型

七圣召唤中保存牌实体的区域只有牌堆和手牌。“弃牌”是销毁牌实体的操作，不会把牌移动到一个弃牌区；“挑选”则是临时选择流程，不是牌区域。牌还可能携带附着状态。目标设计需要同时满足两个方向：

- 从事件响应角度，手牌、牌堆等区域的上下文不同。
- 从转移角度，牌在手牌和牌堆之间移动时，牌本身状态和牌上附着状态应原样保留。

当前设计是：牌实体数据同构、定义 id 同构，但运行期 view 和实体 id 必须按区域区分。

## Card Data

手牌和牌堆中的真实牌都使用同一种 `card_data` 结构：

- `card_definition_id definition_id`
- `card_state state`（当前为空类型，暂未暴露到创建指令字段）
- `size_t first_status`
- `size_t last_status`

card status 数据存放在 table 级 `status_slots` 池中，牌只保存链首尾。`status_data` 是区域无关的数据：

- `card_status_definition_id definition_id`
- `card_status_state state`

因此，牌在牌堆和手牌之间移动时，不需要重建牌的状态，也不需要重映射牌上 status 的 definition id。移动的是完整 `card_data` 及其 status 链首尾，但其运行期实体 id 会从来源区域的 ID 域变为目标区域的 ID 域。

## 区域与 Entity/View

虽然 `card_data` 同构，但两个区域必须使用不同的 entity/view：

- `deck_card_entity`
- `hand_card_entity`

对应的实体 id 也必须是不同的强类型：

- `deck_card_id`
- `hand_card_id`

entity/view 和实体 id 共同携带玩家与区域内 index。区域是实体身份的一部分，不能用一个同构 id 再附加运行期区域枚举来区分。

当前实现中，`hand_card_id.index` 指向手牌数组位置；`deck_card_id.index` 指向牌堆稳定槽位，而不是当前牌堆顺序中的位置。牌堆按位置访问通过单独的 `deck_card_order` 完成，因此插入或重排只移动槽位索引，不改变 cleanup 前已存在牌堆实体的 ID。

两种实体仍指向相同结构的 `card_data`，并持有同一种 `definition_id<card_definition>`。这里共享的是定义 ID 类型，不是实体 ID 类型。

公共类型 `card_id` 是 `std::variant<hand_card_id, deck_card_id>`，供同步 event 描述广播当下的牌实体身份；实体离场后，同一个值可以作为历史 ID 随通知传递。若规则语义本身是“牌堆顶第 N 张”“某名称的牌”等动态目标，后续固定程序仍应按位置、定义、名称或标签重新搜索，而不是把动态目标捕获进指令字段。

## 同一张牌的转移

牌在区域之间转移时应优先移动 `card_data`，而不是按目标区域创建新 data。

推荐语义：

1. 从源区域取出一个 `card_data`。
2. 将该 `card_data` 移入目标区域。
3. 进入目标区域时生成该区域的新实体 ID；旧区域的实体 ID 立即失效。
4. 源区域槽位立即标记为 invalid，并保留到之后的 `table.clean_up()` safe point。
5. 牌上的 `card_status_data` 随 `card_data` 一起移动。

手牌槽位和牌堆槽位都使用无效标记，避免尚未结算的 event、stack frame 或外层观察记录所持 ID 因压缩而漂移。删除或转移会使对应旧 ID 立即成为 invalid，但不会复用它的槽位；`table.clean_up()` 才会压缩存储并使此前保存的所有槽位 ID 不再可用。牌堆位置不是实体 ID；任何跨指令的动态牌堆目标都必须重新搜索，而不能保存牌堆位置索引。

## 弃牌语义

弃牌不是区域转移。手牌可以由 `hand_card_id` 定向操作；牌堆中的牌必须由命令按定义或标签检索，不能把 `deck_card_id` 作为 command 参数。弃牌后不存在可通过“弃牌区 ID”继续访问的牌实体。

因此系统不需要也不应定义：

- `discard_card_id`
- `discard_card_entity`
- `discard_card_view`

真正的容器压缩仍留到 clean_up safe point。

## 挑选不是实体

“挑选”生成的候选牌通常不是来自牌堆或手牌，也不需要携带状态。它们应被建模为无状态候选项，而不是实体。

候选项可以只保存：

- `card_definition_id`
- 可选展示信息
- 可选来源信息

玩家选择后，系统才创建新的 `card_data` 并加入手牌或牌堆。

这避免了为临时候选牌分配实体 id，也避免它们参与事件响应、clean_up 和生命周期管理。

## 卡牌附着状态

新版七圣召唤中，卡牌本身可以携带附着状态。牌堆中的牌和手牌中的牌都可能有这些子实体，因此附着状态不能只挂在手牌区域。

设计上应把卡牌附着状态视为 `card_data` 的子级实体列表：

- 它们随牌转移。
- 它们使用同一套 `card_status_definition_id`。
- 它们的状态不因区域转移而改变。
- 它们是否响应某事件，由当前区域 handle 和定义库中的 context handler 决定。

如果某个 card status 只在手牌中响应事件，那么它的 handler 可以只注册在 `hand_card_status_view` 对应的事件接口上，而不是改变 status 的 definition id。

## 定义与上下文

同一张牌在牌堆和手牌中共享 `card_definition_id`，但定义库为两个上下文准备不同事件接口表：

- `hand_card_view` 的 handler 表。
- `deck_card_view` 的 handler 表。

这些 handler 表可以指向同一个 compiled card instance data。区别只在于调用时传入的 view 类型不同。

这能同时满足：

- 转移时不重映射 definition id。
- 响应事件时仍能区分手牌牌和牌堆牌。
- 热路径只按 issued id 查表，不需要 variant 包装 card handle。

## 与支援、召唤物等实体的关系

支援、召唤物、角色装备等场上实体和“对应卡牌”不是同一个运行时 data。原因是它们在不同 manifestation 下状态差异很大：

- 支援牌在手牌中通常只有卡牌状态。
- 支援区中的支援实体有计数器、持续时间或其他场上状态。
- 召唤物牌与场上召唤物也类似。

因此它们不应强行使用同一 `data`。更合理的设计是：

- 一个定义源可以注册多个类别的定义 aspect。
- 例如同一个源对象可以生成 card definition 和 support definition。
- card definition id 与 support definition id 分属不同 definition set。
- 二者的关联通过未编译源层的依赖或编译期解析建立，而不是运行时按名称查找。

这样可以让“牌”在牌区之间高效移动，同时不把“卡牌形态”和“场上实体形态”强行揉成一个 data。
