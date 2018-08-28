#include "simple_ik/module.h"

#include <spdlog/spdlog.h>

#include <ik/ik.h>

#include <crsf/CRModel/TActorObject.h>
#include <crsf/CoexistenceInterface/TAvatarMemoryObject.h>

CRSEEDLIB_MODULE_CREATOR(SimpleIKModule)

// ************************************************************************************************
SimpleIKModule::SimpleIKModule(): crsf::TDynamicModuleInterface(CRMODULE_ID_STRING)
{
}

void SimpleIKModule::OnLoad()
{
    if (ik.init() != IK_OK)
    {
        m_logger->error("Failed to initialize IK");
        return;
    }

    /* Create a solver using the FABRIK algorithm */
    ik_solver_ = ik.solver.create(IK_FABRIK);

    /* Create a simple 3-bone structure */
    ik_nodes_.push_back(ik_solver_->node->create(0));
    ik_nodes_.back()->user_data = nullptr;
    ik_nodes_.push_back(ik_solver_->node->create_child(ik_nodes_.back(), 1));
    ik_nodes_.back()->user_data = nullptr;
    ik_nodes_.push_back(ik_solver_->node->create_child(ik_nodes_.back(), 2));
    ik_nodes_.back()->user_data = nullptr;
    ik_nodes_.push_back(ik_solver_->node->create_child(ik_nodes_.back(), 3));
    ik_nodes_.back()->user_data = nullptr;

    /* Attach an effector at the end */
    ik_effector_ = ik_solver_->effector->create();
    ik_solver_->effector->attach(ik_effector_, ik_nodes_.back());

    //ik_solver_->flags |= IK_ENABLE_TARGET_ROTATIONS;
    ik_solver_->flags &= ~IK_ENABLE_JOINT_ROTATIONS;

    /* Assign our tree to the solver, rebuild data and calculate solution */
    ik.solver.set_tree(ik_solver_, ik_nodes_.front());
    ik.solver.rebuild(ik_solver_);
}

void SimpleIKModule::OnStart()
{
}

void SimpleIKModule::OnExit()
{
    if (update_ik_task_)
        update_ik_task_->remove();
    update_ik_task_ = nullptr;

    ik.deinit();
}

void SimpleIKModule::SetActor(crsf::TActorObject* actor)
{
    if (!actor)
        return;

    NodePath np = actor->GetNodePath().find("**/r_acromioclavicular");
    if (!np)
        return;

    actor_joints_.clear();
    actor_joints_.reserve(ik_nodes_.size() * 2);     // prevent re-allocation

    for (auto&& node : ik_nodes_)
    {
        actor_joints_.push_back(np);

        const auto pos = np.get_pos();
        const auto quat = np.get_quat();
        node->position = ik.vec3.vec3(pos[0], pos[1], pos[2]);
        node->rotation = ik.quat.quat(quat.get_i(), quat.get_j(), quat.get_k(), quat.get_r());
        node->user_data = &actor_joints_.back();
        np = np.get_child(0);
    }

    ik.solver.update_distances(ik_solver_);

    use_actor_ = true;
}

void SimpleIKModule::SetAvatarMemoryObject(crsf::TAvatarMemoryObject* amo)
{
    if (!amo)
        return;

    const auto& am = amo->GetAvatarMemory();

    if (am.size() < 50)
        return;

    size_t index = 45;                  // r_acromioclavicular
    for (auto&& node : ik_nodes_)
    {
        const auto& pose = am[index];

        const auto pos = pose.GetPosition();
        const auto quat = pose.GetQuaternion();
        node->position = ik.vec3.vec3(pos[0], pos[1], pos[2]);
        node->rotation = ik.quat.quat(quat.get_i(), quat.get_j(), quat.get_k(), quat.get_r());
        node->user_data = amo;
        node->guid = index;

        ++index;
    }

    ik.solver.update_distances(ik_solver_);

    use_actor_ = false;
}

void SimpleIKModule::SolveIK()
{
    if (!ik_solver_)
        return;

    LVecBase3f pos;
    if (end_effector_)
    {
        pos = end_effector_.get_pos(actor_joints_.front().get_parent());
    }
    else if (end_effector_pos_)
    {
        pos = *end_effector_pos_;
    }
    else
    {
        m_logger->error("No end effector");
        return;
    }

    ik_effector_->target_position = ik.vec3.vec3(pos[0], pos[1], pos[2]);

    ik.solver.solve(ik_solver_);

    if (use_actor_)
    {
        ik.solver.iterate_affected_nodes(ik_solver_, [](ik_node_t* ikNode) {
            if (!ikNode->user_data)
                return;

            NodePath* node = (NodePath*)ikNode->user_data;
            node->set_pos(ikNode->position.x, ikNode->position.y, ikNode->position.z);
        });
    }
    else
    {
        ik.solver.iterate_affected_nodes(ik_solver_, [](ik_node_t* ikNode) {
            if (!ikNode->user_data)
                return;

            crsf::TAvatarMemoryObject* amo = (crsf::TAvatarMemoryObject*)ikNode->user_data;
            amo->GetAvatarMemory()[ikNode->guid].SetPosition(LVecBase3f(ikNode->position.x, ikNode->position.y, ikNode->position.z));
        });
    }
}

void SimpleIKModule::StartSolveIKLoop()
{
    if (update_ik_task_)
        return;

    update_ik_task_ = add_task([this](const rppanda::FunctionalTask* task) {
        SolveIK();
        return AsyncTask::DoneStatus::DS_cont;
    }, "SimpleIKModule::StartSolveIKLoop");
}

void SimpleIKModule::StopSolveIKLoop()
{
    if (update_ik_task_)
        update_ik_task_->remove();
}
