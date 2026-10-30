[givm](../../../reference.md) / [执行](../../executor.md) / [executor](../executor.md) / **view_in**

# givm::executor::view_in

定义于头文件 `<givm/runtime.hpp>`

```cpp
template<execution_state State, role Role = role::full>
constexpr execution_view<State, Role> view_in() noexcept(/* Release 为 true，Debug 为 false */);
```
[`execution_state`](../execution_state.md)
[`execution_view`](../execution_view.md)

取得当前指定种类的执行现场视图。

`State` 必须与最近一次初始化或推进到达的现场种类一致。调用方已知该条件时，可以直接取得视图；本函数不执行对局，也不改变现场。

## 模板参数

| | |
| --- | --- |
| `Role` | 读取及输入身份，默认为 `role::full`；见 [`role`](../../enums/role.md) |
| `State` | 当前执行现场的种类 |

## 返回值

与 `State` 对应的现场访问对象，可用成员见相应[现场视图](../execution_view.md)的说明；观察现场通过 `resume` 继续执行。调用方可用 `auto` 接收结果。

## 角色与接口

`role::full` 保留完整接口。本人窗口具有相应的提交接口；对手窗口仅提供公开读取；观众可读取双方私有信息和费用预览，但不能提交输入。`resume` 仅由全权视图提供。

双方选择窗口的玩家版本将输入绑定本人，省略原有的 `player_id` 参数。`state`、`access_role`、`accepts_input` 为类型的静态常量。角色不会增加视图的数据成员，也不会增加运行期权限检查。

## 注意

有数据的视图借用当前执行器现场。开始下一次推进、重新进入、移动或替换执行器后，原有借用失效。复制视图仍借用原执行器；复制执行器后，应从副本重新取得视图。

即使下一次推进返回相同种类，先前借用的现场也不能继续访问。未定义 `NDEBUG` 时，指定错误种类或访问已经失效的现场会抛出 [`execution_view_error`](../execution_view_error.md)；Release 不进行这些检查，违反要求属于未定义行为。

## 参阅

| | |
| --- | --- |
| [`execution_view`](../execution_view.md) | 对局执行现场的视图 |
