#pragma once

#include <crsf/CRAPI/TDynamicModuleInterface.h>
#include <render_pipeline/rppanda/showbase/direct_object.hpp>

#include <nodePath.h>

namespace crsf {
class TActorObject;
class TAvatarMemoryObject;
}

struct ik_solver_t;
struct ik_effector_t;
struct ik_node_t;

class SimpleIKModule: public crsf::TDynamicModuleInterface, public rppanda::DirectObject
{
public:
    SimpleIKModule();

    void OnLoad() override;
    void OnStart() override;
    void OnExit() override;

    virtual void SetActor(crsf::TActorObject* actor);
    virtual void SetAvatarMemoryObject(crsf::TAvatarMemoryObject* amo);

    void SetEndEffector(NodePath np);
    void SetEndEffector(LVecBase3f* pos);

    virtual void SolveIK();
    virtual void StartSolveIKLoop();
    virtual void StopSolveIKLoop();

private:
    ik_solver_t* ik_solver_ = nullptr;
    ik_effector_t* ik_effector_ = nullptr;
    std::vector<ik_node_t*> ik_nodes_;

    rppanda::FunctionalTask* update_ik_task_ = nullptr;

    NodePath end_effector_;
    LVecBase3f* end_effector_pos_ = nullptr;

    bool use_actor_ = false;
    std::vector<NodePath> actor_joints_;
};

// ************************************************************************************************

inline void SimpleIKModule::SetEndEffector(NodePath np)
{
    end_effector_ = np;
}

inline void SimpleIKModule::SetEndEffector(LVecBase3f* pos)
{
    end_effector_pos_ = pos;
}
