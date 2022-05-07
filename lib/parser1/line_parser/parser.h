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
    int rsrp = -140;
    Info() : g5(false) {}
    Info(int rsrp) : rsrp(rsrp) {}
    Info(bool g5) : g5(g5) {}
    virtual ~Info() {}
};

// struct InfoLte : public Info {
//     int rsrp = -140; // reference signal received power
//     int rssi = INT_MIN; // Received Signal Strength Indication
//     int rsrq = INT_MIN; // reference signal received quality
//     int rssnr = INT_MIN; //  reference signal signal-to-noise ratio
//     InfoLte() = default;
//     InfoLte(int rsrp, int rssi, int rsrq, int rssnr) :
//         rssi(rssi), rsrq(rsrq), rssnr(rssnr) {}
// };

// struct InfoNr : public Info {
//     int ssrsrp = -140; // SsRsrp
//     int csicqi = INT_MIN; // CsiCqi = getCsiCqiReport().get(getCsiCqiTableIndex())
//     int csirsrp = INT_MIN; // CsiRsrp
//     int csirsrq = INT_MIN; // CsiRsrq
//     int csisinr = INT_MIN; // CsiSinr
//     int ssrsrq = INT_MIN; // SsRsrq
//     int sssinr = INT_MIN; // SsSinr
//     InfoNr() : Info(true) {}
//     InfoNr(int csicqi, int csirsrp, int csirsrq, int csisinr, int ssrsrp, int ssrsrq, int sssinr)
//     :
//         Info(true), csicqi(csicqi), csirsrp(csirsrp), csirsrq(csirsrq),
//         csisinr(csisinr), ssrsrp(ssrsrp), ssrsrq(ssrsrq), sssinr(sssinr) {}
// };

struct CellInfo {
    int loc;
    std::list<std::pair<int, Info*>> pci_info_list;
    CellInfo() : loc(-1) {}
    ~CellInfo() {
        for (auto& p : pci_info_list) {
            delete p.second;
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
