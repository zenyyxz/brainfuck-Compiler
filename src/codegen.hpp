#pragma once

#include "ir.hpp"
#include <ostream>

namespace bf {

/**
 * CodeGen is our assembly artist. It takes the action plan and 
 * translates it into high-performance x86-64 NASM assembly.
 */
class CodeGen {
public:
    static void generate_nasm(const ActionPlan& plan, std::ostream& out);
};

} // namespace bf
