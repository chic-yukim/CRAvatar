#include "main.hpp"

#include <spdlog/spdlog.h>

#include <render_pipeline/rppanda/showbase/showbase.hpp>
#include <render_pipeline/rpcore/render_pipeline.hpp>
#include <render_pipeline/rpcore/loader.hpp>
#include <render_pipeline/rpcore/globals.hpp>
#include <render_pipeline/rpcore/util/primitives.hpp>

#include <crsf/RenderingEngine/TGraphicRenderEngine.h>
#include <crsf/CoexistenceInterface/TDynamicStageMemory.h>
#include <crsf/CoexistenceInterface/TAvatarMemoryObject.h>
#include <crsf/CRModel/TActorObject.h>
#include <crsf/CRModel/TCharacter.h>
#include <crsf/CRModel/TWorld.h>
#include <crsf/CRModel/TCube.h>
#include <crsf/CREngine/TAvatarEngine.h>
#include <crsf/CREngine/TAvatarEngineConnector.h>
#include <crsf/CREngine/TDynamicModuleManager.h>
#include <crsf/CREngine/TPhysicsManager.h>

#include <ik/ik.h>

#include "objects/floor.hpp"
#include "openvr_manager.hpp"

CRSEEDLIB_MODULE_CREATOR(MainApp);

// ************************************************************************************************
spdlog::logger* global_logger = nullptr;

MainApp::MainApp(): crsf::TDynamicModuleInterface(CRMODULE_ID_STRING)
{
    global_logger = m_logger.get();

    setup_physics();
}

MainApp::~MainApp() = default;

void MainApp::OnLoad()
{
    rendering_engine_ = crsf::TGraphicRenderEngine::GetInstance();
    pipeline_ = rendering_engine_->GetRenderPipeline();

    rendering_engine_->SetWindowTitle(CRMODULE_ID_STRING);

    openvr_manager_ = std::make_unique<OpenVRManager>(*pipeline_);
    if (!openvr_manager_->is_available())
        openvr_manager_.reset();

    rendering_engine_->EnableControl();
    if (!openvr_manager_)
        rendering_engine_->SetControllerInitialPosHpr(LVecBase3(0, -10, 1), LVecBase3(0, 0, 0));
    rendering_engine_->ResetControllerInitial();
}

void MainApp::OnStart()
{
    LVecBase3f floor_scale(15.0f, 15.0f, 0.02f);
    floor_ = std::make_unique<Floor>("floor",
        LMatrix4f::scale_mat(floor_scale) *
        LMatrix4f::translate_mat(0, 0, -floor_scale[3] / 2.0f));
    floor_->setup_graphics();
    floor_->setup_physics();
    rendering_engine_->GetWorld()->AddWorldObject(floor_->get_object());

    setup_ik();
    setup_avatar();
    setup_chair();
    setup_gui();

    add_task([this](const rppanda::FunctionalTask* task) {
        update();
        return AsyncTask::DoneStatus::DS_cont;
    }, "MainApp::update");

    do_method_later(1.0f, [this](rppanda::FunctionalTask* task) {
        crsf::TPhysicsManager::GetInstance()->Start();
        return AsyncTask::DoneStatus::DS_done;
    }, "MainApp::start_physics");
}

void MainApp::OnExit()
{
    // release some resources
    floor_.reset();

    openvr_manager_.reset();

    ik.deinit();

    crsf::TPhysicsManager::GetInstance()->Exit();
}

void MainApp::setup_physics()
{
    crsf::TPhysicsManager* physics_manager = crsf::TPhysicsManager::GetInstance();
    physics_manager->Init(crsf::EPHYX_ENGINE_BULLET);
    physics_manager->SetGravity(LVecBase3(0.0f, 0.0f, -0.98f));
}

void MainApp::setup_ik()
{
    if (ik.init() != IK_OK)
    {
        global_logger->error("Failed to initialize IK");
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

    np_effector_ = rpcore::Globals::render.attach_new_node("effector");
}

void MainApp::setup_avatar()
{
    crsf::TWorld* cr_world = rendering_engine_->GetWorld();

    // 1 meter axis on origin
    NodePath axis_model = rpcore::RPLoader::load_model("/$$crsf/examples/resources/models/zup-axis.bam");
    axis_model.reparent_to(cr_world->GetNodePath());
    axis_model.set_scale(0.1f);

    const GlobPattern glob_bam("**/*.bam");
    const Filename model_dir("resources/models/avatars");
    vector_string paths;
    glob_bam.match_files(paths, model_dir);

    const GlobPattern glob_egg("**/*.egg");
    glob_egg.match_files(paths, model_dir);

    for (const auto& path: paths)
    {
        auto actor = crsf::CreateObject<crsf::TActorObject>();
        actor->CreateActor(rppanda::Actor::ModelsType(model_dir / path));       // unit is cm
        actor->SetScale(0.01f);
        cr_world->AddWorldObject(actor);
        actors_.push_back(actor);
        actor->DisableTestBounding();
        actor->GetMainCharacter()->MakeAllControlJoint();    // create joints
        actor->Hide();

        crsf::TCRProperty avatar_props;
        avatar_props.m_strName = actor->GetName();
        avatar_props.m_propAvatar.SetJointNumber(71);
        crsf::TDynamicStageMemory::GetInstance()->CreateAvatarMemoryObject(avatar_props);
    }

    change_actor(actors_.front().get());
}

void MainApp::setup_chair()
{
    crsf::TWorld* cr_world = rendering_engine_->GetWorld();

    auto ikea_ekero = crsf::CreateObject(rpcore::RPLoader::load_model("resources/models/ikea-ekero/ikea-ekero.bam"));
    cr_world->AddWorldObject(ikea_ekero);
    ikea_ekero->SetScale(0.4f);
    ikea_ekero->SetPosition(3, 0, 0);
    ikea_ekero->SetHPR(-25, 0, 0);

    auto ikea_tullsta = crsf::CreateObject(rpcore::RPLoader::load_model("resources/models/ikea-tullsta/ikea-tullsta.bam"));
    cr_world->AddWorldObject(ikea_tullsta);
    ikea_tullsta->SetScale(0.075f);
    ikea_tullsta->SetPosition(-3, 1, 0);
    ikea_tullsta->SetHPR(35, 0, 0);

    auto ikea_henriksdal = crsf::CreateObject(rpcore::RPLoader::load_model("resources/models/ikea-henriksdal/henriksdal.bam"));
    cr_world->AddWorldObject(ikea_henriksdal);
    ikea_henriksdal->SetScale(0.01f);
    ikea_henriksdal->SetPosition(-2, -2, 0);
    ikea_henriksdal->SetHPR(45, 0, 0);
}

void MainApp::update()
{
}

void MainApp::change_actor(crsf::TActorObject* new_actor)
{
    if (current_actor_)
        current_actor_->Hide();

    current_actor_ = new_actor;

    rebuild_ik();

    current_actor_->Show();
}

void MainApp::rebuild_ik()
{
    if (!current_actor_)
        return;

    r_acromioclavicular_ = current_actor_->GetNodePath().find("**/r_acromioclavicular");
    if (!r_acromioclavicular_)
        return;

    actor_joints_.clear();
    actor_joints_.reserve(ik_nodes_.size() * 2);     // prevent re-allocation

    auto np = r_acromioclavicular_;
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

    np_effector_.set_pos(actor_joints_.back().get_pos(rpcore::Globals::render));

    static rppanda::FunctionalTask* update_ik_task = nullptr;

    if (update_ik_task)
        update_ik_task->remove();

    update_ik_task = add_task([this](const rppanda::FunctionalTask* task) {
        update_ik();
        return AsyncTask::DoneStatus::DS_cont;
    }, "MainApp::update_ik");
}

void MainApp::update_ik()
{
    const auto pos = np_effector_.get_pos(actor_joints_.front().get_parent());
    ik_effector_->target_position = ik.vec3.vec3(pos[0], pos[1], pos[2]);

    ik.solver.solve(ik_solver_);

    ik.solver.iterate_affected_nodes(ik_solver_, [](ik_node_t* ikNode) {
        if (!ikNode->user_data)
            return;

        NodePath* node = (NodePath*)ikNode->user_data;
        node->set_pos(ikNode->position.x, ikNode->position.y, ikNode->position.z);
    });
}
