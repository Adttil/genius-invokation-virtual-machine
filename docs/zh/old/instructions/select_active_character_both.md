[返回](../events.md)

# select_active_character_both

准备并推进开局双方出战角色选择。流程提供两个输入挂起点，分别等待双方写入栈顶 `character_id` 输入槽；双方都提交后，再次执行本指令时同时设置双方出战角色并准备广播。

## 字段

无字段。阶段由栈顶 frame 的 `stage_t` 后缀表达。

## 执行

本命令使用栈顶 `stage_t` 区分下一次进入时应如何解释栈顶；选择结果仍保存在 executor stack。

### `stage == prepare`

本阶段不要求特定栈顶布局。执行时压入一个 frame：

```cpp
stack.top<character_id, character_id, stage_t>();
```

约定结构化绑定为：

```cpp
auto&& [first_selection, input_selection, stage] =
    stack.top<character_id, character_id, stage_t>();
```

`first_selection` 暂不读取；`input_selection` 是 `stage_t` 前的外层输入槽，初始为玩家 0 的 0 号角色。随后命令将栈顶 stage 改为 `first` 并返回 `false`，当前执行位置仍指向本指令。

执行后可保证：

```cpp
stack.top<character_id, character_id, stage_t>();
```

### `stage == first`

进入本阶段时假定栈顶仍是：

```cpp
stack.top<character_id, character_id, stage_t>();
```

且外层已经按协议写好尾部 `input_selection`。

1. 将 `input_selection` 复制到 `first_selection`。
2. 将 `input_selection` 重置为另一名玩家的默认输入槽。
3. 将栈顶 stage 改为 `second`。
4. 返回 `false`，当前执行位置仍指向本指令。

执行后栈顶布局不变：

```cpp
stack.top<character_id, character_id, stage_t>();
```

### `stage == second`

进入本阶段时假定栈顶仍是：

```cpp
stack.top<character_id, character_id, stage_t>();
```

且外层已经按协议写好尾部 `input_selection`。

1. 将 `first_selection` 与 `input_selection` 视为双方选择，要求二者属于不同玩家。
2. 弹出整个选择 frame；执行后栈顶恢复为本指令第一次执行前的布局。
3. 先将双方出战角色都写入 table。
4. 再按玩家 0、玩家 1 的顺序推进 [`active_character_changed`](../events/active_character_changed.md) 广播。

## 注意

- 本命令主要用于开局双方同步选择出战角色。
- 广播第一个 `active_character_changed` 前，双方出战角色都已经写入 table。
- 两次广播的响应者列表都在双方出战角色写入后、第一次广播开始前采样；第一次通知新创建的实体不会加入第二次通知的响应者列表。
- `stage_t` 前的完整 `character_id` 是唯一外层输入槽；`first_selection` 与 stage 由核心指令维护。
- 核心假定外层写入合法角色；合法性检查、候选项展示和错误报告属于上层。
