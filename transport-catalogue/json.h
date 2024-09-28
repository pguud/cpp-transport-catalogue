#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>
#include <optional>

namespace json {

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    const Value& GetValue() const { return value_; }

    Node() = default;
    Node(std::nullptr_t value);
    Node(Array value);
    Node(Dict value);
    Node(bool value);
    Node(int value);
    Node(double value);
    Node(std::string value);

    // Следующие методы Node сообщают, хранится ли внутри значение некоторого типа:
    bool IsInt() const;
    bool IsDouble() const; // Возвращает true, если в Node хранится int либо double.
    bool IsPureDouble() const; // озвращает true, если в Node хранится double.
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;

    // Ниже перечислены методы, которые возвращают хранящееся внутри Node 
    // значение заданного типа. Если внутри содержится значение другого типа, 
    // должно выбрасываться исключение std::logic_error.
    int AsInt() const;
    bool AsBool() const;
    // Возвращает значение типа double, если внутри хранится double либо int. 
    // В последнем случае возвращается приведённое в double значение.
    double AsDouble() const; 
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;

private:
    Value value_;
};

struct NodeGetter {
    std::optional<std::nullptr_t> operator()(std::nullptr_t) const { return std::nullopt; }
    std::optional<Array> operator()(Array arr) const { return arr; }
    std::optional<Dict> operator()(Dict dict) const { return dict; }
    std::optional<bool> operator()(bool val) const { return val; }
    std::optional<int> operator()(int val) const { return val; }
    std::optional<double> operator()(double val) const { return val; }
    std::optional<std::string> operator()(std::string st) const { return st; }
};

bool operator==(const Node& lhs, const Node& rhs);
bool operator!=(const Node& lhs, const Node& rhs);

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

bool operator==(const Document& lhs, const Document& rhs);
bool operator!=(const Document& lhs, const Document& rhs);

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json
