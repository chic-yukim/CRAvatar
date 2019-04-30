#include "main_gui/main_gui.hpp"

#include <imgui.h>

#include <crsf/CREngine/TDynamicModuleManager.h>

#include "crmodules/foot_mouse/module.h"

const char* convert_to_string(FootMouseModule::MotionCommand command)
{
    switch (command)
    {
    case FootMouseModule::MotionCommand::STOP:
        return "STOP";

    case FootMouseModule::MotionCommand::STAND:
        return "STAND";

    case FootMouseModule::MotionCommand::WALK:
        return "WALK";

    case FootMouseModule::MotionCommand::WALKR:
        return "WALKR";

    case FootMouseModule::MotionCommand::RSIDEWALK:
        return "RSIDEWALK";

    case FootMouseModule::MotionCommand::BACKWALKR:
        return "BACKWALKR";

    case FootMouseModule::MotionCommand::BACKWALK:
        return "BACKWALK";

    case FootMouseModule::MotionCommand::BACKWALKL:
        return "BACKWALKL";

    case FootMouseModule::MotionCommand::LSIDEWALK:
        return "LSIDEWALK";

    case FootMouseModule::MotionCommand::WALKL:
        return "WALKL";

    case FootMouseModule::MotionCommand::RUN:
        return "RUN";

    case FootMouseModule::MotionCommand::RUNR:
        return "RUNR";

    case FootMouseModule::MotionCommand::RSIDERUN:
        return "RSIDERUN";

    case FootMouseModule::MotionCommand::BACKRUNR:
        return "BACKRUNR";

    case FootMouseModule::MotionCommand::BACKRUN:
        return "BACKRUN";

    case FootMouseModule::MotionCommand::BACKRUNL:
        return "BACKRUNL";

    case FootMouseModule::MotionCommand::LSIDERUN:
        return "LSIDERUN";

    case FootMouseModule::MotionCommand::RUNL:
        return "RUNL";

    case FootMouseModule::MotionCommand::LTURN:
        return "LTURN";

    case FootMouseModule::MotionCommand::RTURN:
        return "RTURN";

    case FootMouseModule::MotionCommand::JUMP:
        return "JUMP";

    case FootMouseModule::MotionCommand::SIT:
        return "SIT";

    case FootMouseModule::MotionCommand::SQUAT:
        return "SQUAT";

    case FootMouseModule::MotionCommand::TIPTOE:
        return "TIPTOE";

    default:
        return "INVALID";
    }
}

void MainGUI::setup_foot_mouse()
{
    auto module_manager = crsf::TDynamicModuleManager::GetInstance();
    if (!module_manager->IsModuleEnabled("foot_mouse"))
        return;

    const auto& foot_mouse_module = std::dynamic_pointer_cast<FootMouseModule>(module_manager->GetModuleInstance("foot_mouse"));
}

void MainGUI::ui_foot_mouse()
{
    auto module_manager = crsf::TDynamicModuleManager::GetInstance();
    if (!module_manager->IsModuleEnabled("foot_mouse"))
        return;

    const auto& foot_mouse_module = std::dynamic_pointer_cast<FootMouseModule>(module_manager->GetModuleInstance("foot_mouse"));

    if (!ImGui::CollapsingHeader("Foot Mouse"))
        return;

    ImGui::LabelText("Status", foot_mouse_module->GetStatus(foot_mouse_module->GetStatus()));

    auto mouse_data = foot_mouse_module->GetMouseData(FootMouseModule::FootIndex::FOOT_INDEX_LEFT);
    ImGui::LabelText("Mouse Data (Left)", "(%d, %d)", mouse_data.x, mouse_data.y);

    mouse_data = foot_mouse_module->GetMouseData(FootMouseModule::FootIndex::FOOT_INDEX_RIGHT);
    ImGui::LabelText("Mouse Data (Right)", "(%d, %d)", mouse_data.x, mouse_data.y);

    const auto& motion_data = foot_mouse_module->GetMotionData();

    ImGui::LabelText("Motion Command", "%s", convert_to_string(motion_data.command));

    if (ImGui::Button("Vibrate Left"))
    {
        foot_mouse_module->SendHapticVibration(FootMouseModule::FootIndex::FOOT_INDEX_LEFT);
    }

    if (ImGui::Button("Vibrate Right"))
    {
        foot_mouse_module->SendHapticVibration(FootMouseModule::FootIndex::FOOT_INDEX_RIGHT);
    }

    int data_window_size = static_cast<int>(foot_mouse_module->GetDataWindowSize());
    if (ImGui::SliderInt("Data Window Size", &data_window_size, 0, 1000))
    {
        foot_mouse_module->SetDataWindowSize(static_cast<size_t>(data_window_size));
    }
}
