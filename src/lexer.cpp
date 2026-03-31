#include "lexer.hpp"
#include <stdexcept>

namespace bf {

/**
 * Our Lexer's job is to read raw Brainfuck source code and convert 
 * it into a simple list of actions that our compiler can then optimize.
 */
ActionPlan Lexer::parse(std::string_view source) {
    ActionPlan plan;
    std::stack<size_t> nested_loops;

    for (char c : source) {
        switch (c) {
            case '>': plan.push_back({ActionType::MovePointer, 1, 0, 0}); break;
            case '<': plan.push_back({ActionType::MovePointer, -1, 0, 0}); break;
            case '+': plan.push_back({ActionType::ChangeValue, 1, 0, 0}); break;
            case '-': plan.push_back({ActionType::ChangeValue, -1, 0, 0}); break;
            case '.': plan.push_back({ActionType::Speak, 0, 0, 0}); break;
            case ',': plan.push_back({ActionType::Listen, 0, 0, 0}); break;
            case '[': {
                size_t start_at = plan.size();
                plan.push_back({ActionType::BeginLoop, 0, 0, 0});
                nested_loops.push(start_at);
                break;
            }
            case ']': {
                if (nested_loops.empty()) {
                    throw std::runtime_error("Found a closing ']' with no matching '['.");
                }
                size_t start_at = nested_loops.top();
                nested_loops.pop();
                size_t end_at = plan.size();
                plan.push_back({ActionType::EndLoop, 0, start_at, 0});
                plan[start_at].jump_target = end_at;
                break;
            }
            default: break; // Ignore anything else (like comments)
        }
    }

    if (!nested_loops.empty()) {
        throw std::runtime_error("Oh no! An opening '[' was left hanging.");
    }

    return plan;
}

} // namespace bf
