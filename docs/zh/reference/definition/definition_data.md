[givm](../../reference.md) / [定义](../definition.md) / **definition_data**

# givm::definition_data

定义于头文件 `<givm/definition.hpp>`

```cpp
using definition_data = std::any;
```

保存一份已编译定义的数据对象。不同卡牌和角色可以使用不同的配置类型，定义库以此统一持有它们；对应的查询或事件响应仍按原有配置类型读取。

## 注意

定义源的 `compile` 返回非 `void`、非引用且可复制构造的值。通常不需要手动操作这个别名；编译与查询、事件响应之间的类型对应由定义系统处理。

## 示例

```cpp
#include <any>
#include <print>

#include <givm/givm.hpp>

int main()
{
    struct healing_config
    {
        int amount;
    };
    const givm::definition_data data{ healing_config{ .amount = 2 } };
    const auto& healing = std::any_cast<const healing_config&>(data);
    std::println("定义的治疗量: {}", healing.amount);
}
```

输出

```text
定义的治疗量: 2
```
