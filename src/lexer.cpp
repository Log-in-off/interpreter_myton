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
    tableOneChars_[','] = token_type::Char{','};
    tableOneChars_['.'] = token_type::Char{'.'};

    std::istreambuf_iterator <char> it_ = std::istreambuf_iterator<char>(input_);
    std::istreambuf_iterator <char> end_ = std::istreambuf_iterator<char>();
    while (it_ != end_)
    {
        GetToken(it_, end_);
    }
    if (!tokens_.empty() && !tokens_.back().Is<token_type::Newline>() && !tokens_.back().Is<token_type::Dedent>() )
        tokens_.push_back(token_type::Newline());

    tokens_.push_back(token_type::Eof());
}

const Token& Lexer::CurrentToken() const {

    if (indexCurrentToken_ < tokens_.size())
        return tokens_.at(indexCurrentToken_);
    return tokens_.back();
}

Token Lexer::NextToken() {
    if (indexCurrentToken_ < tokens_.size())
        indexCurrentToken_++;
    return CurrentToken();
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

void Lexer::GetToken(std::istreambuf_iterator <char> it_, std::istreambuf_iterator <char> end_)
{
    const char c = *it_;
    if (isString(c)) {
        return LoadString(it_, end_);
    } else if (isNumber(c)) {
        return LoadNumber(it_, end_);
    } else if (isId(c)) {
        return LoadId(it_, end_);
    } else
        return LoadChar(it_, end_);
}

void Lexer::LoadChar(std::istreambuf_iterator<char> it_, std::istreambuf_iterator<char> end_)
{
    const char ch = *(it_++);

    auto findOneChar = tableOneChars_.find(ch);
    if (findOneChar != tableOneChars_.end())
    {
        tokens_.push_back(findOneChar->second);
    }
    else
    {
        char nextC  = *it_;
        if ('\n' == ch)
        {
            if (tokens_.empty())
                return;
            if (tokens_.back().Is<token_type::Newline>())
                return;

            tokens_.push_back(token_type::Newline());
            while(it_ != end_ && *it_ == '\n')
            {
                it_++;
            }

            if (*it_ != ' ')
            {
                while(currentDent_ > 0)
                {
                    currentDent_--;
                    tokens_.push_back(token_type::Dedent());
                }
            }
        }
        else if ('=' == ch)
        {
            if ('=' == nextC)
            {
                tokens_.push_back(token_type::Eq());
                it_++;
            }
            else
            {
                tokens_.push_back(token_type::Char{'='});
            }
        }
        else if ('!' == ch)
        {
            if ('=' == nextC)
            {
                tokens_.push_back(token_type::NotEq());
                it_++;
            }
            else
                throw LexerError("Unexpected char "s + nextC);
        }
        else if ('>' == ch)
        {
            if ('=' == nextC)
            {
                tokens_.push_back(token_type::GreaterOrEq());
                it_++;
            }
            else
            {
                tokens_.push_back(token_type::Char{'>'});
            }
        }
        else if ('<' == ch)
        {
            if ('=' == nextC)
            {
                tokens_.push_back(token_type::LessOrEq());
                it_++;
            }
            else
            {
                tokens_.push_back(token_type::Char{'<'});
            }
        }
        else if (' ' == ch)
        {
            if (tokens_.back() == token_type::Newline())
            {
                size_t counSpace = 1;
                while(true)
                {
                    if (' ' ==  *it_)
                    {
                        it_++;
                        counSpace++;
                    }
                    else
                    {
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
        else if ('#' == ch)
        {
            while (*it_ != '\n' && it_ != end_)
            {
                it_++;
            }
        }
    }


}

void Lexer::LoadString(std::istreambuf_iterator <char> it_, std::istreambuf_iterator <char> end_)
{
    using namespace std::literals;
    token_type::String token;
    const char charEnd = *(it_++);
    while (true) {
        if (it_ == end_) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw LexerError("String parsing error");
        }
        const char ch = *it_;
        if (charEnd == ch) {
            // Встретили закрывающий символ
            ++it_;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it_;
            if (it_ == end_) {
                // Поток завершился сразу после символа обратной косой черты
                throw LexerError("String parsing error");
            }
            const char escaped_char = *(it_);
            // Обрабатываем одну из последовательностей: \n, \t
            switch (escaped_char) {
                case 'n':
                    token.value.push_back('\n');
                    break;
                case 't':
                    token.value.push_back('\t');
                    break;
                 case '"':
                    token.value.push_back('"');
                    break;
                 case '\'':
                    token.value.push_back('\'');
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
        ++it_;
    }
    tokens_.push_back(token);
}

void Lexer::LoadNumber(std::istreambuf_iterator <char> it_, std::istreambuf_iterator <char> end_)
{
    std::string number;
    while (it_ != end_ &&  isNumber(*it_))
    {
        number += *(it_++);
    }
    tokens_.push_back(token_type::Number{std::stoi(number)});
}

void Lexer::LoadId(std::istreambuf_iterator <char> it_, std::istreambuf_iterator <char> end_)
{
    std::string str;

    str+= *(it_++);

    while (it_ != end_ &&  (isId(*it_) || isNumber(*it_)))
    {
        str+= *(it_++);
    }

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
