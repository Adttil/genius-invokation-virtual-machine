[givm](../../reference.md) / [执行](../executor.md) / **execution_view_error**

# givm::execution_view_error

定义于头文件 `<givm/executor.hpp>`

```cpp
class execution_view_error : public std::exception;
```

执行现场种类不匹配或视图已经失效的调试异常。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `reason` | `const execution_view_error_reason` | 现场或视图有效性的结构化错误原因 |

## 成员函数

| | |
| --- | --- |
| `what()` | 取得异常对象保存的格式化文本，返回 `const char*`，不抛异常。 |

## 相关类型别名

```cpp
using execution_view_error_reason = std::variant<unexpected_execution_state, expired_execution_view>;
```

## 相关类

```cpp
struct unexpected_execution_state;
struct expired_execution_view;
```

`unexpected_execution_state` 表示视图要求的现场种类与执行器当前现场不符。

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `expected` | `execution_state` | 操作要求的现场种类 |
| `actual` | `std::optional<execution_state>` | 当前现场种类；没有可访问现场时为空 |

`expired_execution_view` 表示当前操作使用了旧现场的视图。

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `expected_version` | `std::size_t` | 视图建立时的现场版本 |
| `actual_version` | `std::size_t` | 执行器当前版本 |

## 注意

异常类型在所有构建模式下存在。未定义 `NDEBUG` 时，取得视图和调用其接口会检查现场；Release 不保存相关诊断记录、不执行检查。推进即使再次到达同种类现场，也会使旧视图失效。

检查不能延长执行器的生命周期，也不能检查已经借出的引用或 span 的后续访问。执行器必须比借用它的视图活得更久。

## 参阅

| | |
| --- | --- |
| [`execution_view`](execution_view.md) | 一处对局执行现场的视图 |
| [`error_string`](error_string.md) | 格式化异常及具体错误原因 |
