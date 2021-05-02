#include " json_builder.h"

namespace json {

    KeyItemContext Builder::Key(std::string key) {
        KeyItemContext keyItemContext(this);
        if (json_structure_.empty() && nodes_stack_.size() == 1) { // json документ готов
            throw std::logic_error("Method Key call when document is finished");
        }
        if (is_key) { // второй вызов Key
            throw std::logic_error("Second method Key call");
        }
        if (!json_structure_.empty() && json_structure_.back()->IsArray()) { // вызов снаружи словаря
            throw std::logic_error("Method Key call outside the dictionary");
        }
        if (json_structure_.empty()) { // вызов сразу после конструтора
            throw std::logic_error("Method Key call after constructor");
        }
        const Node node(std::move(key));
        nodes_stack_.emplace_back(std::make_shared<Node>(node));
        is_key = true;
        return keyItemContext;
    }

    Builder& Builder::Value(Node::Value value) {
        if (json_structure_.empty() && nodes_stack_.size() == 1) { // json документ готов
            throw std::logic_error("Method Value call when document is finished");
        }
        if (!is_key && !json_structure_.empty() &&
            (!json_structure_.empty() && json_structure_.back()->IsDict())) {
            throw std::logic_error(
                    "Calling Value not after a constructor, not after Key, not after a previous array element");
        }
        if (std::holds_alternative<std::nullptr_t>(value)) {
            const Node node(std::get<std::nullptr_t>(std::move(value)));
            root_ = *nodes_stack_.emplace_back(std::make_shared<Node>(node));
        }
        if (std::holds_alternative<Array>(value)) {
            const Node node(std::get<Array>(std::move(value)));
            root_ = *nodes_stack_.emplace_back(std::make_shared<Node>(node));
        }
        if (std::holds_alternative<Dict>(value)) {
            const Node node(std::get<Dict>(std::move(value)));
            root_ = *nodes_stack_.emplace_back(std::make_shared<Node>(node));
        }
        if (std::holds_alternative<bool>(value)) {
            const Node node(std::get<bool>(std::move(value)));
            root_ = *nodes_stack_.emplace_back(std::make_shared<Node>(node));
        }
        if (std::holds_alternative<int>(value)) {
            const Node node(std::get<int>(std::move(value)));
            root_ = *nodes_stack_.emplace_back(std::make_shared<Node>(node));
        }
        if (std::holds_alternative<double>(value)) {
            const Node node(std::get<double>(std::move(value)));
            root_ = *nodes_stack_.emplace_back(std::make_shared<Node>(node));
        }
        if (std::holds_alternative<std::string>(value)) {
            const Node node(std::get<std::string>(std::move(value)));
            root_ = *nodes_stack_.emplace_back(std::make_shared<Node>(node));
        }
        is_key = false;
        return *this;
    }

    DictItemContext Builder::StartDict() {
        DictItemContext dictItemContext(this);
        if (json_structure_.empty() && nodes_stack_.size() == 1) { // json документ готов
            throw std::logic_error("Method StarDict call when document is finished");
        }
        if (!is_key && !json_structure_.empty() &&
            (!json_structure_.empty() && json_structure_.back()->IsDict())) {
            throw std::logic_error(
                    "Calling StartDict not after a constructor, not after Key, not after a previous array element");
        }
        auto node = nodes_stack_.emplace_back(std::make_shared<Node>(Node(Dict())));
        json_structure_.push_back(node);
        is_key = false;
        return dictItemContext;
    }

    ArrayItemContext Builder::StartArray() {
        ArrayItemContext arrayItemContext(this);
        if (json_structure_.empty() && nodes_stack_.size() == 1) { // json документ готов
            throw std::logic_error("Method StartArray call when document is finished");
        }
        if (!is_key && !json_structure_.empty() &&
            (!json_structure_.empty() && json_structure_.back()->IsDict())) {
            throw std::logic_error(
                    "Calling StartArray not after a constructor, not after Key, not after a previous array element");
        }

        auto node = nodes_stack_.emplace_back(std::make_shared<Node>(Node(Array())));
        json_structure_.push_back(node);
        is_key = false;
        return arrayItemContext;
    }

    Builder& Builder::EndDict() {
        if (json_structure_.empty() && nodes_stack_.size() == 1) { // json документ готов
            throw std::logic_error("Method EndDict call when document is finished");
        }
        if (!json_structure_.empty() && json_structure_.back()->IsArray()) {
            throw std::logic_error("Calling EndDict in the context of another container");
        }
        Dict currentDict;
        auto iterator_for_erase = std::find(nodes_stack_.begin(), nodes_stack_.end(),
                                            json_structure_.back());
        auto it = iterator_for_erase + 1;
        if (std::distance(it, nodes_stack_.end()) > 1) {
            while (it != nodes_stack_.end()) {
                auto key = std::move((**it));
                if (!key.IsString()) {
                    throw std::logic_error("error key");
                }
                auto value = std::move(**(std::next(it)));
                currentDict[std::move(key.AsString())] = value;
                it += 2;
            }
        }
        nodes_stack_.erase(iterator_for_erase, nodes_stack_.end());
        json_structure_.pop_back();
        root_ = *nodes_stack_.emplace_back(std::make_shared<Node>(std::move(currentDict)));
        is_key = false;
        return *this;
    }

    Builder& Builder::EndArray() {
        if (json_structure_.empty() && nodes_stack_.size() == 1) { // json документ готов
            throw std::logic_error("Method EndArray call when document is finished");
        }
        if (!json_structure_.empty() && json_structure_.back()->IsDict()) {
            throw std::logic_error("Calling EndArray in the context of another container");
        }

        auto iterator_for_erase = std::find(nodes_stack_.begin(), nodes_stack_.end(),
                                            json_structure_.back());
        Array currentArray;
        auto it = iterator_for_erase + 1;
        if (std::distance(it, nodes_stack_.end()) > 1) { // если массив не пустой
            while (it != nodes_stack_.end()) {
                currentArray.push_back(std::move(**it));
                ++it;
            }
        }
        nodes_stack_.erase(iterator_for_erase, nodes_stack_.end());
        json_structure_.pop_back();
        root_ = *nodes_stack_.emplace_back(std::make_shared<Node>(std::move(currentArray)));
        is_key = false;
        return *this;
    }

    json::Node Builder::Build() {
        if (!(json_structure_.empty() && nodes_stack_.size() == 1)) { // json документ не готов
            throw std::logic_error("Method Build call on unfinished document");
        }
        if (!nodes_stack_.empty() && !json_structure_.empty()) { // незаконченые массивы
            throw std::logic_error("Calling a method Build with an incomplete container");
        }
        return root_;
    }

    DictItemContext::DictItemContext(Builder* builder) : builder_(builder) {}

    KeyItemContext DictItemContext::Key(std::string key) {
        return builder_->Key(std::move(key));
    }

    Builder& DictItemContext::EndDict() {
        return builder_->EndDict();
    }

    ArrayItemContext::ArrayItemContext(Builder* builder) : builder_(builder) {}

    ValueAfterArrayItemContext ArrayItemContext::Value(Node::Value value) {
        ValueAfterArrayItemContext valueAfterArrayItemContext(&builder_->Value(std::move(value)));
        return valueAfterArrayItemContext;
    }

    DictItemContext ArrayItemContext::StartDict() {
        return builder_->StartDict();
    }

    ArrayItemContext ArrayItemContext::StartArray() {
        return builder_->StartArray();
    }

    Builder& ArrayItemContext::EndArray() {
        return builder_->EndArray();
    }

    KeyItemContext::KeyItemContext(Builder* builder) : builder_(builder) {}

    ValueAfterKeyItemContext KeyItemContext::Value(Node::Value value) {
        ValueAfterKeyItemContext valueAfterKeyItemContext(&builder_->Value(std::move(value)));
        return valueAfterKeyItemContext;
    }

    DictItemContext KeyItemContext::StartDict() {
        return builder_->StartDict();
    }

    ArrayItemContext KeyItemContext::StartArray() {
        return builder_->StartArray();
    }

    KeyItemContext ValueAfterKeyItemContext::Key(std::string key) {
        return builder_->Key(std::move(key));
    }

    Builder& ValueAfterKeyItemContext::EndDict() {
        return builder_->EndDict();
    }

    ValueAfterKeyItemContext::ValueAfterKeyItemContext(Builder* builder) : builder_(builder) {}


    ValueAfterArrayItemContext::ValueAfterArrayItemContext(Builder* builder) : builder_(builder) {}

    ValueAfterArrayItemContext ValueAfterArrayItemContext::Value(Node::Value value) {
        ValueAfterArrayItemContext valueAfterArrayItemContext(&builder_->Value(std::move(value)));
        return valueAfterArrayItemContext;
    }

    DictItemContext ValueAfterArrayItemContext::StartDict() {
        return builder_->StartDict();
    }

    ArrayItemContext ValueAfterArrayItemContext::StartArray() {
        return builder_->StartArray();
    }

    Builder& ValueAfterArrayItemContext::EndArray() {
        return builder_->EndArray();
    }
}
