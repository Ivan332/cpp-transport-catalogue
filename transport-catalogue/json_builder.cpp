#include "json_Builder.h"
 
namespace transport_catalogue {
namespace detail {
namespace json {
namespace Builder {
 
BaseContext::BaseContext(Builder& Builder) 
    : Builder_(Builder) {
    }
 
KeyContext BaseContext::Key(const std::string& Key) {
    return Builder_.Key(Key);
}

Builder& BaseContext::Value(const Node::Value& Value) {
    return Builder_.Value(Value);
}
 
DictionaryContext BaseContext::StartDict() {
    return DictionaryContext(Builder_.StartDict());
}

Builder& BaseContext::EndDict() {
    return Builder_.EndDict();
}
 
ArrayContext BaseContext::StartArray() {
    return ArrayContext(Builder_.StartArray());
}

Builder& BaseContext::EndArray() {
    return Builder_.EndArray();
}
 
KeyContext::KeyContext(Builder& Builder) 
    : BaseContext(Builder) {
}
 
DictionaryContext KeyContext::Value(const Node::Value& Value) {
    return BaseContext::Value(std::move(Value));
}
 
DictionaryContext::DictionaryContext(Builder& Builder) 
    : BaseContext(Builder) {
    }
 
ArrayContext::ArrayContext(Builder& Builder) 
    : BaseContext(Builder) {
    }
 
ArrayContext ArrayContext::Value(const Node::Value& Value) {
    return BaseContext::Value(std::move(Value));
}
 
Node Builder::MakeNode(const Node::Value& Value_) {
    Node node;
 
    if (std::holds_alternative<bool>(Value_)) {
        bool bol = std::get<bool>(Value_);
        node = Node(bol);
 
    } else if (std::holds_alternative<int>(Value_)) {
        int intt = std::get<int>(Value_);
        node = Node(intt);
 
    } else if (std::holds_alternative<double>(Value_)) {
        double doble = std::get<double>(Value_);
        node = Node(doble);
 
    } else if (std::holds_alternative<std::string>(Value_)) {
        std::string str = std::get<std::string>(Value_);
        node = Node(std::move(str));
 
    } else if (std::holds_alternative<Array>(Value_)) {
        Array arr = std::get<Array>(Value_);
        node = Node(std::move(arr));
 
    } else if (std::holds_alternative<Dict>(Value_)) {
        Dict dictionary = std::get<Dict>(Value_);
        node = Node(std::move(dictionary));
 
    } else {
        node = Node();
    }
 
    return node;
}
 
void Builder::AddNode(const Node& node) {
    if (nodes_stack_.empty()) {
 
        if (!root_.IsNull()) {
            throw std::logic_error("root has been added");
        }
 
        root_ = node;
        return;
 
    } else {
 
        if (!nodes_stack_.back()->IsArray()
            && !nodes_stack_.back()->IsString()) {
 
            throw std::logic_error("unable to create node");
        }
 
        if (nodes_stack_.back()->IsArray()) {
            Array arr = nodes_stack_.back()->AsArray();
            arr.emplace_back(node);
 
            nodes_stack_.pop_back();
            auto arr_ptr = std::make_unique<Node>(arr);
            nodes_stack_.emplace_back(std::move(arr_ptr));
 
            return;
        }
 
        if (nodes_stack_.back()->IsString()) {
            std::string str = nodes_stack_.back()->AsString();
            nodes_stack_.pop_back();
 
            if (nodes_stack_.back()->IsDict()) {
                Dict dictionary = nodes_stack_.back()->AsDict();
                dictionary.emplace(std::move(str), node);
 
                nodes_stack_.pop_back();
                auto dictionary_ptr = std::make_unique<Node>(dictionary);
                nodes_stack_.emplace_back(std::move(dictionary_ptr));
            }
 
            return;
        }
    }
}
 
KeyContext Builder::Key(const std::string& Key_) {
    if (nodes_stack_.empty()) {
        throw std::logic_error("unable to create Key");
    }
 
    auto Key_ptr = std::make_unique<Node>(Key_);
 
    if (nodes_stack_.back()->IsDict()) {
        nodes_stack_.emplace_back(std::move(Key_ptr));
    }
 
    return KeyContext(*this);
}
 
Builder& Builder::Value(const Node::Value& Value_) {
    AddNode(MakeNode(Value_));
 
    return *this;
}
 
DictionaryContext Builder::StartDict() {
    nodes_stack_.emplace_back(std::move(std::make_unique<Node>(Dict())));
 
    return DictionaryContext(*this);
}
 
Builder& Builder::EndDict() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("unable to close as without opening");
    }
 
    Node node = *nodes_stack_.back();
 
    if (!node.IsDict()) {
        throw std::logic_error("object isn't dictionary");
    }
 
    nodes_stack_.pop_back();
    AddNode(node);
 
    return *this;
}
 
ArrayContext Builder::StartArray() {
    nodes_stack_.emplace_back(std::move(std::make_unique<Node>(Array())));
 
    return ArrayContext(*this);
}
 
Builder& Builder::EndArray() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("unable to close without opening");
    }
 
    Node node = *nodes_stack_.back();
 
    if (!node.IsArray()) {
        throw std::logic_error("object isn't array");
    }
 
    nodes_stack_.pop_back();
    AddNode(node);
 
    return *this;
}
 
Node Builder::Build() {
    if (root_.IsNull()) {
        throw std::logic_error("empty json");
    }
 
    if (!nodes_stack_.empty()) {
        throw std::logic_error("invalid json");
    }
 
    return root_;
}
 
} // namespace Builder
} // namespace json
} // namespace detail
} // namespace transport_catalogue
