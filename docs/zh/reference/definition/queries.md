[givm](../../reference.md) / [定义](../definition.md) / **查询**

# 查询

查询向一项定义取得规则信息或检查结果，例如角色初始状态、卡牌初始费用，以及当前选择的目标是否合法。定义源通过统一的静态 `query` 接口返回结果，查询参数中包含所需的全部对局信息。

每种查询以一个参数结构体表示，其中 `result_t` 指定返回类型。当 `std::is_empty_v<Q>` 为 `true` 时，结果在编译定义库时求出并保存，游戏运行期间直接读取；非空查询按调用方本次提供的参数求值。具体约定见[定义源协议](source_protocol.md#查询)。

## 查询类型

| | |
| --- | --- |
| [`character_initial_state`](queries/character_initial_state.md) | 角色初始状态 |
| [`card_initial_cost`](queries/card_initial_cost.md) | 卡牌自身的骰子费用与行动速度 |
| [`card_target_validation`](queries/card_target_validation.md) | 分步检查目标与用牌条件 |

## 相关类型与函数

| | |
| --- | --- |
| [`target_validation`](queries/target_validation.md) | 目标检查的四种结果 |
| [`supported_queries`](supported_queries.md) | 各定义类别支持的查询类型 |
| [`query_default`](query_default.md) | 定义源未提供查询时的默认结果 |
| [`definition_library::query`](../executor/definition_library/query.md) | 向定义库查询 |
