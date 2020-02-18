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
    public: struct null;
    public: struct boolean;
    public: struct integer;
    public: struct number;
    public: struct string;
    public: struct array;
    public: struct object;

    private: static void quote_and_escape(const char* s, std::size_t n, std::string& jstr) {
        jstr += '\"';
        for (std::size_t i = 0; i < n; i++) {
            switch (*(s + i)) {
                case '\b': jstr += "\\b";    break;
                case '\f': jstr += "\\f";    break;
                case '\n': jstr += "\\n";    break;
                case '\r': jstr += "\\r";    break;
                case '\t': jstr += "\\t";    break;
                case '\"': jstr += "\\\"";   break;
                case '\\': jstr += '\\';     break;
                default  : jstr += *(s + i); break;
            }
        }
        jstr += '\"';
    }

    private: struct base {
        private: static void str(json::base* that, std::string& jstr) {
            if (dynamic_cast<const json::null*>(that)) {
                jstr += "null";
            } else if (json::boolean* j = dynamic_cast<json::boolean*>(that)) {
                jstr += static_cast<bool>(*j) ? "true" : "false";
            } else if (json::integer* j = dynamic_cast<json::integer*>(that)) {
                jstr += std::to_string(*j);
            } else if (json::number* j = dynamic_cast<json::number*>(that)) {
                jstr += std::to_string(*j);
            } else if (const json::string* j = dynamic_cast<const json::string*>(that)) {
                json::quote_and_escape(j->c_str(), j->size(), jstr);
            } else if (const json::array* j = dynamic_cast<const json::array*>(that)) {
                jstr += '[';
                for (const json::value& v : *j) {
                    json::base::str(v.v, jstr);
                    jstr += ',';
                }
                jstr.pop_back();
                jstr += ']';
            } else if (const json::object* j = dynamic_cast<const json::object*>(that)) {
                jstr += '{';
                for (const std::pair<std::string, json::value>& p : *j) {
                    json::quote_and_escape(p.first.c_str(), p.first.size(), jstr);
                    jstr += ':';
                    json::base::str(p.second.v, jstr);
                    jstr += ',';
                }
                jstr.pop_back();
                jstr += '}';
            }
        }

        public: virtual ~base() = default;

        public: template<typename T> bool is_type_of() const {
            if (dynamic_cast<const T*>(this)) return true;
            else                              return false;
        }

        public: std::string str() {
            std::string jstr;
            json::base::str(this, jstr);
            return jstr;
        }
    };

    public: struct null : json::base {};

    public: struct boolean : json::base {
        private: bool v;
        public:  boolean(bool v) : v(v) {}
        public:  operator bool() {return this->v;}
    };

    public: struct integer : json::base {
        private: long v;
        public:  integer(long v) : v(v) {}
        public:  operator long() {return this->v;}
    };

    public: struct number : json::base {
        private: double v;
        public:  number(double v) : v(v) {}
        public:  operator double() {return this->v;}
    };

    public: struct string : json::base {
        private: std::string v;
        public:  string(const std::string& v) : v(v) {}
        public:  std::size_t size() const {return this->v.size();}
        public:  const char* c_str() const {return this->v.c_str();}
        public:  operator std::string() {return this->v;}
    };

    private: struct value {
        json::base* v = nullptr;

        value() {}
        value(const json::value& that) {*this = that;}

        value(const json::null&    v) : v(new json::null   (v)) {}
        value(bool                 v) : v(new json::boolean(v)) {}
        value(const json::boolean& v) : v(new json::boolean(v)) {}
        value(int                  v) : v(new json::integer(v)) {}
        value(long                 v) : v(new json::integer(v)) {}
        value(const json::integer& v) : v(new json::integer(v)) {}
        value(float                v) : v(new json::number (v)) {}
        value(double               v) : v(new json::number (v)) {}
        value(const json::number&  v) : v(new json::number (v)) {}
        value(const json::string&  v) : v(new json::string (v)) {}
        value(const json::array&   v) : v(new json::array  (v)) {}
        value(const json::object&  v) : v(new json::object (v)) {}

        value(const char* v) {
            if (v) this->v = new json::string(v);
            else   this->v = new json::null();
        }

        json::value& operator=(const json::value& that) {
            if (this != &that) {
                if (this->v) this->~value();

                if      (                         dynamic_cast<const json::null*   >(that.v)) this->v = new json::null   (  );
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

        template<typename T> bool is_type_of() const {
            return this->v->is_type_of<T>();
        }

        void push_back(const json::value& v) {
            if (json::array* that_v = dynamic_cast<json::array*>(this->v)) {
                that_v->push_back(v);
            } else {
                std::string type;
                if      (this->v->is_type_of<json::null   >()) type = "json::null"   ;
                else if (this->v->is_type_of<json::boolean>()) type = "json::boolean";
                else if (this->v->is_type_of<json::integer>()) type = "json::integer";
                else if (this->v->is_type_of<json::number >()) type = "json::number" ;
                else if (this->v->is_type_of<json::string >()) type = "json::string" ;
                else if (this->v->is_type_of<json::object >()) type = "json::object" ;
                throw std::runtime_error("'" + type + "' has no member named 'push_back'");
            }
        }

        void insert(const std::pair<std::string, json::value>& v) {
            if (json::object* that_v = dynamic_cast<json::object*>(this->v)) {
                that_v->insert(v);
            } else {
                std::string type;
                if      (this->v->is_type_of<json::null   >()) type = "json::null"   ;
                else if (this->v->is_type_of<json::boolean>()) type = "json::boolean";
                else if (this->v->is_type_of<json::integer>()) type = "json::integer";
                else if (this->v->is_type_of<json::number >()) type = "json::number" ;
                else if (this->v->is_type_of<json::string >()) type = "json::string" ;
                else if (this->v->is_type_of<json::array  >()) type = "json::array"  ;
                throw std::runtime_error("'" + type + "' has no member named 'insert'");
            }
        }

        operator bool       () const {return *static_cast<json::boolean*>(this->v);}
        operator long       () const {return *static_cast<json::integer*>(this->v);}
        operator double     () const {return *static_cast<json::number* >(this->v);}
        operator std::string() const {return *static_cast<json::string* >(this->v);}

        operator json::null   () const {return *static_cast<json::null*   >(this->v);}
        operator json::boolean() const {return *static_cast<json::boolean*>(this->v);}
        operator json::integer() const {return *static_cast<json::integer*>(this->v);}
        operator json::number () const {return *static_cast<json::number* >(this->v);}
        operator json::string () const {return *static_cast<json::string* >(this->v);}
        operator json::array  () const {return *static_cast<json::array*  >(this->v);}
        operator json::object () const {return *static_cast<json::object* >(this->v);}
    };

    public: struct array : json::base {
        private: std::vector<json::value> v;

        public: array() {}
        public: array(const std::vector<json::value>& v) : v(v) {}
        public: array(const json::value& j) {this->v = static_cast<json::array*>(j.v)->v;}

        public: bool empty() const {return this->v.empty();}

        public: std::size_t size() const {return this->v.size();}

        public: json::value& back() {return this->v.back();}

        public: void push_back(const json::value& v) {this->v.push_back(v);}

        public: std::vector<json::value>::iterator       begin()       {return this->v.begin();}
        public: std::vector<json::value>::const_iterator begin() const {return this->v.begin();}

        public: std::vector<json::value>::iterator       end()       {return this->v.end();}
        public: std::vector<json::value>::const_iterator end() const {return this->v.end();}
    };

    public: struct object : json::base {
        private: std::unordered_map<std::string, json::value> v;

        public: object() {}
        public: object(const std::unordered_map<std::string, json::value>& v) : v(v) {}
        public: object(const json::value& j) {this->v = static_cast<json::object*>(j.v)->v;}

        public: bool empty() const {return this->v.empty();}

        public: std::size_t size() const {return this->v.size();}

        public: std::unordered_map<std::string, json::value>::iterator find(const std::string& k) {return this->v.find(k);}

        public: void insert(const std::pair<std::string, json::value>& p) {this->v[std::get<0>(p)] = json::value(std::get<1>(p));}

        public: std::unordered_map<std::string, json::value>::iterator       begin()       noexcept {return this->v.begin();}
        public: std::unordered_map<std::string, json::value>::const_iterator begin() const noexcept {return this->v.begin();}

        public: std::unordered_map<std::string, json::value>::iterator       end()       noexcept {return this->v.end();}
        public: std::unordered_map<std::string, json::value>::const_iterator end() const noexcept {return this->v.end();}
    };

    private: struct parsing {
        enum status {
            OBJECT_KEY, OBJECT_KEY_IS_DONE,
            OBJECT_VALUE, OBJECT_VALUE_IS_DONE,
            ARRAY_VALUE, ARRAY_VALUE_IS_DONE,
            OTHER_VALUE
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

        parsing(const std::string& s) : iss(s) {}

        void finish_reading() {
            this->ps.pop();  // parsing::OBJECT/ARRAY_VALUE_IS_DONE -> parsing::OBJECT/ARRAY_VALUE
            if (this->ps.top() == json::parsing::OBJECT_VALUE) { // parsing::OBJECT_VALUE
                this->ps.pop();
                this->ps.pop();  // parsing::OBJECT_KEY
            }
        }
    };
    public: static json::value parse(const std::string& s) {
        json::value j;
        json::parsing p(s);

        while (p.iss.get(p.c)) {
            if (p.c == ' ') {
            } else if (p.c == '{') {
                p.ps.push(json::parsing::OBJECT_KEY);
                j = json::object();
                p.js.push(j.v);
                break;
            } else if (p.c == '[') {
                p.ps.push(json::parsing::ARRAY_VALUE);
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
        if (p.ps.top() == json::parsing::OBJECT_VALUE) {
            static_cast<json::object*>(p.js.top())->insert({p.key, json::object()});
            p.js.push(static_cast<json::object*>(p.js.top())->find(p.key)->second.v);
        } else if (p.ps.top() == json::parsing::ARRAY_VALUE) {
            static_cast<json::array*>(p.js.top())->push_back(json::object());
            p.js.push(static_cast<json::array*>(p.js.top())->back().v);
        } else {
            throw std::invalid_argument(std::string("Invalid character '{' at "));
        }
        p.no++;
        p.ps.push(json::parsing::OBJECT_KEY);
    }
    private: static void parse__right_brace(json::parsing& p) {
        if (p.no == 0) {
            // error
        }
        p.no--;

        if (p.ps.top() == json::parsing::OTHER_VALUE) {
            p.finish_reading();
            static_cast<json::object*>(p.js.top())->insert({p.key, p.value});
        } else if (
            p.ps.top() == json::parsing::OBJECT_VALUE_IS_DONE ||
            p.ps.top() == json::parsing::ARRAY_VALUE_IS_DONE
        ) {
            p.finish_reading();
        } else {
            throw std::invalid_argument(std::string("Invalid character 'finish' at "));
        }
        p.ps.pop();
        p.ps.push(json::parsing::OBJECT_VALUE_IS_DONE);
        p.js.pop();
    }
    private: static void parse__left_bracket(json::parsing& p) {
        if (p.ps.top() == json::parsing::OBJECT_VALUE) {
            static_cast<json::object*>(p.js.top())->insert({p.key, json::array()});
            p.js.push(static_cast<json::object*>(p.js.top())->find(p.key)->second.v);
        } else if (p.ps.top() == json::parsing::ARRAY_VALUE) {
            static_cast<json::array*>(p.js.top())->push_back(json::array());
            p.js.push(static_cast<json::array*>(p.js.top())->back().v);
        } else {
            throw std::invalid_argument(std::string("Invalid character '{' at "));
        }
        p.na++;
        p.ps.push(json::parsing::ARRAY_VALUE);
    }
    private: static void parse__right_bracket(json::parsing& p) {
        if (p.na == 0) {
            // error
        }
        p.na--;

        if (p.ps.top() == json::parsing::OTHER_VALUE) {
            p.finish_reading();
            static_cast<json::array*>(p.js.top())->push_back(p.value);
        } else if (
            p.ps.top() == json::parsing::OBJECT_VALUE_IS_DONE ||
            p.ps.top() == json::parsing::ARRAY_VALUE_IS_DONE
        ) {
            p.finish_reading();
        } else {
            throw std::invalid_argument(std::string("Invalid character 'finish bracket' at "));
        }
        p.ps.pop();
        p.ps.push(json::parsing::ARRAY_VALUE_IS_DONE);
        p.js.pop();
    }
    private: static void parse__colon(json::parsing& p) {
        if (p.ps.top() == json::parsing::OBJECT_KEY_IS_DONE) {
            p.ps.push(json::parsing::OBJECT_VALUE);
        } else {
            throw std::invalid_argument(std::string("Invalid character ':' at "));
        }
    }
    private: static void parse__comma(json::parsing& p) {
        if (p.ps.top() == json::parsing::OTHER_VALUE) {
            p.finish_reading();
            if (p.ps.top() == json::parsing::OBJECT_KEY) {
                static_cast<json::object*>(p.js.top())->insert({p.key, p.value});
            } else {  // json::parsing::ARRAY_VALUE
                static_cast<json::array*>(p.js.top())->push_back(p.value);
            }
        } else if (
            p.ps.top() == json::parsing::OBJECT_VALUE_IS_DONE ||
            p.ps.top() == json::parsing::ARRAY_VALUE_IS_DONE
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
        if (p.ps.top() == json::parsing::OBJECT_KEY) {
            p.key = s;
            p.ps.push(json::parsing::OBJECT_KEY_IS_DONE);
        } else if (
            p.ps.top() == json::parsing::OBJECT_VALUE ||
            p.ps.top() == json::parsing::ARRAY_VALUE
        ) {
            p.value = json::value(s);
            p.ps.push(json::parsing::OTHER_VALUE);
        }
    }
    private: static void parse__else(json::parsing& p) {
        if (
            p.ps.top() == json::parsing::OBJECT_VALUE ||
            p.ps.top() == json::parsing::ARRAY_VALUE
        ) {
            std::stringstream ss;
            ss << p.c;
            while (p.iss.get(p.c)) {
                if (p.c == ' ' || p.c == ',' || p.c == ']' || p.c == '}') {
                    std::string v = ss.str();
                    if (v == "null") {
                        p.value = json::null();
                    } else if (v == "true") {
                        p.value = json::boolean(true);
                    } else if (v == "false") {
                        p.value = json::boolean(false);
                    } else {
                        if (v.find('.') != std::string::npos) {
                            p.value = json::number(std::stod(v));
                        } else {
                            p.value = json::integer(std::stol(v));
                        }
                    }
                    p.ps.push(json::parsing::OTHER_VALUE);
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

    friend std::ostream& operator<<(std::ostream& os, json::null&    j);
    friend std::ostream& operator<<(std::ostream& os, json::boolean& j);
    friend std::ostream& operator<<(std::ostream& os, json::integer& j);
    friend std::ostream& operator<<(std::ostream& os, json::number&  j);
    friend std::ostream& operator<<(std::ostream& os, json::string&  j);
    friend std::ostream& operator<<(std::ostream& os, json::value&   j);
};

std::ostream& operator<<(std::ostream& os, json::null& j) {
    (void) j;
    os << "null";
    return os;
}

std::ostream& operator<<(std::ostream& os, json::boolean& j) {
    os << (static_cast<bool>(j) ? "true" : "false");
    return os;
}

//std::ostream& operator<<(std::ostream& os, json::integer& j) {
//    os << static_cast<long>(j);
//    return os;
//}

//std::ostream& operator<<(std::ostream& os, json::number& j) {
//    os << static_cast<double>(j);
//    return os;
//}

std::ostream& operator<<(std::ostream& os, json::string& j) {
    os << j.c_str();
    return os;
}

std::ostream& operator<<(std::ostream& os, json::array& j) {
    os << "json::array of size " << j.size();
    return os;
}

std::ostream& operator<<(std::ostream& os, json::object& j) {
    os << "json::object of size " << j.size();
    return os;
}

std::ostream& operator<<(std::ostream& os, json::value& j) {
    if      (json::null*    v = dynamic_cast<json::null*   >(j.v)) os << *v;
    else if (json::boolean* v = dynamic_cast<json::boolean*>(j.v)) os << *v;
    //else if (json::integer* v = dynamic_cast<json::integer*>(j.v)) os << *v;
    //else if (json::number*  v = dynamic_cast<json::number* >(j.v)) os << *v;
    else if (json::string*  v = dynamic_cast<json::string* >(j.v)) os << *v;
    else if (json::array*   v = dynamic_cast<json::array*  >(j.v)) os << *v;
    else if (json::object*  v = dynamic_cast<json::object* >(j.v)) os << *v;
    return os;
}

