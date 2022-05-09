#include "lexer.h"
#include <iostream>
#include <sstream>

namespace rxy {

TokenPtr 
    Token::DELIM = std::make_shared<Token>(","), 
    Token::COLON = std::make_shared<Token>(":"), 
    Token::L_B = std::make_shared<Token>("["), 
    Token::R_B = std::make_shared<Token>("]"), 
    Token::L_A = std::make_shared<Token>("<"), 
    Token::R_A = std::make_shared<Token>(">"), 
    Token::G4 = std::make_shared<Token>("4G"), 
    Token::G5 = std::make_shared<Token>("5G"), 
    Token::ENDL = std::make_shared<Token>("END of LINE", 1);

double Lexer::next_number() {
    int x = 0;
    while (in.good() && is_digit(in.peek())) {
        x *= 10;
        x += (in.peek() - '0');
        ++_col;
        in.get();
    }
    if (in.good() && in.peek() == '.') {
        ++_col;
        in.get();
        double fraction = 0.;
        while (in.good() && is_digit(in.peek())) {
            fraction += (in.peek() - '0');
            fraction /= 10.;
            ++_col;
            in.get();
        }
        return x + fraction;
    } else {
        return x;
    }
}

void Lexer::next() {
    while (in.good() && (in.peek() == ' ' || in.peek() == '\t')) {
        ++_col;
        in.get();
    }
    if (!in.good()) {
        token.reset();
        return;
    }
    if (is_digit(in.peek())) {
        char ch = in.get();
        if (in.good() && in.peek() == 'G') {
            in.get();
            if (ch == '4') {
                token = Token::G4;
            } else if (ch == '5') {
                token = Token::G5;
            } else {
                throw LexError("can not recognize pci type as a token at line " + std::to_string(_line) + ", col " + std::to_string(_col));
            }
            _col += 2;
        } else {
            in.putback(ch);
            int pos = _col;
            double x = next_number();
            if (x == (int)x) {
                token = std::make_shared<Number>(x, _col - pos);
            } else {
                token = std::make_shared<Float>(x, _col - pos);
            }
        }
        return;
    }
    if (in.peek() == '-') {
        int pos = _col;
        ++_col;
        in.get();
        while (in.good() && in.peek() == ' ') {
            ++_col;
            in.get();
        }
        double x = next_number();
        if (x == (int)x) {
            token = std::make_shared<Number>(-x, _col - pos);
        } else {
            token = std::make_shared<Float>(-x, _col - pos);
        }
        return;
    }
    char c;
    switch (in.peek()) {
        case '\r':
            token = Token::ENDL;
            ++_line;
            _col = 0;
            c = in.get();
            if (c != '\n') {
                in.putback(c);
            }
            break;
        case '\n':
            token = Token::ENDL;
            ++_line;
            _col = 0;
            break;
        case ',':
            token = Token::DELIM;
            break;
        case ':':
            token = Token::COLON;
            break;
        case '[':
            token = Token::L_B;
            break;
        case ']':
            token = Token::R_B;
            break;
        case '<':
            token = Token::L_A;
            break;
        case '>':
            token = Token::R_A;
            break;
    }
    in.get();
    _col++;
}

}
