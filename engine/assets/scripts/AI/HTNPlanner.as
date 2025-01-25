class HTNPlanner {

    // we need ony copy of state
    array<const PrimitiveTask@> GeneratePlan(WorldState workingState, const Domain &in domain) {

        PlanGenerationContext context = PlanGenerationContext(workingState, domain.GetRoot());

        while(!context.unprocessedTasks.isEmpty()) {
            const Task@ currentTask = context.PopTask();
            if (currentTask.isPrimitive()) {
                const PrimitiveTask@ primitiveTask = cast<const PrimitiveTask@>(currentTask);
                if (primitiveTask.checkPrecondition(workingState)) {
                    primitiveTask.applyEffect(workingState);
                    context.finalPlan.insertLast(primitiveTask);
                } else {
                    context.RestoreToLastDecomposedTask();
                }
            } else {
                const CompoundTask@ compoundTask = cast<const CompoundTask@>(currentTask);
                const Method@ method = compoundTask.FindSatisfyingMethod(workingState);
                if (method !is null) {
                    context.RecordCurrentState(compoundTask);
                    const array<Task@> subtasks = method.GetSubtasks();
                    for (uint i = 0; i < subtasks.length(); i++) {
                        context.unprocessedTasks.insertAt(0, subtasks[i]);
                    }
                } else {
                    context.RestoreToLastDecomposedTask();
                }
            }
        }
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