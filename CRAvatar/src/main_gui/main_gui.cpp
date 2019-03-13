#include "main_gui/main_gui.hpp"

#include <imgui.h>

#include <fmt/format.h>

#include <render_pipeline/rppanda/showbase/showbase.hpp>
#include <render_pipeline/rpcore/render_pipeline.hpp>
#include <render_pipeline/rpcore/util/primitives.hpp>

#include <crsf/CRModel/TActorObject.h>

#include "main.hpp"

MainGUI::MainGUI(MainApp& app) : app_(app)
{
    rppanda::Messenger::get_global_instance()->send(
        "imgui-setup-context",
        EventParameter(new rppanda::FunctionalTask([this](rppanda::FunctionalTask* task) {
            ImGui::SetCurrentContext(std::static_pointer_cast<ImGuiContext>(task->get_user_data()).get());
            accept("imgui-new-frame", [this](const Event*) { on_imgui_new_frame(); });
            return AsyncTask::DS_done;
        }, "MainApp::setup-imgui"))
    );

    setup_foot_mouse();
}

MainGUI::~MainGUI() = default;

void MainGUI::on_imgui_new_frame()
{
    static bool window = true;

    ImGui::Begin("CRAvatar", &window);

    static ImGuiComboFlags comobo_flags = 0;
    static std::string actor_name;
    if (app_.current_actor_)
        actor_name = app_.current_actor_->GetName();
    if (ImGui::BeginCombo("Actors", actor_name.c_str(), comobo_flags))
    {
        for (const auto& actor : app_.actors_)
        {
            bool is_selected = (app_.current_actor_ == actor.get());

            auto id = fmt::format("{}###{}", actor->GetName(), (void*)actor.get());
            if (ImGui::Selectable(id.c_str(), is_selected))
                app_.change_actor(actor.get());

            if (is_selected)
                ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
        }
        ImGui::EndCombo();
    }

    ui_foot_mouse();

    ImGui::End();
}
