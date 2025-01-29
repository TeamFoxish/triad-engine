namespace AI {
    namespace VandalDrone {

        // PRECONDITIONS

        bool IsNearEnemy(const WorldState &in state) {
            return state.GetBool("IsNearEnemy");
        }

        bool IsReloading(const WorldState &in state) {
            return state.GetBool("IsReloading");
        }

        bool IsNotReloading(const WorldState &in state) {
            return !state.GetBool("IsReloading");
        }

        // OPERATORS

        AI::ExecutionResult ChaseEnemy(AIComponent@ controller, WorldState &inout state, float deltaTime, dictionary@ executionState) {
            HealthComponent@ enemy;
            if (!executionState.exists("timeToChangeDir")) {
                executionState["timeToChangeDir"] = 0.2f;
                @enemy = @Moon3ad::gameState.findNearestAlly(controller.parentTransform.GetPosition());
                if (enemy is null) {
                    return AI::ExecutionResult::FAILED;
                }
                log_debug("Vandal " + controller.GetParentName() + " found target: " + enemy.GetParentName());
                state.SetRef("Enemy", @any(@enemy));
            } else {
                state.GetRef("Enemy").retrieve(@enemy);
            }
            if (enemy !is null && enemy.GetParent() !is null) {
                if (controller is null) {
                    return AI::ExecutionResult::FAILED;
                }
                float leftTimeToChangeDir = float(executionState["timeToChangeDir"]) - deltaTime;
                bool needChangeDir = leftTimeToChangeDir < 0;
                if (needChangeDir) {
                    executionState["timeToChangeDir"] = 0.2f;
                    return AI::Drone::MoveTo(controller, state, enemy.GetParent().GetTransform().GetPosition(), true);
                } else {
                    executionState["timeToChangeDir"] = leftTimeToChangeDir;
                    return AI::Drone::MoveTo(controller, state, enemy.GetParent().GetTransform().GetPosition());
                }
            } else {
                return AI::ExecutionResult::FAILED;
            }
        }

        AI::ExecutionResult AttackEnemy(AIComponent@ controller, WorldState &inout state, float deltaTime, dictionary@ executionState) {
            HealthComponent@ enemy;
            state.GetRef("Enemy").retrieve(@enemy);
            if (enemy !is null) {
                enemy.AddHealth(-state.GetFloat("Damage"));
                return AI::ExecutionResult::FINISHED;
            } else {
                return AI::ExecutionResult::FAILED;
            }
        }

        AI::ExecutionResult Reload(AIComponent@ controller, WorldState &inout state, float deltaTime, dictionary@ executionState) {
            return AI::Drone::Wait(state.GetFloat("ReloadTime"), controller, state, deltaTime, executionState);
        }

        // EFFECTS

        void ReloadAfterAttack(WorldState &inout state) {
            state.SetBool("IsReloading", true);
        }

        void Reloaded(WorldState &inout state) {
            state.SetBool("IsReloading", false);
        }

        void EnemyGoesAway(WorldState &inout state) {
            state.SetBool("IsNearEnemy", false);
        }

        void EnemyApproached(WorldState &inout state) {
            state.SetBool("IsNearEnemy", true);
        }
    }
}

class VandalDroneDomain : Domain {

    void Init(WorldState &inout state, ICompositer@ parent, ColliderComponent@ visionPerception) override {
        
        // INITIAL STATE

        state.SetBool("IsFoundEnemy", false);
        state.SetBool("IsReloading", false);
        state.SetFloat("ReloadTime", 2.0f);
        state.SetFloat("Damage", 50.0f);

        // DOMAIN

        DomainBuilder@ domainBuilder = DomainBuilder(this);

        domainBuilder

        //PRIMITIVE

            .primitiveTask("Chase")
                .precondition(AI::VandalDrone::IsNotReloading)
                .operator(AI::VandalDrone::ChaseEnemy)
                .effect(AI::VandalDrone::EnemyApproached)
                .end()
            .primitiveTask("Attack")
                .precondition(AI::VandalDrone::IsNotReloading)
                .precondition(AI::VandalDrone::IsNearEnemy)
                .operator(AI::VandalDrone::AttackEnemy)
                .effect(AI::VandalDrone::ReloadAfterAttack)
                .end()
            .primitiveTask("Reload")
                .precondition(AI::VandalDrone::IsReloading)
                .operator(AI::VandalDrone::Reload)
                .effect(AI::VandalDrone::Reloaded)
                .effect(AI::VandalDrone::EnemyGoesAway)
                .end()

        // COMPOUND TASK

            .compoundTask("AttackAnything")
                .method()
                    .subtask("Chase")
                    .subtask("Attack")
                    .subtask("Reload")
                    .end()
                .end()

        // HIERARCHY

            .hierarchy("Vandal")
                .method()
                    .subtask("AttackAnything")
                    .end()
                .end()
            .end();
    }
}