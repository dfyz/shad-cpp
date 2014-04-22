#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <cctype>
#include <sstream>

#include <gtest/gtest.h>

class TJsonValue {
    public:
        virtual void Print(size_t indent, std::ostream& out, bool printFirstIndent) const = 0;

    protected:
        void PrintIndentation(size_t indent, std::ostream& out) const {
            out << std::string(4 * indent, ' ');
        }

        void PrintCompositeHeader(char ch, bool empty, size_t indent, std::ostream& out, bool printFirstIndent) const {
            if (printFirstIndent) {
                PrintIndentation(indent, out);
            }
            out << ch << std::endl;
            if (empty) {
                out << std::endl;
            }
        }

        void PrintCompositeFooter(char ch, bool empty, size_t indent, std::ostream& out) const {
            PrintIndentation(indent, out);
            out << ch;
        }
};

class TJsonScalar : public TJsonValue {
    public:
        TJsonScalar(const std::string& value)
            : Value(value)
        {
        }

        void Print(size_t indent, std::ostream& out, bool printFirstIndent) const {
            if (printFirstIndent) {
                PrintIndentation(indent, out);
            }
            out << Value;
        }

    private:
        std::string Value;
};

class TJsonArray : public TJsonValue {
    public:
        void Add(std::auto_ptr<TJsonValue> child) {
            Children.push_back(std::shared_ptr<TJsonValue>(child.release()));
        }

        void Print(size_t indent, std::ostream& out, bool printFirstIndent) const {
            PrintCompositeHeader('[', Children.empty(), indent, out, printFirstIndent);
            for (size_t i = 0; i < Children.size() - 1; i++) {
                Children[i]->Print(indent + 1, out, true);
                out << "," << std::endl;
            }
            if (!Children.empty()) {
                Children[Children.size() - 1]->Print(indent + 1, out, true);
                out << std::endl;
            }
            PrintCompositeFooter(']', Children.empty(), indent, out);
        }

    private:
        std::vector< std::shared_ptr<TJsonValue> > Children;
};

class TJsonDict : public TJsonValue {
    public:
        void Add(const TJsonScalar key, std::auto_ptr<TJsonValue> value) {
            Children.push_back(std::make_pair(key, std::shared_ptr<TJsonValue>(value.release())));
        }

        void Print(size_t indent, std::ostream& out, bool printFirstIndent) const {
            PrintCompositeHeader('{', Children.empty(), indent, out, printFirstIndent);
            size_t index = 0;
            for (size_t i = 0; i < Children.size(); i++) {
                Children[i].first.Print(indent + 1, out, true);
                out << ": ";
                Children[i].second->Print(indent + 1, out, false);
                if (index + 1 != Children.size()) {
                    out << ",";
                }
                out << std::endl;
                index++;
            }
            PrintCompositeFooter('}', Children.empty(), indent, out);
        }

    private:
        std::vector< std::pair< TJsonScalar, std::shared_ptr<TJsonValue> > > Children;
};

char GetChar(std::istream& in) {
    while (in.good() && isspace(in.peek())) {
        in.get();
    }
    if (!in.good()) {
        throw std::runtime_error("Unexpected EOF or something");
    }
    return static_cast<char>(in.get());
}

static bool IsDigit(char ch) {
    return ch >= '0' && ch < '9';
}

std::string GetDigits(std::istream &in) {
    std::string result;
    while (in.good() && IsDigit(in.peek())) {
        result += in.get();
    }
    return result;
}

std::auto_ptr<TJsonScalar> ParseString(std::istream& in) {
    std::string result("\"");
    while (true) {
        char ch = in.get();
        if (ch == '"') {
            break;
        }
        result += ch;
        if (ch == '\\') {
            char ch2 = in.get();
            if (ch != '\\' && ch != '\"') {
                throw std::runtime_error("\\\\ or \" expected");
            }
            result += ch2;
        }
    }
    return std::auto_ptr<TJsonScalar>(new TJsonScalar(result + "\""));
}

std::auto_ptr<TJsonValue> ReadLiteral(std::istream& in, const char* str, size_t len) {
    for (size_t i = 0; i + 1 < len; i++) {
        if (in.get() != str[i + 1]) {
            throw std::runtime_error(str[i] + std::string(" expected"));
        }
    }
    return std::auto_ptr<TJsonValue>(new TJsonScalar(str));
}

std::auto_ptr<TJsonValue> ParseNumber(char prefix, std::istream& in) {
    std::string result(prefix + GetDigits(in));
    if (result.empty()) {
        throw std::runtime_error("Digits expected in the integral part");
    }
    if (result.size() > 1 && result[0] == '0') {
        throw std::runtime_error("Superfluous zeroes");
    }
    if (in.peek() == '.') {
        result += in.get();
        std::string fracPart(GetDigits(in));
        if (fracPart.empty()) {
            throw std::runtime_error("Digits expected in the fractional part");
        }
        result += fracPart;
    }
    char next = in.peek();
    tolower(next);
    if (next == 'e') {
        result += in.get();
        if (std::string(1, in.peek()).find_first_of("-+")) {
            result += in.get();
        }
        std::string expPart(GetDigits(in));
        if (expPart.empty()) {
            throw std::runtime_error("Digits expected in the exponent");
        }
        result += expPart;
    }
    return std::auto_ptr<TJsonValue>(new TJsonScalar(result));
}

std::auto_ptr<TJsonValue> Parse(std::istream& in);

std::auto_ptr<TJsonValue> ParseComposite(char closeChar, bool isDict, std::istream& in) {
    std::auto_ptr<TJsonArray> arrayResult(new TJsonArray());
    std::auto_ptr<TJsonDict> dictResult(new TJsonDict());

    while (isspace(in.peek())) {
        in.get();
    }

    if (in.peek() == closeChar) {
        in.get();
    } else {
        char ch = '\0';
        while (true) {
            std::auto_ptr<TJsonScalar> key;
            if (isDict) {
                ch = GetChar(in);
                if (ch != '"') {
                    throw std::runtime_error("\" expected");
                }
                key = ParseString(in);

                ch = in.get();
                if (ch != ':') {
                    throw std::runtime_error(": expected");
                }
            }

            std::auto_ptr<TJsonValue> value(Parse(in));
            if (isDict) {
                dictResult->Add(*key, value);
            } else {
                arrayResult->Add(value);
            }

            ch = GetChar(in);
            if (ch == ']' || ch == '}') {
                break;
            } else if (ch != ',') {
                throw std::runtime_error(std::string(1, closeChar) + " or , expected");
            }
        }
    }
    return isDict ? std::auto_ptr<TJsonValue>(dictResult.release()) : std::auto_ptr<TJsonValue>(arrayResult.release());
}

std::auto_ptr<TJsonValue> Parse(std::istream& in) {
    char ch = GetChar(in);
    if (ch == '{') {
        return ParseComposite('}', true, in);
    } else if (ch == '[') {
        return ParseComposite(']', false, in);
    } else if (ch == '-' || IsDigit(ch)) {
        return ParseNumber(ch, in);
    } else if (ch == '"') {
        return std::auto_ptr<TJsonValue>(ParseString(in).release());
    } else if (ch == 't') {
        return ReadLiteral(in, "true", strlen("true"));
    } else if (ch == 'f') {
        return ReadLiteral(in, "false", strlen("true"));
    } else if (ch == 'n') {
        return ReadLiteral(in, "null", strlen("null"));
    }
    throw std::runtime_error(std::string("Invalid character: <") + ch + std::string(">"));
}

std::string Format(const std::string& json)
{
    std::istringstream in(json);
    std::auto_ptr<TJsonValue> value(Parse(in));
    if (in.get() >= 0) {
        throw std::runtime_error("Expected EOF");
    }
    std::ostringstream out;
    value->Print(0, out, false);
    return out.str();
}

TEST(JSONParser, Test) {
    ASSERT_EQ("true", Format("true"));
    ASSERT_EQ("[\n    42,\n    [\n        42,\n        [\n            42\n        ]\n    ]\n]", Format("[42, [42, [42]]]"));
    ASSERT_EQ("{\n    \"key\": \"value\"\n}", Format("{\"key\":\"value\"}"));
    ASSERT_EQ("-42.42e42", Format("-42.42e42"));
    ASSERT_THROW(Format("["), std::runtime_error);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

