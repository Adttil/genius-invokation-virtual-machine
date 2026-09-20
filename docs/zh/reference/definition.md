[givm](../reference.md) / **定义**

# 定义

定义模块描述卡牌、角色及其他实体的规则，并准备一场对局所需的内容。这里的“定义”是同类实体共用的规则；牌桌上的某张卡牌、某个召唤物是采用定义的实体，各自的当前状态另由[牌桌模块](table.md)保存。

定义源适合按游戏内容逐项编写；源库负责汇集、访问和选择所需定义，并建立名称与 ID 的映射。[执行模块的编译函数](executor/compile.md)将定义源和对局流程编译为对局使用的定义库。初次编写卡牌或角色效果可以先阅读[定义源协议](definition/source_protocol.md)。

## 类

### 定义的准备与使用

|  |  |
| --- | --- |
| [`definition_source_library`](definition/definition_source_library.md) | 可供编译的定义源集合 |
| [`definition_source_view`](definition/definition_source_view.md) | 定义源的只读视图 |
| [`program_entry`](definition/program_entry.md) | 响应效果的入口 |

### 名称、ID 与标签

|  |  |
| --- | --- |
| [`issued_id_map`](definition/issued_id_map.md) | 定义名称、分类标签与 ID 的对应表 |
| [`tag_mask`](definition/tag_mask.md) | 分类标签集合 |

### 定义类别与响应映射

|  |  |
| --- | --- |
| [`views_of_definition`](definition/views_of_definition.md) | 定义对应的实体形态 |
| [`subscribed_events`](definition/subscribed_events.md) | 实体形态可响应的事件 |
| [`supported_queries`](definition/supported_queries.md) | 定义类别支持的查询 |

## 类型别名

|  |  |
| --- | --- |
| [`any_command`](definition/any_command.md) | 核心命令 variant |
| [`definition_selection`](definition/definition_selection.md) | 按类别指定的定义名称集合 |
| [`definition_types`](definition/definition_types.md) | 全部定义类别 |
| [`support_subscribed_events`](definition/support_subscribed_events.md) | 场上实体共用的事件列表 |
| [`definition_data`](definition/definition_data.md) | 已编译定义的数据对象 |
| [`handle_fn_t`](definition/handle_fn_t.md) | 统一的事件响应函数指针类型 |

## 函数

|  |  |
| --- | --- |
| [`link_deck`](definition/link_deck.md) | 按名称准备牌组 |
| [`query_default`](definition/query_default.md) | 定义源未提供查询时的默认结果 |

## [命令](definition/commands.md)

组合游戏流程和实体响应效果的核心操作。

## [事件](definition/events.md)

对局中的响应时机与事件数据。

## [查询](definition/queries.md)

角色初始状态、卡牌初始费用与当前行动参数的合法性等规则信息。
