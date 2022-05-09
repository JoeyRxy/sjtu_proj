#include "parser.h"

namespace rxy {

void Parser::line() {
    if (lexer.peek() == Token::ENDL) {
        lexer.next();
        return;
    }
    auto loc_tk = dynamic_cast<Number*>(lexer.peek().get());
    if (loc_tk) {
        res.emplace_back();
        res.back().loc = loc_tk->value;
    } else {
        throw ParseError("should be an integer (as location id) at line " + std::to_string(lexer.line()) + ", col " + std::to_string(lexer.col()));
    }
    lexer.next();
    check(Token::COLON);
    lexer.next();
    check(Token::L_B);
    lexer.next();
    if (lexer.peek() != Token::R_B) {
        items();
        check(Token::R_B);
    }
    lexer.next();
    if (not lexer.peek()) {
        return;
    }
    check(Token::ENDL);
    lexer.next();
}

void Parser::items() {
    item();
    while (lexer.peek() == Token::DELIM) {
        lexer.next();
        item();
    }
}

/**
 * Item -> < PCI, rsrp, 4G/5G>
 * */
void Parser::item() {
    check(Token::L_A);
    lexer.next();
    auto pci_tk = dynamic_cast<Number*>(lexer.peek().get());
    if (pci_tk) {
        pci = pci_tk->value;
    } else {
        throw ParseError("should be an integer (as pci) at line " + std::to_string(lexer.line()) + ", col " + std::to_string(lexer.col()));
    }
    lexer.next();
    check(Token::DELIM);
    lexer.next();
    auto info = new Info();
    res.back().pci_info_list.emplace_back(pci, info);
    auto rsrp_tk = lexer.peek().get();
    if (Number* __tk = dynamic_cast<Number*>(rsrp_tk)) {
        info->rsrp = __tk->value;
    } else if (Float* __tk = dynamic_cast<Float*>(rsrp_tk)) {
        info->rsrp = __tk->value;
    } else {
        throw ParseError("should be an integer (as rsrp) at line " + std::to_string(lexer.line()) + ", col " + std::to_string(lexer.col()));
    }
    lexer.next();
    check(Token::DELIM);
    lexer.next();
    if (lexer.peek() == Token::G4) {
        info->g5 = false;
    } else {
        info->g5 = true;
    }
    lexer.next();
    check(Token::R_A);
    lexer.next();
}

}
