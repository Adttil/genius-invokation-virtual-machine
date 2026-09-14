[givm](../../../reference.md) / [牌桌](../../table.md) / [skill_view](../skill_view.md) / **begin**

# givm::skill_view::begin

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr const skill_view* begin() const;
```

取得这个技能的单实体范围起点。

## 返回值

指向该 [`skill_view`](../skill_view.md) 访问对象自身的只读指针。若实体无效，它等于 `end()`。

## 注意

范围依赖访问对象本身的存活。遍历有效实体时执行一次循环；遍历已移除实体时不执行循环。
