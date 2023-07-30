#pragma once
 
#include "json.h"
#include <stack>
#include <string>
#include <memory>
 
namespace transport_catalogue {
namespace detail {
namespace json {
namespace Builder {
 
class KeyContext;
class DictionaryContext;
class ArrayContext;
 
class Builder {
public:
    Node MakeNode(const Node::Value& Value_);
    void AddNode(const Node& node);
 
    KeyContext Key(const std::string& Key_);
    Builder& Value(const Node::Value& Value);
    
    DictionaryContext StartDict();
    Builder& EndDict();
    
    ArrayContext StartArray();
    Builder& EndArray();
 
    Node Build();
 
private:
    Node root_;
    std::vector<std::unique_ptr<Node>> nodes_stack_;
};
 
class BaseContext {
public:
    BaseContext(Builder& Builder);
 
    KeyContext Key(const std::string& Key);
    Builder& Value(const Node::Value& Value);
    
    DictionaryContext StartDict();
    Builder& EndDict();
    
    ArrayContext StartArray();
    Builder& EndArray();
 
protected:
    Builder& Builder_;
};
 
class KeyContext : public BaseContext {
public:
    KeyContext(Builder& Builder);
 
    KeyContext Key(const std::string& Key) = delete;
 
    BaseContext EndDict() = delete;
    BaseContext EndArray() = delete;
 
    DictionaryContext Value(const Node::Value& Value);
};
 
class DictionaryContext : public BaseContext {
public:
    DictionaryContext(Builder& Builder);
 
    DictionaryContext StartDict() = delete;
 
    ArrayContext StartArray() = delete;
    Builder& EndArray() = delete;
 
    Builder& Value(const Node::Value& Value) = delete;
};
 
class ArrayContext : public BaseContext {
public:
    ArrayContext(Builder& Builder);
 
    KeyContext Key(const std::string& Key) = delete;
 
    Builder& EndDict() = delete;
 
    ArrayContext Value(const Node::Value& Value);
};
 
} // namespace Builder
} // namespace json
} // namespace detail
} // namespace transport_catalogue
