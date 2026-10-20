#include "rclcpp/rclcpp.hpp"
#include "print_hello_interfaces/msg/string_hello.hpp"
#include <chrono>
#include <functional>

class Hello_Node : public rclcpp::Node
{
    public:

    Hello_Node() : Node("hello_node")
    {
        RCLCPP_INFO(this->get_logger(), "Hello Node has been started.");
    hello_publisher_ =  this->create_publisher<print_hello_interfaces::msg::StringHello>("hello_topic", 10); //创建发布者，发布消息类型为print_hello_interfaces::msg::StringHello，话题名为hello_topic，队列长度为10

    timer_ = this->create_wall_timer(
        std::chrono::seconds(1),
        std::bind(&Hello_Node::timer_callback, this)
    ); //创建定时器，每隔1秒调用一次timer_callback函数
    }

    private:
    void timer_callback()
    {
        auto message = print_hello_interfaces::msg::StringHello(); //将消息类型print_hello_interfaces::msg::StringHello实例化为message
        message.data = "Hello ROS2";
        RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", message.data.c_str());
        hello_publisher_->publish(message);  //通过发布者发布消息
    }

    rclcpp::Publisher<print_hello_interfaces::msg::StringHello>::SharedPtr hello_publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
};




int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Hello_Node>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}


