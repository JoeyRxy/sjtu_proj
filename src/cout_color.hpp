#pragma once
#include <iostream>

namespace __color {
enum Code {
    FG_RED = 31,
    FG_GREEN = 32,
    FG_YELLOW = 33,
    FG_BLUE = 34,
    FG_DEFAULT = 39,
    BG_RED = 41,
    BG_GREEN = 42,
    BG_YELLOW = 43,
    BG_BLUE = 44,
    BG_DEFAULT = 49
};

class Modifier {
    Code code;

   public:
    constexpr Modifier(Code pCode) : code(pCode) {}
    friend std::ostream& operator<<(std::ostream& os, const Modifier& mod) {
        return os << "\033[" << mod.code << "m";
    }
};
constexpr Modifier gre() { return Modifier(FG_GREEN); }
constexpr Modifier red() { return Modifier(FG_RED); }
constexpr Modifier yel() { return Modifier(FG_YELLOW); }
constexpr Modifier blu() { return Modifier(FG_BLUE); }
constexpr Modifier def() { return Modifier(FG_DEFAULT); }
constexpr Modifier bg_red() { return Modifier(BG_RED); }
constexpr Modifier bg_gre() { return Modifier(BG_GREEN); }
constexpr Modifier bg_yel() { return Modifier(BG_YELLOW); }
constexpr Modifier bg_blu() { return Modifier(BG_BLUE); }
constexpr Modifier bg_def() { return Modifier(BG_DEFAULT); }
}  // namespace __color
