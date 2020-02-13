#include "json.hpp"

int main() {
    //json::value j1(nullptr);
    //json::value j2(true);
    //json::value j3(false);
    //json::value j4(1);
    //json::value j5(2l);
    //json::value j6(3.0f);
    //json::value j7(4.0);
    //json::value j8("five");
    //json::value j9(j8);
    //json::value j10 = j9;

    json::object j1({{"a", 1}});
    json::object j2({{"4", "4"}, {"5", "5"}});
    json::object j3(j2);
    json::object j4({{"d", 5}});

    json::object j11;
    j11.insert({"name", "GyuHyen Choi"});
    j11.insert({"age", 31});
    j11.insert({"eyesight", 1.5});
    j11.insert({"body", "174cm / 62.5kg"});
    j11.insert({"body", json::object()});
    j11.find("body")->second.insert({"height", 174});
    j11.find("body")->second.insert({"weight", 62.5});
    j11.insert({"book", json::array({"incerto 1", "incerto 2", "incerto 3"})});
    j11.insert({"numbers", json::array({2, 4, 8, 11, 15})});
    j11.find("book")->second.push_back(json::object({{"4", 4}}));
    j11.find("book")->second.push_back(true);
    j11.find("book")->second.push_back(1);
    j11.find("book")->second.push_back(1.2);
    std::cout << j11 << std::endl;

    //json::object j12({
    //    {"name", "Wiany Illia"},
    //    {"age", 20},
    //    {"body", json::array({"value", "174cm / 65kg"})}
    //});
    //std::cout << j12 << std::endl;

    std::ifstream f("sample.jsonl");
    std::string line;
    while (std::getline(f, line)) {
        std::cout << line << std::endl;
        json::object j = json::parse(line);
        std::cerr << "main | while"
                  << " | j = " << j
                  << std::endl;
    }

    
    return 0;
}
