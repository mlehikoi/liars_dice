#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <string>

namespace json {

class Value
{
    enum class Type { Null, Bool, Object0, Object1, ObjectN, Int, Double, String, Array } type_;
    union Data
    {
        bool b_;
        std::pair<const char*, const Value*> kv_;
        const std::initializer_list<std::pair<const char*, Value>>* kvs_;
        const std::initializer_list<Value>* a_; 
        int i_;
        double d_;
        const char* s_;

        explicit Data() {}
        explicit Data(bool b) : b_{b} {}
        explicit Data(const char* name, const Value& value) : kv_{name, &value} {}
        explicit Data(const std::initializer_list<std::pair<const char*, Value>>& kvs) : kvs_{&kvs} {}
        explicit Data(const std::initializer_list<Value>* a) : a_{a} {}
        explicit Data(int i) : i_{i} {}
        explicit Data(double d) : d_{d} {}
        explicit Data(const char* s) : s_{s} {}
    } data_;
public:
    Value(std::nullptr_t);
    Value(bool b);
    
    // object
    Value();
    Value(const char* name, const Value& value);
    Value(const std::initializer_list<std::pair<const char*, Value>>&);

    // Array constructor is not support because it is impossible to know
    // whether the user intented to create an object or an array. E.g.,
    // { "key", "value" } => Could be an object or an array
    // Use Array class, instead.

    Value(int i);
    Value(double d);
    Value(const char* str);
    Value(const std::string& str);
    
    void print(rapidjson::Writer<rapidjson::StringBuffer>& w) const;
protected:
    struct Dummy {};
    Value(Dummy&&, const std::initializer_list<Value>& values);
};

// Constructors
inline Value::Value(std::nullptr_t) : type_{Type::Null}, data_{} {}
inline Value::Value(bool b) : type_{Type::Bool}, data_{b} {}
inline Value::Value() : type_{Type::Object0}, data_{} {}
inline Value::Value(const char* name, const Value& value) : type_{Type::Object1}, data_{name, value} {}
inline Value::Value(const std::initializer_list<std::pair<const char*, Value>>& kv) : type_{Type::ObjectN}, data_{kv} {}
inline Value::Value(int i) : type_{Type::Int}, data_{i} {}
inline Value::Value(double d) : type_{Type::Double}, data_{d} {}
inline Value::Value(const char* str) : type_{Type::String}, data_{str} {}
inline Value::Value(const std::string& str) : type_{Type::String}, data_{str.c_str()} {}
inline Value::Value(Dummy&&, const std::initializer_list<Value>& values) : type_{Type::Array}, data_{&values} {}

inline void Value::print(rapidjson::Writer<rapidjson::StringBuffer>& w) const
{
    switch (type_)
    {
    case Type::Null: w.Null(); break;
    case Type::Bool: w.Bool(data_.b_); break;
    case Type::Int: w.Int(data_.i_); break;
    case Type::Double: w.Double(data_.d_); break;
    case Type::String: w.String(data_.s_); break;
    case Type::Object0: w.StartObject(); w.EndObject(); break;
    case Type::Object1:
        w.StartObject();
        w.Key(data_.kv_.first);
        data_.kv_.second->print(w);
        w.EndObject();
        break;
    case Type::ObjectN:
        w.StartObject();
        for (const auto& kv : *data_.kvs_) {
            w.Key(kv.first);
            kv.second.print(w);
        }
        w.EndObject();
        break;
    case Type::Array:
        w.StartArray();
        for (const auto& v : *data_.a_) v.print(w);
        w.EndArray();
        break;
    }
}

class Array : public Value
{
public:
    Array() : Value{Dummy{}, {}} {}
    Array(const std::initializer_list<Value>& values) : Value{Dummy{}, values} {}
};

class Json
{
    rapidjson::StringBuffer s_;
    rapidjson::Writer<rapidjson::StringBuffer> w_;
public:
    Json(const Value& obj) : s_{}, w_{s_} { obj.print(w_); }
    std::string str() const { return s_.GetString(); }
};
} // namespace json
