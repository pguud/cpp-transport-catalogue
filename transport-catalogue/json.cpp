#include "json.h"
#include <iomanip>

using namespace std;

namespace json {
    namespace {
        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;
            char c;

            while (input >> c && c != ']') {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            if (c != ']') {
                throw ParsingError("Array not closed");
            }

            return Node(move(result));
        }

        Node LoadString(istream& input) {
            string line;
            char c;

            while (input.get(c) && c != '"') {
                if (c == '\\') { // escape sequence
                    input.get(c);
                    switch (c) {
                        case '\\':
                            line += '\\';
                            break;
                        case '"':
                            line += '"';
                            break;
                        case 'n':
                            line += '\n';
                            break;
                        case 't':
                            line += '\t';
                            break;
                        case 'r':
                            line += '\r';
                            break;
                        default:
                            throw ParsingError("Invalid escape sequence");
                    }
                } else {
                    line += c;
                }
            }

            if (c != '"') {
                throw ParsingError("String not closed");
            }

            return Node(move(line));
        }

        Node LoadDict(istream& input) {
            Dict result;
            char c;

            while (input >> c && c != '}') {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c; // :
                if (c != ':') {
                    throw ParsingError("Expected colon after key");
                }
                result.insert({move(key), LoadNode(input)});
            }

            if (c != '}') {
                throw ParsingError("Object not closed");
            }

            return Node(move(result));
        }

        Node LoadIntOrDouble(istream& input) {
            string numStr;
            char c;

            while (input.get(c) && (isdigit(c) || c == '.' || c == '-' || c == '+' || c == 'e' || c == 'E')) {
                numStr += c;
            }

            input.putback(c);

            if (numStr.find('.') != string::npos || numStr.find('e') != string::npos || numStr.find('E') != string::npos) {
                return Node(stod(numStr));
            } else {
                return Node(stoi(numStr));
            }
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == 'n') { // null
                string nullStr;
                nullStr += c; 
                
                while (input >> c && c != 'l') {
                    nullStr += c;
                }
                nullStr += c;
                c = ' ';
                input >> c;
                nullStr += c;
                c = ' ';
                input >> c;

                if (nullStr != "null" || (c != ' ' && c != ',')) {
                    input.putback(c);

                    throw ParsingError("Invalid null value");
                }
                    input.putback(c);

                return Node(nullptr);

            } else if (c == 't') { // true
                string trueStr;
                trueStr += c;
                while (input >> c && c != 'e') {
                    trueStr += c;
                }
                trueStr += c;
                c = ' ';
                input >> c;

                if (trueStr != "true" || (c != ' ' && c != ']' && c != ',' && c != '}')) {
                    input.putback(c);
                    throw ParsingError("Invalid true value");
                }
                input.putback(c);
                return Node(true);

            } else if (c == 'f') { // false
                string falseStr;
                falseStr += c;
                while (input >> c && c != 'e') {
                    falseStr += c;
                }
                falseStr += c;
                c = ' ';
                input >> c;
                if (falseStr != "false" || (c != ' ' && c != ']' && c != ',' && c != '}')) {
                    input.putback(c);
                    throw ParsingError("Invalid false value");
                }
                input.putback(c);
                return Node(false);

            } else if (c == '[') { // array
                return LoadArray(input);
            } else if (c == '{') { // object
                return LoadDict(input);
            } else if (c == '"') { // string
                return LoadString(input);

            } else if ((c >= '0' && c <= '9') || c == '-') { // number
                input.putback(c);
                return LoadIntOrDouble(input);

            } else {
                throw ParsingError("Invalid value");

            }
        }
    }  // namespace

    Node::Node(std::nullptr_t value) 
        : value_(value) {
    }
    Node::Node(Array value) 
        : value_(value) {
    }
    Node::Node(Dict value) 
        : value_(value) {
    }
    Node::Node(bool value) 
        : value_(value) {
    }
    Node::Node(int value) 
        : value_(value) {
    }
    Node::Node(double value) 
        : value_(value) {
    }
    Node::Node(std::string value) 
        : value_(value) {
    }

    int Node::AsInt() const {
        if (IsInt()) {
            return std::get<int>(value_);
        }
        throw std::logic_error("Node does not hold an int value");
    }

    bool Node::AsBool() const {
        if (IsBool()) {
            return std::get<bool>(value_);
        }
        throw std::logic_error("Node does not hold a bool value");
    }

    double Node::AsDouble() const {
        if (IsDouble() || IsInt()) {
            if (IsInt()) {
                return static_cast<double>(std::get<int>(value_));
            } else {
                return std::get<double>(value_);
            }
        }
        throw std::logic_error("Node does not hold a double or int value");
    }

    const std::string& Node::AsString() const {
        if (IsString()) {
            return std::get<std::string>(value_);
        }
        throw std::logic_error("Node does not hold a string value");
    }

    const Array& Node::AsArray() const {
        if (IsArray()) {
            return std::get<Array>(value_);
        }
        throw std::logic_error("Node does not hold an array value"); 
    }

    const Dict& Node::AsMap() const {
        if (IsMap()) {
            return std::get<Dict>(value_);
        }
        throw std::logic_error("Node does not hold a map value");
    }

    bool Node::IsInt() const {
        return std::holds_alternative<int>(value_);
    }

    bool Node::IsDouble() const {
        return std::holds_alternative<double>(value_) 
                || std::holds_alternative<int>(value_) ;
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(value_);
    }

    bool Node::IsBool() const {
        return std::holds_alternative<bool>(value_);
    }

    bool Node::IsString() const {
        return std::holds_alternative<std::string>(value_);
    }

    bool Node::IsNull() const {
        return std::holds_alternative<std::nullptr_t>(value_);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(value_);
    }

    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(value_);
    }

    bool operator==(const Node& lhs, const Node& rhs) {
        return std::visit(
            [](const auto& lhsValue, const auto& rhsValue) -> bool {
                using T = std::decay_t<decltype(lhsValue)>;
                using U = std::decay_t<decltype(rhsValue)>;
                if constexpr (std::is_same_v<T, U>) {
                    return lhsValue == rhsValue;
                } else {
                    return false;
                }
            },
            lhs.GetValue(), rhs.GetValue()
        );
    }


    bool operator!=(const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{LoadNode(input)};
    }

    bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }
    bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs == rhs);
    }
    
    template <typename Value>
    void PrintValue(const Value& value, std::ostream& out) {
        std::stringstream stream;
        stream << value;
        out << stream.str();   
    }

    void PrintValue(std::nullptr_t, std::ostream& out) {
        out << "null"sv;
    }

    void PrintValue(bool value, std::ostream& out) {
        out << (value ? "true"sv : "false"sv);
    }

    void PrintValue(const std::string& value, std::ostream& out) {
        out << "\"";
        for (char c : value) {
            switch (c) {
                case '\\':
                    out << "\\" << "\\";
                    break;
                case '"':
                    out << "\\" << "\"";
                    break;
                case '\n':
                    out << "\\" << "n";
                    break;
                case '\t':
                    out << "\\" << "t";
                    break;
                case '\r':
                    out << "\\" << "r";
                    break;
                default:
                    out << c;
            }
        }
        out << "\"";
    }

    void PrintNode(const Node& node, std::ostream& out) {
        std::visit(
            [&out](const auto& value){ PrintValue(value, out); },
            node.GetValue());
    } 

    void PrintValue(const Array& array, std::ostream& out) {
        out << "[";
        bool first = true;
        for (const auto& element : array) {
            if (!first) {
                out << ", ";
            }
            PrintNode(element, out);
            first = false;
        }
        out << "]";
    }

    void PrintValue(const Dict& dict, std::ostream& out) {
        out << "{";
        bool first = true;
        for (const auto& [key, value] : dict) {
            if (!first) {
                out << ", ";
            }
            PrintValue(key, out);
            out << ": ";
            PrintNode(value, out);
            first = false;
        }
        out << "}";
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), output);
    }
}  // namespace json
