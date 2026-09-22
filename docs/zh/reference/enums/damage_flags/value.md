[givm](../../../reference.md) / [枚举值](../../enums.md) / [damage_flags](../damage_flags.md) / **value**

# givm::damage_flags::value

定义于头文件 `<givm/enums/damage_flags.hpp>`

```cpp
[[nodiscard]] constexpr std::uint16_t value() const noexcept;
```

取得便于保存或传递的伤害性质位值。

## 返回值

各标志位的按位或。

## 示例

```cpp
#include <print>

#include <givm/enums/damage_flags.hpp>

int main()
{
    givm::damage_flags flags{ givm::damage_flag_bits::skill_damage };
    const auto restored = givm::damage_flags::from_bits(flags.value());
    std::println("往返后位值相同: {}", restored.value() == flags.value());
}
```

输出

```text
往返后位值相同: true
```
