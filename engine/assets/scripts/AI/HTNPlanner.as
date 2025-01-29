class HTNPlanner {

    // we need ony copy of state
    array<const PrimitiveTask@> GeneratePlan(WorldState workingState, const Domain &in domain) {

        log_debug("=== NEW PLAN GENERATION ===");
        PlanGenerationContext context = PlanGenerationContext(workingState, domain.GetRoot());

        while(!context.unprocessedTasks.isEmpty()) {
            const Task@ currentTask = context.PopTask();
            log_debug("Current task: " + currentTask.GetName());
            if (currentTask.isPrimitive()) {
                log_debug("Task is primitive");
                const PrimitiveTask@ primitiveTask = cast<const PrimitiveTask@>(currentTask);
                if (primitiveTask.checkPrecondition(workingState)) {
                    primitiveTask.applyEffect(workingState);
                    context.finalPlan.insertLast(primitiveTask);
                    log_debug("Primitive task added");
                } else {
                    log_debug("Primitive task caused rollback");
                    context.RestoreToLastDecomposedTask();
                }
            } else {
                log_debug("Task is compound");
                const CompoundTask@ compoundTask = cast<const CompoundTask@>(currentTask);
                const Method@ method = compoundTask.FindSatisfyingMethod(workingState);
                if (method !is null) {
                    log_debug("Satisfied method found");
                    context.RecordCurrentState(compoundTask);
                    const array<Task@> subtasks = method.GetSubtasks();
                    for (uint i = 0; i < subtasks.length(); i++) {
                        log_debug("Adding task to stack: " + subtasks[i].GetName());
                        context.unprocessedTasks.insertLast(subtasks[i]);
                    }
                } else {
                    log_debug("Compound task caused rollback");
                    context.RestoreToLastDecomposedTask();
                }
            }
        }
        log_debug("Generated plan size: " + context.finalPlan.length());
        log_debug("=== GENERATED PLAN ===");
        for (uint i = 0; i < context.finalPlan.length(); i++) {
            log_debug("  " + context.finalPlan[i].GetName());
        }
        log_debug("======================");
        return context.finalPlan;
    }
};

class DecompostionRecord {
    const CompoundTask@ task;
    array<const PrimitiveTask@> plan;
    array<const Task@> unprocessedTasks;
    WorldState state;

    DecompostionRecord() {}

    DecompostionRecord(
        const CompoundTask@ decomposedTask,
        array<const PrimitiveTask@> _plan,
        array<const Task@> _unprocessedTasks,
        WorldState _state) {
        @this.task = @decomposedTask;
        this.plan = _plan;
        this.unprocessedTasks = _unprocessedTasks;
        this.state = _state;
    }
};

class PlanGenerationContext {
    array<DecompostionRecord> history;
    array<const PrimitiveTask@> finalPlan;
    array<const Task@> unprocessedTasks;
    WorldState state;

    PlanGenerationContext() {}

    PlanGenerationContext(WorldState& _state, const CompoundTask@ root) {
        if (root is null) {
            log_critical("ROOT IS NULL");
        }
        this.state = state;
        unprocessedTasks.insertLast(root);
    }

    void RestoreToLastDecomposedTask() {
        DecompostionRecord record = history[history.length() - 1];
        finalPlan = record.plan;
        unprocessedTasks = record.unprocessedTasks;
        state = record.state;
        unprocessedTasks.insertAt(0, record.task);
        history.removeAt(history.length() -1);
    }

    void RecordCurrentState(const CompoundTask@ task) {
        history.insertLast(DecompostionRecord(task, finalPlan, unprocessedTasks, state));
    }

    const Task@ PopTask() {
        const Task@ task = unprocessedTasks[0];
        unprocessedTasks.removeAt(0);
        return task;
    }
};