#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <optional>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};


class Node {
public:
   /* Реализуйте Node, используя std::variant */
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    Node() = default;
    Node(std::nullptr_t)
        : value_(nullptr) {
    }

    Node(Array arr)
        : value_(arr) {
    }

    Node(Dict map)
        : value_(move(map)) {
    }

    Node(bool value)
        : value_(value) {
    }

    Node(int value)
        : value_(value) {
    }

    Node(double value)
        : value_(value) {
    }

    Node(std::string value)
        : value_(move(value)) {
    }

    bool IsInt() const {
        return std::holds_alternative<int>(value_);
    }

    bool IsPureDouble() const {
        return std::holds_alternative<double>(value_);
    }

    bool IsDouble() const {
        return (IsInt() || IsPureDouble());
    }

    bool IsBool() const {
        return std::holds_alternative<bool>(value_);
    }

    bool IsString() const {
        return std::holds_alternative<std::string>(value_);
    }

    bool IsNull() const {
        return std::holds_alternative<std::nullptr_t>(value_);
    }

    bool IsArray() const {
        return std::holds_alternative<Array>(value_);
    }

    bool IsMap() const {
        return std::holds_alternative<Dict>(value_);
    }

    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;

    const Value& GetValue() const { return value_; }

    bool operator==(const Node& other) const {
        return this->value_ == other.GetValue();
    }

    bool operator!=(const Node& other) const {
        return !(*this == other);
    }

private:
    Value value_;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document& other) {
        return root_ == other.GetRoot();
    }

    bool operator!=(const Document& other) {
        return !(*this == other);
    }

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);
void PrintNode(const Node& value, std::ostream& output);

void PrintValue(const std::string& value, std::ostream& output);
void PrintValue(const Array& array, std::ostream& output);
void PrintValue(const Dict& dict, std::ostream& output);
void PrintValue(bool value, std::ostream& out);
}  // namespace json