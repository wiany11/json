#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

class json {
    private: struct value;
    public: struct array;
    public: struct object;

    public: static std::string escape(const std::string& s) {
        std::stringstream ss;
        for (std::size_t i = 0; i < s.size(); i++) {
            switch (s.at(i)) {
                case '\b': ss << "\\b";   break;
                case '\f': ss << "\\f";   break;
                case '\n': ss << "\\n";   break;
                case '\r': ss << "\\r";   break;
                case '\t': ss << "\\t";   break;
                case '\"': ss << "\\\"";  break;
                case '\\': ss << "\\";    break;
                default  : ss << s.at(i); break;
            }
        }
        return ss.str();
    }

    private: struct base {
        virtual ~base() = default;
    };

    public: struct null : json::base {
        null() {}
    };

    public: struct boolean : json::base {
        bool v;

        boolean(bool v) {
            this->v = v;
        }
    };

    public: struct integer : json::base {
        long v;

        integer(long v) {
            this->v = v;
        }
    };

    public: struct number : json::base {
        double v;

        number(double v) {
            this->v = v;
        }
    };

    public: struct string : json::base {
        std::string v;

        string(const std::string& v) {
            this->v = v;
        }
    };

    private: struct value {
        json::base* v = nullptr;

        value() {}

        value(bool v) {
            this->v = new json::boolean(v);
        }

        value(long v) {
            this->v = new json::integer(v);
        }

        value(int v) : value(static_cast<long>(v)) {}

        value(double v) {
            this->v = new json::number(v);
        }

        value(float v) : value(static_cast<double>(v)) {}

        value(const char* v) {
            if (v) {
                this->v = new json::string(v);
            } else {
                this->v = new json::null();
            }
        } 

        value(const std::vector<json::value>& v) {
            this->v = new json::array(v);
        }

        value(const std::unordered_map<std::string, json::value>& v) {
            this->v = new json::object(v);
        }

        value(const json::null& v) {
            this->v = new json::null(v);
        }

        value(const json::boolean& v) {
            this->v = new json::boolean(v);
        }

        value(const json::integer& v) {
            this->v = new json::integer(v);
        }

        value(const json::number& v) {
            this->v = new json::number(v);
        }

        value(const json::string& v) {
            this->v = new json::string(v);
        }

        value(const json::array& v) {
            this->v = new json::array(v);
        }

        value(const json::object& v) {
            this->v = new json::object(v);
        }

        value(const json::value& that) {
            *this = that;
        }

        json::value& operator=(const json::value& that) {
            if (this != &that) {
                if (this->v) {
                    this->~value();
                }
                if (const json::null* v = dynamic_cast<const json::null*>(that.v)) {
                    this->v = new json::null();
                } else if (const json::boolean* v = dynamic_cast<const json::boolean*>(that.v)) {
                    this->v = new json::boolean(*v);
                } else if (const json::integer* v = dynamic_cast<const json::integer*>(that.v)) {
                    this->v = new json::integer(*v);
                } else if (const json::number* v = dynamic_cast<const json::number*>(that.v)) {
                    this->v = new json::number(*v);
                } else if (const json::string* v = dynamic_cast<const json::string*>(that.v)) {
                    this->v = new json::string(*v);
                } else if (const json::array* v = dynamic_cast<const json::array*>(that.v)) {
                    this->v = new json::array(*v);
                } else if (const json::object* v = dynamic_cast<const json::object*>(that.v)) {
                    this->v = new json::object(*v);
                }
            }
            return *this;
        }
        
        ~value() {
            delete this->v;
        }

        void push_back(const json::value& v) {
            static_cast<json::array*>(this->v)->push_back(v);
        }

        void insert(const std::pair<std::string, json::value>& v) {
            static_cast<json::object*>(this->v)->insert(v);
        }
    };

    public: struct array : json::base {
        std::vector<json::value> v;

        array() {}

        array(const std::vector<json::value>& v) {
            this->v = v;
        }

        void push_back(const json::value& v) {
            this->v.push_back(v);            
        }

        bool empty() const {
            return this->v.empty();
        }

        std::size_t size() const {
            return this->v.size();
        }

        json::value& back() {
            return this->v.back();
        }
    };

    public: struct object : json::base {
        std::unordered_map<std::string, json::value> v;

        object() {}

        object(const std::unordered_map<std::string, json::value>& v) {
            this->v = v;
        }

        void insert(const std::pair<std::string, json::value>& p) {
            std::unordered_map<std::string, json::value>::iterator it = this->v.find(std::get<0>(p));
            if (it != this->v.end()) {
                it->second = json::value(std::get<1>(p));
            } else {
                this->v.insert({std::get<0>(p), json::value(std::get<1>(p))});
            }
        }

        bool empty() const {
            return this->v.empty();
        }

        std::size_t size() const {
            return this->v.size();
        }

        std::unordered_map<std::string, json::value>::iterator find(const std::string& k) {
            return this->v.find(k);
        }
    }; 

    private: struct parsing {
        enum status {
            READING_OBJECT_KEY,
            READ_KEY,
            READING_OBJECT_VALUE, READING_ARRAY_VALUE,
            READ_OBJECT_VALUE, READ_ARRAY_VALUE,
            READ_VALUE
        };

        std::stack<json::parsing::status> ps;
        std::stack<json::base*> js;

        json::object jo;
        json::array ja;
        std::string key;
        json::value value;
        std::stringstream s;
        std::size_t no = 0;
        std::size_t na = 0;

        char c;
        std::size_t i = 1;
        std::istringstream iss;
        
        parsing(const std::string& s) {
            this->iss.str(s);
        }

        void finish_reading() {
            this->ps.pop();  // READING_*_VALUE
            if (this->ps.top() == json::parsing::READING_OBJECT_VALUE) { // READING_OBEJCT_VALUE
                this->ps.pop();
                this->ps.pop();  // READING_OBJECT_KEY
            }
        }
    };
    public: static void parse(const std::string& s, json::base* j) {
        json::parsing p(s);
        p.js.push(j);

        while (p.iss.get(p.c)) {
            if (p.c == ' ') {
            } else if (p.c == '{') {
                p.ps.push(json::parsing::READING_OBJECT_KEY);
                break;
            } else if (p.c == '[') {
                p.ps.push(json::parsing::READING_ARRAY_VALUE);
                break;
            }
        }

        while (p.iss.get(p.c)) {
            p.i++;
            if      (p.c == ' ' ) {}
            else if (p.c == '{' ) json::parse__left_brace(p);
            else if (p.c == '}' ) json::parse__right_brace(p);
            else if (p.c == '[' ) json::parse__left_bracket(p);
            else if (p.c == ']' ) json::parse__right_bracket(p);
            else if (p.c == ':' ) json::parse__colon(p); 
            else if (p.c == ',' ) json::parse__comma(p); 
            else if (p.c == '\"') json::parse__quote(p);
            else                  json::parse__else(p);  
        }
    }
    private: static void parse__left_brace(json::parsing& p) {
        if (p.ps.top() == json::parsing::READING_OBJECT_VALUE) {
            static_cast<json::object*>(p.js.top())->insert({p.key, json::object()});
            p.js.push(static_cast<json::object*>(p.js.top())->find(p.key)->second.v);
        } else if (p.ps.top() == json::parsing::READING_ARRAY_VALUE) {
            static_cast<json::array*>(p.js.top())->push_back(json::object());
            p.js.push(static_cast<json::array*>(p.js.top())->back().v);
        } else {
            throw std::invalid_argument(std::string("Invalid character '{' at "));
        }
        p.no++;
        p.ps.push(json::parsing::READING_OBJECT_KEY);
    }
    private: static void parse__right_brace(json::parsing& p) {
        if (p.no == 0) {
            // error
        }
        p.no--;

        if (p.ps.top() == json::parsing::READ_VALUE) {
            p.finish_reading();
            static_cast<json::object*>(p.js.top())->insert({p.key, p.value});
        } else if (
            p.ps.top() == json::parsing::READ_OBJECT_VALUE ||
            p.ps.top() == json::parsing::READ_ARRAY_VALUE
        ) {
            p.finish_reading();
        } else {
            throw std::invalid_argument(std::string("Invalid character 'finish' at "));
        }
        p.ps.pop();
        p.ps.push(json::parsing::READ_OBJECT_VALUE);
        p.js.pop();
    }
    private: static void parse__left_bracket(json::parsing& p) {
        if (p.ps.top() == json::parsing::READING_OBJECT_VALUE) {
            static_cast<json::object*>(p.js.top())->insert({p.key, json::array()});
            p.js.push(static_cast<json::object*>(p.js.top())->find(p.key)->second.v);
        } else if (p.ps.top() == json::parsing::READING_ARRAY_VALUE) {
            static_cast<json::array*>(p.js.top())->push_back(json::array());
            p.js.push(static_cast<json::array*>(p.js.top())->back().v);
        } else {
            throw std::invalid_argument(std::string("Invalid character '{' at "));
        }
        p.na++;
        p.ps.push(json::parsing::READING_ARRAY_VALUE);
    }
    private: static void parse__right_bracket(json::parsing& p) {
        if (p.na == 0) {
            // error
        }
        p.na--;

        if (p.ps.top() == json::parsing::READ_VALUE) {
            p.finish_reading();
            static_cast<json::array*>(p.js.top())->push_back(p.value);
        } else if (
            p.ps.top() == json::parsing::READ_OBJECT_VALUE ||
            p.ps.top() == json::parsing::READ_ARRAY_VALUE
        ) {
            p.finish_reading();
        } else {
            throw std::invalid_argument(std::string("Invalid character 'finish bracket' at "));
        }
        p.ps.pop();
        p.ps.push(json::parsing::READ_ARRAY_VALUE);
        p.js.pop();
    }
    private: static void parse__colon(json::parsing& p) {
        if (p.ps.top() == json::parsing::READ_KEY) {
            p.ps.push(json::parsing::READING_OBJECT_VALUE);
        } else {
            throw std::invalid_argument(std::string("Invalid character ':' at "));
        }
    }
    private: static void parse__comma(json::parsing& p) {
        std::cerr << "parse__comma"
                  << " | p.ps.top() = " << p.ps.top()
                  << std::endl;
        if (p.ps.top() == json::parsing::READ_VALUE) {
            p.finish_reading();
            if (p.ps.top() == json::parsing::READING_OBJECT_KEY) {
                static_cast<json::object*>(p.js.top())->insert({p.key, p.value});
            } else {  // json::parsing::READING_ARRAY_VALUE
                static_cast<json::array*>(p.js.top())->push_back(p.value);
            }
        } else if (
            p.ps.top() == json::parsing::READ_OBJECT_VALUE ||
            p.ps.top() == json::parsing::READ_ARRAY_VALUE
        ) {
            p.finish_reading();
        } else {
            throw std::invalid_argument(std::string("Invalid character ',' at "));
        }
    }
    private: static void parse__quote(json::parsing& p) {
        std::stringstream ss;
        bool escaping = false;
        while (p.iss.get(p.c)) {
            if (escaping) {
                if      (p.c == '\\') ss << '\\';
                else if (p.c == '\"') ss << '\"';
                else if (p.c == 'b' ) ss << '\b';
                else if (p.c == 'f' ) ss << '\f';
                else if (p.c == 'n' ) ss << '\n';
                else if (p.c == 'r' ) ss << '\r';
                else if (p.c == 't' ) ss << '\t';
                else {
                    //exception
                }
                escaping = false;
            } else {
                if      (p.c == '\\') escaping = true;
                else if (p.c == '\"') break;
                else                  ss << p.c;
            }
        }

        std::string s = ss.str();
        if (p.ps.top() == json::parsing::READING_OBJECT_KEY) {
            p.key = s;
            p.ps.push(json::parsing::READ_KEY);
        } else if (
            p.ps.top() == json::parsing::READING_OBJECT_VALUE ||
            p.ps.top() == json::parsing::READING_ARRAY_VALUE
        ) {
            p.value = json::value(s);
            p.ps.push(json::parsing::READ_VALUE);
        }
    }
    private: static void parse__else(json::parsing& p) {
        std::cerr << "json::parse__else"
                  << " | p.c = " << p.c
                  << " | p.ps.top() = " << p.ps.top()
                  << std::endl;
        if (
            p.ps.top() == json::parsing::READING_OBJECT_VALUE || 
            p.ps.top() == json::parsing::READING_ARRAY_VALUE
        ) {
            p.s.str("");
            p.s.clear();
            p.s << p.c;
            while (p.iss.get(p.c)) {
                if (p.c == ' ' || p.c == ',' || p.c == ']' || p.c == '}') {
                    std::string v = p.s.str();
                    std::cerr << "json::parse__else | while | if"
                              << " | v = " << v
                              << std::endl;
                    if (v == "null") {
                        p.value = json::null();
                    } else if (v == "true") {
                        p.value = json::boolean(true);
                    } else if (v == "false") {
                        p.value = json::boolean(false);
                    } else {
                        std::cerr << "json::parse__else | while | else 0"
                                << std::endl;
                        if (v.find('.') != std::string::npos) {
                            p.value = json::number(std::stod(v));
                        } else {
                            p.value = json::integer(std::stol(v));
                        }
                        std::cerr << "json::parse__else | while | else 1"
                                << std::endl;
                    }
                    p.ps.push(json::parsing::READ_VALUE);
                    p.iss.seekg(-1, std::ios::cur);
                    break;
                } else {
                    p.s << p.c;
                }
            }
        } else {
            throw std::invalid_argument(std::string("Invalid character 'else' at "));
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const json::base& j);
};

std::ostream& operator<<(std::ostream& os, const json::base& j) {
    if (const json::null* v = dynamic_cast<const json::null*>(&j)) {
        os << "null";
    } else if (const json::boolean* v = dynamic_cast<const json::boolean*>(&j)) {
        os << (v->v ? "true" : "false");
    } else if (const json::integer* v = dynamic_cast<const json::integer*>(&j)) {
        os << v->v;
    } else if (const json::number* v = dynamic_cast<const json::number*>(&j)) {
        os << std::showpoint << v->v;
    } else if (const json::string* v = dynamic_cast<const json::string*>(&j)) {
        os << '\"' << json::escape(v->v) << '\"';
    } else if (const json::array* v = dynamic_cast<const json::array*>(&j)) {
        if (v->empty()) {
            os << "[]";
        } else {
            os << '[';
            std::vector<json::value>::const_iterator it = v->v.begin();
            for (; it != v->v.end(); ++it) {
                os << *it->v << ',';
            }
            os << "\b]";
        }
    } else if (const json::object* v = dynamic_cast<const json::object*>(&j)) {
        if (v->empty()) {
            os << "{}";
        } else {
            os << '{';
            std::unordered_map<std::string, json::value>::const_iterator it = v->v.begin();
            for (; it != v->v.end(); ++it) {
                os << '\"' << json::escape(it->first) << "\":" << *it->second.v << ',';
            }
            os << "\b}";
        }
    }
    return os;
}

