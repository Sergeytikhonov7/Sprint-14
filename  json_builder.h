#pragma once

#include <memory>
#include <algorithm>

#include "json.h"

namespace json {

    class DictItemContext;

    class ArrayItemContext;

    class KeyItemContext;

    class ValueAfterKeyItemContext;

    class ValueAfterArrayItemContext;

    class Builder {

    public:

        KeyItemContext Key(std::string key);

        Builder& Value(Node::Value value);

        DictItemContext StartDict();

        ArrayItemContext StartArray();

        Builder& EndDict();

        Builder& EndArray();

        json::Node Build();

    private:
        json::Node root_;
        std::vector<std::shared_ptr<Node>> nodes_stack_;
        std::vector<std::shared_ptr<Node>> json_structure_;
        bool is_key_ = false;
    };

    class DictItemContext {
    public:
        explicit DictItemContext(Builder* builder);

        KeyItemContext Key(std::string key);

        Builder& EndDict();

    private:
        Builder* builder_;
    };

    class ArrayItemContext {
    public:
        explicit ArrayItemContext(Builder* builder);

        ValueAfterArrayItemContext Value(Node::Value value);

        DictItemContext StartDict();

        ArrayItemContext StartArray();

        Builder& EndArray();

    private:
        Builder* builder_;
    };

    class KeyItemContext {
    public:
        explicit KeyItemContext(Builder* builder);

        ValueAfterKeyItemContext Value(Node::Value value);

        DictItemContext StartDict();

        ArrayItemContext StartArray();

    private:
        Builder* builder_;
    };


    class ValueAfterKeyItemContext : private Builder {
    public:
        explicit ValueAfterKeyItemContext(Builder* builder);

        KeyItemContext Key(std::string key);

        Builder& Value(Node::Value value) = delete;

        DictItemContext StartDict() = delete;

        ArrayItemContext StartArray() = delete;

        Builder& EndDict();

        Builder& EndArray() = delete;

        json::Node Build() = delete;

    private:
        Builder* builder_;
    };

    class ValueAfterArrayItemContext : private Builder {
    public:
        explicit ValueAfterArrayItemContext(Builder* builder);

        KeyItemContext Key(std::string key) = delete;

        ValueAfterArrayItemContext Value(Node::Value value);

        DictItemContext StartDict();

        ArrayItemContext StartArray();

        Builder& EndDict() = delete;

        Builder& EndArray();

        json::Node Build() = delete;

    private:
        Builder* builder_;
    };
}

