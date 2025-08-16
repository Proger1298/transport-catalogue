#pragma once

#include "json.h"

#include <optional>
#include <stack>
#include <string>


namespace json {

class Builder;
class BuilderContext;
class DictItemContext;
class KeyItemContext;
class ArrayItemContext;

class BuilderContext {
public:
    BuilderContext(Builder& builder)
    : builder_(builder) 
    {

    }

protected:
    Builder& builder_;
};

class DictItemContext : public BuilderContext {
public:
    DictItemContext(Builder& builder);
    KeyItemContext Key(std::string key);
    Builder& EndDict();

};

class KeyItemContext : public BuilderContext {
public:
    KeyItemContext(Builder& builder);
    DictItemContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();

};

class ArrayItemContext : public BuilderContext {
public:
    ArrayItemContext(Builder& builder);
    ArrayItemContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();

};

class Builder {
public:
    Builder();
    KeyItemContext Key(std::string key);
    Builder& Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();
    Node Build();

private:
    Node root_;
    std::optional<std::string> key_;

    std::stack<Node*> nodes_stack_;

    Builder& StartCollectionOrValue(Node::Value value, bool has_to_be_put_on_stack);
    Builder& EndCollection(bool (Node::*isType)() const);
};

} // namespace json