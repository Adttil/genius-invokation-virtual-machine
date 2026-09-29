[givm](../../reference.md) / [执行](../executor.md) / **view_input_error**

# givm::view_input_error

定义于头文件 `<givm/runtime.hpp>`

```cpp
template<class Reason>
class view_input_error : public std::exception;
```

视图输入参数或查询前提不合法的调试异常。具体错误保持原本的强类型，可以按 `Reason` 分别捕获，也可以通过 `std::exception` 统一捕获。

## 模板参数

| | |
| --- | --- |
| `Reason` | 具体操作的验证结果枚举或结构化错误类型。 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `operation` | `const std::string` | 发生错误的视图操作名称 |
| `reason` | `const Reason` | 具体检查结果 |

## 成员函数

| | |
| --- | --- |
| `what()` | 取得包含操作名称和错误原因的格式化文本，返回 `const char*`，不抛异常。 |

## 注意

异常类型在所有构建模式下存在，自动检查与抛出仅在未定义 `NDEBUG` 时进行。独立 `*_validate` 接口仍直接返回检查结果，不把普通的不合法选择转换为异常。

输入检查发生在提交和推进之前。检查失败后通常可以修正输入并重试；即时报价已经成功时，应使用相应 `_with_cached_cost` 接口重试。报价响应本身抛异常时，该候选不能在当前现场再次报价或使用缓存。

开始推进之后发生的执行异常不提供回滚，不应继续使用原执行现场。

## 参阅

| | |
| --- | --- |
| [视图输入错误原因](view_input_error_reason.md) | 视图检查可能使用的具体原因类型 |
| [`execution_view_error`](execution_view_error.md) | 现场种类与视图生命周期错误 |
| [`error_string`](error_string.md) | 格式化异常及具体错误原因 |
