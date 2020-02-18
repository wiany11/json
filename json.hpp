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
        private: static void j_str(json::base* that, std::string& jstr) {
            if (dynamic_cast<const json::null*>(that)) {
                jstr += "null";
            } else if (const json::boolean* j = dynamic_cast<const json::boolean*>(that)) {
                jstr += static_cast<bool>(*j) ? "true" : "false";
            } else if (const json::integer* j = dynamic_cast<const json::integer*>(that)) {
                jstr += std::to_string(*j);
            } else if (const json::number* j = dynamic_cast<const json::number*>(that)) {
                jstr += std::to_string(*j);
            } else if (const json::string* j = dynamic_cast<const json::string*>(that)) {
                json::quote_and_escape(j->c_str(), j->size(), jstr);
            } else if (const json::array* j = dynamic_cast<const json::array*>(that)) {
                jstr += '[';
                for (const json::value& e : *j) {
                    json::base::j_str(e.v, jstr);
                    jstr += ',';
                }
                jstr.pop_back();
                jstr += ']';
            } else if (const json::object* j = dynamic_cast<const json::object*>(that)) {
                jstr += '{';
                for (const std::pair<std::string, json::value>& e : *j) {
                    json::quote_and_escape(e.first.c_str(), e.first.size(), jstr);
                    jstr += ':';
                    json::base::j_str(e.second.v, jstr);
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

        public: std::string j_str() {
            std::string jstr;
            json::base::j_str(this, jstr);
            return jstr;
        }
    };

    public: struct null : json::base {};

    public: struct boolean : json::base {
        private: bool v;
        public:  boolean(bool v) : v(v) {}
        public:  operator bool() const {return this->v;}
    };

    public: struct integer : json::base {
        private: long v;
        public:  integer(long v) : v(v) {}
        public:  operator long() const {return this->v;}
    };

    public: struct number : json::base {
        private: double v;
        public:  number(double v) : v(v) {}
        public:  operator double() const {return this->v;}
    };

    public: struct string : json::base {
        private: std::string v;
        public:  string(const std::string& v) : v(v) {}
        public:  std::size_t size() const {return this->v.size();}
        public:  const char* c_str() const {return this->v.c_str();}
        public:  operator std::string() const {return this->v;}
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
                if      (dynamic_cast<json::null*   >(this->v)) type = "json::null";
                else if (dynamic_cast<json::boolean*>(this->v)) type = "json::boolean";
                else if (dynamic_cast<json::integer*>(this->v)) type = "json::integer";
                else if (dynamic_cast<json::number* >(this->v)) type = "json::number";
                else if (dynamic_cast<json::string* >(this->v)) type = "json::string";
                else if (dynamic_cast<json::object* >(this->v)) type = "json::object";
                throw std::runtime_error("'" + type + "' has no member named 'push_back'");
            }
        }

        void insert(const std::pair<std::string, json::value>& v) {
            if (json::object* that_v = dynamic_cast<json::object*>(this->v)) {
                that_v->insert(v);
            } else {
                std::string type;
                if      (dynamic_cast<json::null*   >(this->v)) type = "json::null";
                else if (dynamic_cast<json::boolean*>(this->v)) type = "json::boolean";
                else if (dynamic_cast<json::integer*>(this->v)) type = "json::integer";
                else if (dynamic_cast<json::number* >(this->v)) type = "json::number";
                else if (dynamic_cast<json::string* >(this->v)) type = "json::string";
                else if (dynamic_cast<json::array*  >(this->v)) type = "json::array";
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
        enum state {
            OBJECT_KEY, OBJECT_KEY_IS_DONE,
            OBJECT_VALUE, OBJECT_VALUE_IS_DONE,
            ARRAY_VALUE, ARRAY_VALUE_IS_DONE,
            VALUE_IS_DONE
        };

        std::stack<json::parsing::state> s;
        std::stack<json::base*> j;
        std::string key;
        json::value value;

        const char* c;
        std::size_t i = 0;
        std::size_t n;
        std::size_t no = 0;
        std::size_t na = 0;

        parsing(const std::string& jstr) : c(jstr.c_str()), n(jstr.size()) {}

        void finish_reading() {
            this->s.pop();  // parsing::OBJECT/ARRAY_VALUE_IS_DONE -> parsing::OBJECT/ARRAY_VALUE
            if (this->s.top() == json::parsing::OBJECT_VALUE) {
                this->s.pop();
                this->s.pop();  // parsing::OBJECT_KEY
            }
        }
    };
    public: static json::value parse(const std::string& jstr) {
        json::value j;
        json::parsing p(jstr);

        p.c--;
        while (p.i++ < p.n) {
            p.c++;
            if (*p.c == ' ') {
            } else if (*p.c == '{') {
                p.s.push(json::parsing::OBJECT_KEY);
                j = json::object();
                p.j.push(j.v);
                break;
            } else if (*p.c == '[') {
                p.s.push(json::parsing::ARRAY_VALUE);
                j = json::array();
                p.j.push(j.v);
                break;
            }
        }

        while (p.i++ < p.n) {
            p.c++;
            if      (*p.c == ' ' ) {}
            else if (*p.c == '{' ) json::parse__left_brace(p);
            else if (*p.c == '}' ) json::parse__right_brace(p);
            else if (*p.c == '[' ) json::parse__left_bracket(p);
            else if (*p.c == ']' ) json::parse__right_bracket(p);
            else if (*p.c == ':' ) json::parse__colon(p);
            else if (*p.c == ',' ) json::parse__comma(p);
            else if (*p.c == '\"') json::parse__quote(p);
            else                   json::parse__else(p);
        }

        return j;
    }
    private: static void parse__left_brace(json::parsing& p) {
        if (p.s.top() == json::parsing::OBJECT_VALUE) {
            static_cast<json::object*>(p.j.top())->insert({p.key, json::object()});
            p.j.push(static_cast<json::object*>(p.j.top())->find(p.key)->second.v);
        } else if (p.s.top() == json::parsing::ARRAY_VALUE) {
            static_cast<json::array*>(p.j.top())->push_back(json::object());
            p.j.push(static_cast<json::array*>(p.j.top())->back().v);
        } else {
            throw std::invalid_argument(std::string("Invalid character '{' at "));
        }
        p.no++;
        p.s.push(json::parsing::OBJECT_KEY);
    }
    private: static void parse__right_brace(json::parsing& p) {
        if (p.no == 0) {
            // error
        }
        p.no--;

        if (p.s.top() == json::parsing::VALUE_IS_DONE) {
            p.finish_reading();
            static_cast<json::object*>(p.j.top())->insert({p.key, p.value});
        } else if (
            p.s.top() == json::parsing::OBJECT_VALUE_IS_DONE ||
            p.s.top() == json::parsing::ARRAY_VALUE_IS_DONE
        ) {
            p.finish_reading();
        } else {
            throw std::invalid_argument(std::string("Invalid character 'finish' at "));
        }
        p.s.pop();
        p.s.push(json::parsing::OBJECT_VALUE_IS_DONE);
        p.j.pop();
    }
    private: static void parse__left_bracket(json::parsing& p) {
        if (p.s.top() == json::parsing::OBJECT_VALUE) {
            static_cast<json::object*>(p.j.top())->insert({p.key, json::array()});
            p.j.push(static_cast<json::object*>(p.j.top())->find(p.key)->second.v);
        } else if (p.s.top() == json::parsing::ARRAY_VALUE) {
            static_cast<json::array*>(p.j.top())->push_back(json::array());
            p.j.push(static_cast<json::array*>(p.j.top())->back().v);
        } else {
            throw std::invalid_argument(std::string("Invalid character '{' at "));
        }
        p.na++;
        p.s.push(json::parsing::ARRAY_VALUE);
    }
    private: static void parse__right_bracket(json::parsing& p) {
        if (p.na == 0) {
            // error
        }
        p.na--;

        if (p.s.top() == json::parsing::VALUE_IS_DONE) {
            p.finish_reading();
            static_cast<json::array*>(p.j.top())->push_back(p.value);
        } else if (
            p.s.top() == json::parsing::OBJECT_VALUE_IS_DONE ||
            p.s.top() == json::parsing::ARRAY_VALUE_IS_DONE
        ) {
            p.finish_reading();
        } else {
            throw std::invalid_argument(std::string("Invalid character 'finish bracket' at "));
        }
        p.s.pop();
        p.s.push(json::parsing::ARRAY_VALUE_IS_DONE);
        p.j.pop();
    }
    private: static void parse__colon(json::parsing& p) {
        if (p.s.top() == json::parsing::OBJECT_KEY_IS_DONE) {
            p.s.push(json::parsing::OBJECT_VALUE);
        } else {
            throw std::invalid_argument(std::string("Invalid character ':' at "));
        }
    }
    private: static void parse__comma(json::parsing& p) {
        if (p.s.top() == json::parsing::VALUE_IS_DONE) {
            p.finish_reading();
            if (p.s.top() == json::parsing::OBJECT_KEY) {
                static_cast<json::object*>(p.j.top())->insert({p.key, p.value});
            } else {  // json::parsing::ARRAY_VALUE
                static_cast<json::array*>(p.j.top())->push_back(p.value);
            }
        } else if (
            p.s.top() == json::parsing::OBJECT_VALUE_IS_DONE ||
            p.s.top() == json::parsing::ARRAY_VALUE_IS_DONE
        ) {
            p.finish_reading();
        } else {
            throw std::invalid_argument(std::string("Invalid character ',' at "));
        }
    }
    private: static void parse__quote(json::parsing& p) {
        std::string s;
        bool escaping = false;
        while (p.i++ < p.n) {
            p.c++;
            if (escaping) {
                if      (*p.c == '\\') s += '\\';
                else if (*p.c == '\"') s += '\"';
                else if (*p.c == 'b' ) s += '\b';
                else if (*p.c == 'f' ) s += '\f';
                else if (*p.c == 'n' ) s += '\n';
                else if (*p.c == 'r' ) s += '\r';
                else if (*p.c == 't' ) s += '\t';
                else {
                    //exception? \u?
                }
                escaping = false;
            } else {
                if      (*p.c == '\\') escaping = true;
                else if (*p.c == '\"') break;
                else                   s += *p.c;
            }
        }

        if (p.s.top() == json::parsing::OBJECT_KEY) {
            p.key = s;
            p.s.push(json::parsing::OBJECT_KEY_IS_DONE);
        } else if (
            p.s.top() == json::parsing::OBJECT_VALUE ||
            p.s.top() == json::parsing::ARRAY_VALUE
        ) {
            p.value = json::value(s);
            p.s.push(json::parsing::VALUE_IS_DONE);
        }
    }
    private: static void parse__else(json::parsing& p) {
        if (
            p.s.top() == json::parsing::OBJECT_VALUE ||
            p.s.top() == json::parsing::ARRAY_VALUE
        ) {
            std::string v;
            v += *p.c;
            while (p.i++ < p.n) {
                p.c++;
                if (*p.c == ' ' || *p.c == ',' || *p.c == ']' || *p.c == '}') {
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
                    p.s.push(json::parsing::VALUE_IS_DONE);
                    p.i--;
                    p.c--;
                    break;
                } else {
                    v += *p.c;
                }
            }
        } else {
            throw std::invalid_argument(std::string("Invalid character 'else' at "));
        }
    }
};

