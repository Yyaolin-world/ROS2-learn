#include <iostream>
#include <functional>

void save_with_free_fun (const std::string &file_name)
{
    std::cout<<"free function has save:"<<file_name<<std::endl;
}

class FileSave
{
    public:
    void save_with_member_fun(const std::string &file_name)
    {
        std::cout<<"take member_fun,save: "<<file_name<<std::endl;
    };
};

int main()
{
    FileSave file_save;
    auto save_with_lambda_fun = [](const std::string &file_name)-> void
    {
        std::cout<<"take Lambda function, save:"<<file_name <<std::endl;
    };

    std::function<void(const std::string &)> save1 = save_with_free_fun;

    std::function<void(const std::string &)> save2 = save_with_lambda_fun;

    std::function<void(const std::string &)> save3 = std::bind(
        &FileSave ::save_with_member_fun, &file_save, std::placeholders ::_1);
    

    save1("file.txt");
    save2("file.txt");
    save3("file.txt");

    return 0;
}