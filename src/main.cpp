#include "main.hpp"

#include <imgui.h>

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

    // setup (mouse) controller
    rendering_engine_->EnableControl();
    rendering_engine_->SetControllerInitialPosHpr(
        LVecBase3(0, -10, 1),
        LVecBase3(0, 0, 0));
    rendering_engine_->ResetControllerInitial();

    openvr_manager_ = std::make_unique<OpenVRManager>(*pipeline_);
    if (!openvr_manager_->is_available())
        openvr_manager_.reset();
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
        avatar_props.m_strName = "blonde";
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
        avatar_props.m_strName = "AvatarMan";
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

void MainApp::setup_gui()
{
    rppanda::Messenger::get_global_instance()->send(
        "imgui-setup-context",
        EventParameter(new rppanda::FunctionalTask([this](rppanda::FunctionalTask* task) {
            ImGui::SetCurrentContext(std::static_pointer_cast<ImGuiContext>(task->get_user_data()).get());
            accept("imgui-new-frame", [this](const Event*) { on_imgui_new_frame(); });
            return AsyncTask::DS_done;
        }, "MainApp::setup-imgui"))
    );
}

void MainApp::update()
{
}

void MainApp::on_imgui_new_frame()
{
    static bool window = true;

    ImGui::Begin("CRAvatar", &window);

    static ImGuiComboFlags comobo_flags = 0;
    static std::string actor_name;
    if (current_actor_)
        actor_name = current_actor_->GetName();
    if (ImGui::BeginCombo("Actors", actor_name.c_str(), comobo_flags))
    {
        for (const auto& actor : actors_)
        {
            bool is_selected = (current_actor_ == actor.get());
            if (ImGui::Selectable(actor->GetName().c_str(), is_selected))
            {
                if (current_actor_)
                    current_actor_->Hide();
                current_actor_ = actor.get();
                current_actor_->Show();
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
        }
        ImGui::EndCombo();
    }

    ImGui::End();
}
