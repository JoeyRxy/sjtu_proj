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
* Item -> < PCI, 4G, rsrp(, rssi(, rsrq(, rssnr)?)?)? >
* Item -> < PCI, 5G, ssrsrp(, csicqi(, csirsrp(, csirsrq(, csisinr(, ssrsrq(, sssinr)?)?)?)?)?)? >
* */
// void Parser::item() {
//     check(Token::L_A);
//     lexer.next();
//     auto pci_tk = dynamic_cast<Number*>(lexer.peek().get());
//     if (pci_tk) {
//         pci = pci_tk->value;
//     } else {
//         throw ParseError("should be an integer (as pci) at line " + std::to_string(lexer.line()) + ", col " + std::to_string(lexer.col()) + " instead of '" + lexer.peek()->str + "'");
//     }
//     lexer.next();
//     check(Token::DELIM);
//     lexer.next();
//     if (lexer.peek() == Token::G4) {
//         lexer.next();
//         check(Token::DELIM);
//         auto info = new InfoLte();
//         res.back().pci_info_list.emplace_back(pci, info);
//         lexer.next();
//         auto num = dynamic_cast<Number*>(lexer.peek().get());
//         if (num) {
//             info->rsrp = num->value;
//         } else {
//             throw ParseError("should be rsrp at line " + std::to_string(lexer.line()) + ", col " + std::to_string(lexer.col()) + " instead of '" + lexer.peek()->str + "'");
//         }
//         lexer.next();
//         if (lexer.peek() != Token::DELIM) {
//             goto jmp;
//         }
//         lexer.next();
//         num = dynamic_cast<Number*>(lexer.peek().get());
//         if (num) {
//             info->rssi = num->value;
//         } else {
//             throw ParseError("should be an integer at line " + std::to_string(lexer.line()) + ", col " + std::to_string(lexer.col()) + " instead of '" + lexer.peek()->str + "'");
//         }
//         lexer.next();
//         if (lexer.peek() != Token::DELIM) {
//             goto jmp;
//         }
//         lexer.next();
//         num = dynamic_cast<Number*>(lexer.peek().get());
//         if (num) {
//             info->rsrq = num->value;
//         } else {
//             throw ParseError("should be an integer at line " + std::to_string(lexer.line()) + ", col " + std::to_string(lexer.col()) + " instead of '" + lexer.peek()->str + "'");
//         }
//         lexer.next();
//         if (lexer.peek() != Token::DELIM) {
//             goto jmp;
//         }
//         lexer.next();
//         num = dynamic_cast<Number*>(lexer.peek().get());
//         if (num) {
//             info->rssnr = num->value;
//         } else {
//             throw ParseError("should be an integer at line " + std::to_string(lexer.line()) + ", col " + std::to_string(lexer.col()) + " instead of '" + lexer.peek()->str + "'");
//         }
//     } else if (lexer.peek() == Token::G5) {
//         lexer.next();
//         if (lexer.peek() != Token::DELIM) {
//             goto jmp;
//         }
//         auto info = new InfoNr();
//         res.back().pci_info_list.emplace_back(pci, info);
//         lexer.next();
//         auto num = dynamic_cast<Number*>(lexer.peek().get());
//         if (num) {
//             info->ssrsrp = num->value;
//         } else {
//             throw ParseError("should be rsrp at line " + std::to_string(lexer.line()) + ", col " + std::to_string(lexer.col()) + " instead of '" + lexer.peek()->str + "'");
//         }
//         lexer.next();
//         if (lexer.peek() != Token::DELIM) {
//             goto jmp;
//         }
//         lexer.next();
//         num = dynamic_cast<Number*>(lexer.peek().get());
//         if (num) {
//             info->csicqi = num->value;
//         } else {
//             throw ParseError("should be an integer at line " + std::to_string(lexer.line()) + ", col " + std::to_string(lexer.col()) + " instead of '" + lexer.peek()->str + "'");
//         }
//         lexer.next();
//         if (lexer.peek() != Token::DELIM) {
//             goto jmp;
//         }
//         lexer.next();
//         num = dynamic_cast<Number*>(lexer.peek().get());
//         if (num) {
//             info->csirsrp = num->value;
//         } else {
//             throw ParseError("should be an integer at line " + std::to_string(lexer.line()) + ", col " + std::to_string(lexer.col()) + " instead of '" + lexer.peek()->str + "'");
//         }
//         lexer.next();
//         if (lexer.peek() != Token::DELIM) {
//             goto jmp;
//         }
//         lexer.next();
//         num = dynamic_cast<Number*>(lexer.peek().get());
//         if (num) {
//             info->csirsrq = num->value;
//         } else {
//             throw ParseError("should be an integer at line " + std::to_string(lexer.line()) + ", col " + std::to_string(lexer.col()) + " instead of '" + lexer.peek()->str + "'");
//         }
//         lexer.next();
//         if (lexer.peek() != Token::DELIM) {
//             goto jmp;
//         }
//         lexer.next();
//         num = dynamic_cast<Number*>(lexer.peek().get());
//         if (num) {
//             info->csisinr = num->value;
//         } else {
//             throw ParseError("should be an integer at line " + std::to_string(lexer.line()) + ", col " + std::to_string(lexer.col()) + " instead of '" + lexer.peek()->str + "'");
//         }
//         lexer.next();
//         if (lexer.peek() != Token::DELIM) {
//             goto jmp;
//         }
//         lexer.next();
//         num = dynamic_cast<Number*>(lexer.peek().get());
//         if (num) {
//             info->ssrsrq = num->value;
//         } else {
//             throw ParseError("should be an integer at line " + std::to_string(lexer.line()) + ", col " + std::to_string(lexer.col()) + " instead of '" + lexer.peek()->str + "'");
//         }
//         lexer.next();
//         if (lexer.peek() != Token::DELIM) {
//             goto jmp;
//         }
//         lexer.next();
//         num = dynamic_cast<Number*>(lexer.peek().get());
//         if (num) {
//             info->sssinr = num->value;
//         } else {
//             throw ParseError("should be an integer at line " + std::to_string(lexer.line()) + ", col " + std::to_string(lexer.col()) + " instead of '" + lexer.peek()->str + "'");
//         }
//         lexer.next();
//         if (lexer.peek() != Token::DELIM) {
//             goto jmp;
//         }
//     } else {
//         throw ParseError("should be 4G/5G at line " + std::to_string(lexer.line()) + ", col " + std::to_string(lexer.col()) + " instead of '" + lexer.peek()->str + "'");
//     }
//     lexer.next();
//     check(Token::DELIM);
//     lexer.next();
// jmp:
//     check(Token::R_A);
//     lexer.next();
// }

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
    auto num_tk = dynamic_cast<Number*>(lexer.peek().get());
    if (num_tk) {
        info->rsrp = num_tk->value;
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
