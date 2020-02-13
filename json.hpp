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
    private: static std::string escape(const std::string& s) {
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

        std::string type() {
            if      (dynamic_cast<const json::null*   >(this)) return "json::null";
            else if (dynamic_cast<const json::boolean*>(this)) return "json::boolean";
            else if (dynamic_cast<const json::integer*>(this)) return "json::integer";
            else if (dynamic_cast<const json::number* >(this)) return "json::number";
            else if (dynamic_cast<const json::string* >(this)) return "json::string";
            else if (dynamic_cast<const json::array*  >(this)) return "json::array";
            else if (dynamic_cast<const json::object* >(this)) return "json::object";
            else throw std::invalid_argument("Not 'json::value' type...");
        }
    };

    public: struct null : json::base {};

    public: struct boolean : json::base {
        bool v;
        boolean(bool v) : v(v) {}
    };

    public: struct integer : json::base {
        long v;
        integer(long v) : v(v) {}
    };

    public: struct number : json::base {
        double v;
        number(double v) : v(v) {}
    };

    public: struct string : json::base {
        std::string v;
        string(const std::string& v) : v(v) {}
    };

    public: struct array;

    public: struct object;

    private: struct value {
        json::base* v = nullptr;

        value() {}

        value(const json::null& v) : v(new json::null(v)) {}

        value(bool v)                 : v(new json::boolean(v)) {}
        value(const json::boolean& v) : v(new json::boolean(v)) {}

        value(int v)                  : v(new json::integer(v)) {}
        value(long v)                 : v(new json::integer(v)) {}
        value(const json::integer& v) : v(new json::integer(v)) {}

        value(float v)               : v(new json::number(v)) {}
        value(double v)              : v(new json::number(v)) {}
        value(const json::number& v) : v(new json::number(v)) {}

        value(const json::string& v) : v(new json::string(v)) {}

        value(const json::array& v) : v(new json::array(v)) {}

        value(const json::object& v) : v(new json::object(v)) {}

        value(const char* v) {
            if (v) this->v = new json::string(v);
            else   this->v = new json::null();
        }

        value(const json::value& that) {*this = that;}

        json::value& operator=(const json::value& that) {
            if (this != &that) {
                if (this->v) this->~value();

                if      (const json::null*    v = dynamic_cast<const json::null*   >(that.v)) this->v = new json::null   (  );
                else if (const json::boolean* v = dynamic_cast<const json::boolean*>(that.v)) this->v = new json::boolean(*v);
                else if (const json::integer* v = dynamic_cast<const json::integer*>(that.v)) this->v = new json::integer(*v);
                else if (const json::number*  v = dynamic_cast<const json::number* >(that.v)) this->v = new json::number (*v);
                else if (const json::string*  v = dynamic_cast<const json::string* >(that.v)) this->v = new json::string (*v);
                else if (const json::array*   v = dynamic_cast<const json::array*  >(that.v)) this->v = new json::array  (*v);
                else if (const json::object*  v = dynamic_cast<const json::object* >(that.v)) this->v = new json::object (*v);
            }
            return *this;
        }

        ~value() {delete this->v;}

        void push_back(const json::value& v) {
            if (json::array* that_v = dynamic_cast<json::array*>(this->v)) that_v->push_back(v);
            else throw std::runtime_error("'" + this->v->type() + "' has no member named 'push_back'");
        }

        void insert(const std::pair<std::string, json::value>& v) {
            if (json::object* that_v = dynamic_cast<json::object*>(this->v)) that_v->insert(v);
            else throw std::runtime_error("'" + this->v->type() + "' has no member named 'insert'");
        }
    };

    public: struct array : json::base {
        std::vector<json::value> v;

        array() {}

        array(const std::vector<json::value>& v) : v(v) {}

        array(const json::value& j) {
            if (json::array* that = dynamic_cast<json::array*>(j.v)) this->v = that->v;
            else throw std::invalid_argument("cannot be json::array...");
        }

        bool empty() const {return this->v.empty();}

        std::size_t size() const {return this->v.size();}

        json::value& back() {return this->v.back();}

        void push_back(const json::value& v) {this->v.push_back(v);}
    };

    public: struct object : json::base {
        std::unordered_map<std::string, json::value> v;

        object() {}

        object(const std::unordered_map<std::string, json::value>& v) : v(v) {}

        object(const json::value& j) {
            if (json::object* that = dynamic_cast<json::object*>(j.v)) this->v = that->v;
            else throw std::invalid_argument("cannot be json::object...");
        }

        bool empty() const {return this->v.empty();}

        std::size_t size() const {return this->v.size();}

        std::unordered_map<std::string, json::value>::iterator find(const std::string& k) {return this->v.find(k);}

        void insert(const std::pair<std::string, json::value>& p) {this->v[std::get<0>(p)] = json::value(std::get<1>(p));}
    };

    private: struct parsing {
        enum status {
            READING_OBJECT_KEY,
            READ_OBJECT_KEY,
            READING_OBJECT_VALUE, READING_ARRAY_VALUE,
            READ_OBJECT_VALUE, READ_ARRAY_VALUE, READ_OTHER_VALUE
        };

        std::stack<json::parsing::status> ps;
        std::stack<json::base*> js;

        std::string key;
        json::value value;
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
    public: static json::value parse(const std::string& s) {
        json::value j;
        json::parsing p(s);

        while (p.iss.get(p.c)) {
            if (p.c == ' ') {
            } else if (p.c == '{') {
                p.ps.push(json::parsing::READING_OBJECT_KEY);
                j = json::object();
                p.js.push(j.v);
                break;
            } else if (p.c == '[') {
                p.ps.push(json::parsing::READING_ARRAY_VALUE);
                j = json::array();
                p.js.push(j.v);
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

        return j;
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

        if (p.ps.top() == json::parsing::READ_OTHER_VALUE) {
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

        if (p.ps.top() == json::parsing::READ_OTHER_VALUE) {
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
        if (p.ps.top() == json::parsing::READ_OBJECT_KEY) {
            p.ps.push(json::parsing::READING_OBJECT_VALUE);
        } else {
            throw std::invalid_argument(std::string("Invalid character ':' at "));
        }
    }
    private: static void parse__comma(json::parsing& p) {
        std::cerr << "parse__comma"
                  << " | p.ps.top() = " << p.ps.top()
                  << std::endl;
        if (p.ps.top() == json::parsing::READ_OTHER_VALUE) {
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
            p.ps.push(json::parsing::READ_OBJECT_KEY);
        } else if (
            p.ps.top() == json::parsing::READING_OBJECT_VALUE ||
            p.ps.top() == json::parsing::READING_ARRAY_VALUE
        ) {
            p.value = json::value(s);
            p.ps.push(json::parsing::READ_OTHER_VALUE);
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
            std::stringstream ss;
            ss << p.c;
            while (p.iss.get(p.c)) {
                if (p.c == ' ' || p.c == ',' || p.c == ']' || p.c == '}') {
                    std::string v = ss.str();
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
                    p.ps.push(json::parsing::READ_OTHER_VALUE);
                    p.iss.seekg(-1, std::ios::cur);
                    break;
                } else {
                    ss << p.c;
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

