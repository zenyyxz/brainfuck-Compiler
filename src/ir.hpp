#pragma once

#include <vector>
#include <cstdint>

namespace bf {

/**
 * These represent the high-level actions our compiler understands.
 * Instead of just mapping 1:1 to Brainfuck symbols, we've taught
 * the compiler more complex "idioms" to make the output faster.
 */
enum class ActionType {
    MovePointer,      // Shift the tape pointer left or right
    ChangeValue,      // Add or subtract from the current cell
    Speak,            // Output the current cell as a character (.)
    Listen,           // Read a character into the current cell (,)
    BeginLoop,        // Start of a while(cell != 0) block ([)
    EndLoop,          // End of a while block (])
    ResetToZero,      // A common idiom [-] or [+]
    MultiplyAndAdd,   // Optimization for loops like [->++<]
    SeekZero,         // Optimization for loops like [>] or [<]
};

struct Action {
    ActionType type;
    int32_t amount;           // For moves and value changes
    size_t jump_target;       // For loop jumping logic
    int32_t relative_offset;  // For cross-cell operations like Multiply
};

// Our Intermediate Representation: a list of actions to perform.
using ActionPlan = std::vector<Action>;

} // namespace bf
