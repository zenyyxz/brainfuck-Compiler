#pragma once

#include "ir.hpp"
#include <string_view>
#include <stack>

namespace bf {

/**
 * The Lexer reads raw Brainfuck source code and turns it into 
 * a list of actions that our compiler can then optimize.
 */
class Lexer {
public:
    static ActionPlan parse(std::string_view source);
};

} // namespace bf
