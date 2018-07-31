#include "main.hpp"

#include <imgui.h>

#include <fmt/format.h>

#include <render_pipeline/rppanda/showbase/showbase.hpp>
#include <render_pipeline/rpcore/render_pipeline.hpp>
#include <render_pipeline/rpcore/util/primitives.hpp>

#include <crsf/CRModel/TActorObject.h>
#include <crsf/CRModel/TCharacter.h>

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

            auto id = fmt::format("{}###{}", actor->GetName(), (void*)actor.get());
            if (ImGui::Selectable(id.c_str(), is_selected))
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
