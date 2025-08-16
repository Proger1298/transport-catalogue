#include "json_builder.h"

namespace json {

    Builder::Builder() {
        nodes_stack_.emplace(&root_);
    }

    KeyItemContext Builder::Key(std::string key) {
        Node* top_node = nodes_stack_.top();
        if (top_node->IsDict() && !key_) {
            key_ = std::move(key);
        } else {
            throw std::logic_error("Incorrect cal of Key() method");
        }
        return *this;
    }

    Builder& Builder::StartCollectionOrValue(Node::Value value_to_make, bool has_to_be_put_on_stack) {
        Node* top_node = nodes_stack_.top();

        if (top_node->IsDict()) {
            if (!key_.has_value()) {
                throw std::logic_error("Start...() or Value() method can not be called in Dict without Key()");
            }

            Dict& top_node_as_dict = top_node->AsDict();
            auto [it, is_inserted] = top_node_as_dict.emplace(std::move(key_.value()), std::move(value_to_make));
            if (has_to_be_put_on_stack) {
                nodes_stack_.emplace(&it->second);
            }
            key_ = std::nullopt;
        } else if (top_node->IsArray()) {
            Array& top_node_as_array = top_node->AsArray();
            top_node_as_array.emplace_back(std::move(value_to_make));
            if (has_to_be_put_on_stack) {
                nodes_stack_.emplace(&top_node_as_array.back());
            }
        } else if (top_node->IsNull()) {
            top_node->GetValue() = std::move(value_to_make);
        } else {
            throw std::logic_error("Incorrect call of Start...() or Value() method");
        }
        return *this;
    }

    DictItemContext Builder::StartDict() {
        return StartCollectionOrValue(Dict(), true);
    }

    ArrayItemContext Builder::StartArray() {
        return StartCollectionOrValue(Array(), true);
    }

    Builder& Builder::Value(Node::Value value) {
        return StartCollectionOrValue(value, false);
    }

    Builder& Builder::EndCollection(bool (Node::*isType)() const) {
        if ((nodes_stack_.top()->*isType)()) {
            nodes_stack_.pop();
        } else {
            throw std::logic_error("Incorrect call of End...() method");
        }
        return *this;
    }

    Builder& Builder::EndDict() {
        return EndCollection(&Node::IsDict);
    }

    Builder& Builder::EndArray() {
        return EndCollection(&Node::IsArray);
    }

    Node Builder::Build() {
        if (root_.IsNull() || nodes_stack_.size() > 1) {
            throw std::logic_error("Can not call Build() with unbuilded json");
        }
        return root_;
    }


    // DICT CONTEXT //

    DictItemContext::DictItemContext(Builder& builder) 
    : BuilderContext(builder)
    {

    }

    KeyItemContext DictItemContext::Key(std::string key) {
        return builder_.Key(std::move(key));
    }

    Builder& DictItemContext::EndDict() {
        return builder_.EndDict();
    }

    // KEY CONTEXT //

    KeyItemContext::KeyItemContext(Builder& builder) 
    : BuilderContext(builder)
    {

    }

    DictItemContext KeyItemContext::Value(Node::Value value) {
        return builder_.Value(std::move(value));
    }

    DictItemContext KeyItemContext::StartDict() {
        return builder_.StartDict();
    }

    ArrayItemContext KeyItemContext::StartArray() {
        return builder_.StartArray();
    }

    // ARRAY CONTEXT //

    ArrayItemContext::ArrayItemContext(Builder& builder) 
    : BuilderContext(builder)
    {

    }

    ArrayItemContext ArrayItemContext::Value(Node::Value value) {
        return builder_.Value(std::move(value));
    }

    DictItemContext ArrayItemContext::StartDict() {
        return builder_.StartDict();
    }

    ArrayItemContext ArrayItemContext::StartArray() {
        return builder_.StartArray();
    }

    Builder& ArrayItemContext::EndArray() {
        return builder_.EndArray();
    }

} // namespace json