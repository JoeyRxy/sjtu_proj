#pragma once
#include <climits>
#include <iostream>
#include <list>

#include "lexer.h"

namespace rxy {

struct ParseError : std::exception {
    const char* reason;
    ParseError(const char* reason) : reason(reason) {}
    ParseError(std::string&& reason) : reason(reason.c_str()) {}

    const char* what() const noexcept override { return reason; }
};

struct Info {
    bool g5;
    double rsrp = -140;
    Info() : g5(false) {}
    Info(double rsrp) : rsrp(rsrp) {}
    Info(bool g5) : g5(g5) {}
    virtual ~Info() {}
};

struct CellInfo {
    int loc;
    std::list<std::pair<int, Info*>> pci_info_list;
    CellInfo() : loc(-1) {}
    ~CellInfo() {
        for (auto& p : pci_info_list) {
            if (p.second) delete p.second;
        }
    }
};

class Parser {
   private:
    Lexer lexer;
    bool succ;
    std::list<CellInfo> res;
    int pci;

    void lines() {
        line();
        while (lexer.peek()) {
            line();
        }
    }
    void line();
    void items();
    void item();

    void check(TokenPtr token) {
        if (!lexer.peek()) {
            throw ParseError("Unexpected end of file: should be a '" + token->str + "' at line " +
                             std::to_string(lexer.line()) + ", col " + std::to_string(lexer.col()));
        }
        if (lexer.peek() != token) {
            throw ParseError("should be a '" + token->str + "' at line " +
                             std::to_string(lexer.line()) + ", col " + std::to_string(lexer.col()) +
                             " instead of '" + lexer.peek()->str + "'");
        }
    }

   public:
    bool parse() {
        if (not lexer.peek()) {
            std::cerr << "contain nothing" << std::endl;
            return succ;
        }
        try {
            lines();
            succ = true;
        } catch (ParseError const& e) {
            std::cerr << e.what() << std::endl;
        } catch (LexError const& e) {
            std::cerr << e.what() << std::endl;
        }
        return succ;
    }

    std::list<CellInfo>& get() {
        if (succ)
            return res;
        else
            throw std::runtime_error("parse failed");
    }

    Parser(std::istream& in) : lexer(in), succ(false), pci(-1) { lexer.next(); }
};

}  // namespace rxy
