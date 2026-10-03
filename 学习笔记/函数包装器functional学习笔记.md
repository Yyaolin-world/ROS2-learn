# 函数包装器 std::function 学习笔记

对应练习代码：`chap2_ws/src/demo_cpp_pkg/src/learn_functional.cpp`

## 1. 一句话理解

`std::function` 是一个**通用函数容器**：凡是"可调用"的东西——全局函数、lambda、成员函数、仿函数——只要**签名相同**，都能装进同一个变量，然后**用同一种方式调用**。

## 2. 签名怎么写

```cpp
std::function<void(const std::string &)> save;
//            ↑返回类型  ↑参数列表
```

- 模板参数里写的是**返回类型 + 参数列表**，和函数指针声明一个套路。
- 赋值时必须**严格匹配**签名，返回值、参数个数、参数类型都要对得上。

## 3. 三种可调用对象的装法（本代码核心）

| 类型 | 写法 | 关键点 |
|------|------|--------|
| 全局函数 | `std::function<...> save1 = save_with_free_fun;` | 直接赋值，函数名退化成函数指针 |
| lambda | `std::function<...> save2 = save_with_lambda_fun;` | lambda 本质是匿名对象，可直接赋值 |
| 成员函数 | `std::function<...> save3 = std::bind(&FileSave::save_with_member_fun, &file_save, std::placeholders::_1);` | 必须用 `std::bind` |

## 4. 难点：成员函数为什么要 std::bind

成员函数比普通函数**多一个隐藏参数 `this`**。写成 `&FileSave::save_with_member_fun` 时它还不能被直接调用，得先"绑"上一个对象实例。

```cpp
std::bind(&FileSave::save_with_member_fun,  // ① 取成员函数地址，必须加类名和 &
          &file_save,                        // ② 绑定对象，充当隐藏的 this
          std::placeholders::_1);            // ③ 第 1 个参数留空，调用时再传
```

- `&类名::成员函数` —— 取成员函数地址，**不能省略 `&`**，也不能省略类名。
- 第二个参数是对象地址 `&file_save`，它顶上 `this` 的位置。
- `std::placeholders::_1` 表示"调用 save3 时传入的第 1 个实参占这个位置"。有几个参数就放 `_1`、`_2`…… 到 `_n`。

## 5. 统一调用

```cpp
save1("file.txt");
save2("file.txt");
save3("file.txt");
```

三种来源不同，**调用语法完全一样**——这就是 `std::function` 的价值：屏蔽掉"到底是哪种函数"的差异，对外只剩一个统一接口。

## 6. 和 ROS2 的联系（为什么学它）

ROS2 里**回调都是 `std::function`**。订阅话题、定时器时，`create_subscription` / `create_wall_timer` 的参数就是 `std::function`，所以回调可以传全局函数、lambda，也可以传成员函数：

```cpp
// 成员函数做回调，同样靠 std::bind
node->create_subscription<std_msgs::msg::String>(
    "topic", 10,
    std::bind(&MyNode::on_msg, this, std::placeholders::_1));
```

学到订阅/定时器那一节会直接用到，提前会写能省不少事。

## 7. 记忆要点

- `std::function<返回类型(参数类型)>` 装"可调用物"，签名要对齐。
- 全局函数、lambda 直接 `=` 赋值；**成员函数必须 `std::bind`**，第二参是对象地址。
- `std::placeholders::_1` 是"留空"占位，调用时按位置传实参。
- 成员函数地址写 `&类名::成员函数`，`&` 和类名都不能省。
- 三种装法装完后，调用方式完全一致。
