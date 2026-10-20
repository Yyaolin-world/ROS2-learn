# HelloROS 笔记：发布者 + 自定义消息

## 0. 先分清这些名字（最容易乱的地方）

| 名字 | 例子 | 写在哪 | 能不能随便改 |
|------|------|--------|--------------|
| 接口包名 | `print_hello_interfaces` | 目录名、`package.xml` 的 `<name>`、`project()`、include 前缀 | 建包时定下，改要全局替换 |
| 消息名 | `StringHello` | `msg/StringHello.msg` 文件名、C++ 类型名 | 大驼峰，必须和文件名一致 |
| C++ 消息类型 | `print_hello_interfaces::msg::StringHello` | `<>` 里、变量类型 | 由「包名::msg::消息名」拼成 |
| 消息头文件 | `print_hello_interfaces/msg/string_hello.hpp` | `#include` | 消息名转 snake_case（全小写） |
| 消息对象 | `message` | 局部变量 | 随便起 |
| 话题名 | `"hello_topic"` | `create_publisher`、`ros2 topic echo` | 随便起，发布和订阅一致即可 |
| 类名 | `Hello_Node` | `class` | 随便起 |
| 节点名 | `"hello_node"` | `Node("hello_node")` | 随便起，`ros2 node list` 看到的就是它 |
| 可执行文件名 | `hello_ros` | `add_executable`、`ros2 run` | 随便起，不必和 cpp 文件名相同 |

一句话口诀：

> **包名管 include 前缀和命名空间；消息名管头文件（转小写）和类型名；话题名、类名、变量名随便起。**

最容易搞混的三组：

- `StringHello`（类型）vs `message`（用这个类型创建出来的对象）
- `"hello_topic"`（话题名字符串）vs `hello_publisher_`（C++ 对象，不是话题）
- `.msg` 里的 `string data`（字段定义）vs 代码里的 `message.data`（给字段赋值）

---

## 1. 为什么要拆成两个包

```
print_hello_interfaces        ← 只放 .msg，编译时生成 C++ 头文件
        ▲  <depend>
print_hello_pkg               ← 只放 .cpp，节点代码
```

- 接口包可以被任意多个节点包复用；
- 节点包不能 `find_package` 自己（原代码的报错就是这条）；
- colcon 会按 `package.xml` 的依赖自动决定编译顺序，先接口、后节点。

---

## 2. 造接口包（4 个文件）

```bash
cd ~/ROS2/chap3/hello_ros_ws/src
ros2 pkg create --build-type ament_cmake print_hello_interfaces
mkdir -p print_hello_interfaces/msg
```

**① `msg/StringHello.msg`** —— 只写「类型 字段名」：

```
string data
```

消息名用大驼峰，字段名用小写下划线；这里不用也不能写 class/struct。

**② `package.xml`** 在 `ament_cmake` 之后补三行：

```xml
<buildtool_depend>rosidl_default_generators</buildtool_depend>
<exec_depend>rosidl_default_runtime</exec_depend>
<member_of_group>rosidl_interface_packages</member_of_group>
```

**③ `CMakeLists.txt`**：

```cmake
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/StringHello.msg"
)
```

纯 `string` 字段不需要额外依赖；以后用 `time`、`Header` 才加 `builtin_interfaces` / `std_msgs`。

**④ 编译后生成什么**：

```
install/print_hello_interfaces/include/print_hello_interfaces/print_hello_interfaces/msg/string_hello.hpp
                              └── include 根路径 ──┘ └──────────── 实际 include 的内容 ────────────┘
```

所以代码里写：

```cpp
#include "print_hello_interfaces/msg/string_hello.hpp"   // 全小写
print_hello_interfaces::msg::StringHello                   // 大驼峰
```

---

## 3. 节点包要改的三处

**`package.xml`**：

```xml
<depend>rclcpp</depend>
<depend>print_hello_interfaces</depend>
```

**`CMakeLists.txt`**（删掉原来 `find_package(print_hello_pkg)` 的自引用）：

```cmake
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(print_hello_interfaces REQUIRED)

add_executable(hello_ros src/hello_ros.cpp)
ament_target_dependencies(hello_ros rclcpp print_hello_interfaces)

install(TARGETS hello_ros DESTINATION lib/${PROJECT_NAME})
```

**`hello_ros.cpp`**（可直接跑的完整骨架）：

```cpp
#include "rclcpp/rclcpp.hpp"
#include "print_hello_interfaces/msg/string_hello.hpp"
#include <chrono>
#include <functional>

class Hello_Node : public rclcpp::Node
{
public:
    Hello_Node() : Node("hello_node")
    {
        // ① 发布者：<消息类型>("话题名", 队列长度)
        hello_publisher_ =
            this->create_publisher<print_hello_interfaces::msg::StringHello>("hello_topic", 10);

        // ② 定时器：每 1 秒调一次回调（函数调用用圆括号）
        timer_ = this->create_wall_timer(
            std::chrono::seconds(1),
            std::bind(&Hello_Node::timer_callback, this));
    }

private:
    void timer_callback()
    {
        auto message = print_hello_interfaces::msg::StringHello(); // ③ 构造空消息
        message.data = "Hello ROS2";                              // ④ 填字段
        RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", message.data.c_str());
        hello_publisher_->publish(message);                        // ⑤ 发送
    }

    rclcpp::Publisher<print_hello_interfaces::msg::StringHello>::SharedPtr hello_publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);                 // 开机
    auto node = std::make_shared<Hello_Node>();
    rclcpp::spin(node);                       // 一直转，回调才会触发
    rclcpp::shutdown();                       // 关机
    return 0;
}
```

要点：

- `create_publisher<T>` 的 `T` 是**完整类型**，`<>` 里不能只写 `StringHello`。
- 队列长度 `10` 是 QoS 缓存深度，来不及发的消息最多积压 10 条。
- `auto message = Type();` 是构造对象；`message.data` 是 `.msg` 里定义的同名字段。
- `publish()` 用 `->`，因为 `hello_publisher_` 是 `SharedPtr`。
- 没有 `spin()`，定时器回调不会执行，节点启动后立刻退出。

---

## 4. 编译、运行、验证

```bash
cd ~/ROS2/chap3/hello_ros_ws          # 必须在工作空间根目录
source /opt/ros/jazzy/setup.bash
colcon build --symlink-install --cmake-args -DPython3_EXECUTABLE=/usr/bin/python3
source install/setup.bash
```

（`-DPython3_EXECUTABLE` 是因为本机 miniconda 会遮住系统 Python。）

```bash
ros2 run print_hello_pkg hello_ros     # 终端 1：跑节点
ros2 topic echo /hello_topic           # 终端 2：收数据，不用写订阅代码
ros2 interface show print_hello_interfaces/msg/StringHello   # 查消息定义
```

预期输出：`[INFO] [hello_node]: Publishing: 'Hello ROS2'` 每秒一条。

---

## 5. 今天踩过的坑（症状 → 原因 → 改法）

| 症状 | 原因 | 改法 |
|------|------|------|
| `No such file ... StringHello.hpp` | 生成的头文件是 snake_case，且 Linux 区分大小写 | 改成 `string_hello.hpp` |
| `expected '}' at end of input`、`private` 处报错 | 构造函数体漏了右花括号，`private:` 被当成函数内语句 | 构造函数写完先闭合 `}` |
| `hello_publisher_ was not declared` | 通常是上面括号错误的连锁反应 | 先修括号，别急着改成员名 |
| `unresolved overloaded function type` | `create_wall_timer{...}` 把花括号当参数列表 | 函数调用改圆括号 `(...)` |
| `1s` 未定义 | 缺 `using namespace std::chrono_literals;` | 用 `std::chrono::seconds(1)`，或补 using |
| `find_package(print_hello_pkg)` 失败 | 包不能依赖自己 | 改为依赖接口包 |
| `ros2 run` 报 `No executable found` | 没 `install(TARGETS ...)` 或没 source | 两样都检查 |
| 编辑器红线但编译通过/不通过 | IntelliSense 靠构建产物找头文件 | 先编译，再 Reset IntelliSense Database |

---

## 6. 记忆卡片

- **结构**：接口包只放 `.msg`，节点包只放 `.cpp`；节点包 `<depend>` 接口包。
- **命名**：类型 `包名::msg::大驼峰`；include `包名/msg/小写下划线.hpp`。
- **发布五步**：建发布者 → 建定时器 → 构造消息 → 填字段 → `publish`。
- **main 三件套**：`init` → `spin` → `shutdown`；少了 `spin` 回调全是摆设。
- **为什么不用 `while(1)`**：`spin()` 是事件驱动引擎，我们只写「消息来了/闹钟响了做什么」，CPU 不会空转。
- **命令**：编译在 ws 根目录，运行前 `source install/setup.bash`，验证用 `ros2 topic echo`。
