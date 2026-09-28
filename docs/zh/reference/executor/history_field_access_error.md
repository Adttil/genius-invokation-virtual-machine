[givm](../../reference.md) / [执行](../executor.md) / **history_field_access_error**

# givm::history_field_access_error

定义于头文件 `<givm/executor.hpp>`

```cpp
struct history_field_access_error;
```

当前编译阶段或定义类别不能取得所请求历史字段的诊断。

## 成员类型

| | |
| --- | --- |
| `reason` | 原因枚举：`layouts_unavailable` 表示尚未完成字段布局，`no_current_summary` 表示当前源不是历史摘要 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `summary` | `std::string` | 请求的摘要名称；自身字段查询时可能为空 |
| `field` | `std::string` | 请求的字段名称 |
| `cause` | `reason` | 失败原因 |
