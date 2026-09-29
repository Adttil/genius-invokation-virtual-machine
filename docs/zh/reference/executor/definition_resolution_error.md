[givm](../../reference.md) / [执行](../executor.md) / **definition_resolution_error**

# givm::definition_resolution_error

定义于头文件 `<givm/compile.hpp>`

```cpp
struct definition_resolution_error;
```

编译上下文通过硬依赖接口解析定义时的错误。

## 成员类型

| | |
| --- | --- |
| `reason` | 原因枚举：`undeclared_dependency` 表示未声明该名称依赖，`not_found` 表示目标定义不存在 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `definition` | [`definition_name`](../definition/definition_name.md) | 待解析定义的类别及名称 |
| `cause` | `reason` | 失败原因 |

## 参阅

| | |
| --- | --- |
| [`resolve_id`](definition_compile_context/resolve_id.md) | 解析已声明的名称依赖 |
