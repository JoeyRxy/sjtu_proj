#pragma once
#include <istream>
#include <memory>
#include <string>

namespace rxy {

struct Token;
using TokenPtr = std::shared_ptr<Token>;

struct Token {
    std::string str;
    size_t len;
    Token() = default;
    Token(std::string&& str) : str(std::move(str)), len(str.size()) {}
    Token(std::string&& str, int len) : str(std::move(str)), len(len) {}
    virtual ~Token() = default;

    static TokenPtr DELIM, COLON, L_B, R_B, L_A, R_A, G4, G5, ENDL;
};

struct Number : public Token {
    int value;
    Number(int value, int len) : value(value) {this->len = len;}
};

struct Float : public Token {
    double value;
    Float(double value, int len) : value(value) {this->len = len;}
};

struct LexError : public std::exception {
    const char* reason;
    LexError(std::string&& reason) noexcept : reason(reason.c_str()) {}
    LexError(const char* reason) noexcept : reason(reason) {}

    const char* what() const noexcept override { return reason; }
};

class Lexer {
   private:
    TokenPtr token;
    // std::string const & str;
    std::istream& in;
    size_t _line;
    size_t _col;

    bool is_digit(char c) { return c >= '0' && c <= '9'; }

    double next_number();

   public:
    Lexer(std::istream& in) : in(in), _line(1), _col(1) {}
    // Lexer(std::string const & str) : str(str), i(0), _line(1), _col(1) {}
    TokenPtr peek() const { return token; }
    void next();

    size_t line() const { return _line; }
    size_t col() const {
        if (token)
            return _col - token->str.size();
        else
            return _col;
    }
};

}  // namespace rxy
