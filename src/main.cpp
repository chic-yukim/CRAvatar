#include "main.hpp"

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
#include <crsf/CREngine/TAvatarEngine.h>
#include <crsf/CREngine/TAvatarEngineConnector.h>
#include <crsf/CREngine/TDynamicModuleManager.h>

#include "openvr_manager.hpp"

CRSEEDLIB_MODULE_CREATOR(MainApp);

// ************************************************************************************************
spdlog::logger* global_logger = nullptr;

MainApp::MainApp(): crsf::TDynamicModuleInterface(CRMODULE_ID_STRING)
{
    global_logger = m_logger.get();
}

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
    setup_avatar();
    setup_chair();
    setup_gui();

    add_task([this](const rppanda::FunctionalTask* task) {
        update();
        return AsyncTask::DoneStatus::DS_cont;
    }, "MainApp::update");
}

void MainApp::OnExit()
{
    openvr_manager_.reset();
}

void MainApp::setup_avatar()
{
    crsf::TWorld* cr_world = rendering_engine_->GetWorld();

    // 1 meter axis on origin
    NodePath axis_model = rpcore::RPLoader::load_model("/$$crsf/examples/resources/models/zup-axis.bam");
    axis_model.reparent_to(cr_world->GetNodePath());
    axis_model.set_scale(0.1f);

    {
        auto actor = crsf::CreateObject<crsf::TActorObject>();
        actor->CreateActor(rppanda::Actor::ModelsType("resources/models/chic/blonde/blonde.bam"));       // unit is cm
        actor->SetScale(0.01f);
        cr_world->AddWorldObject(actor);
        actors_.push_back(actor);
        actor->DisableTestBounding();
        actor->GetMainCharacter()->MakeAllControlJoint();    // create joints

        crsf::TCRProperty avatar_props;
        avatar_props.m_strName = actor->GetName();
        avatar_props.m_propAvatar.SetJointNumber(71);
        crsf::TDynamicStageMemory::GetInstance()->CreateAvatarMemoryObject(avatar_props);

        current_actor_ = actor.get();
    }

    {
        auto actor = crsf::CreateObject<crsf::TActorObject>();
        actor->CreateActor(rppanda::Actor::ModelsType("resources/models/chic/AvatarMan/avatarman_body.bam"));       // unit is meter
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
}

void MainApp::update()
{
}
