#include <QApplication>
#include <QLabel>
#include <QString>
#include <sstream>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "status_interfaces/msg/system_status.hpp"

// 使用 using 指令，为自定义接口建立别名，方便阅读
using status_interfaces::msg::SystemStatus;

class SysStatusDisplay : public rclcpp::Node {
public:
    SysStatusDisplay() : Node("sys_status_display") {
        // 初始化订阅者，使用 Lambda 表达式作为回调函数
        subscription_ = this->create_subscription<SystemStatus>(
            "sys_status", 10, [&](const SystemStatus::SharedPtr msg) -> void {
                label_->setText(get_qstr_from_msg(msg));
            });
            
        // 创建一个空的 SystemStatus 对象，转化成 QString 进行初始显示
        label_ = new QLabel(get_qstr_from_msg(std::make_shared<SystemStatus>()));
        label_->show();
    }

    // 将 msg 中的内容提取出来并组装成字符串
    QString get_qstr_from_msg(const SystemStatus::SharedPtr msg) {
        std::stringstream show_str;
        show_str 
            << "=========== 系统状态可视化显示工具 ===========\n"
            << " 数据时间:\t" << msg->stamp.sec << "\ts\n"
            << " 用 户 名:\t" << msg->host_name << "\t\n"
            << "CPU 使用率:\t" << msg->cpu_percent << "\t%\n"
            << " 内存使用率:\t" << msg->memory_percent << "\t%\n"
            << " 内存总大小:\t" << msg->memory_total << "\tMB\n"
            << " 剩余有效内存:\t" << msg->memory_available << "\tMB\n"
            << " 网络发送量:\t" << msg->net_sent << "\tMB\n"
            << " 网络接收量:\t" << msg->net_recv << "\tMB\n"
            << "==============================================";
            
        return QString::fromStdString(show_str.str());
    }

private:
    rclcpp::Subscription<SystemStatus>::SharedPtr subscription_;
    QLabel* label_;
};

int main(int argc, char* argv[]) {
    // 初始化 ROS 2
    rclcpp::init(argc, argv);
    // 初始化 Qt 应用
    QApplication app(argc, argv);
    
    // 创建节点对象
    auto node = std::make_shared<SysStatusDisplay>();
    
    // 使用多线程处理：将 ROS 2 的 spin 放到独立线程，防止阻塞 Qt 的 UI 循环
    std::thread spin_thread([&]() -> void { rclcpp::spin(node); });
    spin_thread.detach();
    
    // 运行 Qt 的主事件循环
    app.exec();
    
    // 关闭 ROS 2
    rclcpp::shutdown();
    return 0;
}