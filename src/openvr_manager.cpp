/**
 * Coexistence Reality Software Framework (CRSF)
 * Copyright (c) Center of Human-centered Interaction for Coexistence. All rights reserved.
 * See the LICENSE.md file for more details.
 */

#include "openvr_manager.hpp"

#include <spdlog/spdlog.h>

#include <render_pipeline/rpcore/render_pipeline.hpp>
#include <render_pipeline/rpcore/pluginbase/manager.hpp>
#include <render_pipeline/rppanda/task/task_manager.hpp>
#include <render_pipeline/rpcore/globals.hpp>
#include <render_pipeline/rppanda/showbase/showbase.hpp>
#include <render_pipeline/rpcore/loader.hpp>
#include <render_pipeline/rpcore/util/rptextnode.hpp>

#include <openvr_plugin.hpp>

#include <crsf/CRModel/TWorldObject.h>

extern spdlog::logger* global_logger;

OpenVRManager::OpenVRManager(rpcore::RenderPipeline& pipeline) : pipeline_(pipeline)
{
    if (!pipeline_.get_plugin_mgr()->is_plugin_enabled("openvr"))
        return;

    openvr_plugin_ = static_cast<rpplugins::OpenVRPlugin*>(pipeline_.get_plugin_mgr()->get_instance("openvr")->downcast());

    caching_devices();

    toggle_ar();

    add_task([this](rppanda::FunctionalTask*) {
        process_controller_event();
        return AsyncTask::DoneStatus::DS_cont;
    }, "OpenVRManager::process_controller_event");

    accept("OpenVRManager::toggle_ar", [this](const Event*) { toggle_ar(); });
}

OpenVRManager::~OpenVRManager()
{
    remove_all_tasks();
}

void OpenVRManager::toggle_ar()
{
    static bool virtual_mode = true;
    virtual_mode = !virtual_mode;

    auto openvr_devices = rpcore::Globals::render.find("openvr_devices");

    const auto child_count = openvr_devices.get_num_children();
    if (virtual_mode)
    {
        for (int k = 1; k < child_count; ++k)
        {
            auto device = openvr_devices.get_child(k);
            device.show_through(crsf::TWorldObject::GetVirtualObjectMask());
            device.hide(crsf::TWorldObject::GetRealObjectMask());
        }
    }
    else
    {
        for (int k = 1; k < child_count; ++k)
        {
            auto device = openvr_devices.get_child(k);
            device.show_through(crsf::TWorldObject::GetRealObjectMask());
        }
    }
}

void OpenVRManager::create_devices_label()
{
    NodePath axis_model = rpcore::RPLoader::load_model("/$$crsf/examples/resources/models/zup-axis.bam");
    auto openvr_devices = rpcore::Globals::render.find("openvr_devices");

    const auto child_count = openvr_devices.get_num_children();
    for (int k = 1; k < child_count; ++k)
    {
        auto device = openvr_devices.get_child(k);

        auto axis = axis_model.copy_to(device);
        axis.set_scale(0.01f);

        // add label to devcies
        rpcore::RPTextNode label("device_label", device);
        label.set_text(device.get_name() + " : " + device.get_tag("serial_number"));
        label.set_text_color(LColor(1, 0, 0, 1));
        label.set_pixel_size(10.0f);
        label.get_np().set_two_sided(true);
        label.get_np().set_pos(0.0f, 0.0f, 0.1f);
    }
}

void OpenVRManager::create_text2d()
{
    auto openvr_devices = rpcore::Globals::render.find("openvr_devices");

    const auto child_count = openvr_devices.get_num_children();
    for (int k = 0; k < child_count; ++k)
    {
        // create OnscreenText using methods
        rppanda::OnscreenText text("");
        text.set_pos(LVecBase2(0.4f, 0.1 * k - 0.3f));
        text.set_scale(LVecBase2(0.04f));
        text.set_fg(LColor(1, 0, 0, 1));
        text.set_shadow(LColor(0, 0, 0, 1));
        text.set_align(TextProperties::A_left);
        texts_.push_back(text);
    }

    add_task([this](rppanda::FunctionalTask*) {
        update_texts();
        return AsyncTask::DoneStatus::DS_cont;
    }, "OpenVRManager::update_texts");
}

void OpenVRManager::print_serials()
{
    std::string serial_number;
    for (int k = vr::k_unTrackedDeviceIndex_Hmd + 1; k < vr::k_unMaxTrackedDeviceCount; ++k)
    {
        openvr_plugin_->get_tracked_device_property(serial_number, k, vr::Prop_SerialNumber_String);
        if (openvr_plugin_->get_tracked_device_class(k) == vr::TrackedDeviceClass_HMD)
            global_logger->info("HMD serial number: {}", serial_number);
        else if (openvr_plugin_->get_tracked_device_class(k) == vr::TrackedDeviceClass_GenericTracker)
            global_logger->info("Tracker serial number: {}", serial_number);
        else if (openvr_plugin_->get_tracked_device_class(k) == vr::TrackedDeviceClass_Controller)
            global_logger->info("Controller serial number: {}", serial_number);
    }
}

void OpenVRManager::caching_devices()
{
    hmd_np_ = openvr_plugin_->get_device_node(vr::k_unTrackedDeviceIndex_Hmd);
    for (int k = vr::k_unTrackedDeviceIndex_Hmd + 1; k < vr::k_unMaxTrackedDeviceCount; ++k)
    {
        auto np = openvr_plugin_->get_device_node(k);
        switch (openvr_plugin_->get_tracked_device_class(k))
        {
        case vr::TrackedDeviceClass_TrackingReference:
            basestation_np_list_.push_back(np);
            break;
        case vr::TrackedDeviceClass_Controller:
            controller_np_list_.push_back(np);
            break;
        case vr::TrackedDeviceClass_GenericTracker:
            tracker_np_list_.push_back(np);
            break;
        default:
            break;
        }
    }
}

void OpenVRManager::process_controller_event()
{
    auto vr_system = openvr_plugin_->get_vr_system();

    // Process SteamVR controller state
    for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
    {
        if (openvr_plugin_->get_tracked_device_class(unDevice) != vr::TrackedDeviceClass_Controller)
            continue;

        vr::VRControllerState_t state;
        if (!vr_system->GetControllerState(unDevice, &state, sizeof(state)))
            continue;

        const EventParameter ev_param(static_cast<int>(unDevice));

        for (int id = vr::EVRButtonId::k_EButton_System; id < vr::EVRButtonId::k_EButton_Max; ++id)
        {
            const auto button_mask = vr::ButtonMaskFromId(static_cast<vr::EVRButtonId>(id));
            const bool pressed = (button_mask & state.ulButtonPressed) != 0;
            const bool last_pressed = (button_mask & controller_last_states_[unDevice]) != 0;

            if (pressed && !last_pressed)
            {
                const char* name = vr_system->GetButtonIdNameFromEnum(static_cast<vr::EVRButtonId>(id));

                rppanda::Messenger::get_global_instance()->send(
                    std::string("openvr::") + name, ev_param);
            }
            else if (!pressed && last_pressed)
            {
                const char* name = vr_system->GetButtonIdNameFromEnum(static_cast<vr::EVRButtonId>(id));

                rppanda::Messenger::get_global_instance()->send(
                    std::string("openvr::") + name + "-up", ev_param);
            }
        }

        controller_last_states_[unDevice] = state.ulButtonPressed;
    }
}

void OpenVRManager::update_texts()
{
    auto openvr_devices = rpcore::Globals::render.find("openvr_devices");

    auto child_count = openvr_devices.get_num_children();
    for (int k = 0; k < child_count; ++k)
    {
        auto device = openvr_devices.get_child(k);

        auto pos = device.get_pos();
        auto quat = device.get_quat();

        if (k < texts_.size())
        {
            texts_[k].set_text(fmt::format("{} :\t({:.3f}, {:.3f}, {:.3f}),\n\t\t({:.3f}, {:.3f}, {:.3f}, {:.3f})",
                device.get_tag("serial_number"),
                pos[0], pos[1], pos[2],
                quat[0], quat[1], quat[2], quat[3]));
        }
    }
}
