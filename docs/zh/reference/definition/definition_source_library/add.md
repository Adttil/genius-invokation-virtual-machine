[givm](../../../reference.md) / [定义](../../definition.md) / [definition_source_library](../definition_source_library.md) / **add**

# givm::definition_source_library::add

定义于头文件 `<givm/definition.hpp>`

```cpp
bool add(); // (1)
bool add(const definition_source_library& library); // (2)

template<class TSource>
bool add(const TSource& source); // (3)

template<class TFirstSource, class TSecondSource, class... TOtherSource>
bool add(const TFirstSource& first, const TSecondSource& second,
         const TOtherSource&... others); // (4)
```

登记一批可供对局使用的定义源。一次调用中的源可以按名称相互依赖，适合一起加入某张卡牌及其生成的状态、召唤物等相关定义。

(1) 不添加内容。(2) 合并另一个源库。(3) 登记一个源。(4) 一起登记多个源。名称在各定义类别内必须唯一。

## 模板参数

|  |  |
| --- | --- |
| `TSource`、`TFirstSource`、`TSecondSource`、`TOtherSource...` | 符合[定义源协议](../source_protocol.md)的类型 |

## 参数

|  |  |
| --- | --- |
| `library` | 要合并的源库 |
| `source`、`first`、`second`、`others...` | 要登记的定义源对象 |

## 返回值

全部登记成功时返回 `true`。同类别名称冲突，或按名称声明的依赖不在已有源和本次新增源中时，返回 `false`，不登记本次的任何源。(1) 始终返回 `true`。

## 注意

本函数保存对源对象的非拥有引用，源对象不得提前销毁。单个源可声明对自身的依赖。按标签筛选的依赖在选择与编译时解析。

## 示例

```cpp
#include <print>
#include <string_view>

#include <givm/givm.hpp>

struct card_source
{
    using definition_category = givm::card_definition;

    std::string_view source_name;

    std::string_view name() const { return source_name; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

int main()
{
    const card_source potion{ "恢复药剂" };
    const card_source food{ "恢复料理" };
    givm::definition_source_library sources{};
    std::println("批量登记成功: {}", sources.add(potion, food));
    std::println("重复登记成功: {}", sources.add(potion));
    std::println("恢复料理仍存在: {}", sources.has<givm::card_definition>("恢复料理"));
}
```

输出

```text
批量登记成功: true
重复登记成功: false
恢复料理仍存在: true
```
