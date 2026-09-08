[返回](../events.md)

# test_command

用于验证指令执行、event 广播和定义响应分派的测试命令。

## 字段

无。

## 执行

本命令不要求特定栈顶布局，也不直接读写 executor stack。

创建 `test_event` 并推进该事件的广播；不修改 `table`。
