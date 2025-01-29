namespace AI {
    namespace GuardianDrone {

        // PRECONDITIONS

        bool IsFoundEnemy(const WorldState &in state) {
            return state.GetBool("IsFoundEnemy");
        }

        bool IsNearEnemy(const WorldState &in state) {
            return state.GetBool("IsNearEnemy");
        }

        bool IsRetreating(const WorldState &in state) {
            return state.GetBool("IsRetreating");
        }

        bool IsNotRetreating(const WorldState &in state) {
            return !state.GetBool("IsRetreating");
        }

        bool IsReloading(const WorldState &in state) {
            return state.GetBool("IsReloading");
        }

        bool IsNotReloading(const WorldState &in state) {
            return !state.GetBool("IsReloading");
        }

        bool IsNearStorage(const WorldState &in state) {
            return !state.GetBool("IsNearStorage");
        }

        // OPERATORS

        AI::ExecutionResult ChaseEnemy(AIComponent@ controller, WorldState &inout state, float deltaTime, dictionary@ executionState) {
            HealthComponent@ enemy;
            if (!executionState.exists("timeToChangeDir")) {
                executionState["timeToChangeDir"] = 0.2f;
                @enemy = @Moon3ad::gameState.findNearestEnemy(controller.parentTransform.GetPosition());
                if (enemy is null || enemy.GetParent() is null) {
                    return AI::ExecutionResult::FAILED;
                }
                log_debug("Guardian " + controller.GetParentName() + " found target: " + enemy.GetParentName());
                state.SetRef("Enemy", @any(@enemy));
            } else {
                state.GetRef("Enemy").retrieve(@enemy);
            }
            if (enemy !is null && enemy.GetParent() !is null) {
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
                log_critical("Guradian chase failed");
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

        AI::ExecutionResult Retreat(AIComponent@ controller, WorldState &inout state, float deltaTime, dictionary@ executionState) {
            return AI::Drone::MoveTo(controller, state, Moon3ad::gameState.storageLocation);
        }

        AI::ExecutionResult Heal(AIComponent@ controller, WorldState &inout state, float deltaTime, dictionary@ executionState) {
            return AI::Drone::Wait(state.GetFloat("HealTime"), controller, state, deltaTime, executionState);
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

        void StorageApproached(WorldState &inout state) {
            state.SetBool("IsNearStorage", true);
        }

        void Healed(WorldState &inout state) {
            state.SetBool("IsRetreating", false);
            state.SetBool("IsNearStorage", false);
        }
    }
}

class GuardianDroneDomain : Domain {

    void Init(WorldState &inout state, ICompositer@ parent, ColliderComponent@ visionPerception) override {
        
        // INITIAL STATE

        state.SetBool("IsFoundEnemy", false);
        state.SetBool("IsReloading", false);
        state.SetFloat("ReloadTime", 2.0f);
        state.SetBool("IsRetreating", false);
        state.SetBool("IsNearStorage", false);
        state.SetFloat("Damage", 50.0f);
        state.SetFloat("HealTime", 5.0f);

        // DOMAIN

        DomainBuilder@ domainBuilder = DomainBuilder(this);

        domainBuilder

        //PRIMITIVE

            .primitiveTask("Chase")
                .precondition(AI::GuardianDrone::IsNotRetreating)
                .precondition(AI::GuardianDrone::IsNotReloading)
                .operator(AI::GuardianDrone::ChaseEnemy)
                .effect(AI::GuardianDrone::EnemyApproached)
                .end()
            .primitiveTask("Attack")
                .precondition(AI::GuardianDrone::IsNotRetreating)
                .precondition(AI::GuardianDrone::IsNotReloading)
                .precondition(AI::GuardianDrone::IsNearEnemy)
                .operator(AI::GuardianDrone::AttackEnemy)
                .effect(AI::GuardianDrone::ReloadAfterAttack)
                .end()
            .primitiveTask("Reload")
                .precondition(AI::GuardianDrone::IsNotRetreating)
                .precondition(AI::GuardianDrone::IsReloading)
                .operator(AI::GuardianDrone::Reload)
                .effect(AI::GuardianDrone::Reloaded)
                .effect(AI::GuardianDrone::EnemyGoesAway)
                .end()
            .primitiveTask("Retreat")
                .precondition(AI::GuardianDrone::IsRetreating)
                .operator(AI::GuardianDrone::Retreat)
                .effect(AI::GuardianDrone::StorageApproached)
                .end()
            .primitiveTask("Heal")
                .precondition(AI::GuardianDrone::IsRetreating)
                .precondition(AI::GuardianDrone::IsNearStorage)
                .operator(AI::GuardianDrone::Heal)
                .effect(AI::GuardianDrone::Healed)
                .end()

        // COMPOUND TASK

            .compoundTask("Patrol")
                .method()
                    .subtask("Chase")
                    .subtask("Attack")
                    .subtask("Reload")
                    .end()
                .end()
            .compoundTask("RetreatAndHeal")
                .method()
                    .subtask("Retreat")
                    .subtask("Heal")
                    .end()
                .end()

        // HIERARCHY

            .hierarchy("Guardian")
                .method()
                    .precondition(AI::GuardianDrone::IsNotRetreating)
                    .subtask("Patrol")
                    .end()
                .method()
                    .subtask("RetreatAndHeal")
                    .end()
                .end()
            .end();
    }
}