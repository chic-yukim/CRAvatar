#include "main.hpp"

#include <spdlog/spdlog.h>

#include <render_pipeline/rppanda/showbase/showbase.hpp>
#include <render_pipeline/rpcore/render_pipeline.hpp>
#include <render_pipeline/rpcore/loader.hpp>
#include <render_pipeline/rpcore/globals.hpp>
#include <render_pipeline/rpcore/util/primitives.hpp>
#include <render_pipeline/rpcore/util/rpgeomnode.hpp>
#include <render_pipeline/rpcore/pluginbase/manager.hpp>

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

#include "main_gui/main_gui.hpp"
#include "objects/floor.hpp"
#include "openvr_manager.hpp"

#include "simple_ik/module.h"

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
    auto plugin_mgr = pipeline_->get_plugin_mgr();

    rendering_engine_->SetWindowTitle(CRMODULE_ID_STRING);

    openvr_manager_ = std::make_unique<OpenVRManager>(*pipeline_);
    if (!openvr_manager_->is_available())
        openvr_manager_.reset();

    if (plugin_mgr->is_plugin_enabled("ar_render"))
        ar_system_ = std::make_unique<ARSystem>(*pipeline_);

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

    main_gui_ = std::make_unique<MainGUI>(*this);

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
    main_gui_.reset();

    // release some resources
    floor_.reset();

    openvr_manager_.reset();

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
    crsf::TWorld* cr_world = rendering_engine_->GetWorld();

    auto dmm = crsf::TDynamicModuleManager::GetInstance();
    if (dmm->IsModuleEnabled("simple_ik"))
    {
        trackers_[0] = rpcore::RPLoader::load_model("resources/models/vr_tracker_vive.bam");
        trackers_[0].reparent_to(cr_world->GetNodePath());
        trackers_[1] = trackers_[0].copy_to(cr_world->GetNodePath());

        simple_ik_ = std::dynamic_pointer_cast<SimpleIKModule>(dmm->GetModuleInstance("simple_ik")).get();
    }
}

void MainApp::setup_avatar()
{
    crsf::TWorld* cr_world = rendering_engine_->GetWorld();

    // 1 meter axis on origin
    NodePath axis_model = rpcore::RPLoader::load_model("/$$crsf/examples/resources/models/zup-axis.bam");
    axis_model.reparent_to(cr_world->GetNodePath());
    axis_model.set_scale(0.1f);

    const Filename model_base_dir("resources/models/avatars");

    vector_string paths;
    if (!model_base_dir.scan_directory(paths))
        return;

    for (const auto& path: paths)
    {
        Filename model_file = model_base_dir / path / (path + ".bam");
        if (!model_file.exists())
        {
            model_file = model_base_dir / path / (path + ".egg");
            if (!model_file.exists())
                continue;
        }

        auto actor = crsf::CreateObject<crsf::TActorObject>();
        actor->CreateActor(rppanda::Actor::ModelsType(model_file));       // unit is cm
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

    if (simple_ik_)
    {
        simple_ik_->SetActor(current_actor_);
        simple_ik_->SetEndEffector(trackers_[0]);
        simple_ik_->StartSolveIKLoop();
    }

    current_actor_->Show();
}
