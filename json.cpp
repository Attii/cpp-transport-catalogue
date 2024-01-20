#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadBool(istream& input) {
    string bool_str;
    while (isalpha(input.peek())) {
        bool_str.push_back(input.get());
    }
    if (bool_str == "true") {
        return Node(true);
    } else if (bool_str == "false") {
        return Node(false);
    } else {
        throw ParsingError("Invalid boolean value");
    }
}

Node LoadNull(istream& input) {
    // Проверяем начальные символы, соответствующие "null"
    string expected_null = "null";
    for (char ch : expected_null) {
        if (input.get() != ch) {
            throw ParsingError("Parsing error: expected 'null'");
        }
    }

    // Проверяем, что после "null" следует допустимый разделитель JSON
    char next_char = input.peek();
    if (!input.eof() && next_char != ',' && next_char != ']' && next_char != '}' && !isspace(next_char)) {
        throw ParsingError("Parsing error: invalid character after 'null'");
    }

    return Node(nullptr);
}

Node LoadArray(istream& input) {
    Array result;

    char c;
    for (; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    if (c != ']') throw ParsingError("Missing closing ]"s);
 
    return Node(move(result));
}

Node LoadNumber(std::istream& input) {
    string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(stoi(parsed_num));
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(stod(parsed_num));
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return Node(move(s));
}

Node LoadDict(istream& input) {
    Dict result;

    char c;
    for (; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }

    if (c != '}') throw ParsingError("Missing closing }"s);

    return Node(move(result));
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    switch (c) {
        case '[':
            return LoadArray(input);
        case '{':
            return LoadDict(input);
        case '"':
            return LoadString(input);
        case 't':
        case 'f':
            input.putback(c);
            return LoadBool(input);
        case 'n':
            input.putback(c);
            return LoadNull(input);
        default:
            input.putback(c);
            if (input.peek() == '-' || isdigit(input.peek())) {
                return LoadNumber(input);
            } else {
                throw ParsingError("Unexpected character"s);
            }
    }
}

}  // namespace

Document::Document(Node root)
    : root_(move(root)) {
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

int Node::AsInt() const {
    if (const int* ptr_val = std::get_if<int>(&value_)) {
        return *ptr_val;
    } 
    else {
        throw std::logic_error("Wrong type");
    }
}

bool Node::AsBool() const {
    if (const bool* ptr_val = std::get_if<bool>(&value_)) {
        return *ptr_val;
    } 
    else {
        throw std::logic_error("Wrong type");
    }
}

double Node::AsDouble() const {
    if (const double* ptr_val = std::get_if<double>(&value_)) {
        return *ptr_val;
    } 
    else if (const int* ptr_val = std::get_if<int>(&value_)) {
        return (*ptr_val * 1.0);
    }
    else {
        throw std::logic_error("Wrong type");
    }
}

const Array& Node::AsArray() const {
    if (const Array* ptr_val = std::get_if<Array>(&value_)) {
        return *ptr_val;
    } 
    else {
        throw std::logic_error("Wrong type");
    }
}

const Dict& Node::AsMap() const {
    if (const Dict* ptr_val = std::get_if<Dict>(&value_)) {
        return *ptr_val;
    } 
    else {
        throw std::logic_error("Wrong type");
    }
}

const string& Node::AsString() const {
    if (const std::string* ptr_val = std::get_if<std::string>(&value_)) {
        return *ptr_val;
    } 
    else {
        throw std::logic_error("Wrong type");
    }
}

const Node& Document::GetRoot() const {
    return root_;
}

// Print functions
void PrintValue(const Array& array, std::ostream& output) {
    output << '[';
    bool first = true;
    for (const auto& element : array) {
        if (!first) {
            output << ", ";
        }
        PrintNode(element, output);
        first = false;
    }
    output << ']';
}

void PrintValue(const std::string& value, std::ostream& output) {
    output << '"';
    for (char c : value) {
        switch (c) {
            case '\\': output << "\\\\"; break;
            case '\"': output << "\\\""; break;
            case '\n': output << "\\n"; break;
            case '\r': output << "\\r"; break;
            case '\t': output << "\\t"; break;
            default: output << c; break;
        }
    }
    output << '"';
}

void PrintValue(const Dict& dict, std::ostream& output) {
    output << '{';
    bool first = true;
    for (const auto& [key, value] : dict) {
        if (!first) {
            output << ", ";
        }
        PrintValue(std::string(key), output);
        output << ": ";
        PrintNode(value, output);
        first = false;
    }
    output << '}';
}

template <typename Value>
void PrintValue(const Value& value, std::ostream& out) {
    out << value;
}

void PrintValue(std::nullptr_t, std::ostream& out) {
    out << "null"sv;
}

void PrintValue(bool value, std::ostream& out) {
    out << (value ? "true" : "false");
}

void PrintNode(const Node& node, std::ostream& output) {
    std::visit(
        [&output](const auto& val) { PrintValue(val, output); }, 
        node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), output);
}

}  // namespace json