#include "optimizer.hpp"
#include <stack>

namespace bf {

/**
 * Our optimization sequence. We run multiple passes over the 
 * action plan to simplify it as much as possible.
 */
ActionPlan Optimizer::optimize(const ActionPlan& plan) {
    ActionPlan current = group_consecutive(plan);
    current = fold_relative_offsets(current);
    current = identify_multiplications(current);
    current = identify_scans(current);
    return rebuild_jump_targets(current);
}

/**
 * Pass 1: Combine consecutive actions into one.
 * For example: '+++' becomes 'ChangeValue by 3' instead of 
 * three separate actions. This is called Run-Length Encoding.
 */
ActionPlan Optimizer::group_consecutive(const ActionPlan& plan) {
    if (plan.empty()) return {};

    ActionPlan optimized;
    for (const auto& action : plan) {
        if (optimized.empty()) {
            optimized.push_back(action);
            continue;
        }

        auto& last = optimized.back();
        if (action.type == last.type && (action.type == ActionType::MovePointer || action.type == ActionType::ChangeValue)) {
            last.amount += action.amount;
            if (last.amount == 0) {
                optimized.pop_back(); // They cancelled each other out!
            }
        } else {
            optimized.push_back(action);
        }
    }
    return optimized;
}

/**
 * Pass 2: Fold offsets into values. 
 * Instead of 'Move right, change cell 1', we make it 'change cell at offset 1'. 
 * This reduces the number of pointer movement instructions.
 */
ActionPlan Optimizer::fold_relative_offsets(const ActionPlan& plan) {
    ActionPlan optimized;
    int32_t current_offset = 0;

    for (const auto& action : plan) {
        if (action.type == ActionType::MovePointer) {
            current_offset += action.amount;
        } else if (action.type == ActionType::ChangeValue) {
            Action folded = action;
            folded.relative_offset += current_offset;
            optimized.push_back(folded);
        } else {
            // "Break" the offset chain: emit a physical pointer movement first
            if (current_offset != 0) {
                optimized.push_back({ActionType::MovePointer, current_offset, 0, 0});
                current_offset = 0;
            }
            optimized.push_back(action);
        }
    }

    if (current_offset != 0) {
        optimized.push_back({ActionType::MovePointer, current_offset, 0, 0});
    }

    return optimized;
}

/**
 * Pass 3: Look for loop-based multiplications.
 * For example, [->+++<] can be replaced with 'multiply cell 0 by 3, 
 * add to cell 1, then set cell 0 to zero'.
 */
ActionPlan Optimizer::identify_multiplications(const ActionPlan& plan) {
    ActionPlan optimized;
    for (size_t i = 0; i < plan.size(); ++i) {
        if (plan[i].type == ActionType::BeginLoop) {
            size_t j = i + 1;
            bool simple_multiply = true;
            int32_t net_change_to_self = 0;
            int32_t total_pointer_move = 0;
            std::vector<Action> targets;

            while (j < plan.size() && plan[j].type != ActionType::EndLoop) {
                if (plan[j].type == ActionType::ChangeValue) {
                    if (plan[j].relative_offset == 0) {
                        net_change_to_self -= plan[j].amount;
                    } else {
                        targets.push_back(plan[j]);
                    }
                } else if (plan[j].type == ActionType::MovePointer) {
                    total_pointer_move += plan[j].amount;
                } else {
                    simple_multiply = false; // Too complex to optimize here
                    break;
                }
                j++;
            }

            // We look for loops that exactly clear the starting cell (like [-])
            if (simple_multiply && j < plan.size() && net_change_to_self == 1 && total_pointer_move == 0) {
                for (const auto& t : targets) {
                    optimized.push_back({ActionType::MultiplyAndAdd, t.amount, 0, t.relative_offset});
                }
                optimized.push_back({ActionType::ResetToZero, 0, 0, 0});
                i = j; // skip over the loop we just replaced
                continue;
            }
        }
        optimized.push_back(plan[i]);
    }
    return optimized;
}

/**
 * Pass 4: Look for pointer search loops.
 * [>] and [<] are converted into high-speed search actions.
 */
ActionPlan Optimizer::identify_scans(const ActionPlan& plan) {
    if (plan.size() < 3) return plan;

    ActionPlan optimized;
    for (size_t i = 0; i < plan.size(); ++i) {
        if (i + 2 < plan.size() &&
            plan[i].type == ActionType::BeginLoop &&
            plan[i+1].type == ActionType::MovePointer &&
            plan[i+2].type == ActionType::EndLoop) {
            
            optimized.push_back({ActionType::SeekZero, plan[i+1].amount, 0, 0});
            i += 2;
        } else {
            optimized.push_back(plan[i]);
        }
    }
    return optimized;
}

/**
 * Final Pass: Now that we've added and removed actions, all our 
 * jump targets are wrong. We need to re-link BeginLoop and EndLoop actions.
 */
ActionPlan Optimizer::rebuild_jump_targets(const ActionPlan& plan) {
    ActionPlan optimized = plan;
    std::stack<size_t> nested_loops;

    for (size_t i = 0; i < optimized.size(); ++i) {
        if (optimized[i].type == ActionType::BeginLoop) {
            nested_loops.push(i);
        } else if (optimized[i].type == ActionType::EndLoop) {
            if (nested_loops.empty()) continue;
            size_t start_at = nested_loops.top();
            nested_loops.pop();
            optimized[i].jump_target = start_at;
            optimized[start_at].jump_target = i;
        }
    }
    return optimized;
}

} // namespace bf
