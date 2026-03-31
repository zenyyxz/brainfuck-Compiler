#pragma once

#include "ir.hpp"

namespace bf {

/**
 * The Optimizer's job is to take our simple action plan and transform it 
 * into something faster by looking for patterns and replacing them with 
 * more efficient single actions.
 */
class Optimizer {
public:
    static ActionPlan optimize(const ActionPlan& plan);

private:
    static ActionPlan group_consecutive(const ActionPlan& plan);
    static ActionPlan identify_multiplications(const ActionPlan& plan);
    static ActionPlan identify_scans(const ActionPlan& plan);
    static ActionPlan fold_relative_offsets(const ActionPlan& plan);
    static ActionPlan rebuild_jump_targets(const ActionPlan& plan);
};

} // namespace bf
