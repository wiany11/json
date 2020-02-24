#include <fstream>

#include "json.hpp"

bool test_1() {
    json::null jnull;
    
    json::boolean jbool_1 = true;
    json::boolean jbool_2 = false;

    int int_1 = 1;
    long int_2 = 2l;
    json::integer jint_1 = 3;
    json::integer jint_2 = 4l;
    json::integer jint_3 = int_1;
    json::integer jint_4 = int_2;
    json::integer& jint_5 = jint_1;
    json::integer& jint_6 = jint_2;
    const json::integer& jint_7 = jint_3;
    const json::integer& jint_8 = jint_4;
    const json::integer& jint_9 = jint_7;
    const json::integer& jint_10 = jint_8;

    float num_1 = 1.1f;
    double num_2 = 2.2;
    json::number jnum_1 = 3.3f;
    json::number jnum_2 = 4.4;
    json::number& jnum_3 = jnum_1;
    json::number& jnum_4 = jnum_2;
    const json::number& jnum_5 = jnum_3;
    const json::number& jnum_6 = jnum_4;
    float num_3 = jnum_3;
    float num_4 = jnum_4;
    float num_5 = jnum_5;
    float num_6 = jnum_6;
    num_1 = jnum_3;
    num_2 = jnum_4;
    num_1 = jnum_5;
    num_2 = jnum_6;

    std::string str_1 = "Hello, world!";
    json::string jstr_1 = str_1;
    json::string& jstr_2 = jstr_1;
    const json::string jstr_3 = str_1;
    std::string str_2 = jstr_1;
    std::string str_3 = jstr_2;
    std::string str_4 = jstr_3;
    jstr_1 = "Hello, world?";
    jstr_2 = "Hello, world??";
    
}

int main() {
    test_1();

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

    //json::object j1({{"a", 1}});
    //json::object j2({{"4", "4"}, {"5", "5"}});
    //json::object j3(j2);
    //json::object j4({{"d", 5}});

    //json::object j11;
    //j11.insert({"name", "GyuHyen Choi"});
    //j11.insert({"age", 31});
    //j11.insert({"eyesight", 1.5});
    //j11.insert({"body", "174cm / 62.5kg"});
    //j11.insert({"body", json::object()});
    //j11.find("body")->second.insert({"height", 174});
    //j11.find("body")->second.insert({"weight", 62.5});
    //j11.insert({"book", json::array({"incerto 1", "incerto 2", "incerto 3"})});
    //j11.insert({"numbers", json::array({2, 4, 8, 11, 15})});
    //j11.find("book")->second.push_back(json::object({{"4", 4}}));
    //j11.find("book")->second.push_back(true);
    //j11.find("book")->second.push_back(1);
    //j11.find("book")->second.push_back(1.2);
    //std::cout << j11.j_str() << std::endl;

    //json::object j12({
    //    {"name", "Wiany Illia"},
    //    {"age", 20},
    //    {"body", json::array({"value", "174cm / 65kg"})}
    //});
    //std::cout << j12 << std::endl;

    //std::ifstream f("sample.jsonl");
    //std::string line;
    //while (std::getline(f, line)) {
    //    std::cout << "i: " << line 
    //              << std::endl;
    //    json::object j = json::parse(line);
    //    std::cerr << "o: " << j.j_str()
    //              << std::endl
    //              << std::endl;
    //}
    
    return 0;
}
