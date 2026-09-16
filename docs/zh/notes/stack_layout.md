[开发备忘](../notes.md) / **执行栈布局与搬迁约束**

# 执行栈布局与搬迁约束

本页保存栈和帧为何这样组织、哪些访问形态能成立，以及实现重入指令时需要维护的条件。公开操作签名见[frame_stack](../reference/utils/frame_stack.md)。

来源：提交 `b3d6c50` 中的 `docs/zh/old/stack.md`。保留帧布局、后缀定位和内存搬迁的实现约束。

executor stack 是执行期数据栈，用来保存 event、输入等待对象、广播游标和临时结算状态。旧文把它整体作为可按指令协议观察或写入的公开执行状态；当前 executor 不公开原始栈，观察与输入通过 execution_view 完成，帧布局由内部实现维护。独立栈工具的公开接口不授予对执行器现场的访问权限。stack 本身不保存独立的类型、帧目录、析构函数或复制函数元数据。

## Frame

一次 `push(...)` 创建一个 frame。普通 `top<T...>()` 和 `pop<T...>()` 访问栈顶的单个 frame，模板参数按 `push` 参数顺序书写，最后一个类型是物理栈顶。

下文的 `A`、`B`、`C`、`Header` 和 `Input` 都是说明栈操作的示例类型，不代表执行器中的现行类型。

```cpp
const auto frame = stack.push(A{}, B{});
auto&& [a, b] = frame;

auto&& [same_a, same_b] = stack.top<A, B>();
stack.pop(frame);
```

`push` 返回新增 frame 的 `frame_view`。`top` 也返回 `frame_view`，即使只访问单个对象也使用结构化绑定：

```cpp
const auto frame = stack.top<Input>();
auto&& [input] = frame;
```

`frame_view` 是非持有 view。旧文采用保守使用规则：后续任何 `push` 之后都假设已有 view 失效，因为 stack 可能搬迁存储；`pop`、`clear`、`reserve`、移动、赋值或交换后也重新取得 view。需要继续访问时，重新通过 `top` 取得。

该规则不表示每次 `push` 必定重分配；当前实现容量充足时只推进栈顶。真正访问某个旧 view 是否仍有效，取决于对应帧是否存续及存储是否搬迁。对可能进入子程序、广播或压入动态帧的指令实现，保留“操作后重新取 view”的写法可避免依赖容量恰好够用。

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
stack.push(dynamic_array<std::uint32_t>(count), header, Input{});

auto&& [input] = stack.top<Input>();          // 固定部分后缀
auto&& [head, input2] = stack.top<Header, Input>();
```

输入实现可以把输入区放在 frame 尾部的完整固定对象序列中，由对应 execution_view 的参数式操作定位；外层不再读取指令或改写原始输入槽。stack 本身不记录哪些后缀对象是输入，也不检查输入类型。

动态数组不属于可单独后缀访问的固定部分。若 `top<T...>()` 中包含动态数组，则它必须描述整个 frame：

```cpp
auto&& [values, head, input] = stack.top<std::uint32_t[], Header, Input>(); // 整个 frame
```

不要写只覆盖一部分动态区域的 `top`，例如从第二个动态数组开始访问，或只访问动态数组而跳过后面的固定部分。只有当整个 frame 本身就是该动态数组时，`top<T[]>()` 才是完整 frame 访问。

## Pop

`pop<T...>()` 必须按完整 frame 弹出，模板参数应当和对应 `push(...)` 创建的 frame 匹配。不要用固定后缀 view 或固定后缀模板参数弹出 frame。

推荐在消费 frame 时保存完整 view，并直接传给 `pop`：

```cpp
const auto frame = stack.top<Input>();
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

## 从帧尾定位固定后缀

核对位置：[utils/stack.hpp](../../../include/givm/utils/stack.hpp)。固定元素的偏移从帧尾向前按类型尺寸和对齐计算，帧总大小再按 `max_alignment` 对齐。`frame_view` 只保存帧尾指针与编译期类型信息；读取时由模板重新算出偏移。因此相同尾部类型序列可以在不知道此前固定字段的情况下定位后缀。

动态数组在固定区保存 `dynamic_array_span{ offset_to_frame_end, count }`，元素本体位于整帧的动态前部。描述符使用相对帧尾的偏移，而非存储内绝对指针；整块栈搬迁后，数组位置可以由新帧尾重新计算。

```text
低地址                                                     高地址 / 帧尾
动态数组元素（逐数组对齐） | 填充 | 数组描述符 | 固定字段 | 输入等固定后缀
```

动态数组必须连续位于类型序列开头；含动态数组的 `top` 按使用约定需要描述整帧。若从第二个动态数组起读，但保留其后全部字段，当前偏移算法仍可能定位该数组的描述符和元素；问题在于该 view 的 `aligned_begin()` 将指向第二个数组的起点，不能再作为原帧起点用于弹栈或定位更低的帧。若连后面的固定字段也跳过，则连描述符相对帧尾的位置都无法按原布局计算。只有固定后缀属于独立观察的约定；弹出帧始终需要完整类型序列或完整 view。

多帧 `top` 先取得较高帧，再以其对齐后的起点作为较低帧的帧尾，最终按从下到上的顺序返回。`pop(view...)` 使用第一个完整 view 的起点作为新栈顶；它不逐项验证传入 view 是否连续，也没有运行时类型检查来修复错误边界。

## 复制、扩容与对象选择

栈的复制构造复制已用字节；容量不够时按新大小的 `bit_ceil` 分配并搬迁已用字节。`reserve` 则直接按指定容量重新分配，即使请求的容量并不更大，旧 view 也不能沿用。`clear` 和 `pop` 只改变栈顶，不逐对象调用析构。

因此“不在栈里保存依赖外部生命周期的 span 或局部引用”是保证快照可独立继续的设计约束，不仅是检查类型是否平凡可复制。一个指针或 span 在 C++ 类型层面可能满足平凡复制，逐字节复制它也不会复制其指向的数据。需要随执行器复制的变长内容应放入栈自己的动态数组。

动态数组的无符号长度形式只分配空间，range 形式先计数再复制；对无大小的 range，要特别检查第一次计数会不会消费数据。连续范围可按字节复制，非连续范围逐元素复制，但二者都不为单遍数据源提供自动缓存。

本页解释内存搬迁策略，不定义可持久化的二进制格式；字节序、类型大小、对齐、代码指针及外部引用仍依赖构建环境。
