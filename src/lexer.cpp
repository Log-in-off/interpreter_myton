#include "lexer.h"

#include <algorithm>
#include <charconv>
#include <unordered_map>

using namespace std;

namespace parse {

bool operator==(const Token& lhs, const Token& rhs) {
    using namespace token_type;

    if (lhs.index() != rhs.index()) {
        return false;
    }
    if (lhs.Is<Char>()) {
        return lhs.As<Char>().value == rhs.As<Char>().value;
    }
    if (lhs.Is<Number>()) {
        return lhs.As<Number>().value == rhs.As<Number>().value;
    }
    if (lhs.Is<String>()) {
        return lhs.As<String>().value == rhs.As<String>().value;
    }
    if (lhs.Is<Id>()) {
        return lhs.As<Id>().value == rhs.As<Id>().value;
    }
    return true;
}

bool operator!=(const Token& lhs, const Token& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, const Token& rhs) {
    using namespace token_type;

#define VALUED_OUTPUT(type) \
    if (auto p = rhs.TryAs<type>()) return os << #type << '{' << p->value << '}';

    VALUED_OUTPUT(Number);
    VALUED_OUTPUT(Id);
    VALUED_OUTPUT(String);
    VALUED_OUTPUT(Char);

#undef VALUED_OUTPUT

#define UNVALUED_OUTPUT(type) \
    if (rhs.Is<type>()) return os << #type;

    UNVALUED_OUTPUT(Class);
    UNVALUED_OUTPUT(Return);
    UNVALUED_OUTPUT(If);
    UNVALUED_OUTPUT(Else);
    UNVALUED_OUTPUT(Def);
    UNVALUED_OUTPUT(Newline);
    UNVALUED_OUTPUT(Print);
    UNVALUED_OUTPUT(Indent);
    UNVALUED_OUTPUT(Dedent);
    UNVALUED_OUTPUT(And);
    UNVALUED_OUTPUT(Or);
    UNVALUED_OUTPUT(Not);
    UNVALUED_OUTPUT(Eq);
    UNVALUED_OUTPUT(NotEq);
    UNVALUED_OUTPUT(LessOrEq);
    UNVALUED_OUTPUT(GreaterOrEq);
    UNVALUED_OUTPUT(None);
    UNVALUED_OUTPUT(True);
    UNVALUED_OUTPUT(False);
    UNVALUED_OUTPUT(Eof);

#undef UNVALUED_OUTPUT

    return os << "Unknown token :("sv;
}

Lexer::Lexer(std::istream& input):input_(input), indexCurrentToken_(0), currentDent_(0) {
    "class return if else def print or None and not True False"s;
    tableLexems_["class"] = token_type::Class();
    tableLexems_["return"] = token_type::Return();
    tableLexems_["if"] = token_type::If();
    tableLexems_["else"] = token_type::Else();
    tableLexems_["def"] = token_type::Def();
    tableLexems_["print"] = token_type::Print();
    tableLexems_["None"] = token_type::None();
    tableLexems_["or"] = token_type::Or();
    tableLexems_["and"] = token_type::And();
    tableLexems_["not"] = token_type::Not();
    tableLexems_["True"] = token_type::True();
    tableLexems_["False"] = token_type::False();

    tableOneChars_['+'] = token_type::Char{'+'};
    tableOneChars_['-'] = token_type::Char{'-'};
    tableOneChars_['*'] = token_type::Char{'*'};
    tableOneChars_['/'] = token_type::Char{'/'};
    tableOneChars_[':'] = token_type::Char{':'};
    tableOneChars_['('] = token_type::Char{'('};
    tableOneChars_[')'] = token_type::Char{')'};
    tableOneChars_['\n'] = token_type::Newline();

    char c;
    while (input_ >> c)
    {
        GetToken(c);
    }
    tokens_.push_back(token_type::Eof());
}

const Token& Lexer::CurrentToken() const {

    if (indexCurrentToken_ < tokens_.size())
        return tokens_.at(indexCurrentToken_);
    // Заглушка. Реализуйте метод самостоятельно
    //throw std::logic_error("Not implemented"s);
    return tokens_.back();
}

Token Lexer::NextToken() {
    if (indexCurrentToken_ < tokens_.size())
        indexCurrentToken_++;
    return CurrentToken();

    // Заглушка. Реализуйте метод самостоятельно
    //throw std::logic_error("Not implemented"s);
}

bool Lexer::isNumber(char c)
{
    return (c >= '0') && (c <= '9');
}

bool Lexer::isString(char c)
{
    return (c == '\'') || (c == '"');
}

bool Lexer::isId(char c)
{
    return ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <='z')) || c == '_';
}

void Lexer::GetToken(char c)
{
    if (isString(c)) {
        return LoadString();
    } else if (isNumber(c)) {
        return LoadNumber(c);
    } else if (isId(c)) {
        return LoadId(c);
    } else
        return LoadChar(c);
}

void Lexer::LoadChar(char c)
{
    auto findOneChar = tableOneChars_.find(c);
    if (findOneChar != tableOneChars_.end())
    {
        tokens_.push_back(findOneChar->second);
    }
    else
    {
        char nextC;
        if ('=' == c)
        {
            input_ >> nextC;
            if ('=' == nextC)
            {
                tokens_.push_back(token_type::Eq());
            }
            else
            {
                tokens_.push_back(token_type::Char{'='});
                if (' ' != nextC)
                    input_.putback(nextC);
            }
        }
        else if ('!' == c)
        {
            input_ >> nextC;//вычли следующий за ним знак =. Возможно потом нужно будет проверять и возвращать как ошибку
            if ('=' == nextC)
                tokens_.push_back(token_type::NotEq());
            else
                throw LexerError("Unexpected char "s + nextC);
        }
        else if ('>' == c)
        {
            input_ >> nextC;
            if ('=' == nextC)
            {
                tokens_.push_back(token_type::GreaterOrEq());
            }
            else
            {
                tokens_.push_back(token_type::Char{'>'});
                if (' ' != nextC)
                    input_.putback(nextC);
            }
        }
        else if ('<' == c)
        {
            input_ >> nextC;
            if ('=' == nextC)
            {
                tokens_.push_back(token_type::LessOrEq());
            }
            else
            {
                tokens_.push_back(token_type::Char{'<'});
                if (' ' != nextC)
                    input_.putback(nextC);
            }
        }
        else if (' ' == c)
        {
            if (tokens_.back() == token_type::Newline())
            {
                size_t counSpace = 1;
                while(true)
                {
                    input_ >> nextC;
                    if (' ' == nextC)
                        counSpace++;
                    else
                    {
                        input_.putback(nextC);
                        break;
                    }
                }
                if (counSpace %2 != 0)
                {
                    ;//нужно бы бросить ошибку
                }
                size_t countDent = counSpace/2;

                while(countDent != currentDent_)
                {
                    if (countDent > currentDent_)
                    {
                        tokens_.push_back(token_type::Indent());
                        currentDent_++;
                    }
                    else
                    {
                        tokens_.push_back(token_type::Dedent());
                        currentDent_--;
                    }
                }
            }
        }
        else if ('#' == c)
        {
            ;
        }
    }


}

void Lexer::LoadString()
{
    using namespace std::literals;

    auto it = std::istreambuf_iterator<char>(input_);
    auto end = std::istreambuf_iterator<char>();
    //std::string s;
    token_type::String token;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw LexerError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"' || ch) {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw LexerError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \n, \t, \', \"
            switch (escaped_char) {
                case 'n':
                    token.value.push_back('\n');
                    break;
                case 't':
                    token.value.push_back('\t');
                    break;
                case '\'':
                    token.value.push_back('\'');
                    break;
                case '"':
                    token.value.push_back('"');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw LexerError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw LexerError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            token.value.push_back(ch);
        }
        ++it;
    }
    tokens_.push_back(token);
}

void Lexer::LoadNumber(char c)
{
    token_type::Number token;
    input_.putback(c);
    input_ >> token.value;
    tokens_.push_back(token);
}

void Lexer::LoadId(char c)
{
    input_.putback(c);
    std::string str;
    input_ >> str;
    auto findLex = tableLexems_.find(str);
    if (findLex != tableLexems_.end())
    {
        tokens_.push_back(findLex->second);
    }
    else
    {
        token_type::Id token;
        token.value = str;
        tokens_.push_back(token);
    }
}

}  // namespace parse
