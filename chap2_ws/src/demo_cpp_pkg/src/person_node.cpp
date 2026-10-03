#include <string>
#include "rclcpp/rclcpp.hpp"

class  PersonNode :public rclcpp::Node
{
    private:
    std::string name_;
    int age_;

    public:
    PersonNode(const std::string &node_name , 
        const int &age,
        const std::string &name
    )
    :Node(node_name),
    name_(name),
    age_(age)
    {};


    void eat(const std::string &food_name)
    {
        RCLCPP_INFO(this->get_logger(),"I am %s, %d years old,eating %s now",
        name_.c_str(),
        age_,
        food_name.c_str());
    };

};

int main(int argc, char **argv)
{
    rclcpp::init(argc,argv);
    auto node = std::make_shared<PersonNode>("cpp_node",18,"wwssdd");
    node->eat("fan");
    rclcpp::spin(node);
    rclcpp::shutdown();

    
    return 0;
}



