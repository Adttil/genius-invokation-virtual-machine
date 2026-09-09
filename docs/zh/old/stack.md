# Executor Stack

executor stack 是执行期数据栈，用来保存 event、输入等待对象、广播游标和临时结算状态。它是公开执行状态，外层可以在知道当前指令协议时观察或写入，但 stack 本身不保存独立的类型、边界、析构函数或复制函数元数据。

## Frame

一次 `push(...)` 创建一个 frame。普通 `top<T...>()` 和 `pop<T...>()` 访问栈顶的单个 frame，模板参数按 `push` 参数顺序书写，最后一个类型是物理栈顶。

```cpp
const auto frame = stack.push(A{}, B{});
auto&& [a, b] = frame;

auto&& [same_a, same_b] = stack.top<A, B>();
stack.pop(frame);
```

`push` 返回新增 frame 的 `frame_view`。`top` 也返回 `frame_view`，即使只访问单个对象也使用结构化绑定：

```cpp
const auto frame = stack.top<selector>();
auto&& [input] = frame;
```

`frame_view` 是非持有 view。后续任何 `push` 之后都必须假设已有 `frame_view` 失效，因为 stack 可能搬迁存储；`pop`、`clear`、`reserve`、移动、赋值或交换 stack 后也不得继续使用旧 view。需要继续访问时，重新通过 `top` 取得当前 view。

## Object

stack object 必须平凡可复制、平凡析构，且对齐不超过 `std::max_align_t`。非平凡对象、局部引用、以及依赖外部生命周期的 `std::span` 不应直接作为 stack object 保存。

若需要变长数据，使用栈内动态数组，而不是把外部 span 存入 stack。

## Dynamic Array

动态数组通过 `dynamic_array<T>(...)` 压入，只能集中出现在 frame 开头。访问时在模板参数中写作 `T[]`；可变 stack 返回 `std::span<T>`，只读 stack 返回 `std::span<const T>`。

```cpp
const auto frame = stack.push(
    dynamic_array<std::uint32_t>(count),
    header
);
auto&& [values, header_ref] = frame;
```

传入无符号整数时只指定数组长度，不初始化元素；数组内容必须由调用方随后通过返回的 span 写入后才能读取。传入 range 时复制 range 内容，其元素值类型去掉 const 后必须与 `T` 相同。range 必须在计数与复制期间保持长度和内容一致；没有大小信息的 range 必须支持重复遍历，不能使用计数后即被消费的单遍来源。

## Top

`top<T...>()` 的模板参数必须和当前栈布局协议一致。调用方有责任保证类型、顺序和 frame 边界正确。

对最上层 frame，`top<T...>()` 可以访问该 frame 的固定部分后缀：

```cpp
stack.push(dynamic_array<std::uint32_t>(count), header, selector{});

auto&& [input] = stack.top<selector>();          // 固定部分后缀
auto&& [head, input2] = stack.top<Header, selector>();
```

需要外部输入的指令通常把输入区设计为 frame 尾部的完整固定对象序列；外层按 `table.definition_library().instruction(executor.position())` 所识别的当前指令协议访问这个后缀。stack 本身不记录哪些后缀对象是输入槽，也不检查输入类型。

动态数组不属于可单独后缀访问的固定部分。若 `top<T...>()` 中包含动态数组，则它必须描述整个 frame：

```cpp
auto&& [values, head, input] = stack.top<std::uint32_t[], Header, selector>(); // 整个 frame
```

不要写只覆盖一部分动态区域的 `top`，例如从第二个动态数组开始访问，或只访问动态数组而跳过后面的固定部分。只有当整个 frame 本身就是该动态数组时，`top<T[]>()` 才是完整 frame 访问。

## Pop

`pop<T...>()` 必须按完整 frame 弹出，模板参数应当和对应 `push(...)` 创建的 frame 匹配。不要用固定后缀 view 或固定后缀模板参数弹出 frame。

推荐在消费 frame 时保存完整 view，并直接传给 `pop`：

```cpp
const auto frame = stack.top<selector>();
auto&& [input] = frame;
const auto selected = input.selected;
stack.pop(frame);
```

`pop(frame_view...)` 用于弹出一个或多个连续 frame。传入多个 view 时，按从下到上的顺序传入，通常直接使用多 frame `top` 的返回值：

```cpp
auto&& [lower, upper] = stack.top<frame<A, B>, frame<C>>();
stack.pop(lower, upper);
```

传给 `pop(frame_view...)` 的 view 必须来自完整 frame 访问。不要把 `top` 取得的固定后缀 view 传给 `pop`。

## Multiple Frames

需要同时观察多个连续 frame 时，使用 `frame<...>` 分块。每个 `frame<...>` 都应描述完整 frame，返回值按从下到上的顺序排列。

```cpp
stack.push(A{}, B{});
stack.push(C{});

auto&& [lower, upper] = stack.top<frame<A, B>, frame<C>>();
auto&& [a, b] = lower;
auto&& [c] = upper;
```

多 frame `top` 用于组合协议边界处的读写。若后续 `push` 了新 frame，应重新取得这些 view。
