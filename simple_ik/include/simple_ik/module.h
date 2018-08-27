#pragma once

#include <crsf/CRAPI/TDynamicModuleInterface.h>
#include <render_pipeline/rppanda/showbase/direct_object.hpp>

#include <nodePath.h>

namespace crsf {
class TActorObject;
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
    virtual void SetEndEffector(NodePath np);

private:
    void UpdateIK();

    ik_solver_t* ik_solver_ = nullptr;
    ik_effector_t* ik_effector_ = nullptr;
    std::vector<ik_node_t*> ik_nodes_;

    rppanda::FunctionalTask* update_ik_task_ = nullptr;

    NodePath end_effector_;
    std::vector<NodePath> actor_joints_;
    NodePath r_acromioclavicular_;
};
