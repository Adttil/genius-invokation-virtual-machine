[givm](../../reference.md) / [执行](../executor.md) / **history_access_error**

# givm::history_access_error

定义于头文件 `<givm/runtime.hpp>`

```cpp
class history_access_error;
```

通过编译后的定义库取得历史摘要字段时，摘要 ID、字段名称或类型不正确的调试异常。它直接公开继承 `std::exception`。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `reason` | `const history_access_error_reason` | 摘要或字段访问失败的具体原因 |

## 成员函数

| | |
| --- | --- |
| `const char* what() const noexcept override` | 取得具体错误文本 |

## 构造

```cpp
explicit history_access_error(history_access_error_reason cause);
```

以具体字段访问原因构造异常。

## 相关类型别名

```cpp
using history_access_error_reason = std::variant<definition_metadata_error,
    history_field_not_found, history_field_type_mismatch>;
```

| | |
| --- | --- |
| [`definition_metadata_error`](definition_metadata_error.md) | 摘要定义 ID 无效或越界 |
| [`history_field_not_found`](history_field_not_found.md) | 摘要中没有该名称的字段 |
| [`history_field_type_mismatch`](history_field_type_mismatch.md) | 字段的数值类型或标量、数组形态不符 |

## 注意

仅在未定义 `NDEBUG` 时由 `definition_library::history_field` 检查并抛出。发布构建不进行这些检查，违反字段访问约定属于未定义行为。异常类型本身在两种构建模式下均可使用。

编译上下文的字段查询使用 [`compile_error`](compile_error.md) 收集错误，不抛出本异常。若本异常由对局响应传播出 [`resume`](execution_view/resume.md)，该次推进不提供回滚保证，不应在原现场继续推进。
